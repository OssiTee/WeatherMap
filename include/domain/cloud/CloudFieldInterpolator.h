#pragma once

#include "domain/cloud/NormalizedCloudPoint.h"
#include <vector>

namespace domain {

    /**
     * @brief Builds a regular cloud field from sparse normalized points.
     *
     * The interpolated output is UI-independent and can be mapped to any
     * visual style by the caller.
     */
    class CloudFieldInterpolator {
      public:
        /**
         * @brief Interpolates cloud cover percentages on a regular grid.
         * @param points Normalized sample points in [0,1] map space.
         * @param width Output field width.
         * @param height Output field height.
         * @param sigma Gaussian sigma in normalized map-space units.
         * @return Row-major field of size width*height, or empty on invalid input.
         */
        static std::vector<double>
        buildWeightedCloudField(const std::vector<NormalizedCloudPoint> &points,
                                int width, int height, double sigma);
    };

} // namespace domain
