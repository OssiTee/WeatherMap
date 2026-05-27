#pragma once

#include "IWeatherRepository.h"
#include "IWeatherService.h"
#include "domain/weather/NormalizedWeatherPoint.h"
#include "shared/BoundingBox.h"
#include "shared/ForecastHorizon.h"
#include "shared/Result.h"
#include "shared/TemperatureUnit.h"
#include <memory>
#include <vector>

namespace domain {

    /**
     * @class WeatherService
     * @brief Domain-level service for retrieving and transforming weather data.
     *
     * WeatherService provides the domain abstraction over weather data
     * retrieval. It fetches raw forecast data from an IWeatherRepository
     * implementation and applies domain-specific transformations such as:
     *
     *   - temperature unit conversion (Celsius ↔ Fahrenheit)
     *   - preparing WeatherPoint objects for map rendering
     *
     * The service contains no UI logic and is intended to be used by ViewModels
     * or other domain components.
     */
    class WeatherService : public IWeatherService {
      public:
        /**
         * @brief Constructs the WeatherService with a repository dependency.
         *
         * The service takes ownership of the provided repository instance.
         * The repository must remain valid for the lifetime of the service.
         *
         * @param repo Unique pointer to an IWeatherRepository implementation.
         *             Must not be null. Ownership is transferred.
         */
        explicit WeatherService(std::unique_ptr<IWeatherRepository> repo);

        /**
         * @brief Returns weather points prepared for map rendering.
         *
         * Fetches raw weather data for the given forecast horizon and applies
         * domain-level transformations such as temperature unit conversion
         * using the currently active temperature unit.
         *
         * @param horizon Forecast horizon (e.g., Now, +3h, +6h).
         * @return Result containing a vector of WeatherPoint objects on
         * success, or an error message on failure.
         */
        shared::Result<std::vector<domain::NormalizedWeatherPoint>>
        getWeatherForMap(shared::ForecastHorizon horizon,
                         const shared::BoundingBox &box) override;

        /**
         * @brief Sets the temperature unit used for returned weather data.
         *
         * All subsequent calls to getWeatherForMap() will use this unit.
         *
         * @param unit Temperature unit (Celsius or Fahrenheit).
         */
        void setTemperatureUnit(shared::TemperatureUnit unit) override;

        /**
         * @brief Returns the currently active temperature unit.
         *
         * @return TemperatureUnit enum value.
         */
        shared::TemperatureUnit temperatureUnit() const override;

        /**
         * @brief Returns weather points prepared for map rendering using the
         * specified temperature unit.
         *
         * This overload allows callers to request weather data in a specific
         * unit without modifying the global service state.
         *
         * @param horizon Forecast horizon.
         * @param unit Temperature unit to apply for this request.
         * @return Result containing a vector of WeatherPoint objects on
         * success, or an error message on failure.
         */
        shared::Result<std::vector<domain::NormalizedWeatherPoint>>
        getWeatherForMap(shared::ForecastHorizon horizon,
                         shared::TemperatureUnit unit);

      private:
        std::unique_ptr<IWeatherRepository> m_repo; // Data source for weather.
        shared::TemperatureUnit m_unit =
            shared::TemperatureUnit::Celsius; // Active temperature unit.
    };

} // namespace domain
