#include "domain/map/IMapProvider.h"
#include "domain/map/MapService.h"
#include "shared/Result.h"

#include <QtTest/QtTest>

using namespace domain;

// -----------------------------------------------------------------------------
// Fake provider for testing MapService
// -----------------------------------------------------------------------------
class FakeMapProvider : public IMapProvider {
  public:
    RawMapData fakeShape;
    bool returnError = false;

    shared::Result<RawMapData> loadShape() override {
        if (returnError) {
            return shared::Result<RawMapData>::error("load failed");
        }
        return shared::Result<RawMapData>::success(fakeShape);
    }
};

// -----------------------------------------------------------------------------
// Test class
// -----------------------------------------------------------------------------
class TestMapService : public QObject {
    Q_OBJECT

  private slots:
    void testSuccessfulLoad();
    void testErrorPropagation();
};

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------
void TestMapService::testSuccessfulLoad() {
    auto provider = std::make_unique<FakeMapProvider>();

    // Simple square polygon
    provider->fakeShape.polygons = {
        {{10.0, 20.0}, {20.0, 20.0}, {20.0, 30.0}, {10.0, 30.0}}};

    provider->fakeShape.minLat = 10.0;
    provider->fakeShape.maxLat = 20.0;
    provider->fakeShape.minLon = 20.0;
    provider->fakeShape.maxLon = 30.0;

    MapService service(std::move(provider));

    auto result = service.loadMap();
    QVERIFY(result.isSuccess());

    const auto &data = result.value();

    // Check metadata
    QCOMPARE(data.bbox.minLat, 10.0);
    QCOMPARE(data.bbox.maxLat, 20.0);
    QCOMPARE(data.bbox.minLon, 20.0);
    QCOMPARE(data.bbox.maxLon, 30.0);

    // Check polygon count
    QCOMPARE(data.polygons.size(), size_t(1));
    QCOMPARE(data.polygons[0].size(), size_t(4));

    // Check normalization: first point (10,20) → (0,1)
    QVERIFY(qAbs(data.polygons[0][0].x - 0.0) < 1e-9);
    QVERIFY(qAbs(data.polygons[0][0].y - 1.0) < 1e-9);

    // Check normalization: second point (20,20) → (0,0)
    QVERIFY(qAbs(data.polygons[0][1].x - 0.0) < 1e-9);
    QVERIFY(qAbs(data.polygons[0][1].y - 0.0) < 1e-9);
}

void TestMapService::testErrorPropagation() {
    auto provider = std::make_unique<FakeMapProvider>();
    provider->returnError = true;

    MapService service(std::move(provider));

    auto result = service.loadMap();
    QVERIFY(result.isError());
    QCOMPARE(result.errorMessage(), std::string("load failed"));
}

QTEST_MAIN(TestMapService)
#include "test_map_service.moc"
