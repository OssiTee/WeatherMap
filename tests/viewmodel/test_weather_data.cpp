#include <QtTest/QtTest>

#include "domain/weather/IWeatherService.h"
#include "shared/ForecastHorizon.h"
#include "shared/Result.h"
#include "shared/TemperatureUnit.h"
#include "viewmodel/weather/WeatherDataViewModel.h"

// -----------------------------------------------------------------------------
// Stub service used by the ViewModel tests
// Returns pre-defined NormalizedWeatherPoint data as-is.
// Temperature conversion is the responsibility of the real WeatherService
// and is tested separately in test_weather_service.cpp.
// -----------------------------------------------------------------------------
class StubWeatherService : public domain::IWeatherService {
  public:
    std::vector<domain::NormalizedWeatherPoint> data;
    bool usedExplicitUnitFetch = false;
    shared::TemperatureUnit explicitUnit = shared::TemperatureUnit::Celsius;

    shared::Result<std::vector<domain::NormalizedWeatherPoint>>
    getWeatherForMap(shared::ForecastHorizon, const shared::BoundingBox &,
                     shared::TemperatureUnit requestedUnit) override {
        usedExplicitUnitFetch = true;
        explicitUnit = requestedUnit;
        return shared::Result<
            std::vector<domain::NormalizedWeatherPoint>>::success(data);
    }
};

// -----------------------------------------------------------------------------
// Test class for WeatherDataViewModel
// -----------------------------------------------------------------------------
class TestWeatherDataViewModel : public QObject {
    Q_OBJECT

  private slots:
    void testLoadSuccess();
    void testTemperatureUnitConversion();
};

// -----------------------------------------------------------------------------
// Test: Successful loading of weather data
// -----------------------------------------------------------------------------
void TestWeatherDataViewModel::testLoadSuccess() {
    auto service = std::make_unique<StubWeatherService>();

    // Provide one normalized weather point
    service->data = {
        {0.5, 0.5, 10.0, 5.0, 180.0, 1} // xNorm, yNorm, temp, wind, dir, symbol
    };

    viewmodel::WeatherDataViewModel vm(std::move(service));

    // Set bounding box for the ViewModel
    shared::BoundingBox box{0, 1, 0, 1};
    vm.setBoundingBox(box);

    QSignalSpy spy(&vm, &viewmodel::WeatherDataViewModel::weatherPointsReady);

    vm.load(shared::ForecastHorizon::Now);

    // Wait for async completion
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() == 1, 500);

    const auto &pts = vm.points();
    QCOMPARE(pts.size(), size_t(1));
    QCOMPARE(pts[0].temperature, 10.0);
}

// -----------------------------------------------------------------------------
// Test: ViewModel forwards temperature unit to the service
// -----------------------------------------------------------------------------
void TestWeatherDataViewModel::testTemperatureUnitConversion() {
    auto service = std::make_unique<StubWeatherService>();
    auto *serviceRaw = service.get();
    service->data = {{0.5, 0.5, 10.0, 5.0, 180.0, 1}};

    viewmodel::WeatherDataViewModel vm(std::move(service));

    shared::BoundingBox box{0, 1, 0, 1};
    vm.setBoundingBox(box);
    vm.setTemperatureUnit(shared::TemperatureUnit::Fahrenheit);

    QSignalSpy spy(&vm, &viewmodel::WeatherDataViewModel::weatherPointsReady);
    vm.load(shared::ForecastHorizon::Now);

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() == 1, 500);

    // The ViewModel must have forwarded the unit selection to the service.
    QVERIFY(serviceRaw->usedExplicitUnitFetch);
    QCOMPARE(serviceRaw->explicitUnit, shared::TemperatureUnit::Fahrenheit);

    // The stub returns data unchanged — ViewModel must not modify temperatures.
    const auto &pts = vm.points();
    QCOMPARE(pts.size(), size_t(1));
    QCOMPARE(pts[0].temperature, 10.0);
}

QTEST_MAIN(TestWeatherDataViewModel)
#include "test_weather_data.moc"
