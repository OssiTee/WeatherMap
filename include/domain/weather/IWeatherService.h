#pragma once

#include "domain/weather/NormalizedWeatherPoint.h"
#include "shared/BoundingBox.h"
#include "shared/ForecastHorizon.h"
#include "shared/Result.h"
#include "shared/TemperatureUnit.h"
#include <vector>

namespace domain {

    /**
     * @interface IWeatherService
     * @brief Abstract interface for domain-level weather data services.
     *
     * IWeatherService defines the domain-facing contract for retrieving and
     * transforming weather forecast data. Implementations may fetch data from
     * various sources (e.g., FMI, cache, mock repositories) but must provide
     * a consistent domain API for ViewModels and other domain components.
     *
     * Responsibilities:
     *   - retrieve weather data for a given forecast horizon
     *   - apply temperature unit conversion (Celsius ↔ Fahrenheit)
     *   - expose and modify the active temperature unit
     *
     * The interface contains no UI logic and is safe to use from any layer.
     */
    class IWeatherService {
      public:
        /**
         * @brief Virtual destructor for safe polymorphic deletion.
         */
        virtual ~IWeatherService() = default;

        /**
         * @brief Returns weather points prepared for map rendering.
         *
         * Implementations must:
         *   - fetch raw weather data for the given forecast horizon
         *   - apply temperature unit conversion using the currently active unit
         *
         * @param horizon Forecast horizon (e.g., Now, +3h, +6h).
         * @return Result containing a vector of WeatherPoint objects on
         * success, or an error message on failure.
         */
        virtual shared::Result<std::vector<domain::NormalizedWeatherPoint>>
        getWeatherForMap(shared::ForecastHorizon horizon,
                         const shared::BoundingBox &box) = 0;

        /**
         * @brief Sets the temperature unit used for returned weather data.
         *
         * All subsequent calls to getWeatherForMap() must use this unit unless
         * an overload explicitly overrides it.
         *
         * @param unit Temperature unit (Celsius or Fahrenheit).
         */
        virtual void setTemperatureUnit(shared::TemperatureUnit unit) = 0;

        /**
         * @brief Returns the currently active temperature unit.
         *
         * @return TemperatureUnit enum value.
         */
        virtual shared::TemperatureUnit temperatureUnit() const = 0;
    };

} // namespace domain
