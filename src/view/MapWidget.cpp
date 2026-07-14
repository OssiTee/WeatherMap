#include "view/MapWidget.h"
#include "view/Colors.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>
#include <cmath>

namespace {

    static constexpr QPointF TEMPERATURE_TEXT_OFFSET{14, -14};

    static constexpr double WIND_ARROW_BASE_LEN = 20.0;
    static constexpr double WIND_ARROW_SPEED_FACTOR = 1.5;
    static constexpr int WIND_PEN_WIDTH = 2;
    static constexpr int WIND_ARROW_OFFSET_X = -25;
    static constexpr int WIND_ARROW_OFFSET_Y = -25;
    static constexpr double WIND_ARROW_HEAD_SIZE = 6.0;
    static constexpr double METEO_TO_QT_ANGLE_OFFSET = 90.0;
    static constexpr double ARROW_HEAD_ANGLE = M_PI * 0.75;

    static constexpr int WEATHER_ICON_SIZE = 64;
    static constexpr double INITIAL_WINDOW_SCALE = 0.8;
    static constexpr qreal CLOUD_OVERLAY_OPACITY = 0.70;

    QString weatherIconPath(viewmodel::WeatherIcon icon) {
        switch (icon) {
        case viewmodel::WeatherIcon::Clear:
            return ":/icons/1.svg";
        case viewmodel::WeatherIcon::PartlyCloudy:
            return ":/icons/2.svg";
        case viewmodel::WeatherIcon::Cloudy:
            return ":/icons/3.svg";
        case viewmodel::WeatherIcon::Rain:
            return ":/icons/31.svg";
        case viewmodel::WeatherIcon::Snow:
            return ":/icons/22.svg";
        case viewmodel::WeatherIcon::Thunder:
            return ":/icons/63.svg";
        default:
            return ":/icons/3.svg";
        }
    }

} // namespace

namespace view {

    MapWidget::MapWidget(QWidget *parent) : QWidget(parent) {
        QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sp.setHeightForWidth(true);
        setSizePolicy(sp);
    }

    void MapWidget::setMapShape(
        std::shared_ptr<const std::vector<std::vector<QPointF>>> polygons) {
        m_polygons = std::move(polygons);
        update();
    }

    void MapWidget::setWeatherPoints(
        std::shared_ptr<const std::vector<viewmodel::WeatherPointItem>>
            points) {
        m_weatherPoints = std::move(points);
        update();
    }

    void MapWidget::setCloudOverlayImage(QImage image) {
        m_cloudOverlayImage = std::move(image);
        update();
    }

    void MapWidget::setCloudOverlayVisible(bool visible) {
        m_cloudOverlayVisible = visible;
        update();
    }

    void MapWidget::setCityLabels(
        std::shared_ptr<const std::vector<viewmodel::CityLabelItem>> labels) {
        m_cityLabels = std::move(labels);
        update();
    }

    void MapWidget::setMapAspect(double aspect) {
        m_mapAspect = aspect;
        update();
    }

    QSize MapWidget::sizeHint() const {
        QRect screen = QGuiApplication::primaryScreen()->availableGeometry();

        int h = int(screen.height() * INITIAL_WINDOW_SCALE);
        int w = int(h * m_mapAspect);

        return QSize(w, h);
    }

    MapWidget::Scale MapWidget::computeScale() const {
        double widgetAspect = width() / double(height());
        double mapAspect = m_mapAspect;

        Scale s;

        if (widgetAspect > mapAspect) {
            s.sy = height();
            s.sx = s.sy * mapAspect;
        } else {
            s.sx = width();
            s.sy = s.sx / mapAspect;
        }

        return s;
    }

    bool MapWidget::hasHeightForWidth() const { return true; }

    int MapWidget::heightForWidth(int w) const { return int(w / m_mapAspect); }

    void MapWidget::paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        drawMap(painter);
        drawCloudOverlay(painter);
        drawCityLabels(painter);
        drawWeatherIconsAndTemperatures(painter);
        drawWindArrows(painter);
    }

    void MapWidget::drawMap(QPainter &painter) {
        if (!m_polygons)
            return;

        auto scale = computeScale();

        painter.setPen(view::Colors::MapBorder());
        painter.setBrush(view::Colors::MapFill());

        for (const auto &poly : *m_polygons) {
            QPolygonF qpoly;
            for (const QPointF &p : poly) {
                qpoly << QPointF(p.x() * scale.sx, p.y() * scale.sy);
            }
            painter.drawPolygon(qpoly);
        }
    }

    void MapWidget::drawCityLabels(QPainter &painter) {
        if (!m_cityLabels)
            return;

        auto scale = computeScale();

        painter.setPen(view::Colors::CityLabel());
        painter.setFont(QFont("Arial", 10, QFont::Bold));

        for (const auto &c : *m_cityLabels) {
            QPointF pos(c.x * scale.sx, c.y * scale.sy);

            QFontMetrics fm(painter.font());
            int textWidth = fm.horizontalAdvance(c.name);
            int ascent = fm.ascent();

            QPointF textPos(pos.x() - textWidth * 0.5, pos.y() + ascent * 0.5);
            painter.drawText(textPos, c.name);
        }
    }

    void MapWidget::drawCloudOverlay(QPainter &painter) {
        if (!m_cloudOverlayVisible || m_cloudOverlayImage.isNull()) {
            return;
        }

        auto scale = computeScale();
        const QRectF targetRect(0.0, 0.0, scale.sx, scale.sy);

        painter.setOpacity(CLOUD_OVERLAY_OPACITY);
        painter.drawImage(targetRect, m_cloudOverlayImage);
        painter.setOpacity(1.0);
    }

    void MapWidget::drawWeatherIconsAndTemperatures(QPainter &painter) {
        if (!m_weatherPoints)
            return;

        auto scale = computeScale();

        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.setPen(view::Colors::Temperature());

        for (const auto &wp : *m_weatherPoints) {
            QPointF pos(wp.xNorm * scale.sx, wp.yNorm * scale.sy);

            drawWeatherIcon(painter, pos, wp.icon);

            QString unit =
                (wp.unit == shared::TemperatureUnit::Fahrenheit) ? "°F" : "°C";
            QString tempText =
                QString::number(std::round(wp.temperature)) + unit;

            painter.drawText(pos + TEMPERATURE_TEXT_OFFSET, tempText);
        }
    }

    void MapWidget::drawWindArrows(QPainter &painter) {
        if (!m_weatherPoints)
            return;

        auto scale = computeScale();

        painter.setPen(QPen(view::Colors::WindArrow(), WIND_PEN_WIDTH));

        for (const auto &wp : *m_weatherPoints) {
            QPointF center(wp.xNorm * scale.sx, wp.yNorm * scale.sy);
            center += QPointF(WIND_ARROW_OFFSET_X, WIND_ARROW_OFFSET_Y);

            double len =
                WIND_ARROW_BASE_LEN + wp.windSpeedMs * WIND_ARROW_SPEED_FACTOR;
            double targetDir = wp.windDirectionDeg + 180.0;
            double angleRad =
                qDegreesToRadians(METEO_TO_QT_ANGLE_OFFSET - targetDir);

            double dx = std::cos(angleRad);
            double dy = -std::sin(angleRad);

            QPointF start(center.x() - dx * (len / 2.0),
                          center.y() - dy * (len / 2.0));
            QPointF end(center.x() + dx * (len / 2.0),
                        center.y() + dy * (len / 2.0));

            painter.drawLine(start, end);

            QPointF left(end.x() + WIND_ARROW_HEAD_SIZE *
                                       std::cos(angleRad + ARROW_HEAD_ANGLE),
                         end.y() - WIND_ARROW_HEAD_SIZE *
                                       std::sin(angleRad + ARROW_HEAD_ANGLE));

            QPointF right(end.x() + WIND_ARROW_HEAD_SIZE *
                                        std::cos(angleRad - ARROW_HEAD_ANGLE),
                          end.y() - WIND_ARROW_HEAD_SIZE *
                                        std::sin(angleRad - ARROW_HEAD_ANGLE));

            painter.drawLine(end, left);
            painter.drawLine(end, right);
        }
    }

    void MapWidget::drawWeatherIcon(QPainter &painter, const QPointF &pos,
                                    viewmodel::WeatherIcon icon) {
        const QPixmap &iconPixmap = weatherIconPixmap(icon);

        const QPointF topLeft(pos.x() - WEATHER_ICON_SIZE / 2.0,
                              pos.y() - WEATHER_ICON_SIZE / 2.0);

        painter.drawPixmap(topLeft, iconPixmap);
    }

    const QPixmap &MapWidget::weatherIconPixmap(viewmodel::WeatherIcon icon) {
        int index = static_cast<int>(icon);
        int safeIndex = (index >= 0 && index < WEATHER_ICON_COUNT)
                            ? index
                            : static_cast<int>(viewmodel::WeatherIcon::Unknown);

        if (!m_weatherIconPixmapLoaded[safeIndex]) {
            QPixmap pix(WEATHER_ICON_SIZE, WEATHER_ICON_SIZE);
            pix.fill(Qt::transparent);

            QPainter pixPainter(&pix);
            pixPainter.setRenderHint(QPainter::Antialiasing, true);

            QSvgRenderer renderer(weatherIconPath(icon));
            renderer.render(&pixPainter,
                            QRectF(0, 0, WEATHER_ICON_SIZE, WEATHER_ICON_SIZE));

            m_weatherIconPixmaps[safeIndex] = pix;
            m_weatherIconPixmapLoaded[safeIndex] = true;
        }

        return m_weatherIconPixmaps[safeIndex];
    }

} // namespace view
