#pragma once

#include "domain/cloud/ICloudCoverageRepository.h"
#include "domain/cloud/ICloudCoverageService.h"
#include <memory>

namespace domain {

    /**
     * @brief Default cloud coverage service implementation.
     *
     * Converts repository-level raw cloud values into normalized map-space
     * points suitable for viewmodel rendering.
     */
    class CloudCoverageService : public ICloudCoverageService {
      public:
        /**
         * @brief Constructs the cloud coverage service.
         * @param repository Repository used to fetch raw cloud values.
         */
        explicit CloudCoverageService(
            std::unique_ptr<ICloudCoverageRepository> repository);

        /**
         * @brief Returns normalized cloud points for the provided map area.
         * @param box Map bounding box in geographic coordinates.
         * @param latSamples Number of latitude samples in the query grid.
         * @param lonSamples Number of longitude samples in the query grid.
         * @return Normalized cloud points on success, or an error result.
         */
        shared::Result<std::vector<NormalizedCloudPoint>>
        getCloudForMap(const shared::BoundingBox &box, int latSamples,
                 int lonSamples) override;

      private:
        std::unique_ptr<ICloudCoverageRepository> m_repository;
    };

} // namespace domain
