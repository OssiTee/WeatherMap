#include "domain/city/CityService.h"
#include "domain/city/ICityLoader.h"
#include <QtTest>

using namespace shared;

class FakeCityLoader : public domain::ICityLoader {
  public:
    std::vector<domain::ICityLoader::RawCity> fakeCities;

    Result<std::vector<domain::ICityLoader::RawCity>> loadCities() override {
        return Result<std::vector<domain::ICityLoader::RawCity>>::success(
            fakeCities);
    }
};

class TestCityService : public QObject {
    Q_OBJECT
  private slots:
    void testCityLoading();
};

void TestCityService::testCityLoading() {
    auto loader = std::make_unique<FakeCityLoader>();
    loader->fakeCities = {{"Helsinki", 60.17, 24.94}, {"Oslo", 59.91, 10.75}};

    domain::CityService service(std::move(loader));

    shared::BoundingBox box{60, 65, 20, 25};
    auto result = service.cities(box);

    QVERIFY(result.isSuccess());

    const auto &cities = result.value();
    QCOMPARE(cities.size(), size_t(1));
    QCOMPARE(cities[0].name, std::string("Helsinki"));
    QVERIFY(cities[0].xNorm >= 0.0 && cities[0].xNorm <= 1.0);
    QVERIFY(cities[0].yNorm >= 0.0 && cities[0].yNorm <= 1.0);
}

QTEST_MAIN(TestCityService)
#include "test_city_service.moc"
