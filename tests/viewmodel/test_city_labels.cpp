#include "domain/city/ICityService.h"
#include "viewmodel/city/CityLabelsViewModel.h"
#include <QtTest>

using namespace shared;

namespace {

    class StubCityService : public domain::ICityService {
      public:
        std::vector<domain::NormalizedCity> data;
        bool returnError = false;
        std::string errorMessage;

        shared::Result<std::vector<domain::NormalizedCity>>
        cities(const shared::BoundingBox &) const override {
            if (returnError) {
                return shared::Result<
                    std::vector<domain::NormalizedCity>>::error(errorMessage);
            }

            return shared::Result<std::vector<domain::NormalizedCity>>::success(
                data);
        }
    };

} // namespace

class TestCityLabels : public QObject {
    Q_OBJECT
  private slots:
    void testLabelProjection();
    void testErrorPropagation();
};

void TestCityLabels::testLabelProjection() {
    auto stub = std::make_unique<StubCityService>();
    stub->data = {{"Helsinki", 0.0, 1.0}, {"Oulu", 1.0, 0.0}};

    viewmodel::CityLabelsViewModel vm(std::move(stub));

    // REQUIRED: CityLabelsViewModel refuses to run without a bounding box
    vm.setBoundingBox(shared::BoundingBox{60.0, 65.0, 24.0, 26.0});

    QSignalSpy spy(&vm, &viewmodel::CityLabelsViewModel::labelsReady);

    vm.updateLabels();

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() == 1, 500);

    const auto &labels = vm.labels();
    QCOMPARE(labels.size(), size_t(2));

    QCOMPARE(labels[0].name, QString("Helsinki"));
    QCOMPARE(labels[1].name, QString("Oulu"));

    QVERIFY(labels[0].x >= 0.0 && labels[0].x <= 1.0);
    QVERIFY(labels[0].y >= 0.0 && labels[0].y <= 1.0);
}

void TestCityLabels::testErrorPropagation() {
    auto stub = std::make_unique<StubCityService>();
    stub->returnError = true;
    stub->errorMessage = "CityService failed";

    viewmodel::CityLabelsViewModel vm(std::move(stub));

    // REQUIRED
    vm.setBoundingBox(shared::BoundingBox{0, 1, 0, 1});

    QSignalSpy spy(&vm, &viewmodel::CityLabelsViewModel::errorOccurred);

    vm.updateLabels();

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() == 1, 500);

    QCOMPARE(spy.takeFirst().at(0).toString(), QString("CityService failed"));
}

QTEST_MAIN(TestCityLabels)
#include "test_city_labels.moc"
