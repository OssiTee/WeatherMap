#include "data/weather/FmiXmlParser.h"
#include <QtTest/QtTest>

class TestFmiXmlParser : public QObject {
    Q_OBJECT

  private slots:
    void testParseSimpleXml();
    void testIncompleteSampleIgnored();
    void testSelectClosestSample();
};

void TestFmiXmlParser::testParseSimpleXml() {
    const QByteArray xml = R"(
    <root>
        <BsWfsElement>
            <Time>2024-01-01T12:00:00Z</Time>
            <ParameterName>Temperature</ParameterName>
            <ParameterValue>5.0</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T12:00:00Z</Time>
            <ParameterName>WindSpeedMS</ParameterName>
            <ParameterValue>3.2</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T12:00:00Z</Time>
            <ParameterName>WindDirection</ParameterName>
            <ParameterValue>180</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T12:00:00Z</Time>
            <ParameterName>Precipitation1h</ParameterName>
            <ParameterValue>-1.0</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T12:00:00Z</Time>
            <ParameterName>WeatherSymbol3</ParameterName>
            <ParameterValue>3</ParameterValue>
        </BsWfsElement>
    </root>
    )";

    data::FmiXmlParser parser;
    auto result = parser.parse(
        xml, 61.0, 25.0,
        QDateTime::fromString("2024-01-01T12:00:00Z", Qt::ISODate));

    QVERIFY(result.has_value());
    QCOMPARE(result->temperature, 5.0);
    QCOMPARE(result->windSpeedMs, 3.2);
    QCOMPARE(result->windDirectionDeg, 180.0);
    QCOMPARE(result->precipitationMm, 0.0);
    QCOMPARE(result->symbolCode, 3);
}

void TestFmiXmlParser::testIncompleteSampleIgnored() {
    const QByteArray xml = R"(
    <root>
        <BsWfsElement>
            <Time>2024-01-01T12:00:00Z</Time>
            <ParameterName>Temperature</ParameterName>
            <ParameterValue>5.0</ParameterValue>
        </BsWfsElement>
    </root>
    )";

    data::FmiXmlParser parser;
    auto result = parser.parse(
        xml, 61.0, 25.0,
        QDateTime::fromString("2024-01-01T12:00:00Z", Qt::ISODate));

    QVERIFY(!result.has_value());
}

void TestFmiXmlParser::testSelectClosestSample() {
    const QByteArray xml = R"(
    <root>
        <BsWfsElement>
            <Time>2024-01-01T10:00:00Z</Time>
            <ParameterName>Temperature</ParameterName>
            <ParameterValue>1</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T10:00:00Z</Time>
            <ParameterName>WindSpeedMS</ParameterName>
            <ParameterValue>2</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T10:00:00Z</Time>
            <ParameterName>WindDirection</ParameterName>
            <ParameterValue>3</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T10:00:00Z</Time>
            <ParameterName>Precipitation1h</ParameterName>
            <ParameterValue>4</ParameterValue>
        </BsWfsElement>
        <BsWfsElement>
            <Time>2024-01-01T10:00:00Z</Time>
            <ParameterName>WeatherSymbol3</ParameterName>
            <ParameterValue>5</ParameterValue>
        </BsWfsElement>
    </root>
    )";

    data::FmiXmlParser parser;
    auto result = parser.parse(
        xml, 61.0, 25.0,
        QDateTime::fromString("2024-01-01T12:00:00Z", Qt::ISODate));

    QVERIFY(result.has_value());
    QCOMPARE(result->temperature, 1.0);
    QCOMPARE(result->windSpeedMs, 2.0);
    QCOMPARE(result->windDirectionDeg, 3.0);
    QCOMPARE(result->precipitationMm, 4.0);
    QCOMPARE(result->symbolCode, 5);
}

QTEST_MAIN(TestFmiXmlParser)
#include "test_fmi_xml_parser.moc"
