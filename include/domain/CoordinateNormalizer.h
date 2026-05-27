#pragma once

#include "map/IMapProvider.h"
#include "shared/BoundingBox.h"
#include <vector>

namespace domain {

    /**
     * @struct NormalizedPoint
     * @brief Represents a 2D point in normalized map space.
     *
     * A normalized coordinate where both X and Y are in the range [0,1].
     * The X‑axis corresponds to longitude and the Y‑axis corresponds to
     * latitude. The Y‑axis is inverted so that higher latitudes map toward
     * smaller Y values, matching typical screen coordinate conventions.
     */
    struct NormalizedPoint {
        double x; // Normalized X coordinate in the range [0,1].
        double y; // Normalized Y coordinate in the range [0,1], inverted
                  // vertically.
    };

    /**
     * @brief Provides geographic coordinate normalization for map rendering.
     *
     * Converts raw latitude/longitude coordinates into a normalized
     * [0,1] × [0,1] coordinate space using a geographic bounding box.
     * The normalized coordinates are independent of any UI framework and
     * suitable for further transformation by ViewModels or rendering layers.
     */
    class CoordinateNormalizer {
      public:
        /**
         * @brief Normalizes a single geographic coordinate.
         *
         * Maps a latitude/longitude pair into normalized [0,1] coordinates
         * using the given bounding box. The bounding box edges map directly
         * to the edges of the unit square. The Y‑axis is inverted so that
         * northern locations appear higher on the screen.
         *
         * @param lat Latitude in degrees.
         * @param lon Longitude in degrees.
         * @param box Geographic bounding box defining the normalization range.
         *
         * @return NormalizedPoint containing the normalized X/Y coordinates.
         */
        static NormalizedPoint normalize(double lat, double lon,
                                         const shared::BoundingBox &box);

        /**
         * @brief Normalizes a list of polygons into [0,1] coordinate space.
         *
         * Each polygon is represented as a list of LatLon points. The method
         * applies the same linear transformation as normalize(), using the
         * provided bounding box limits.
         *
         * @param polys Input polygons in geographic coordinates.
         * @param box Geographic bounding box defining the normalization range.
         *
         * @return A list of polygons where each point is normalized to the
         *         [0,1] × [0,1] coordinate range.
         */
        static std::vector<std::vector<NormalizedPoint>>
        normalizePolygons(const std::vector<std::vector<LatLon>> &polys,
                          const shared::BoundingBox &box);

      private:
        static inline NormalizedPoint
        normalizePoint(double lat, double lon, const shared::BoundingBox &box,
                       double latSpan, double lonSpan);
    };

} // namespace domain