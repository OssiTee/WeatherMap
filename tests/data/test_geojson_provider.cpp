#include "data/map/GeoJsonFileProvider.h"
#include <QtTest/QtTest>

class TestGeoJsonFileProvider : public QObject {
    Q_OBJECT

  private slots:
    void testLoadShapeFromResource();
};

void TestGeoJsonFileProvider::testLoadShapeFromResource() {
    data::GeoJsonFileProvider provider(":/map/fin.geojson");

    auto result = provider.loadShape();
    QVERIFY(result.isSuccess());

    auto &loadResult = result.value();
    QVERIFY(!loadResult.polygons.empty());
    QVERIFY(loadResult.minLat < loadResult.maxLat);
    QVERIFY(loadResult.minLon < loadResult.maxLon);
}

QTEST_MAIN(TestGeoJsonFileProvider)
#include "test_geojson_provider.moc"
