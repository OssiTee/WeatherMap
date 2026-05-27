#pragma once

#include "domain/city/NormalizedCity.h"
#include "shared/BoundingBox.h"
#include "shared/Result.h"
#include <vector>

namespace domain {

    /**
     * @class ICityService
     * @brief Domain-level interface for querying normalized city data.
     *
     * Provides access to city information within a specified geographic area.
     * Implementations may retrieve the data from any source (e.g. in-memory,
     * database, loader), but this interface does not dictate how.
     *
     * Results are wrapped in Result<T> to allow callers to handle errors
     * in a uniform way.
     */
    class ICityService {
      public:
        virtual ~ICityService() = default;

        /**
         * @brief Returns normalized cities within the given bounding box.
         *
         * @param box Geographic area to query.
         * @return Result containing a vector of NormalizedCity objects on
         * success, or an error message on failure.
         */
        virtual shared::Result<std::vector<domain::NormalizedCity>>
        cities(const shared::BoundingBox &box) const = 0;
    };

} // namespace domain
