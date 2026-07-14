#include "data/city/CityLoader.h"
#include "data/cloud/CloudCoverageRepository.h"
#include "data/map/GeoJsonFileProvider.h"
#include "data/weather/NetworkClient.h"
#include "data/weather/WeatherRepository.h"
#include "domain/city/CityService.h"
#include "domain/cloud/CloudCoverageService.h"
#include "domain/map/MapService.h"
#include "domain/weather/WeatherService.h"
#include "view/MainWindow.h"
#include "viewmodel/city/CityLabelsViewModel.h"
#include "viewmodel/cloud/CloudOverlayViewModel.h"
#include "viewmodel/map/MapLoaderViewModel.h"
#include "viewmodel/weather/WeatherDataViewModel.h"
#include <QApplication>
#include <memory>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    // --- Services ---
    SPDLOG_INFO("Initializing services...");

    // Map loader
    auto mapLoaderVM = std::make_unique<viewmodel::MapLoaderViewModel>(
        std::make_unique<domain::MapService>(
            std::make_unique<data::GeoJsonFileProvider>(":/map/fin.geojson")));

    // City labels
    auto cityLabelsVM = std::make_unique<viewmodel::CityLabelsViewModel>(
        std::make_unique<domain::CityService>(
            std::make_unique<data::CityLoader>(":/cities/cities.json")));

    // Weather
    auto weatherVM = std::make_unique<viewmodel::WeatherDataViewModel>(
        std::make_unique<domain::WeatherService>(
            std::make_unique<data::WeatherRepository>(
                ":/queries/fmi_request.txt", ":/queries/weather_coords.txt")));

    // Cloud coverage image overlay
    auto cloudOverlayVM = std::make_unique<viewmodel::CloudOverlayViewModel>(
        std::make_unique<domain::CloudCoverageService>(
            std::make_unique<data::CloudCoverageRepository>(
                std::make_unique<data::NetworkClient>())));

    // --- MainWindow ---
    SPDLOG_INFO("Creating main window...");
    view::MainWindow window(std::move(mapLoaderVM), std::move(cityLabelsVM),
                            std::move(cloudOverlayVM), std::move(weatherVM));

    SPDLOG_INFO("Application started successfully");
    window.show();

    int result = app.exec();
    SPDLOG_INFO("Application exiting with code {}", result);
    return result;
}
