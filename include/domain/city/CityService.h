#pragma once

#include "ICityService.h"
#include "domain/city/ICityLoader.h"
#include "domain/city/NormalizedCity.h"
#include "shared/BoundingBox.h"
#include "shared/Result.h"

#include <memory>
#include <vector>

namespace domain {

    /**
     * @class CityService
     * @brief Concrete implementation of ICityService.
     *
     * CityService is responsible for loading city data from an injected
     * ICityLoader implementation. The service stores the loaded data as a
     * Result<std::vector<City>> to preserve both success and error states.
     */
    class CityService : public ICityService {
      public:
        /**
         * @brief Constructs the service.
         *
         * @param loader Unique pointer to a domain-level city loader.
         *               Ownership is transferred to CityService.
         */
        explicit CityService(std::unique_ptr<ICityLoader> loader);

        /**
         * @brief Loads city list or an error.
         *
         * The returned Result contains either:
         *   - a vector of City objects on success
         *   - an error message on failure
         *
         * This allows ViewModels to surface loading errors to the UI.
         *
         * @return Result containing a vector of City objects or an error
         * string.
         */
        shared::Result<std::vector<domain::NormalizedCity>>
        cities(const shared::BoundingBox &box) const override;

      private:
        std::unique_ptr<ICityLoader> m_loader;
    };

} // namespace domain
