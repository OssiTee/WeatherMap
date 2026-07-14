#pragma once

#include "data/weather/NetworkClient.h"
#include "domain/cloud/ICloudCoverageRepository.h"
#include <QString>
#include <memory>

namespace data {

    /**
     * @brief FMI-backed repository for cloud coverage point queries.
     *
     * Builds per-coordinate WFS request URLs from a query template resource and
     * returns raw cloud coverage points for further domain normalization.
     */
    class CloudCoverageRepository : public domain::ICloudCoverageRepository {
      public:
        /**
         * @brief Constructs the cloud coverage repository.
         * @param client Network client used for FMI requests.
         * @param queryTemplateResource Qt resource path to the FMI query template.
         */
        explicit CloudCoverageRepository(
            std::unique_ptr<NetworkClient> client,
            const QString &queryTemplateResource =
                ":/queries/fmi_cloud_request.txt");

        /**
         * @brief Fetches cloud coverage values for the requested coordinates.
         * @param coords Coordinate list as latitude/longitude pairs.
         * @return Raw cloud coverage points on success, or an error result.
         */
        shared::Result<std::vector<domain::ICloudCoverageRepository::RawCloudPoint>>
        fetchCloudCoverage(const std::vector<std::pair<double, double>> &coords)
          override;

      private:
        QString buildRequestUrl(double lat, double lon) const;

        std::unique_ptr<NetworkClient> m_client;
        QString m_queryTemplate;
    };

} // namespace data
