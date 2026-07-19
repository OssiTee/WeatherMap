#include <QtTest/QtTest>

#include "domain/cloud/CloudFieldInterpolator.h"

class TestCloudFieldInterpolator : public QObject {
    Q_OBJECT

  private slots:
    void testReturnsEmptyOnInvalidInput();
    void testUniformInputStaysUniform();
    void testInterpolationWeightsNearbyPointsMore();
};

void TestCloudFieldInterpolator::testReturnsEmptyOnInvalidInput() {
    const std::vector<domain::NormalizedCloudPoint> points = {
        {0.5, 0.5, 60.0}};

    auto noPoints = domain::CloudFieldInterpolator::buildWeightedCloudField(
        {}, 8, 8, 0.06);
    QVERIFY(noPoints.empty());

    auto badWidth = domain::CloudFieldInterpolator::buildWeightedCloudField(
        points, 1, 8, 0.06);
    QVERIFY(badWidth.empty());

    auto badSigma = domain::CloudFieldInterpolator::buildWeightedCloudField(
        points, 8, 8, 0.0);
    QVERIFY(badSigma.empty());
}

void TestCloudFieldInterpolator::testUniformInputStaysUniform() {
    const std::vector<domain::NormalizedCloudPoint> points = {
        {0.25, 0.25, 40.0}, {0.75, 0.25, 40.0},
        {0.25, 0.75, 40.0}, {0.75, 0.75, 40.0}};

    const int width = 12;
    const int height = 10;
    auto field = domain::CloudFieldInterpolator::buildWeightedCloudField(
        points, width, height, 0.06);

    QCOMPARE(field.size(), static_cast<size_t>(width * height));

    for (double v : field) {
        QVERIFY(qAbs(v - 40.0) < 1e-6);
    }
}

void TestCloudFieldInterpolator::testInterpolationWeightsNearbyPointsMore() {
    const std::vector<domain::NormalizedCloudPoint> points = {
        {0.1, 0.1, 90.0}, {0.9, 0.9, 10.0}};

    const int width = 11;
    const int height = 11;
    auto field = domain::CloudFieldInterpolator::buildWeightedCloudField(
        points, width, height, 0.06);

    QCOMPARE(field.size(), static_cast<size_t>(width * height));

    const double topLeft = field[0];
    const double bottomRight =
        field[static_cast<size_t>(width * height - 1)];

    QVERIFY(topLeft > bottomRight);
}

QTEST_MAIN(TestCloudFieldInterpolator)
#include "test_cloud_field_interpolator.moc"
