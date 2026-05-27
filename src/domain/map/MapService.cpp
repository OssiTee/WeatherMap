#include "domain/map/MapService.h"
#include "domain/CoordinateNormalizer.h"
#include "domain/map/NormalizedMap.h"

namespace domain {

    MapService::MapService(std::unique_ptr<IMapProvider> provider)
        : m_provider(std::move(provider)) {}

    shared::Result<domain::NormalizedMap> MapService::loadMap() const {
        // Load raw geographic polygons (lat/lon) from the provider
        auto raw = m_provider->loadShape();
        if (raw.isError()) {
            return shared::Result<domain::NormalizedMap>::error(
                raw.errorMessage());
        }

        const auto &r = raw.value();

        // Domain-level normalization into [0,1] coordinate space
        shared::BoundingBox box(r.minLat, r.maxLat, r.minLon, r.maxLon);
        auto normalized =
            CoordinateNormalizer::normalizePolygons(r.polygons, box);

        domain::NormalizedMap data;
        data.polygons = std::move(normalized);
        data.bbox = box;

        return shared::Result<domain::NormalizedMap>::success(std::move(data));
    }

} // namespace domain
