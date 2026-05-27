#pragma once

#include "domain/weather/IWeatherRepository.h"
#include "shared/ForecastHorizon.h"
#include <QDateTime>
#include <unordered_map>
#include <vector>

namespace data {

    /**
     * @class WeatherCache
     * @brief In-memory time-based cache for weather forecast results.
     *
     * WeatherCache stores weather data grouped by forecast horizon (Now, +6h,
     * +12h, +24h). Each entry is associated with a timestamp indicating when
     * the data was fetched. Cached data is considered valid only for a limited
     * duration, after which it is treated as stale and must be refreshed.
     *
     * Responsibilities:
     *  - Store weather data for each forecast horizon
     *  - Track when each entry was last updated
     *  - Determine whether cached data is still fresh
     *  - Provide cached data to callers
     *
     * The returned reference from get() is a non-owning view into internally
     * stored data. It remains valid as long as WeatherCache remains alive.
     */
    class WeatherCache {
      public:
        /**
         * @brief Checks whether cached data for the given horizon is still
         * fresh.
         *
         * @param horizon Forecast horizon key.
         * @param now Current UTC timestamp.
         * @return true if the cache contains an entry and it is still valid.
         */
        [[nodiscard]]
        bool hasFresh(shared::ForecastHorizon horizon,
                      const QDateTime &now) const noexcept;

        /**
         * @brief Retrieves cached data for the given horizon.
         *
         * Assumes that hasFresh() has already been checked.
         *
         * @param horizon Forecast horizon key.
         * @return Reference to the cached vector of WeatherPoint objects.
         *         The reference remains valid as long as WeatherCache lives.
         */
        [[nodiscard]]
        const std::vector<domain::IWeatherRepository::RawWeatherPoint> &
        get(shared::ForecastHorizon horizon) const noexcept;

        /**
         * @brief Stores new weather data for the given horizon.
         *
         * @param horizon Forecast horizon key.
         * @param data Weather points to store.
         * @param now Timestamp when the data was fetched.
         */
        void put(shared::ForecastHorizon horizon,
                 std::vector<domain::IWeatherRepository::RawWeatherPoint> data,
                 const QDateTime &now);

      private:
        /**
         * @brief Internal cache entry containing weather data and timestamp.
         */
        struct Entry {
            std::vector<domain::IWeatherRepository::RawWeatherPoint> data;
            QDateTime timestamp;
        };

        // Map of forecast horizon → cached entry
        std::unordered_map<shared::ForecastHorizon, Entry> cache_;

        // Cache validity duration in seconds (10 minutes)
        static constexpr int CACHE_SECONDS = 10 * 60;
    };

} // namespace data
