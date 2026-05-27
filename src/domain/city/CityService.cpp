#include "domain/city/CityService.h"
#include "domain/CoordinateNormalizer.h"
#include "domain/city/ICityLoader.h"
#include "domain/city/NormalizedCity.h"

namespace domain {

    CityService::CityService(std::unique_ptr<ICityLoader> loader)
        : m_loader(std::move(loader)) {}

    shared::Result<std::vector<domain::NormalizedCity>>
    CityService::cities(const shared::BoundingBox &box) const {

        // 1) Load raw cities
        auto rawResult = m_loader->loadCities();
        if (rawResult.isError()) {
            return shared::Result<std::vector<NormalizedCity>>::error(
                rawResult.errorMessage());
        }
        const auto &raw = rawResult.value();

        // 2) Prepare output
        std::vector<NormalizedCity> out;
        out.reserve(raw.size());

        // 3) Normalize each city
        for (const auto &c : raw) {

            // Skip cities outside bounding box
            if (c.latitude < box.minLat || c.latitude > box.maxLat ||
                c.longitude < box.minLon || c.longitude > box.maxLon) {
                continue;
            }

            auto xy =
                CoordinateNormalizer::normalize(c.latitude, c.longitude, box);

            out.emplace_back(
                NormalizedCity{.name = c.name, .xNorm = xy.x, .yNorm = xy.y});
        }

        // 4) Return normalized domain data
        return shared::Result<std::vector<NormalizedCity>>::success(
            std::move(out));
    }

} // namespace domain
