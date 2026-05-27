#pragma once

#include "domain/weather/IWeatherRepository.h"
#include <QByteArray>
#include <QDateTime>
#include <map>
#include <optional>

namespace data {

    /**
     * @class FmiXmlParser
     * @brief Parses FMI HARMONIE XML forecast responses into WeatherPoint
     * objects.
     *
     * This class is responsible for interpreting the XML returned by the
     * Finnish Meteorological Institute (FMI) WFS API. It extracts individual
     * forecast samples, groups them by timestamp, and selects the most suitable
     * sample for a given forecast horizon.
     *
     * Responsibilities:
     *  - Parse XML elements and extract parameter values
     *  - Group values by timestamp
     *  - Determine which timestamp best matches the requested forecast time
     *  - Construct a WeatherPoint with the selected values
     *
     * The parser performs no network operations and has no dependencies on
     * WeatherRepository or caching logic.
     */
    class FmiXmlParser {
      public:
        /**
         * @brief Parses an FMI XML document and returns the best matching
         * weather point.
         *
         * @param xmlData Raw XML data returned by the FMI API.
         * @param lat Latitude of the queried coordinate.
         * @param lon Longitude of the queried coordinate.
         * @param targetUtc The desired forecast timestamp (UTC).
         * @return Optional WeatherPoint. std::nullopt if no complete sample is
         * found.
         */
        std::optional<domain::IWeatherRepository::RawWeatherPoint>
        parse(const QByteArray &xmlData, double lat, double lon,
              const QDateTime &targetUtc) const;

      private:
        // ---- Internal helper structures ----

        struct ElementState {
            QString time;
            QString param;
            double value = 0.0;
            bool hasTime = false;
            bool hasParam = false;
            bool hasValue = false;
        };

        struct CollectedValues {
            bool haveTemp = false;
            bool haveWind = false;
            bool haveDir = false;
            bool havePrecip = false;
            bool haveSymbol = false;

            double temp = 0.0;
            double wind = 0.0;
            double dir = 0.0;
            double rain = 0.0;
            int symbol = 0;
        };

        // ---- Parsing steps ----

        /**
         * @brief Reads the XML document and fills a timestamp → CollectedValues
         * map.
         *
         * @param xmlData Raw XML data.
         * @return Map of timestamps to collected parameter values.
         */
        std::map<QDateTime, CollectedValues>
        parseDocument(const QByteArray &xmlData) const;

        /**
         * @brief Applies a completed ElementState to the sample map.
         *
         * @param state Parsed element state.
         * @param samples Map of timestamp → CollectedValues.
         */
        void applyElement(const ElementState &state,
                          std::map<QDateTime, CollectedValues> &samples) const;

        /**
         * @brief Selects the best sample for the given target time.
         *
         * Strategy:
         *  1) Prefer the earliest complete sample >= targetUtc
         *  2) If none exist, choose the closest complete sample
         *
         * @param samples Map of timestamp → CollectedValues.
         * @param targetUtc Desired forecast timestamp.
         * @return Optional pair of (timestamp, values).
         */
        std::optional<std::pair<QDateTime, CollectedValues>>
        selectBestSample(const std::map<QDateTime, CollectedValues> &samples,
                         const QDateTime &targetUtc) const;

        /**
         * @brief Checks whether a CollectedValues struct contains all required
         * fields.
         */
        bool isComplete(const CollectedValues &v) const;
    };

} // namespace data