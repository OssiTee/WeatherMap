#include "domain/map/IMapService.h"
#include "domain/map/NormalizedMap.h"
#include "shared/Result.h"
#include "viewmodel/map/MapLoaderViewModel.h"
#include <QtTest/QtTest>

// Stub service that returns predefined NormalizedMap.
class StubMapService : public domain::IMapService {
  public:
    domain::NormalizedMap data; // <-- This is what the test writes into

    shared::Result<domain::NormalizedMap> loadMap() const override {
        return shared::Result<domain::NormalizedMap>::success(data);
    }
};

class TestMapLoaderViewModel : public QObject {
    Q_OBJECT

  private slots:
    void testMapLoading();
};

void TestMapLoaderViewModel::testMapLoading() {
    // Prepare stub service with already normalized polygon data.
    auto service = std::make_unique<StubMapService>();

    // Normalized coordinates for:
    // (60,20) → (0,1)
    // (65,25) → (1,0)
    service->data.polygons = {{{0.0, 1.0}, // normalized x,y
                               {1.0, 0.0}}};

    service->data.bbox.minLat = 60.0;
    service->data.bbox.maxLat = 65.0;
    service->data.bbox.minLon = 20.0;
    service->data.bbox.maxLon = 25.0;

    viewmodel::MapLoaderViewModel vm(std::move(service));

    QSignalSpy spy(&vm, &viewmodel::MapLoaderViewModel::mapShapeReady);

    vm.loadMap();

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() == 1, 500);

    const QList<QVariant> args = spy.takeFirst();

    auto polysPtr =
        args.at(0)
            .value<std::shared_ptr<const std::vector<std::vector<QPointF>>>>();
    QVERIFY(polysPtr != nullptr);

    const auto &polys = *polysPtr;

    QCOMPARE(polys.size(), size_t(1));
    QCOMPARE(polys[0].size(), size_t(2));

    QVERIFY(qAbs(polys[0][0].x() - 0.0) < 1e-6);
    QVERIFY(qAbs(polys[0][0].y() - 1.0) < 1e-6);

    QVERIFY(qAbs(polys[0][1].x() - 1.0) < 1e-6);
    QVERIFY(qAbs(polys[0][1].y() - 0.0) < 1e-6);
}

QTEST_MAIN(TestMapLoaderViewModel)
#include "test_map_loader.moc"
