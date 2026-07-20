#include "viewmodel/weather/WeatherDataViewModel.h"
#include "viewmodel/weather/WeatherPointItem.h"
#include <QtConcurrent/QtConcurrent>
#include <cassert>

namespace viewmodel {

    // Construct the weather ViewModel and register async signal types.
    WeatherDataViewModel::WeatherDataViewModel(
        std::unique_ptr<domain::IWeatherService> service)
        : m_service(std::move(service)) {

        assert(m_service && "WeatherService must not be null");

        qRegisterMetaType<std::shared_ptr<const std::vector<WeatherPointItem>>>(
            "std::shared_ptr<const std::vector<viewmodel::WeatherPointItem>>");

        connect(&m_weatherFutureWatcher,
                &QFutureWatcher<
                    std::vector<domain::NormalizedWeatherPoint>>::finished,
                this, &WeatherDataViewModel::onWeatherFetchFinished);
    }

    void WeatherDataViewModel::setBoundingBox(const shared::BoundingBox &box) {
        m_bbox = box;
        m_hasBBox = true;
    }

    void
    WeatherDataViewModel::setTemperatureUnit(shared::TemperatureUnit unit) {
        m_lastRequestedUnit = unit;
    }

    // Convert the FMI WeatherSymbol3 numeric code into a UI weather icon.
    static WeatherIcon convertSymbolCode(int code) {
        if (code == 1) {
            return WeatherIcon::Clear;
        }
        if (code == 2) {
            return WeatherIcon::PartlyCloudy;
        }
        if (code == 3) {
            return WeatherIcon::Cloudy;
        }

        // Rain (21–39)
        if (code >= 21 && code <= 39) {
            return WeatherIcon::Rain;
        }

        // Snow (41–49)
        if (code >= 41 && code <= 49) {
            return WeatherIcon::Snow;
        }

        // Thunder (61–69)
        if (code >= 61 && code <= 69) {
            return WeatherIcon::Thunder;
        }

        return WeatherIcon::Clear;
    }

    // Start an asynchronous weather data fetch for the selected horizon.
    void WeatherDataViewModel::load(shared::ForecastHorizon horizon) {
        if (!m_hasBBox || m_weatherFutureWatcher.isRunning()) {
            return;
        }

        m_lastHorizon = horizon;

        // Launch asynchronous weather loading on a background thread using
        // QtConcurrent. Instead of capturing 'this', we copy only the
        // required values (unit, bbox, service pointer). This avoids
        // lifetime issues and ensures the lambda does not access QObject
        // members from a worker thread.
        //
        // The heavy network request + XML parsing happens off the UI
        // thread, and the QFutureWatcher delivers the result back to the
        // main thread.
        auto unit = m_lastRequestedUnit;
        m_inFlightUnit = unit;
        auto bbox = m_bbox;
        auto service = m_service.get();

        auto future = QtConcurrent::run([service, unit, bbox, horizon]() {
            return service->getWeatherForMap(horizon, bbox, unit);
        });
        m_weatherFutureWatcher.setFuture(std::move(future));
    }

    // Handle completion of the async weather fetch, update view state,
    // and emit view-model notifications.
    void WeatherDataViewModel::onWeatherFetchFinished() {
        const auto result = m_weatherFutureWatcher.result();

        // 1) Error handling
        if (result.isError()) {
            emit errorOccurred(QString::fromStdString(result.errorMessage()));
            return;
        }

        // 2) Successful data
        const auto &raw = result.value();

        auto points = std::make_shared<std::vector<WeatherPointItem>>();
        points->reserve(raw.size());

        for (const auto &p : raw) {
            WeatherPointItem item;

            item.temperature = p.temperature;
            item.windSpeedMs = p.windSpeedMs;
            item.windDirectionDeg = p.windDirectionDeg;
            item.unit = m_inFlightUnit;
            item.icon = convertSymbolCode(p.symbolCode);

            item.xNorm = p.xNorm;
            item.yNorm = p.yNorm;

            points->push_back(item);
        }

        m_points = points;
        m_lastUpdated = QDateTime::currentDateTime();
        emit lastUpdatedChanged();
        emit weatherPointsReady(std::move(points));
    }

} // namespace viewmodel
