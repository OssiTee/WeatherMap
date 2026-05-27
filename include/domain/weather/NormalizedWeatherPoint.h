#pragma once

namespace domain {

    // Weather data point with normalized map coordinates and basic
    // meteorological values.
    struct NormalizedWeatherPoint {
        double xNorm;
        double yNorm;
        double temperature;
        double windSpeedMs;
        double windDirectionDeg;
        int symbolCode;
    };

} // namespace domain
