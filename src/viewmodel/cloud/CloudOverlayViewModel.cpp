#include "viewmodel/cloud/CloudOverlayViewModel.h"
#include <QtConcurrent/QtConcurrent>
#include <QColor>
#include <QPainter>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {

    constexpr int OVERLAY_WIDTH = 1024;
    constexpr int OVERLAY_HEIGHT = 1024;
    constexpr int FIELD_WIDTH = 320;
    constexpr int FIELD_HEIGHT = 320;
    // Repository performs one WFS request per sample point.
    // This denser grid smooths visible banding while staying responsive.
    constexpr int COARSE_GRID_LAT_SAMPLES = 8;
    constexpr int COARSE_GRID_LON_SAMPLES = 12;
    constexpr int DETAILED_GRID_LAT_SAMPLES = 16;
    constexpr int DETAILED_GRID_LON_SAMPLES = 24;

    QImage createTransparentImage(int width, int height) {
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        return image;
    }

    void paintCloudField(const std::vector<domain::NormalizedCloudPoint> &points,
                         QImage &field) {
        // Gaussian weighting turns sparse samples into a smoother cloud field.
        constexpr double sigma = 0.06;
        constexpr double invTwoSigma2 = 1.0 / (2.0 * sigma * sigma);

        auto [minCloudIt, maxCloudIt] = std::minmax_element(
            points.begin(), points.end(),
            [](const auto &a, const auto &b) {
                return a.cloudCoverPercent < b.cloudCoverPercent;
            });
        const double minCloud = minCloudIt->cloudCoverPercent;
        const double maxCloud = maxCloudIt->cloudCoverPercent;
        const double cloudSpan = std::max(1.0, maxCloud - minCloud);

        for (int y = 0; y < FIELD_HEIGHT; ++y) {
            const double yNorm = static_cast<double>(y) /
                                 static_cast<double>(FIELD_HEIGHT - 1);
            for (int x = 0; x < FIELD_WIDTH; ++x) {
                const double xNorm = static_cast<double>(x) /
                                     static_cast<double>(FIELD_WIDTH - 1);

                double weightedCloud = 0.0;
                double weightSum = 0.0;

                for (const auto &p : points) {
                    const double dx = xNorm - p.xNorm;
                    const double dy = yNorm - p.yNorm;
                    const double dist2 = dx * dx + dy * dy;

                    const double w = std::exp(-dist2 * invTwoSigma2);
                    weightedCloud += w * std::clamp(p.cloudCoverPercent, 0.0, 100.0);
                    weightSum += w;
                }

                if (weightSum <= 0.0) {
                    continue;
                }

                const double cloud = weightedCloud / weightSum;
                const double normalized =
                    std::clamp((cloud - minCloud) / cloudSpan, 0.0, 1.0);
                const double emphasized = std::pow(normalized, 0.62);

                // Keep low values light so the map stays readable.
                const int alpha = static_cast<int>(emphasized * 245.0);
                const int gray = static_cast<int>(215.0 - emphasized * 175.0);

                field.setPixelColor(x, y, QColor(gray, gray, gray, alpha));
            }
        }
    }

    QImage buildOverlayImage(
        const std::vector<domain::NormalizedCloudPoint> &points) {
        QImage image = createTransparentImage(OVERLAY_WIDTH, OVERLAY_HEIGHT);
        QImage field = createTransparentImage(FIELD_WIDTH, FIELD_HEIGHT);

        paintCloudField(points, field);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawImage(QRect(0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT), field);

        return image;
    }

} // namespace

namespace viewmodel {

    void CloudOverlayViewModel::startFetch(int latSamples, int lonSamples,
                                           bool detailed) {
        auto *service = m_service.get();
        auto bbox = m_bbox;

        m_loadingDetailed = detailed;

        auto future = QtConcurrent::run([service, bbox, latSamples,
                                         lonSamples]() {
            auto result = service->getCloudForMap(bbox, latSamples, lonSamples);
            if (result.isError()) {
                return shared::Result<QImage>::error(result.errorMessage());
            }

            auto points = std::move(result).value();

            return shared::Result<QImage>::success(
                buildOverlayImage(points));
        });

        m_watcher.setFuture(std::move(future));
    }

    CloudOverlayViewModel::CloudOverlayViewModel(
        std::unique_ptr<domain::ICloudCoverageService> service)
        : m_service(std::move(service)) {
        assert(m_service && "ICloudCoverageService must not be null");

        connect(&m_watcher, &QFutureWatcher<shared::Result<QImage>>::finished,
                this, &CloudOverlayViewModel::onFetchFinished);
    }

    void CloudOverlayViewModel::setBoundingBox(const shared::BoundingBox &box) {
        m_bbox = box;
        m_hasBBox = true;
        m_cachedImage = QImage();
    }

    void CloudOverlayViewModel::load(bool forceRefresh) {
        if (!m_hasBBox || m_watcher.isRunning()) {
            return;
        }

        // Keep the latest good image around so repeated loads can return fast.
        if (!forceRefresh && !m_cachedImage.isNull()) {
            emit overlayImageReady(m_cachedImage);
            return;
        }

        // Start with a coarse pass and refine once the first image is ready.
        m_pendingDetailedFetch = true;
        startFetch(COARSE_GRID_LAT_SAMPLES, COARSE_GRID_LON_SAMPLES, false);
    }

    void CloudOverlayViewModel::onFetchFinished() {
        auto result = m_watcher.result();

        if (result.isError()) {
            emit errorOccurred(QString::fromStdString(result.errorMessage()));
            return;
        }

        m_cachedImage = std::move(result).value();
        emit overlayImageReady(m_cachedImage);

        // Progressive loading: publish coarse result first, then refine.
        if (!m_loadingDetailed && m_pendingDetailedFetch) {
            m_pendingDetailedFetch = false;
            startFetch(DETAILED_GRID_LAT_SAMPLES, DETAILED_GRID_LON_SAMPLES,
                       true);
        }
    }

} // namespace viewmodel
