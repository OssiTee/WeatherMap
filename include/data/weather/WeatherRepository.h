#pragma once

#include <QString>
#include <memory>
#include <utility>
#include <vector>

#include "domain/weather/IWeatherRepository.h"
#include "shared/ForecastHorizon.h"
#include "shared/Result.h"

namespace data {
    class NetworkClient;
    class FmiXmlParser;
    class WeatherCache;
} // namespace data

namespace data {

    /**
     * @class WeatherRepository
     * @brief Concrete implementation of IWeatherRepository using FMI Open Data
     * API.
     *
     * WeatherRepository orchestrates the process of fetching weather forecasts:
     *  - Builds the FMI query URL for each coordinate
     *  - Uses NetworkClient to download XML data
     *  - Uses FmiXmlParser to extract weather samples
     *  - Uses WeatherCache to avoid unnecessary network requests
     *
     * This class contains no XML parsing logic and no networking logic.
     * It delegates responsibilities to dedicated helper classes.
     */
    class WeatherRepository : public domain::IWeatherRepository {
      public:
        /**
         * @brief Constructs a WeatherRepository using Qt resource files only.
         *
         * The repository loads the FMI request template and coordinate list
         * from the provided resource paths. Missing or invalid resource data
         * is not substituted with fallback values.
         *
         * @param fmiQueryResource Qt resource path for the FMI request
         * template.
         * @param coordsResource Qt resource path for weather coordinates.
         */
        explicit WeatherRepository(
            const QString &fmiQueryResource = ":/queries/fmi_request.txt",
            const QString &coordsResource = ":/queries/weather_coords.txt");

        /**
         * @brief Destructor for WeatherRepository. Defaulted since we use smart
         * pointers for resource management.
         */
        ~WeatherRepository();

        /**
         * @brief Fetches weather data for the given forecast horizon.
         *
         * If cached data is still fresh, a successful Result containing the
         * cached vector of WeatherPoint objects is returned.
         *
         * Otherwise, new data is downloaded from FMI, parsed and returned.
         *
         * In case of network errors, empty responses or XML parsing failures,
         * an error Result is returned containing a descriptive error message.
         *
         * @param horizon Forecast horizon (Now, +6h, +12h, +24h).
         * @return Result containing either a vector of WeatherPoint objects on
         * success or an error message on failure.
         */
        shared::Result<std::vector<domain::IWeatherRepository::RawWeatherPoint>>
        fetchWeather(shared::ForecastHorizon horizon) override;

      private:
        std::unique_ptr<NetworkClient> client_;
        std::unique_ptr<data::FmiXmlParser> parser_;
        std::unique_ptr<WeatherCache> cache_;
        const QString m_fmiQueryTemplate;
        const std::vector<std::pair<double, double>> m_coords;
    };

} // namespace data
