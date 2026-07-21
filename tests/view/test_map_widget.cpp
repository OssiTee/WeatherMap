#include "view/MapWidget.h"
#include <QApplication>
#include <QtTest/QtTest>

class TestMapWidget : public QObject {
    Q_OBJECT

  private slots:
    void testSettersDoNotCrash();
};

void TestMapWidget::testSettersDoNotCrash() {
    view::MapWidget widget;
    auto mapShape = std::make_shared<const std::vector<std::vector<QPointF>>>(
        std::vector<std::vector<QPointF>>{
            {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}}});
    widget.setMapShape(std::move(mapShape));

    viewmodel::CityLabelItem cityItem;
    cityItem.name = "Test";
    cityItem.x = 0.5;
    cityItem.y = 0.5;
    auto cityLabels =
        std::make_shared<const std::vector<viewmodel::CityLabelItem>>(
            std::vector<viewmodel::CityLabelItem>{cityItem});
    widget.setCityLabels(std::move(cityLabels));

    viewmodel::WeatherPointItem weatherItem;
    weatherItem.xNorm = 0.25;
    weatherItem.yNorm = 0.75;
    auto weatherPoints =
        std::make_shared<const std::vector<viewmodel::WeatherPointItem>>(
            std::vector<viewmodel::WeatherPointItem>{weatherItem});
    widget.setWeatherPoints(std::move(weatherPoints));

    widget.setCloudOverlayLoading(true);
    widget.setCloudOverlayLoading(false);

    QVERIFY(widget.width() >= 0);
    QVERIFY(widget.height() >= 0);
}

QTEST_MAIN(TestMapWidget)
#include "test_map_widget.moc"
