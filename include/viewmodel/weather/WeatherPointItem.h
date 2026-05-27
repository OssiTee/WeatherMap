#pragma once

#include "WeatherIcon.h"
#include "shared/TemperatureUnit.h"

namespace viewmodel {

    /**
     * @brief View model for weather points displayed on the map.
     */
    struct WeatherPointItem {
        QString name; // Optional city name

        double temperature = 0.0; // Temperature in Celsius or in Fahrenheit
        double windSpeedMs = 0.0; // Wind speed in m/s
        double windDirectionDeg = 0.0; // Wind direction in degrees

        WeatherIcon icon = WeatherIcon::Clear; // Weather icon type
        shared::TemperatureUnit unit =
            shared::TemperatureUnit::Celsius; // Temperature unit

        // Normalized coordinates [0,1]
        double xNorm = 0.0; // Normalized x-coordinate
        double yNorm = 0.0; // Normalized y-coordinate
    };

} // namespace viewmodel