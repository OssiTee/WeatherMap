#pragma once

namespace shared {

    /**
     * @brief Represents a geographic bounding box in degrees.
     *
     * Used by CoordinateNormalizer and ViewModels to normalize geographic
     * coordinates (lat/lon) into [0,1] coordinate space.
     */
    struct BoundingBox {
        double minLat = 0.0;
        double maxLat = 0.0;
        double minLon = 0.0;
        double maxLon = 0.0;

        BoundingBox() = default;

        BoundingBox(double minLat_, double maxLat_, double minLon_,
                    double maxLon_)
            : minLat(minLat_), maxLat(maxLat_), minLon(minLon_),
              maxLon(maxLon_) {}
    };

} // namespace shared
