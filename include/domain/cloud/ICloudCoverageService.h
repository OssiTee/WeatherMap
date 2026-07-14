#pragma once

#include "domain/cloud/NormalizedCloudPoint.h"
#include "shared/BoundingBox.h"
#include "shared/Result.h"
#include <vector>

namespace domain {

    /**
     * @brief Domain service interface for map-ready cloud coverage data.
     */
    class ICloudCoverageService {
      public:
        virtual ~ICloudCoverageService() = default;

        /**
         * @brief Returns normalized cloud coverage points for a map viewport.
         * @param box Map bounding box in geographic coordinates.
         * @param latSamples Number of latitude samples in the query grid.
         * @param lonSamples Number of longitude samples in the query grid.
         * @return Normalized cloud points on success, or an error result.
         */
        virtual shared::Result<std::vector<NormalizedCloudPoint>>
        getCloudForMap(const shared::BoundingBox &box, int latSamples,
                 int lonSamples) = 0;
    };

} // namespace domain
