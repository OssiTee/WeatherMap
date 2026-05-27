#pragma once

#include "shared/Result.h"
#include <vector>

namespace domain {

    /**
     * @class ICityLoader
     * @brief Data-layer abstraction for loading city information.
     *
     * Implementations of this interface are responsible for retrieving
     * city data from a specific source such as:
     *   - JSON files
     *   - databases
     *   - network APIs
     *
     * The loader returns a Result<T> to preserve both success and error
     * information for the domain layer.
     */
    class ICityLoader {
      public:
        virtual ~ICityLoader() = default;

        /**
         * @struct RawCity
         * @brief Represents a single city with unprocessed geographic
         * coordinates.
         *
         * This struct is used to transfer raw city data loaded from external
         * sources (e.g., JSON, CSV, or resource files) before any normalization
         * or projection is applied. Coordinates are expected to be in WGS84
         * decimal degrees.
         *
         * Valid ranges:
         *  - latitude  ∈ [-90, 90]
         *  - longitude ∈ [-180, 180]
         */
        struct RawCity {
            std::string name; // City name in plain text
            double latitude;  // Latitude in WGS84 decimal degrees
            double longitude; // Longitude in WGS84 decimal degrees
        };

        /**
         * @brief Loads cities from the underlying data source.
         *
         * @return Result containing a vector of RawCity objects on success,
         *         or an error message on failure.
         */
        virtual shared::Result<std::vector<RawCity>> loadCities() = 0;
    };

} // namespace domain
