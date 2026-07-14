#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

#include <memory>

#include "MapWidget.h"
#include "shared/BoundingBox.h"
#include "viewmodel/city/CityLabelsViewModel.h"
#include "viewmodel/cloud/CloudOverlayViewModel.h"
#include "viewmodel/map/MapLoaderViewModel.h"
#include "viewmodel/weather/WeatherDataViewModel.h"

namespace view {

    /**
     * @class MainWindow
     * @brief Top-level application window coordinating ViewModels and UI.
     *
     * This class owns the MapWidget and the toolbar controls. It receives
     * normalized map geometry, weather data, and city labels from the
     * ViewModels and updates the UI accordingly. The window is implemented as a
     * QWidget instead of QMainWindow to allow the map widget to control the
     * aspect ratio.
     */
    class MainWindow : public QWidget {
        Q_OBJECT

      public:
        /**
         * @brief Constructs the MainWindow with injected ViewModels.
         *
         * @param mapLoaderVM   ViewModel responsible for loading map geometry.
         * @param cityLabelsVM  ViewModel responsible for producing city labels.
         * @param cloudOverlayVM ViewModel responsible for cloud coverage
         *                       overlay.
         * @param weatherVM     ViewModel responsible for loading weather data.
         * @param parent        Optional parent widget.
         */
        explicit MainWindow(
            std::unique_ptr<viewmodel::MapLoaderViewModel> mapLoaderVM,
            std::unique_ptr<viewmodel::CityLabelsViewModel> cityLabelsVM,
            std::unique_ptr<viewmodel::CloudOverlayViewModel> cloudOverlayVM,
            std::unique_ptr<viewmodel::WeatherDataViewModel> weatherVM,
            QWidget *parent = nullptr);

      private slots:
        /**
         * @brief Called when the map geometry and bounding box are ready.
         *
         * Updates the MapWidget, forwards the bounding box to ViewModels,
         * computes the aspect ratio, and resizes the window to match the map.
         */
        void onMapShapeReady(
            std::shared_ptr<const std::vector<std::vector<QPointF>>> polygons,
            const shared::BoundingBox &bbox);

        /**
         * @brief Validates and forwards the bounding box to ViewModels.
         */
        void setBoundingBox(const shared::BoundingBox &box);

      private:
        void refreshWeatherDataImpl(bool forceOverlayRefresh);

        /**
         * @brief Builds the toolbar panel and its UI controls.
         *
         * The toolbar includes:
         *  - Temperature unit selector
         *  - Forecast horizon selector
         *  - Last-updated label
         *  - Refresh button
         *
         * It is placed above the map widget inside a vertical layout.
         */
        void createToolbar();

        /**
         * @brief Connects all ViewModel and UI signals.
         */
        void connectSignals();

        /**
         * @brief Performs initial startup actions (loads map).
         */
        void setupInitialState();

        /**
         * @brief Reloads weather data using the current UI selections.
         */
        void refreshWeatherData();

        // --- ViewModels ---
        std::unique_ptr<viewmodel::MapLoaderViewModel> m_mapLoaderVM;
        std::unique_ptr<viewmodel::CityLabelsViewModel> m_cityLabelsVM;
        std::unique_ptr<viewmodel::CloudOverlayViewModel> m_cloudOverlayVM;
        std::unique_ptr<viewmodel::WeatherDataViewModel> m_weatherVM;

        // --- UI ---
        QWidget *m_toolbarPanel = nullptr;
        MapWidget *m_widget = nullptr;

        // Toolbar controls
        QComboBox *m_unitCombo = nullptr;
        QComboBox *m_horizonCombo = nullptr;
        QCheckBox *m_cloudLayerCheck = nullptr;
        QLabel *m_lastUpdatedLabel = nullptr;
        QPushButton *m_refreshButton = nullptr;

        QTimer m_autoRefreshTimer;
    };

} // namespace view
