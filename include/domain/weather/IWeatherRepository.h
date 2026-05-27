#pragma once

#include "shared/ForecastHorizon.h"
#include "shared/Result.h"
#include <vector>

namespace domain {

    /**
     * @brief Interface for weather data retrieval.
     *
     * IWeatherRepository defines the abstraction used by the domain layer
     * to obtain weather forecast data. Concrete implementations may fetch
     * data from remote services, local caches, files, or test doubles.
     *
     * The domain layer depends only on this interface, not on any specific
     * data source or technology.
     */
    class IWeatherRepository {
      public:
        virtual ~IWeatherRepository() = default;

        /**
         * @struct RawWeatherPoint
         * @brief A single unprocessed weather sample loaded directly from the
         * data source.
         *
         * Represents raw meteorological data before any normalization,
         * projection, or unit conversion is applied. Coordinates are expected
         * to be in WGS84 decimal degrees. This struct is used in the data layer
         * and transformed into domain-level WeatherPoint objects before
         * reaching the ViewModel.
         *
         * Valid ranges:
         *  - latitude  ∈ [-90, 90]
         *  - longitude ∈ [-180, 180]
         *  - windDirectionDeg ∈ [0, 360)
         *
         * Notes:
         *  - precipitationMm is given as mm/h.
         *  - symbolCode follows FMI WeatherSymbol3 (0–99).
         */
        struct RawWeatherPoint {
            double latitude = 0.0;  // Geographic latitude (WGS84)
            double longitude = 0.0; // Geographic longitude (WGS84)

            double temperature = 0.0; // Temperature in Celsius (raw)
            double windSpeedMs = 0.0; // Wind speed in m/s
            double windDirectionDeg =
                0.0; // Meteorological wind direction in degrees
            double precipitationMm = 0.0; // Precipitation intensity (mm/h)

            int symbolCode = 0; // FMI WeatherSymbol3 code (0–99)
        };

        /**
         * @brief Fetches weather data for the given forecast horizon.
         *
         * Implementations must return a vector of RawWeatherPoint objects
         * representing the weather model output for a predefined set of
         * coordinates.
         *
         * @param horizon Forecast horizon (e.g., Now, +6h, +12h, +24h).
         * @return Vector of RawWeatherPoint objects.
         */
        virtual shared::Result<
            std::vector<domain::IWeatherRepository::RawWeatherPoint>>
        fetchWeather(shared::ForecastHorizon horizon) = 0;
    };

} // namespace domain
