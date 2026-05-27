#include "data/weather/WeatherCache.h"

namespace data {

    bool WeatherCache::hasFresh(shared::ForecastHorizon horizon,
                                const QDateTime &now) const noexcept {
        auto it = cache_.find(horizon);
        if (it == cache_.end()) {
            return false;
        }

        const auto &entry = it->second;
        const int age = entry.timestamp.secsTo(now);
        return age >= 0 && age < CACHE_SECONDS;
    }

    const std::vector<domain::IWeatherRepository::RawWeatherPoint> &
    WeatherCache::get(shared::ForecastHorizon horizon) const noexcept {
        return cache_.at(horizon).data;
    }

    void WeatherCache::put(
        shared::ForecastHorizon horizon,
        std::vector<domain::IWeatherRepository::RawWeatherPoint> data,
        const QDateTime &now) {
        cache_[horizon] = Entry{.data = std::move(data), .timestamp = now};
    }

} // namespace data
