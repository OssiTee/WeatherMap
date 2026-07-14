#include <QtTest/QtTest>

#include "data/cloud/CloudCoverageRepository.h"
#include "data/weather/NetworkClient.h"

class StubNetworkClient : public data::NetworkClient {
  public:
    mutable QString lastUrl;
    mutable int callCount = 0;
    QByteArray response;

    QByteArray get(const QString &url) const override {
        ++callCount;
        lastUrl = url;
        return response;
    }
};

class TestCloudCoverageRepository : public QObject {
    Q_OBJECT

  private slots:
    void testFetchCloudCoverageSuccess();
    void testFetchCloudCoverageWithEmptyCoordsFails();
    void testFetchCloudCoverageWithBadXmlFails();
};

static QByteArray makeCloudXml(const QString &time1, double value1,
                               const QString &time2, double value2) {
    return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                      "<root>"
                      "<BsWfsElement>"
                      "<Time>") +
           time1.toUtf8() +
           QByteArray("</Time>"
                      "<ParameterName>TotalCloudCover</ParameterName>"
                      "<ParameterValue>") +
           QByteArray::number(value1) +
           QByteArray("</ParameterValue>"
                      "</BsWfsElement>"
                      "<BsWfsElement>"
                      "<Time>") +
           time2.toUtf8() +
           QByteArray("</Time>"
                      "<ParameterName>TotalCloudCover</ParameterName>"
                      "<ParameterValue>") +
           QByteArray::number(value2) +
           QByteArray("</ParameterValue>"
                      "</BsWfsElement>"
                      "</root>");
}

void TestCloudCoverageRepository::testFetchCloudCoverageSuccess() {
    auto client = std::make_unique<StubNetworkClient>();
    auto *clientRaw = client.get();

    const auto now = QDateTime::currentDateTimeUtc();
    const QString currentTime = now.toString(Qt::ISODate);
    const QString laterTime = now.addSecs(7200).toString(Qt::ISODate);
    clientRaw->response = makeCloudXml(currentTime, 37.5, laterTime, 12.0);

    data::CloudCoverageRepository repository(std::move(client));

    const std::vector<std::pair<double, double>> coords = {{61.1, 25.2}};
    auto result = repository.fetchCloudCoverage(coords);

    QVERIFY(result.isSuccess());
    QCOMPARE(clientRaw->callCount, 1);
    QVERIFY(clientRaw->lastUrl.contains("parameters=TotalCloudCover"));
    QVERIFY(clientRaw->lastUrl.contains("latlon=61.10000,25.20000"));

    const auto &points = result.value();
    QCOMPARE(points.size(), size_t(1));
    QCOMPARE(points[0].latitude, 61.1);
    QCOMPARE(points[0].longitude, 25.2);
    QCOMPARE(points[0].cloudCoverPercent, 37.5);
}

void TestCloudCoverageRepository::testFetchCloudCoverageWithEmptyCoordsFails() {
    auto client = std::make_unique<StubNetworkClient>();
    auto *clientRaw = client.get();

    data::CloudCoverageRepository repository(std::move(client));
    auto result = repository.fetchCloudCoverage({});

    QVERIFY(result.isError());
    QCOMPARE(clientRaw->callCount, 0);
    QCOMPARE(result.errorMessage(), std::string("Cloud coordinate list is empty"));
}

void TestCloudCoverageRepository::testFetchCloudCoverageWithBadXmlFails() {
    auto client = std::make_unique<StubNetworkClient>();
    auto *clientRaw = client.get();
    clientRaw->response = QByteArray("<root><broken></root>");

    data::CloudCoverageRepository repository(std::move(client));
    auto result = repository.fetchCloudCoverage({{61.1, 25.2}});

    QVERIFY(result.isError());
    QCOMPARE(clientRaw->callCount, 1);
    QCOMPARE(result.errorMessage(), std::string("Failed to fetch cloud coverage data (no usable points)"));
}

QTEST_MAIN(TestCloudCoverageRepository)
#include "test_cloud_coverage_repository.moc"
