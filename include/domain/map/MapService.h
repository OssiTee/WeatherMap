#pragma once
#include "domain/map/IMapProvider.h"
#include "domain/map/IMapService.h"
#include "domain/map/NormalizedMap.h"
#include <memory>

namespace domain {

    class MapService : public IMapService {
      public:
        explicit MapService(std::unique_ptr<IMapProvider> provider);

        /**
         * @brief Loads polygon geometry from the underlying provider.
         *
         * This layer exists to:
         *  - keep ViewModels independent of data-layer providers
         *  - allow future domain logic (validation, caching, filtering)
         *  - unify architecture with CityService and WeatherService
         */
        shared::Result<domain::NormalizedMap> loadMap() const override;

      private:
        std::unique_ptr<IMapProvider> m_provider;
    };

} // namespace domain
