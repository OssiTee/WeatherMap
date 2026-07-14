#include <QtTest/QtTest>

#include "domain/cloud/CloudCoverageService.h"
#include "domain/cloud/ICloudCoverageRepository.h"

class StubCloudCoverageRepository : public domain::ICloudCoverageRepository {
  public:
    std::vector<std::pair<double, double>> requestedCoords;
    std::vector<RawCloudPoint> points;
    bool returnError = false;

    shared::Result<std::vector<RawCloudPoint>> fetchCloudCoverage(
        const std::vector<std::pair<double, double>> &coords) override {
        requestedCoords = coords;
        if (returnError) {
            return shared::Result<std::vector<RawCloudPoint>>::error(
                "fetch failed");
        }
        return shared::Result<std::vector<RawCloudPoint>>::success(points);
    }
};

class TestCloudCoverageService : public QObject {
    Q_OBJECT

  private slots:
    void testGetCloudForMapSuccess();
    void testGetCloudForMapRejectsInvalidGrid();
    void testGetCloudForMapPropagatesRepositoryError();
};

void TestCloudCoverageService::testGetCloudForMapSuccess() {
    auto repository = std::make_unique<StubCloudCoverageRepository>();
    auto *repositoryRaw = repository.get();
    repositoryRaw->points = {
        {50.0, 20.0, 10.0}, {50.0, 25.0, 20.0}, {50.0, 30.0, 30.0},
        {70.0, 20.0, 40.0}, {70.0, 25.0, 50.0}, {70.0, 30.0, 60.0}};

    domain::CloudCoverageService service(std::move(repository));

    shared::BoundingBox box{50.0, 70.0, 20.0, 30.0};
    auto result = service.getCloudForMap(box, 2, 3);

    QVERIFY(result.isSuccess());
    QCOMPARE(repositoryRaw->requestedCoords.size(), size_t(6));
    QCOMPARE(repositoryRaw->requestedCoords[0].first, 50.0);
    QCOMPARE(repositoryRaw->requestedCoords[0].second, 20.0);
    QCOMPARE(repositoryRaw->requestedCoords[5].first, 70.0);
    QCOMPARE(repositoryRaw->requestedCoords[5].second, 30.0);

    const auto &points = result.value();
    QCOMPARE(points.size(), size_t(6));

    QVERIFY(qAbs(points[0].xNorm - 0.0) < 1e-9);
    QVERIFY(qAbs(points[0].yNorm - 1.0) < 1e-9);
    QCOMPARE(points[0].cloudCoverPercent, 10.0);

    QVERIFY(qAbs(points[5].xNorm - 1.0) < 1e-9);
    QVERIFY(qAbs(points[5].yNorm - 0.0) < 1e-9);
    QCOMPARE(points[5].cloudCoverPercent, 60.0);
}

void TestCloudCoverageService::testGetCloudForMapRejectsInvalidGrid() {
    auto repository = std::make_unique<StubCloudCoverageRepository>();
    domain::CloudCoverageService service(std::move(repository));

    shared::BoundingBox box{50.0, 70.0, 20.0, 30.0};
    auto result = service.getCloudForMap(box, 1, 3);

    QVERIFY(result.isError());
    QCOMPARE(result.errorMessage(), std::string("Cloud sample grid must be at least 2x2"));
}

void TestCloudCoverageService::testGetCloudForMapPropagatesRepositoryError() {
    auto repository = std::make_unique<StubCloudCoverageRepository>();
    auto *repositoryRaw = repository.get();
    repositoryRaw->returnError = true;

    domain::CloudCoverageService service(std::move(repository));

    shared::BoundingBox box{50.0, 70.0, 20.0, 30.0};
    auto result = service.getCloudForMap(box, 2, 2);

    QVERIFY(result.isError());
    QCOMPARE(result.errorMessage(), std::string("fetch failed"));
}

QTEST_MAIN(TestCloudCoverageService)
#include "test_cloud_service.moc"
