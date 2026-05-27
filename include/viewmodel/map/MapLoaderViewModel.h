#pragma once

#include <QObject>
#include <QPointF>
#include <QVector>
#include <memory>

#include "domain/map/IMapService.h"

namespace viewmodel {

    /**
     * @class MapLoaderViewModel
     * @brief Asynchronously loads and normalizes map polygon data for the UI.
     *
     * This ViewModel coordinates the loading of geographic polygon data
     * (e.g., country borders) through an IMapService implementation.
     *
     * The loading is performed on a background thread using QtConcurrent to
     * avoid blocking the UI. Once the map data is successfully loaded and
     * normalized into [0,1] coordinate space, the ViewModel emits
     * mapShapeReady() on the main thread.
     *
     * If loading fails (e.g., file not found, JSON parse error, invalid
     * GeoJSON), the ViewModel emits errorOccurred() with a descriptive
     * message.
     */
    class MapLoaderViewModel : public QObject {
        Q_OBJECT
      public:
        /**
         * @brief Constructs the ViewModel with a map service.
         *
         * @param service Unique pointer to an IMapService implementation.
         * The ViewModel takes ownership of the service.
         * The service will live for the entire lifetime of the ViewModel.
         * @param parent Optional QObject parent.
         */
        explicit MapLoaderViewModel(
            std::unique_ptr<domain::IMapService> service,
            QObject *parent = nullptr);

        /**
         * @brief Starts asynchronous loading of the map shape.
         *
         * The loading is executed on a background thread. On success,
         * mapShapeReady() is emitted on the UI thread. On failure,
         * errorOccurred() is emitted.
         */
        void loadMap();

      signals:
        /**
         * @brief Emitted when the map polygons have been successfully loaded
         *        and normalized.
         *
         * @param polygons Normalized polygons in Qt QPointF format.
         * @param bbox Bounding box of the original geographic coordinates
         * (min/max lat/lon).
         */
        // The polygons parameter is delivered as a shared_ptr to const data
        // so the heavy polygon vector is not copied across signal/slot
        // boundaries. Receivers that need ownership can keep a copy of the
        // shared_ptr.
        void mapShapeReady(
            std::shared_ptr<const std::vector<std::vector<QPointF>>> polygons,
            const shared::BoundingBox &bbox);

        /**
         * @brief Emitted when loading fails due to file errors, JSON parsing
         *        issues, or invalid GeoJSON structure.
         *
         * @param message Human‑readable error message.
         */
        void errorOccurred(const QString &message);

      private:
        // Map service used to load raw polygon data.
        std::unique_ptr<domain::IMapService> m_service;
    };

} // namespace viewmodel

Q_DECLARE_METATYPE(std::shared_ptr<const std::vector<std::vector<QPointF>>>)
