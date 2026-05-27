#pragma once

namespace viewmodel {

    /**
     * @brief Weather icon types for UI display.
     */
    enum class WeatherIcon {
        Clear,        // Clear sky
        PartlyCloudy, // Partly cloudy
        Cloudy,       // Cloudy
        Rain,         // Rain
        Snow,         // Snow
        Thunder,      // Thunderstorm
        Unknown       // Unknown weather condition
    };

} // namespace viewmodel