#pragma once
#include "domain/map/IMapProvider.h"
#include "shared/Result.h"
#include <QString>

namespace data {

    /**
     * @class GeoJsonFileProvider
     * @brief Loads map polygon data from a GeoJSON file.
     *
     * This class implements the IMapProvider interface and reads polygon
     * geometry from a GeoJSON file stored in the Qt resource system or
     * filesystem.
     *
     * The provider supports both `Polygon` and `MultiPolygon` geometries and
     * extracts:
     *   - a list of polygon rings (each ring is a vector of LatLon points)
     *   - the geographic bounding box (min/max latitude and longitude)
     */
    class GeoJsonFileProvider : public domain::IMapProvider {
      public:
        /**
         * @brief Constructs a GeoJsonFileProvider.
         * @param resourcePath Path to the GeoJSON file (Qt resource or
         *        filesystem).
         */
        explicit GeoJsonFileProvider(QString resourcePath);

        /**
         * @brief Loads polygon geometry from the GeoJSON file.
         *
         * This function parses the GeoJSON document and extracts all polygon
         * geometries. Both `Polygon` and `MultiPolygon` types are supported.
         *
         * @return Result containing RawMapData on success, or an error
         *         message on failure.
         */
        shared::Result<domain::IMapProvider::RawMapData> loadShape() override;

      private:
        /**
         * @brief Path to the GeoJSON file.
         *
         * This may be a Qt resource path (e.g. `:/maps/finland.json`)
         * or a filesystem path.
         */
        QString m_resourcePath;
    };

} // namespace data
