#pragma once

#include <QDateTime>
#include <QFutureWatcher>
#include <QObject>
#include <QTimer>
#include <memory>
#include <vector>

#include "domain/weather/WeatherService.h"
#include "shared/BoundingBox.h"
#include "shared/ForecastHorizon.h"
#include "shared/TemperatureUnit.h"
#include "viewmodel/weather/WeatherPointItem.h"

namespace viewmodel {

    /**
     * @class WeatherDataViewModel
     * @brief ViewModel responsible for presenting weather data to the UI.
     *
     * Responsibilities:
     *  - Request weather data from the domain WeatherService.
     *  - Receive already-normalized weather points from the domain layer.
     *  - Convert domain weather points into UI-friendly view models.
     *  - Automatically refresh weather data at fixed intervals.
     */
    class WeatherDataViewModel : public QObject {
        Q_OBJECT

        /**
         * @brief Human-readable timestamp of the last successful weather
         * update.
         */
        Q_PROPERTY(
            QString lastUpdated READ lastUpdated NOTIFY lastUpdatedChanged)

      public:
        /**
         * @brief Constructs the ViewModel with a WeatherService dependency.
         *
         * @param service Unique pointer to an IWeatherService implementation.
         *                Ownership is transferred to the ViewModel.
         *                Must not be null.
         */
        explicit WeatherDataViewModel(
            std::unique_ptr<domain::IWeatherService> service);

        /**
         * @brief Sets the geographic bounding box used for coordinate
         * normalization.
         *
         * The bounding box defines the latitude/longitude range of the map
         * area. All weather points will be normalized into [0,1] coordinate
         * space using this bounding box. The box must be set before calling
         * load().
         *
         * @param box Geographic bounding box for normalization.
         */
        void setBoundingBox(const shared::BoundingBox &box);

        /**
         * @brief Sets the temperature unit used when presenting weather data.
         *
         * Changing the unit triggers a reload of weather data on the next call
         * to load().
         *
         * @param unit Temperature unit (Celsius or Fahrenheit).
         */
        void setTemperatureUnit(shared::TemperatureUnit unit);

        /**
         * @brief Loads weather data for the given forecast horizon.
         *
         * The loading is performed asynchronously. Only value copies are
         * captured into the worker lambda, ensuring no QObject members are
         * accessed from a background thread. When the data is ready,
         * weatherPointsReady() is emitted with a shared_ptr to the resulting
         * WeatherPointItem list. If an error occurs, errorOccurred() is
         * emitted.
         */
        void load(shared::ForecastHorizon horizon);

        /**
         * @brief Returns the most recently loaded weather points.
         *
         * If no data has been loaded yet, an empty vector is returned.
         *
         * @return Reference to the current list of WeatherPointItem objects.
         */
        const std::vector<WeatherPointItem> &points() const {
            static const std::vector<WeatherPointItem> empty;
            return m_points ? *m_points : empty;
        }

        /**
         * @brief Returns the timestamp of the last weather update as a
         * formatted string.
         */
        QString lastUpdated() const { return m_lastUpdated.toString("HH:mm"); }

      signals:
        /**
         * @brief Emitted when new weather point data is available.
         */
        void weatherPointsReady(
            std::shared_ptr<const std::vector<WeatherPointItem>> points);

        /**
         * @brief Emitted when the lastUpdated timestamp changes.
         */
        void lastUpdatedChanged();

        /**
         * @brief Emitted when an error occurs during weather data loading.
         *
         * The message parameter contains a human-readable description of the
         * error, suitable for displaying to the user.
         */
        void errorOccurred(QString message);

      private slots:
        void onWeatherFetchFinished();

      private:
        std::unique_ptr<domain::IWeatherService> m_service;
        shared::BoundingBox m_bbox;
        bool m_hasBBox = false;

        std::shared_ptr<const std::vector<WeatherPointItem>> m_points;

        QTimer m_refreshTimer; // Automatic refresh timer
        QFutureWatcher<
            shared::Result<std::vector<domain::NormalizedWeatherPoint>>>
            m_weatherFutureWatcher; // Background weather fetch watcher
        shared::TemperatureUnit m_lastRequestedUnit =
            shared::TemperatureUnit::Celsius; // Unit used for most recent
                                              // request
        QDateTime m_lastUpdated; // Timestamp of last successful update
        shared::ForecastHorizon m_lastHorizon = shared::ForecastHorizon::Now;
    };

} // namespace viewmodel

Q_DECLARE_METATYPE(
    std::shared_ptr<const std::vector<viewmodel::WeatherPointItem>>)
