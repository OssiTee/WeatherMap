#pragma once
#include "shared/Result.h"
#include <vector>

namespace domain {

    /**
     * @brief Simple latitude/longitude coordinate pair.
     *
     * Represents a single geographic point in degrees.
     */
    struct LatLon {
        double lat; // Latitude in degrees
        double lon; // Longitude in degrees
    };

    /**
     * @brief Interface for loading raw map polygon data.
     *
     * Implementations of this interface provide geographic shapes (e.g.
     * Finland) as a list of polygons, each polygon being a list of LatLon
     * points, together with the geographic bounding box.
     *
     * The data returned here is still in geographic coordinates (lat/lon).
     * Domain services (e.g. MapService) are responsible for normalizing
     * this data into [0,1] coordinate space for rendering.
     */
    class IMapProvider {
      public:
        /**
         * @brief Structure to hold the raw loaded map data.
         *
         * This is the low‑level representation returned by data providers.
         * It contains polygons in geographic coordinates and the associated
         * bounding box. No normalization is performed at this level.
         */
        struct RawMapData {
            std::vector<std::vector<LatLon>> polygons;
            double minLat;
            double maxLat;
            double minLon;
            double maxLon;
        };

        virtual ~IMapProvider() = default;

        /**
         * @brief Loads polygon geometry in geographic coordinates.
         *
         * @return Result containing RawMapData on success, or an error message
         *         on failure.
         */
        virtual shared::Result<RawMapData> loadShape() = 0;
    };

} // namespace domain
