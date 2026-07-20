#include "domain/weather/WeatherService.h"
#include "domain/CoordinateNormalizer.h"
#include "domain/weather/NormalizedWeatherPoint.h"
#include <cassert>

namespace {

    constexpr double FAHRENHEIT_SCALE_NUM = 9.0;
    constexpr double FAHRENHEIT_SCALE_DEN = 5.0;
    constexpr double FAHRENHEIT_OFFSET = 32.0;

    inline double toFahrenheit(double celsius) {
        return celsius * (FAHRENHEIT_SCALE_NUM / FAHRENHEIT_SCALE_DEN) +
               FAHRENHEIT_OFFSET;
    }

} // namespace

namespace domain {

    namespace {

        inline double convertTemperature(double celsius,
                                         shared::TemperatureUnit unit) {
            if (unit == shared::TemperatureUnit::Fahrenheit) {
                return toFahrenheit(celsius);
            }

            return celsius;
        }

    } // namespace

    WeatherService::WeatherService(std::unique_ptr<IWeatherRepository> repo)
        : m_repo(std::move(repo)) {
        assert(m_repo && "IWeatherRepository must not be null");
    }

    shared::Result<std::vector<NormalizedWeatherPoint>>
    WeatherService::getWeatherForMap(shared::ForecastHorizon horizon,
                                     const shared::BoundingBox &box,
                                     shared::TemperatureUnit unit) {

        // 1) Fetch raw data
        auto rawResult = m_repo->fetchWeather(horizon);
        if (rawResult.isError()) {
            return shared::Result<std::vector<NormalizedWeatherPoint>>::error(
                rawResult.errorMessage());
        }

        auto raw = rawResult.value();

        // 2) Prepare output
        std::vector<NormalizedWeatherPoint> out;
        out.reserve(raw.size());

        // 3) Convert + normalize
        for (const auto &p : raw) {

            // Skip points outside bounding box
            if (p.latitude < box.minLat || p.latitude > box.maxLat ||
                p.longitude < box.minLon || p.longitude > box.maxLon) {
                continue;
            }

            auto xy =
                CoordinateNormalizer::normalize(p.latitude, p.longitude, box);

            const double temp = convertTemperature(p.temperature, unit);

            out.emplace_back(
                NormalizedWeatherPoint{.xNorm = xy.x,
                                       .yNorm = xy.y,
                                       .temperature = temp,
                                       .windSpeedMs = p.windSpeedMs,
                                       .windDirectionDeg = p.windDirectionDeg,
                                       .symbolCode = p.symbolCode});
        }

        return shared::Result<std::vector<NormalizedWeatherPoint>>::success(
            std::move(out));
    }

} // namespace domain
