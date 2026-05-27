#include "data/weather/FmiXmlParser.h"
#include <QXmlStreamReader>
#include <cmath>

namespace {

    // XML element names used in FMI WFS response
    const QString XML_ELEMENT_ROOT = "BsWfsElement";
    const QString XML_ELEMENT_TIME = "Time";
    const QString XML_ELEMENT_PARAM_NAME = "ParameterName";
    const QString XML_ELEMENT_PARAM_VAL = "ParameterValue";

    // FMI parameter names (raw API identifiers)
    const QString PARAM_TEMPERATURE = "Temperature";
    const QString PARAM_WIND_SPEED = "WindSpeedMS";
    const QString PARAM_WIND_DIR = "WindDirection";
    const QString PARAM_PRECIP_1H = "Precipitation1h";
    const QString PARAM_SYMBOL_3 = "WeatherSymbol3";

    // Internal normalized keys used by parser
    const QString KEY_TEMP = "temperature";
    const QString KEY_WIND = "windspeed";
    const QString KEY_DIR = "winddirection";
    const QString KEY_RAIN = "precipitation";
    const QString KEY_SYMBOL = "symbol";

    // Normalize FMI parameter names to internal keys
    QString mapParameterName(const QString &name) {
        if (name == PARAM_TEMPERATURE) {
            return KEY_TEMP;
        }
        if (name == PARAM_WIND_SPEED) {
            return KEY_WIND;
        }
        if (name == PARAM_WIND_DIR) {
            return KEY_DIR;
        }
        if (name == PARAM_PRECIP_1H) {
            return KEY_RAIN;
        }
        if (name == PARAM_SYMBOL_3) {
            return KEY_SYMBOL;
        }
        return {};
    }

} // namespace

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------

namespace data {

    // Parse FMI XML and return best matching WeatherPoint for target time
    std::optional<domain::IWeatherRepository::RawWeatherPoint>
    FmiXmlParser::parse(const QByteArray &xmlData, double lat, double lon,
                        const QDateTime &targetUtc) const {
        auto samples = parseDocument(xmlData);
        if (samples.empty()) {
            return std::nullopt;
        }

        auto best = selectBestSample(samples, targetUtc);
        if (!best.has_value()) {
            return std::nullopt;
        }

        const auto &[time, vals] = *best;

        return domain::IWeatherRepository::RawWeatherPoint{
            lat, lon, vals.temp, vals.wind, vals.dir, vals.rain, vals.symbol};
    }

    // ---------------------------------------------------------
    // Step 1: Parse XML document into timestamp → CollectedValues
    // ---------------------------------------------------------
    std::map<QDateTime, FmiXmlParser::CollectedValues>
    FmiXmlParser::parseDocument(const QByteArray &xmlData) const {
        QXmlStreamReader xml(xmlData);

        ElementState state;
        bool inElement = false;
        std::map<QDateTime, CollectedValues> samples;

        // Parse FMI XML into timestamp → collected parameter values
        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.isStartElement()) {

                if (xml.name() == XML_ELEMENT_ROOT) {
                    // Start of a single FMI data element (BsWfsElement)
                    inElement = true;
                    state = {};
                } else if (inElement) {
                    if (xml.name() == XML_ELEMENT_TIME) {
                        // Extract ISO timestamp for this measurement
                        state.time = xml.readElementText().trimmed();
                        state.hasTime = !state.time.isEmpty();
                    } else if (xml.name() == XML_ELEMENT_PARAM_NAME) {
                        // Map FMI parameter name to internal key
                        state.param =
                            mapParameterName(xml.readElementText().trimmed());
                        state.hasParam = !state.param.isEmpty();
                    } else if (xml.name() == XML_ELEMENT_PARAM_VAL) {
                        // Parse numeric value for the current parameter
                        bool ok = false;
                        state.value = xml.readElementText().toDouble(&ok);
                        state.hasValue = ok;
                    }
                }
            }

            else if (xml.isEndElement() && xml.name() == XML_ELEMENT_ROOT) {
                inElement = false;

                if (state.hasTime && state.hasParam && state.hasValue) {
                    // Store parsed parameter into the correct timestamp bucket
                    applyElement(state, samples);
                }
            }
        }

        return samples;
    }

    // ---------------------------------------------------------
    // Step 2: Apply parsed element to sample map
    // ---------------------------------------------------------
    void FmiXmlParser::applyElement(
        const ElementState &state,
        std::map<QDateTime, CollectedValues> &samples) const {

        QDateTime t = QDateTime::fromString(state.time, Qt::ISODate).toUTC();
        if (!t.isValid()) {
            return;
        }
        auto &v = samples[t];

        if (state.param == KEY_TEMP && !v.haveTemp) {
            v.temp = state.value;
            v.haveTemp = true;
        } else if (state.param == KEY_WIND && !v.haveWind) {
            v.wind = state.value;
            v.haveWind = true;
        } else if (state.param == KEY_DIR && !v.haveDir) {
            v.dir = state.value;
            v.haveDir = true;
        } else if (state.param == KEY_RAIN && !v.havePrecip) {
            // Clamp negative precipitation values to zero (FMI sometimes
            // returns -0.0)
            v.rain = std::max(0.0, state.value);
            v.havePrecip = true;
        } else if (state.param == KEY_SYMBOL && !v.haveSymbol) {
            v.symbol = static_cast<int>(state.value);
            v.haveSymbol = true;
        }
    }

    // ---------------------------------------------------------
    // Step 3: Select best sample for target time
    // ---------------------------------------------------------
    std::optional<std::pair<QDateTime, FmiXmlParser::CollectedValues>>
    FmiXmlParser::selectBestSample(
        const std::map<QDateTime, CollectedValues> &samples,
        const QDateTime &targetUtc) const {

        bool found = false;
        QDateTime bestTime;
        CollectedValues bestValues;

        // First: earliest complete sample >= target time
        for (const auto &[time, vals] : samples) {
            if (!isComplete(vals)) {
                continue;
            }

            if (time < targetUtc) {
                continue;
            }

            if (!found || time < bestTime) {
                found = true;
                bestTime = time;
                bestValues = vals;
            }
        }

        // Second: if none >= target, pick closest complete sample
        if (!found) {
            bool first = true;
            qint64 bestDelta = 0;

            for (const auto &[time, vals] : samples) {
                if (!isComplete(vals)) {
                    continue;
                }

                qint64 delta = std::llabs(time.secsTo(targetUtc));

                if (first || delta < bestDelta) {
                    first = false;
                    bestDelta = delta;
                    bestTime = time;
                    bestValues = vals;
                    found = true;
                }
            }
        }

        if (!found) {
            return std::nullopt;
        }

        return std::make_pair(bestTime, bestValues);
    }

    // ---------------------------------------------------------
    // Step 4: Check completeness of collected values
    // ---------------------------------------------------------
    bool FmiXmlParser::isComplete(const CollectedValues &v) const {
        // Ensure all required weather parameters are present
        return v.haveTemp && v.haveWind && v.haveDir && v.havePrecip &&
               v.haveSymbol;
    }

} // namespace data
