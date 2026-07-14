#include "data/cloud/CloudCoverageRepository.h"
#include <QDateTime>
#include <QFile>
#include <QXmlStreamReader>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <optional>
#include <thread>

namespace {

    constexpr int SECONDS_PER_HOUR = 3600;
    constexpr size_t MAX_PARALLEL_REQUESTS = 6;

    // Load the WFS query template from Qt resources so the request URL can be
    // configured without hard-coding it into the transport logic.
    QString loadQueryTemplate(const QString &resourcePath) {
        QFile file(resourcePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return QString();
        }

        const QString contents = QString::fromUtf8(file.readAll()).trimmed();
        return contents;
    }

    // Parse FMI WFS payload and return the TotalCloudCover sample closest
    // to the current time.
    QDateTime cloudTargetTime() {
        return QDateTime::currentDateTimeUtc();
    }

    // Cloud requests ask FMI for a 24 hour window starting from now.
    QDateTime cloudRequestEndTime() {
        return QDateTime::currentDateTimeUtc().addSecs(24 * SECONDS_PER_HOUR);
    }

    std::optional<double> parseClosestCloudCover(const QByteArray &xml,
                                                 const QDateTime &targetTime) {
        QXmlStreamReader reader(xml);

        std::optional<double> closestValue;
        qint64 closestDeltaMs = std::numeric_limits<qint64>::max();

        while (!reader.atEnd()) {
            reader.readNext();

            if (!reader.isStartElement() ||
                reader.name() != QStringLiteral("BsWfsElement")) {
                continue;
            }

            QDateTime sampleTime;
            QString parameterName;
            double parameterValue = 0.0;
            bool valueParsed = false;

            while (!(reader.isEndElement() &&
                     reader.name() == QStringLiteral("BsWfsElement")) &&
                   !reader.atEnd()) {
                reader.readNext();
                if (!reader.isStartElement()) {
                    continue;
                }

                const auto tag = reader.name();
                if (tag == QStringLiteral("Time")) {
                    sampleTime =
                        QDateTime::fromString(reader.readElementText(), Qt::ISODate);
                    sampleTime = sampleTime.toUTC();
                } else if (tag == QStringLiteral("ParameterName")) {
                    parameterName = reader.readElementText();
                } else if (tag == QStringLiteral("ParameterValue")) {
                    bool ok = false;
                    parameterValue = reader.readElementText().toDouble(&ok);
                    valueParsed = ok;
                }
            }

            if (parameterName != QStringLiteral("TotalCloudCover") ||
                !valueParsed || !sampleTime.isValid()) {
                continue;
            }

            const qint64 deltaMs =
                std::llabs(sampleTime.msecsTo(targetTime));
            if (deltaMs < closestDeltaMs) {
                closestDeltaMs = deltaMs;
                closestValue = parameterValue;
            }
        }

        if (reader.hasError()) {
            return std::nullopt;
        }

        return closestValue;
    }

    size_t workerCountFor(size_t coordCount) {
        const unsigned int hw = std::thread::hardware_concurrency();
        const size_t hwWorkers = (hw > 0) ? static_cast<size_t>(hw) : size_t(4);
        return std::max<size_t>(
            1, std::min({coordCount, MAX_PARALLEL_REQUESTS, hwWorkers}));
    }

} // namespace

namespace data {

    CloudCoverageRepository::CloudCoverageRepository(
        std::unique_ptr<NetworkClient> client,
        const QString &queryTemplateResource)
        : m_client(std::move(client))
        , m_queryTemplate(loadQueryTemplate(queryTemplateResource)) {
        assert(m_client && "NetworkClient must not be null");
    }

    QString CloudCoverageRepository::buildRequestUrl(double lat, double lon) const {
        const QDateTime nowUtc = cloudTargetTime();
        const QDateTime endUtc = cloudRequestEndTime();

        return m_queryTemplate.arg(
            QString::number(lat, 'f', 5), QString::number(lon, 'f', 5),
            nowUtc.toString(Qt::ISODate), endUtc.toString(Qt::ISODate));
    }

    shared::Result<std::vector<domain::ICloudCoverageRepository::RawCloudPoint>>
    CloudCoverageRepository::fetchCloudCoverage(
        const std::vector<std::pair<double, double>> &coords) {
        if (m_queryTemplate.isEmpty()) {
            return shared::Result<std::vector<
                domain::ICloudCoverageRepository::RawCloudPoint>>::error(
                "Cloud query template is not configured");
        }

        if (coords.empty()) {
            return shared::Result<std::vector<
                domain::ICloudCoverageRepository::RawCloudPoint>>::error(
                "Cloud coordinate list is empty");
        }

        const QDateTime targetTime = cloudTargetTime();
        const size_t workerCount = workerCountFor(coords.size());

        std::vector<domain::ICloudCoverageRepository::RawCloudPoint> out;
        out.reserve(coords.size());
        std::mutex outMutex;

        std::atomic<size_t> nextIndex{0};

        auto fetchPoint = [&](double lat, double lon)
            -> std::optional<domain::ICloudCoverageRepository::RawCloudPoint> {
            const QString url = buildRequestUrl(lat, lon);
            const QByteArray xml = m_client->get(url);

            if (xml.isEmpty()) {
                return std::nullopt;
            }

            const auto parsed = parseClosestCloudCover(xml, targetTime);
            if (!parsed.has_value()) {
                return std::nullopt;
            }

            return domain::ICloudCoverageRepository::RawCloudPoint{
                .latitude = lat,
                .longitude = lon,
                .cloudCoverPercent = *parsed,
            };
        };

        // Static work distribution via atomic index keeps threading simple and
        // avoids queue synchronization overhead for small/medium grids.
        auto worker = [&]() {
            while (true) {
                const size_t idx = nextIndex.fetch_add(1);
                if (idx >= coords.size()) {
                    break;
                }

                const auto [lat, lon] = coords[idx];
                const auto point = fetchPoint(lat, lon);
                if (point.has_value()) {
                    std::lock_guard<std::mutex> lock(outMutex);
                    out.push_back(*point);
                }
            }
        };

        std::vector<std::thread> workers;
        workers.reserve(workerCount);
        for (size_t i = 0; i < workerCount; ++i) {
            workers.emplace_back(worker);
        }
        for (auto &t : workers) {
            t.join();
        }

        // If every point failed, return an error instead of an empty success
        // result so the caller can surface the issue to the user.
        if (out.empty()) {
            return shared::Result<std::vector<
                domain::ICloudCoverageRepository::RawCloudPoint>>::error(
                "Failed to fetch cloud coverage data (no usable points)");
        }

        return shared::Result<std::vector<
            domain::ICloudCoverageRepository::RawCloudPoint>>::success(
            std::move(out));
    }

} // namespace data
