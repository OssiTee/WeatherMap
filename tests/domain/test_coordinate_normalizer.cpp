#include "domain/CoordinateNormalizer.h"
#include <QtTest/QtTest>

class TestCoordinateNormalizer : public QObject {
    Q_OBJECT

  private slots:
    void testNormalize();
    void testNormalizePolygons();
    void testNormalize_invalidBBox();
};

void TestCoordinateNormalizer::testNormalize() {
    domain::CoordinateNormalizer service;

    shared::BoundingBox box;
    box.minLat = 50.0;
    box.maxLat = 70.0;
    box.minLon = 20.0;
    box.maxLon = 30.0;

    const auto point = service.normalize(60.0, 25.0, box);

    QVERIFY(qAbs(point.x - 0.5) < 1e-6);
    QVERIFY(qAbs(point.y - 0.5) < 1e-6);
}

void TestCoordinateNormalizer::testNormalizePolygons() {
    domain::CoordinateNormalizer service;

    std::vector<std::vector<domain::LatLon>> polygons = {
        {{50.0, 20.0}, {70.0, 30.0}}};

    shared::BoundingBox box(50.0, 70.0, 20.0, 30.0);
    const auto normalized = service.normalizePolygons(polygons, box);

    QCOMPARE(normalized.size(), size_t(1));
    QCOMPARE(normalized[0].size(), size_t(2));

    QVERIFY(qAbs(normalized[0][0].x - 0.0) < 1e-6);
    QVERIFY(qAbs(normalized[0][0].y - 1.0) < 1e-6);

    QVERIFY(qAbs(normalized[0][1].x - 1.0) < 1e-6);
    QVERIFY(qAbs(normalized[0][1].y - 0.0) < 1e-6);
}

void TestCoordinateNormalizer::testNormalize_invalidBBox() {
    domain::CoordinateNormalizer service;

    shared::BoundingBox box;
    box.minLat = 60.0;
    box.maxLat = 60.0; // invalid
    box.minLon = 20.0;
    box.maxLon = 20.0; // invalid

    const auto p = service.normalize(60.0, 20.0, box);

    QCOMPARE(p.x, 0.0);
    QCOMPARE(p.y, 0.0);
}

QTEST_MAIN(TestCoordinateNormalizer)
#include "test_coordinate_normalizer.moc"
