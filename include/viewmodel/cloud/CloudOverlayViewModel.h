#pragma once

#include "domain/cloud/ICloudCoverageService.h"
#include "shared/BoundingBox.h"
#include "shared/Result.h"
#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <QString>
#include <memory>

namespace viewmodel {

    /**
     * @brief ViewModel that produces a rendered cloud overlay image.
     *
     * Fetches map-normalized cloud coverage points from the domain layer and
     * emits a ready-to-draw overlay image for the view layer.
     */
    class CloudOverlayViewModel : public QObject {
        Q_OBJECT

      public:
        /**
         * @brief Constructs the cloud overlay viewmodel.
         * @param service Domain service providing normalized cloud points.
         */
        explicit CloudOverlayViewModel(
            std::unique_ptr<domain::ICloudCoverageService> service);

        /**
         * @brief Sets the map area used for cloud queries and rendering.
         * @param box Geographic bounding box of the current map view.
         */
        void setBoundingBox(const shared::BoundingBox &box);

        /**
         * @brief Starts cloud overlay loading for the current time.
         * @param forceRefresh If true, bypasses the cached image shortcut.
         */
        void load(bool forceRefresh = false);

      signals:
        /**
         * @brief Emitted when a new overlay image is ready for drawing.
         */
        void overlayImageReady(const QImage &image);

        /**
         * @brief Emitted when overlay fetch or render fails.
         */
        void errorOccurred(QString message);

      private slots:
        /**
         * @brief Handles completion of the asynchronous cloud fetch task.
         */
        void onFetchFinished();

      private:
        /**
         * @brief Starts one asynchronous cloud fetch stage.
         * @param latSamples Number of latitude samples in query grid.
         * @param lonSamples Number of longitude samples in query grid.
         * @param detailed True for detailed pass, false for coarse pass.
         */
        void startFetch(int latSamples, int lonSamples, bool detailed);

        std::shared_ptr<domain::ICloudCoverageService> m_service;
        shared::BoundingBox m_bbox;
        bool m_hasBBox = false;

        QImage m_cachedImage;

        bool m_loadingDetailed = false;
        bool m_pendingDetailedFetch = false;

        QFutureWatcher<shared::Result<QImage>> m_watcher;
    };

} // namespace viewmodel
