#pragma once

#include "shared/Result.h"
#include <utility>
#include <vector>

namespace domain {

    /**
     * @brief Repository interface for raw cloud coverage queries.
     */
    class ICloudCoverageRepository {
      public:
        /**
         * @brief Raw cloud value at geographic coordinates.
         */
        struct RawCloudPoint {
            double latitude = 0.0;
            double longitude = 0.0;
            double cloudCoverPercent = 0.0;
        };

        virtual ~ICloudCoverageRepository() = default;

        /**
         * @brief Fetches raw cloud coverage points for the given coordinates.
         * @param coords Coordinate list as latitude/longitude pairs.
         * @return Raw cloud points on success, or an error result.
         */
        virtual shared::Result<std::vector<RawCloudPoint>> fetchCloudCoverage(
            const std::vector<std::pair<double, double>> &coords) = 0;
    };

} // namespace domain
