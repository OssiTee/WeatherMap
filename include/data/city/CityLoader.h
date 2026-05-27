#pragma once

#include "domain/city/ICityLoader.h"
#include "shared/Result.h"

#include <QString>
#include <vector>

namespace data {

    /**
     * @class CityLoader
     * @brief Loads a list of cities from a JSON resource file.
     *
     * CityLoader implements domain::ICityLoader and returns
     * domain::ICityLoader::RawCity objects wrapped in a Result type. This
     * allows the caller to distinguish between successful loads and error
     * conditions (missing file, malformed JSON, etc.).
     */
    class CityLoader : public domain::ICityLoader {
      public:
        /**
         * @brief Constructs a CityLoader with a given resource path.
         * @param resourcePath Path to the city data file (e.g.
         * :/cities/cities.json).
         */
        explicit CityLoader(QString resourcePath);

        /**
         * @brief Loads all cities from the resource file.
         *
         * @return Result containing either:
         *         - a vector of domain::ICityLoader::RawCity on success
         *         - an error message on failure
         */
        shared::Result<std::vector<domain::ICityLoader::RawCity>>
        loadCities() override;

      private:
        QString m_resourcePath; // Path to the city data file
    };

} // namespace data
