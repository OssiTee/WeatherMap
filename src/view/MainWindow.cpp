#include "view/MainWindow.h"
#include <QGridLayout>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QVBoxLayout>
#include <spdlog/spdlog.h>

namespace view {

    namespace {
        // 10 minute refresh timer for periodic weather and cloud updates.
        constexpr int AUTO_REFRESH_MS = 10 * 60 * 1000;
    }

    MainWindow::MainWindow(
        std::unique_ptr<viewmodel::MapLoaderViewModel> mapLoaderVM,
        std::unique_ptr<viewmodel::CityLabelsViewModel> cityLabelsVM,
        std::unique_ptr<viewmodel::CloudOverlayViewModel> cloudOverlayVM,
        std::unique_ptr<viewmodel::WeatherDataViewModel> weatherVM,
        QWidget *parent)
        : QWidget(parent), m_mapLoaderVM(std::move(mapLoaderVM)),
          m_cityLabelsVM(std::move(cityLabelsVM)),
          m_cloudOverlayVM(std::move(cloudOverlayVM)),
          m_weatherVM(std::move(weatherVM)) {
        m_widget = new MapWidget(this);

        QVBoxLayout *v = new QVBoxLayout(this);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(0);

        createToolbar();
        v->addWidget(m_toolbarPanel);

        v->addWidget(m_widget);

        connectSignals();

        connect(&m_autoRefreshTimer, &QTimer::timeout, this,
            [this]() { refreshWeatherDataImpl(true); });
        m_autoRefreshTimer.start(AUTO_REFRESH_MS);

        setupInitialState();
    }

    /**
     * @brief Creates the toolbar panel and its UI controls.
     */
    void MainWindow::createToolbar() {
        m_toolbarPanel = new QWidget(this);
        m_toolbarPanel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        QGridLayout *layout = new QGridLayout(m_toolbarPanel);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setHorizontalSpacing(10);
        layout->setVerticalSpacing(4);

        // Row 0: Unit + Horizon
        m_unitCombo = new QComboBox(this);
        m_unitCombo->addItem(
            "Celsius", static_cast<int>(shared::TemperatureUnit::Celsius));
        m_unitCombo->addItem(
            "Fahrenheit",
            static_cast<int>(shared::TemperatureUnit::Fahrenheit));
        m_unitCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        layout->addWidget(m_unitCombo, 0, 0);

        m_horizonCombo = new QComboBox(this);
        m_horizonCombo->addItem("Now",
                                static_cast<int>(shared::ForecastHorizon::Now));
        m_horizonCombo->addItem(
            "+6h", static_cast<int>(shared::ForecastHorizon::Plus6h));
        m_horizonCombo->addItem(
            "+12h", static_cast<int>(shared::ForecastHorizon::Plus12h));
        m_horizonCombo->addItem(
            "+24h", static_cast<int>(shared::ForecastHorizon::Plus24h));
        m_horizonCombo->setSizePolicy(QSizePolicy::Preferred,
                                      QSizePolicy::Fixed);

        layout->addWidget(m_horizonCombo, 0, 1);

        m_cloudLayerCheck = new QCheckBox("Cloud layer", this);
        m_cloudLayerCheck->setChecked(true);
        layout->addWidget(m_cloudLayerCheck, 0, 2);

        // Row 1: Refresh + Last updated
        m_refreshButton = new QPushButton("Refresh", this);
        m_refreshButton->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Fixed);
        layout->addWidget(m_refreshButton, 1, 0);

        m_lastUpdatedLabel = new QLabel("Last updated: --:--", this);
        layout->addWidget(m_lastUpdatedLabel, 1, 1);
    }

    /**
     * @brief Connects all ViewModel and UI signals.
     */
    void MainWindow::connectSignals() {
        connect(m_mapLoaderVM.get(),
                &viewmodel::MapLoaderViewModel::mapShapeReady, this,
                &MainWindow::onMapShapeReady);

        connect(
            m_weatherVM.get(),
            &viewmodel::WeatherDataViewModel::weatherPointsReady, this,
            [this](
                std::shared_ptr<const std::vector<viewmodel::WeatherPointItem>>
                    points) { m_widget->setWeatherPoints(std::move(points)); });

        connect(
            m_cityLabelsVM.get(), &viewmodel::CityLabelsViewModel::labelsReady,
            this,
            [this](std::shared_ptr<const std::vector<viewmodel::CityLabelItem>>
                       labels) { m_widget->setCityLabels(std::move(labels)); });

        connect(m_cloudOverlayVM.get(),
                &viewmodel::CloudOverlayViewModel::overlayImageReady, this,
                [this](const QImage &image) {
                    m_widget->setCloudOverlayImage(image);
                });

        connect(m_cloudOverlayVM.get(),
                &viewmodel::CloudOverlayViewModel::loadingChanged, this,
                [this](bool loading) {
                    m_widget->setCloudOverlayLoading(loading);
                });

        connect(m_weatherVM.get(),
                &viewmodel::WeatherDataViewModel::errorOccurred, this,
                [this](const QString &msg) {
                    QMessageBox::warning(this, "Weather error", msg);
                });

        connect(m_cityLabelsVM.get(),
                &viewmodel::CityLabelsViewModel::errorOccurred, this,
                [this](const QString &msg) {
                    QMessageBox::warning(this, "City data error", msg);
                });

        connect(m_mapLoaderVM.get(),
                &viewmodel::MapLoaderViewModel::errorOccurred, this,
                [this](const QString &msg) {
                    QMessageBox::warning(this, "Map loading error", msg);
                });

        connect(m_cloudOverlayVM.get(),
                &viewmodel::CloudOverlayViewModel::errorOccurred, this,
                [this](const QString &msg) {
                    QMessageBox::warning(this, "Cloud overlay error", msg);
                });

        // Toolbar interactions
        connect(m_refreshButton, &QPushButton::clicked, this,
            [this]() { refreshWeatherDataImpl(true); });

        connect(m_unitCombo, &QComboBox::currentIndexChanged, this,
                &MainWindow::refreshWeatherData);

        connect(m_horizonCombo, &QComboBox::currentIndexChanged, this,
                &MainWindow::refreshWeatherData);

        connect(m_cloudLayerCheck, &QCheckBox::toggled, this,
                [this](bool checked) {
                    m_widget->setCloudOverlayVisible(checked);
                    if (checked) {
                        m_cloudOverlayVM->load();
                    }
                });

        connect(m_weatherVM.get(),
                &viewmodel::WeatherDataViewModel::lastUpdatedChanged, this,
                [this]() {
                    m_lastUpdatedLabel->setText("Last updated: " +
                                                m_weatherVM->lastUpdated());
                });
    }

    /**
     * @brief Performs initial startup actions.
     */
    void MainWindow::setupInitialState() { m_mapLoaderVM->loadMap(); }

    /**
     * @brief Called when map geometry and bounding box are ready.
     */
    void MainWindow::onMapShapeReady(
        std::shared_ptr<const std::vector<std::vector<QPointF>>> polys,
        const shared::BoundingBox &bbox) {
        m_widget->setMapShape(std::move(polys));

        setBoundingBox(bbox);

        double latCenter = (bbox.minLat + bbox.maxLat) * 0.5;
        // Cosine correction: longitude distances shrink toward the poles
        double widthDeg =
            (bbox.maxLon - bbox.minLon) * std::cos(latCenter * M_PI / 180.0);
        // Latitude span is linear, no correction needed
        double heightDeg = (bbox.maxLat - bbox.minLat);

        double aspect = (heightDeg > 0) ? widthDeg / heightDeg : 1.0;
        m_widget->setMapAspect(aspect);

        m_cityLabelsVM->updateLabels();
        m_cloudOverlayVM->load();
        m_weatherVM->load(shared::ForecastHorizon::Now);

        // Resize window to match map aspect ratio
        resize(m_widget->sizeHint().width(),
               m_widget->sizeHint().height() +
                   m_toolbarPanel->sizeHint().height());
    }

    /**
     * @brief Validates and forwards bounding box to ViewModels.
     */
    void MainWindow::setBoundingBox(const shared::BoundingBox &box) {
        if (box.minLat >= box.maxLat || box.minLon >= box.maxLon) {
            SPDLOG_WARN("Invalid bounding box received");
            return;
        }

        m_weatherVM->setBoundingBox(box);
        m_cityLabelsVM->setBoundingBox(box);
        m_cloudOverlayVM->setBoundingBox(box);
    }

    /**
     * @brief Reloads weather data using current UI selections.
     */
    void MainWindow::refreshWeatherData() {
        refreshWeatherDataImpl(false);
    }

    void MainWindow::refreshWeatherDataImpl(bool forceOverlayRefresh) {
        auto unit = static_cast<shared::TemperatureUnit>(
            m_unitCombo->currentData().toInt());
        m_weatherVM->setTemperatureUnit(unit);

        auto horizon = static_cast<shared::ForecastHorizon>(
            m_horizonCombo->currentData().toInt());

        const bool cloudAllowed = (horizon == shared::ForecastHorizon::Now);
        if (!cloudAllowed) {
            // Cloud layer is only available for current-time view.
            m_cloudLayerCheck->setChecked(false);
            m_cloudLayerCheck->setEnabled(false);
            m_widget->setCloudOverlayVisible(false);
            m_widget->setCloudOverlayLoading(false);
        } else {
            m_cloudLayerCheck->setEnabled(true);
        }

        if (cloudAllowed && m_cloudLayerCheck->isChecked()) {
            m_cloudOverlayVM->load(forceOverlayRefresh);
        }
        m_weatherVM->load(horizon);
    }

} // namespace view
