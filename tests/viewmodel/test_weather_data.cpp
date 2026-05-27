#include <QtTest/QtTest>

#include "domain/weather/IWeatherRepository.h"
#include "domain/weather/IWeatherService.h"
#include "domain/weather/WeatherService.h"
#include "shared/ForecastHorizon.h"
#include "shared/Result.h"
#include "shared/TemperatureUnit.h"
#include "viewmodel/weather/WeatherDataViewModel.h"

// -----------------------------------------------------------------------------
// Stub service used by the ViewModel tests
// This simulates the domain::IWeatherService interface.
// It returns pre-defined NormalizedWeatherPoint data and performs
// temperature conversion exactly like the real WeatherService.
// -----------------------------------------------------------------------------
class StubWeatherService : public domain::IWeatherService {
  public:
    // Test fills this vector directly
    std::vector<domain::NormalizedWeatherPoint> data;

    // Internal temperature unit state
    shared::TemperatureUnit unit = shared::TemperatureUnit::Celsius;

    // Set the temperature unit (Celsius or Fahrenheit)
    void setTemperatureUnit(shared::TemperatureUnit u) override { unit = u; }

    // Return the currently active temperature unit
    shared::TemperatureUnit temperatureUnit() const override { return unit; }

    // Return weather data for the given horizon and bounding box.
    // This stub performs temperature conversion exactly like the real service.
    shared::Result<std::vector<domain::NormalizedWeatherPoint>>
    getWeatherForMap(shared::ForecastHorizon,
                     const shared::BoundingBox &) override {
        // Make a copy so we can modify it
        auto out = data;

        // Apply Fahrenheit conversion if needed
        if (unit == shared::TemperatureUnit::Fahrenheit) {
            for (auto &p : out) {
                p.temperature = p.temperature * 9.0 / 5.0 + 32.0;
            }
        }

        return shared::Result<std::vector<domain::NormalizedWeatherPoint>>::
            success(std::move(out));
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
// Test: Temperature unit conversion (Celsius → Fahrenheit)
// -----------------------------------------------------------------------------
void TestWeatherDataViewModel::testTemperatureUnitConversion() {
    auto service = std::make_unique<StubWeatherService>();

    // Provide Celsius temperature (10°C)
    service->data = {{0.5, 0.5, 10.0, 5.0, 180.0, 1}};

    viewmodel::WeatherDataViewModel vm(std::move(service));

    shared::BoundingBox box{0, 1, 0, 1};
    vm.setBoundingBox(box);

    // Request Fahrenheit output
    vm.setTemperatureUnit(shared::TemperatureUnit::Fahrenheit);

    QSignalSpy spy(&vm, &viewmodel::WeatherDataViewModel::weatherPointsReady);

    vm.load(shared::ForecastHorizon::Now);

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() == 1, 500);

    const auto &pts = vm.points();
    QCOMPARE(pts.size(), size_t(1));

    // 10°C → 50°F
    QVERIFY(qAbs(pts[0].temperature - 50.0) < 1e-6);
}

QTEST_MAIN(TestWeatherDataViewModel)
#include "test_weather_data.moc"
