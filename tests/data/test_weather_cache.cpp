#include "data/weather/WeatherCache.h"
#include <QtTest/QtTest>

class TestWeatherCache : public QObject {
    Q_OBJECT

  private slots:
    void testCachePutAndGet();
    void testFreshness();
};

void TestWeatherCache::testCachePutAndGet() {
    data::WeatherCache cache;

    std::vector<domain::IWeatherRepository::RawWeatherPoint> pts = {
    {61.0, 25.0, 5.0, 3.0, 180.0, 0.0, 1}};

    QDateTime now = QDateTime::currentDateTimeUtc();
    cache.put(shared::ForecastHorizon::Now, pts, now);

    auto out = cache.get(shared::ForecastHorizon::Now);
    QCOMPARE(out.size(), size_t(1));
    QCOMPARE(out[0].temperature, 5.0);
}

void TestWeatherCache::testFreshness() {
    data::WeatherCache cache;

    std::vector<domain::IWeatherRepository::RawWeatherPoint> pts = {
    {61.0, 25.0, 5.0, 3.0, 180.0, 0.0, 1}};

    QDateTime now = QDateTime::currentDateTimeUtc();
    cache.put(shared::ForecastHorizon::Now, pts, now);

    // Fresh data → hasFresh = true
    QVERIFY(cache.hasFresh(shared::ForecastHorizon::Now, now));

    QDateTime old = now.addSecs(-600 - 1); // 10 min + 1s

    QVERIFY(!cache.hasFresh(shared::ForecastHorizon::Now, old));
}

QTEST_MAIN(TestWeatherCache)
#include "test_weather_cache.moc"
