#include "domain/cloud/CloudCoverageService.h"
#include "domain/CoordinateNormalizer.h"
#include <algorithm>
#include <cassert>

namespace domain {

    CloudCoverageService::CloudCoverageService(
        std::unique_ptr<ICloudCoverageRepository> repository)
        : m_repository(std::move(repository)) {
        // The repository is injected so the domain layer stays independent of
        // the concrete FMI data source.
        assert(m_repository && "ICloudCoverageRepository must not be null");
    }

    shared::Result<std::vector<NormalizedCloudPoint>>
    CloudCoverageService::getCloudForMap(const shared::BoundingBox &box,
                                         int latSamples, int lonSamples) {
        if (latSamples < 2 || lonSamples < 2) {
            return shared::Result<std::vector<NormalizedCloudPoint>>::error(
                "Cloud sample grid must be at least 2x2");
        }

        if (box.minLat >= box.maxLat || box.minLon >= box.maxLon) {
            return shared::Result<std::vector<NormalizedCloudPoint>>::error(
                "Invalid bounding box for cloud coverage");
        }

        // Build a regular lat/lon query grid over the active map bounds.
        std::vector<std::pair<double, double>> coords;
        coords.reserve(static_cast<size_t>(latSamples) *
                       static_cast<size_t>(lonSamples));

        const double latSpan = box.maxLat - box.minLat;
        const double lonSpan = box.maxLon - box.minLon;

        for (int y = 0; y < latSamples; ++y) {
            const double yNorm = static_cast<double>(y) / (latSamples - 1);
            const double lat = box.minLat + yNorm * latSpan;

            for (int x = 0; x < lonSamples; ++x) {
                const double xNorm = static_cast<double>(x) / (lonSamples - 1);
                const double lon = box.minLon + xNorm * lonSpan;
                coords.emplace_back(lat, lon);
            }
        }

        // Repository data is still in geographic coordinates; convert it into
        // normalized map-space points for the viewmodel.
        auto rawResult = m_repository->fetchCloudCoverage(coords);
        if (rawResult.isError()) {
            return shared::Result<std::vector<NormalizedCloudPoint>>::error(
                rawResult.errorMessage());
        }

        auto rawPoints = std::move(rawResult).value();
        std::vector<NormalizedCloudPoint> out;
        out.reserve(rawPoints.size());

        // Convert geographic coordinates into normalized map-space points.
        for (const auto &p : rawPoints) {
            auto xy =
                CoordinateNormalizer::normalize(p.latitude, p.longitude, box);
            out.push_back(NormalizedCloudPoint{
                .xNorm = xy.x,
                .yNorm = xy.y,
                .cloudCoverPercent =
                    std::clamp(p.cloudCoverPercent, 0.0, 100.0),
            });
        }

        return shared::Result<std::vector<NormalizedCloudPoint>>::success(
            std::move(out));
    }

} // namespace domain
