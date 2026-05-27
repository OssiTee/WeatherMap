#pragma once

namespace shared {

    /**
     * @brief Forecast horizon used by WeatherDataViewModel and
     * WeatherRepository.
     */
    enum class ForecastHorizon {
        Now,     // Current weather conditions
        Plus6h,  // Weather forecast for +6 hours
        Plus12h, // Weather forecast for +12 hours
        Plus24h  // Weather forecast for +24 hours
    };

} // namespace shared