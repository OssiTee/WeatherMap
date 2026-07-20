#include "viewmodel/cloud/CloudOverlayViewModel.h"
#include "domain/cloud/CloudFieldInterpolator.h"
#include <QColor>
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {

    constexpr int OVERLAY_SIZE = 1024;
    constexpr int FIELD_WIDTH = 320;
    constexpr int FIELD_HEIGHT = 320;

    constexpr double MIN_CLOUD_SPAN = 1.0;

    constexpr double GAUSSIAN_SIGMA = 0.06;
    constexpr double EMPHASIS_GAMMA = 0.62;

    constexpr double MAX_ALPHA = 245.0;
    constexpr double BASE_GRAY = 215.0;
    constexpr double GRAY_DARKEN_RANGE = 175.0;

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

    inline size_t fieldIndex(int x, int y, int width) {
        return static_cast<size_t>(y) * static_cast<size_t>(width) +
               static_cast<size_t>(x);
    }

    void
    paintCloudField(const std::vector<domain::NormalizedCloudPoint> &points,
                    QImage &field) {
        // Domain interpolation produces a UI-independent cloud field.
        auto cloudField =
            domain::CloudFieldInterpolator::buildWeightedCloudField(
                points, FIELD_WIDTH, FIELD_HEIGHT, GAUSSIAN_SIGMA);
        if (cloudField.empty()) {
            return;
        }

        auto [minCloudIt, maxCloudIt] =
            std::minmax_element(cloudField.begin(), cloudField.end());
        const double minCloud = *minCloudIt;
        const double maxCloud = *maxCloudIt;
        // Use local range normalization so contrast adapts to current data.
        const double cloudSpan = std::max(MIN_CLOUD_SPAN, maxCloud - minCloud);

        for (int y = 0; y < FIELD_HEIGHT; ++y) {
            for (int x = 0; x < FIELD_WIDTH; ++x) {
                const double cloud = cloudField[fieldIndex(x, y, FIELD_WIDTH)];
                // Convert cloud cover to [0,1] before applying visual emphasis.
                const double normalized =
                    std::clamp((cloud - minCloud) / cloudSpan, 0.0, 1.0);
                // Gamma < 1 boosts mid-range differences that would otherwise
                // look flat in the overlay.
                const double emphasized = std::pow(normalized, EMPHASIS_GAMMA);

                // Keep low values light so the map stays readable.
                const int alpha = static_cast<int>(emphasized * MAX_ALPHA);
                // Darker gray means denser cloud area.
                const int gray = static_cast<int>(
                    BASE_GRAY - emphasized * GRAY_DARKEN_RANGE);

                field.setPixelColor(x, y, QColor(gray, gray, gray, alpha));
            }
        }
    }

    QImage
    buildOverlayImage(const std::vector<domain::NormalizedCloudPoint> &points) {
        QImage image = createTransparentImage(OVERLAY_SIZE, OVERLAY_SIZE);
        QImage field = createTransparentImage(FIELD_WIDTH, FIELD_HEIGHT);

        paintCloudField(points, field);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawImage(QRect(0, 0, OVERLAY_SIZE, OVERLAY_SIZE), field);

        return image;
    }

} // namespace

namespace viewmodel {

    void CloudOverlayViewModel::startFetch(int latSamples, int lonSamples,
                                           bool detailed) {
        auto service = m_service;
        auto bbox = m_bbox;

        m_loadingDetailed = detailed;

        auto future = QtConcurrent::run([service = std::move(service), bbox,
                                         latSamples, lonSamples]() {
            auto result = service->getCloudForMap(bbox, latSamples, lonSamples);
            if (result.isError()) {
                return shared::Result<QImage>::error(result.errorMessage());
            }

            auto points = std::move(result).value();

            return shared::Result<QImage>::success(buildOverlayImage(points));
        });

        m_watcher.setFuture(std::move(future));
    }

    CloudOverlayViewModel::CloudOverlayViewModel(
        std::unique_ptr<domain::ICloudCoverageService> service)
        : m_service(
              std::shared_ptr<domain::ICloudCoverageService>(std::move(service))) {
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
