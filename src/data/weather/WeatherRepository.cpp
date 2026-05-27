#include "data/weather/WeatherRepository.h"
#include "data/weather/FmiXmlParser.h"
#include "data/weather/NetworkClient.h"
#include "data/weather/WeatherCache.h"
#include <spdlog/spdlog.h>

#include "shared/Result.h"

#include <QDateTime>
#include <QFile>
#include <QString>

namespace {

    constexpr int SECONDS_PER_HOUR = 3600;

    // Load the FMI URL template from a Qt resource file. If the resource is
    // unavailable or empty, the repository remains misconfigured and will
    // return an error when fetching weather.
    QString loadFmiQueryTemplate(const QString &resourcePath) {
        QFile file(resourcePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            SPDLOG_WARN("Failed to open FMI query resource: {}",
                        qPrintable(resourcePath));
            return QString();
        }

        const QString contents = QString::fromUtf8(file.readAll()).trimmed();
        if (contents.isEmpty()) {
            SPDLOG_WARN("FMI query resource is empty: {}",
                        qPrintable(resourcePath));
            return QString();
        }

        return contents;
    }

    // Build a fully-qualified FMI request URL for a single coordinate point.
    QString buildFmiUrl(double lat, double lon, const QString &base) {
        const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
        const QDateTime endUtc = nowUtc.addSecs(24 * SECONDS_PER_HOUR);

        return base.arg(
            QString::number(lat, 'f', 5), QString::number(lon, 'f', 5),
            nowUtc.toString(Qt::ISODate), endUtc.toString(Qt::ISODate));
    }

    // Load weather coordinate pairs from a text resource file. Each line
    // should contain latitude and longitude separated by a comma.
    // Invalid or missing resource data is reported, but no fallback data is
    // supplied.
    std::vector<std::pair<double, double>>
    loadCoords(const QString &resourcePath) {
        QFile file(resourcePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            SPDLOG_WARN("Failed to open weather coords resource: {}",
                        qPrintable(resourcePath));
            return {};
        }

        std::vector<std::pair<double, double>> coords;
        const QString contents = QString::fromUtf8(file.readAll());
        const auto lines = contents.split('\n', Qt::SkipEmptyParts);
        coords.reserve(lines.size());

        for (const QString &line : lines) {
            const QString trimmed = line.trimmed();
            if (trimmed.isEmpty() || trimmed.startsWith('#')) {
                continue;
            }

            const QStringList parts = trimmed.split(',', Qt::SkipEmptyParts);
            if (parts.size() != 2) {
                SPDLOG_WARN("Invalid coord line in resource: {}",
                            qPrintable(trimmed));
                continue;
            }

            bool okLat = false;
            bool okLon = false;
            const double lat = parts[0].toDouble(&okLat);
            const double lon = parts[1].toDouble(&okLon);
            if (!okLat || !okLon) {
                SPDLOG_WARN("Invalid coord values in resource: {}",
                            qPrintable(trimmed));
                continue;
            }

            coords.emplace_back(lat, lon);
        }

        if (coords.empty()) {
            SPDLOG_WARN("No valid weather coordinates found in resource: {}",
                        qPrintable(resourcePath));
        }

        return coords;
    }

} // namespace

namespace data {

    // Initialize the repository with the FMI query template and coordinate
    // resource paths. The constructor loads both resources once.
    WeatherRepository::WeatherRepository(const QString &fmiQueryResource,
                                         const QString &coordsResource)
        : client_(std::make_unique<NetworkClient>()),
          parser_(std::make_unique<data::FmiXmlParser>()),
          cache_(std::make_unique<WeatherCache>()),
          m_fmiQueryTemplate(loadFmiQueryTemplate(fmiQueryResource)),
          m_coords(loadCoords(coordsResource)) {}

    WeatherRepository::~WeatherRepository() = default;

    shared::Result<std::vector<domain::IWeatherRepository::RawWeatherPoint>>
    WeatherRepository::fetchWeather(shared::ForecastHorizon horizon) {
        const QDateTime now = QDateTime::currentDateTimeUtc();

        if (m_fmiQueryTemplate.isEmpty()) {
            return shared::Result<
                std::vector<domain::IWeatherRepository::RawWeatherPoint>>::
                error("FMI query template is not configured");
        }

        if (m_coords.empty()) {
            return shared::Result<
                std::vector<domain::IWeatherRepository::RawWeatherPoint>>::
                error("No weather coordinate data available");
        }

        // 1) Cache
        if (cache_->hasFresh(horizon, now)) {
            return shared::Result<
                std::vector<domain::IWeatherRepository::RawWeatherPoint>>::
                success(cache_->get(horizon));
        }

        // 2) Horizon → timestamp
        int offsetHours = 0;
        switch (horizon) {
        case shared::ForecastHorizon::Now:
            offsetHours = 0;
            break;
        case shared::ForecastHorizon::Plus6h:
            offsetHours = 6;
            break;
        case shared::ForecastHorizon::Plus12h:
            offsetHours = 12;
            break;
        case shared::ForecastHorizon::Plus24h:
            offsetHours = 24;
            break;
        }

        const QDateTime targetUtc = now.addSecs(offsetHours * SECONDS_PER_HOUR);

        std::vector<domain::IWeatherRepository::RawWeatherPoint> out;
        out.reserve(m_coords.size());

        // 3) Fetch all points
        for (auto [lat, lon] : m_coords) {
            const QString url = buildFmiUrl(lat, lon, m_fmiQueryTemplate);
            QByteArray xml = client_->get(url);
            if (xml.isEmpty()) {
                return shared::Result<
                    std::vector<domain::IWeatherRepository::RawWeatherPoint>>::
                    error("Network error: empty response from FMI");
            }

            auto parsed = parser_->parse(xml, lat, lon, targetUtc);
            if (!parsed.has_value()) {
                return shared::Result<
                    std::vector<domain::IWeatherRepository::RawWeatherPoint>>::
                    error("XML parse error from FMI");
            }

            out.push_back(std::move(*parsed));
        }

        // 4) Cache
        cache_->put(horizon, out, now);

        return shared::Result<
            std::vector<domain::IWeatherRepository::RawWeatherPoint>>::
            success(std::move(out));
    }

} // namespace data
