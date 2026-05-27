#include "data/city/CityLoader.h"
#include <QtTest>

class TestCityLoader : public QObject {
    Q_OBJECT
  private slots:
    void testLoadCitiesFromJson();
};

void TestCityLoader::testLoadCitiesFromJson() {
    // JSON that matches CityLoader's expected structure
    QByteArray json = R"(
    {
        "cities": [
            { "name": "Testville", "lat": 60.0, "lon": 25.0 },
            { "name": "Sampletown", "lat": 61.0, "lon": 26.0 }
        ]
    }
    )";

    // Create a real temporary file (Qt6: open() without flags)
    QTemporaryFile file;
    QVERIFY(file.open());

    // Write JSON into the temporary file
    file.write(json);
    file.flush();
    file.close(); // CityLoader will reopen it

    // Load using CityLoader
    data::CityLoader loader(file.fileName());
    auto result = loader.loadCities();

    QVERIFY(result.isSuccess());

    const auto &cities = result.value();
    QCOMPARE(cities.size(), size_t(2));
    QCOMPARE(QString::fromStdString(cities[0].name), QString("Testville"));
    QCOMPARE(QString::fromStdString(cities[1].name), QString("Sampletown"));
}

QTEST_MAIN(TestCityLoader)
#include "test_city_loader.moc"
