#include "domain/weather/IWeatherRepository.h"
#include "domain/weather/WeatherService.h"
#include "shared/ForecastHorizon.h"
#include "shared/Result.h"
#include "shared/TemperatureUnit.h"

#include <QtTest/QtTest>

// -----------------------------------------------------------------------------
// Fake repository for testing WeatherService
// -----------------------------------------------------------------------------
class FakeWeatherRepository : public domain::IWeatherRepository {
  public:
    std::vector<domain::IWeatherRepository::RawWeatherPoint> fakePoints;

    shared::Result<std::vector<domain::IWeatherRepository::RawWeatherPoint>>
    fetchWeather(shared::ForecastHorizon) override {
        return shared::Result<std::vector<
            domain::IWeatherRepository::RawWeatherPoint>>::success(fakePoints);
    }
};

// -----------------------------------------------------------------------------
// Test class
// -----------------------------------------------------------------------------
class TestWeatherService : public QObject {
    Q_OBJECT

  private slots:
    void testCelsiusPassThrough();
    void testFahrenheitConversion();
};

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------
void TestWeatherService::testCelsiusPassThrough() {
    auto repo = std::make_unique<FakeWeatherRepository>();
    repo->fakePoints = {{60.0, 24.0, 10.0, 5.0, 180.0, 0.0, 1}};

    domain::WeatherService service(std::move(repo));

    shared::BoundingBox box{60, 65, 20, 25};
    auto result = service.getWeatherForMap(shared::ForecastHorizon::Now, box);

    QVERIFY(result.isSuccess());
    const auto &points = result.value();

    QCOMPARE(points.size(), size_t(1));
    QCOMPARE(points[0].temperature, 10.0);
}

void TestWeatherService::testFahrenheitConversion() {
    auto repo = std::make_unique<FakeWeatherRepository>();
    repo->fakePoints = {
        {60.0, 24.0, 0.0, 5.0, 180.0, 0.0, 1} // 0°C
    };

    domain::WeatherService service(std::move(repo));
    service.setTemperatureUnit(shared::TemperatureUnit::Fahrenheit);

    shared::BoundingBox box{60, 65, 20, 25};
    auto result = service.getWeatherForMap(shared::ForecastHorizon::Now, box);

    QVERIFY(result.isSuccess());
    const auto &pts = result.value();

    QCOMPARE(pts.size(), size_t(1));

    // 0°C = 32°F
    QVERIFY(qAbs(pts[0].temperature - 32.0) < 1e-6);
}

QTEST_MAIN(TestWeatherService)
#include "test_weather_service.moc"
