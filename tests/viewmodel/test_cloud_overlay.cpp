#include <QtTest/QtTest>

#include "domain/cloud/ICloudCoverageService.h"
#include "shared/ForecastHorizon.h"
#include "viewmodel/cloud/CloudOverlayViewModel.h"

class StubCloudCoverageService : public domain::ICloudCoverageService {
  public:
    std::vector<domain::NormalizedCloudPoint> points;
    mutable int callCount = 0;

    shared::Result<std::vector<domain::NormalizedCloudPoint>> getCloudForMap(
        const shared::BoundingBox &, int, int) override {
        ++callCount;
        return shared::Result<std::vector<domain::NormalizedCloudPoint>>::
            success(points);
    }
};

class TestCloudOverlayViewModel : public QObject {
    Q_OBJECT

  private slots:
    void testLoadWithoutBoundingBoxDoesNothing();
    void testLoadSuccess();
    void testUsesCacheWhenNotForced();
    void testForceRefreshBypassesCache();
};

void TestCloudOverlayViewModel::testLoadWithoutBoundingBoxDoesNothing() {
    auto stub = std::make_unique<StubCloudCoverageService>();
    auto *stubRaw = stub.get();
    stubRaw->points = {{0.5, 0.5, 75.0}};

    viewmodel::CloudOverlayViewModel vm(std::move(stub));

    QSignalSpy readySpy(&vm,
                        &viewmodel::CloudOverlayViewModel::overlayImageReady);

    vm.load();

    QTest::qWait(50);
    QCOMPARE(readySpy.count(), 0);
    QCOMPARE(stubRaw->callCount, 0);
}

void TestCloudOverlayViewModel::testLoadSuccess() {
    auto stub = std::make_unique<StubCloudCoverageService>();
    auto *stubRaw = stub.get();
    stubRaw->points = {{0.4, 0.4, 80.0}, {0.7, 0.6, 45.0}};

    viewmodel::CloudOverlayViewModel vm(std::move(stub));
    vm.setBoundingBox(shared::BoundingBox{59.0, 70.0, 19.0, 32.0});

    QSignalSpy readySpy(&vm,
                        &viewmodel::CloudOverlayViewModel::overlayImageReady);

    vm.load();

    QTRY_VERIFY_WITH_TIMEOUT(readySpy.count() >= 1, 1000);
    QCOMPARE(stubRaw->callCount, 1);

    const auto args = readySpy.takeFirst();
    QVERIFY(args.size() == 1);
    const QImage image = qvariant_cast<QImage>(args.at(0));
    QVERIFY(!image.isNull());
}

void TestCloudOverlayViewModel::testUsesCacheWhenNotForced() {
    auto stub = std::make_unique<StubCloudCoverageService>();
    auto *stubRaw = stub.get();
    stubRaw->points = {{0.4, 0.4, 60.0}};

    viewmodel::CloudOverlayViewModel vm(std::move(stub));
    vm.setBoundingBox(shared::BoundingBox{59.0, 70.0, 19.0, 32.0});

    QSignalSpy readySpy(&vm,
                        &viewmodel::CloudOverlayViewModel::overlayImageReady);

    vm.load();
    QTRY_VERIFY_WITH_TIMEOUT(readySpy.count() >= 1, 1500);
    QCOMPARE(stubRaw->callCount, 1);

    vm.load();
    QTRY_VERIFY_WITH_TIMEOUT(readySpy.count() >= 2, 500);
    QCOMPARE(stubRaw->callCount, 1);
}

void TestCloudOverlayViewModel::testForceRefreshBypassesCache() {
    auto stub = std::make_unique<StubCloudCoverageService>();
    auto *stubRaw = stub.get();
    stubRaw->points = {{0.5, 0.5, 90.0}};

    viewmodel::CloudOverlayViewModel vm(std::move(stub));
    vm.setBoundingBox(shared::BoundingBox{59.0, 70.0, 19.0, 32.0});

    QSignalSpy readySpy(&vm,
                        &viewmodel::CloudOverlayViewModel::overlayImageReady);

    vm.load();
    QTRY_VERIFY_WITH_TIMEOUT(readySpy.count() >= 1, 1500);
    QCOMPARE(stubRaw->callCount, 1);

    vm.load(true);
    QTRY_VERIFY_WITH_TIMEOUT(readySpy.count() >= 2, 2000);

    QCOMPARE(stubRaw->callCount, 2);
}

QTEST_MAIN(TestCloudOverlayViewModel)
#include "test_cloud_overlay.moc"
