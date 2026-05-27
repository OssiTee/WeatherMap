#pragma once
#include "domain/map/IMapProvider.h"
#include "domain/map/NormalizedMap.h"
#include "shared/Result.h"
#include <memory>

namespace domain {

    class IMapService {
      public:
        virtual ~IMapService() = default;

        /**
         * @brief Loads and returns map polygon geometry.
         *
         * Wraps the underlying IMapProvider and provides a stable domain-level
         * API.
         */
        virtual shared::Result<domain::NormalizedMap> loadMap() const = 0;
    };

} // namespace domain
