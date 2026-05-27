#pragma once

#include <QPixmap>
#include <QPointF>
#include <QWidget>
#include <array>
#include <memory>
#include <vector>

#include "viewmodel/city/CityLabelItem.h"
#include "viewmodel/weather/WeatherPointItem.h"

namespace view {

    /**
     * @class MapWidget
     * @brief Custom QWidget responsible for rendering the Finland map and all
     * weather layers.
     *
     * MapWidget is the central drawing surface of the application. It renders:
     *  - The geographic map polygons
     *  - City labels
     *  - Weather icons and temperatures
     *  - Wind arrows
     *
     * The widget uses a dynamic aspect-ratio system: the map's aspect ratio is
     * computed from geographic coordinates and passed via setMapAspect(). The
     * widget overrides heightForWidth() so that Qt layouts automatically
     * preserve the correct map proportions regardless of window size.
     *
     * All drawing is performed in paintEvent() using QPainter.
     */
    class MapWidget : public QWidget {
        Q_OBJECT

      public:
        /**
         * @brief Constructs the MapWidget and enables height-for-width layout
         * behavior.
         *
         * @param parent Optional parent widget.
         */
        explicit MapWidget(QWidget *parent = nullptr);

        /**
         * @brief Sets the polygon geometry of the map.
         *
         * The polygons are normalized to the range [0,1] in both axes.
         * Calling this triggers a repaint.
         *
         * @param polygons Shared pointer to a list of polygon coordinate lists.
         */
        void setMapShape(
            std::shared_ptr<const std::vector<std::vector<QPointF>>> polygons);

        /**
         * @brief Sets the weather point data (temperature, icon, wind).
         *
         * Coordinates are normalized to [0,1].
         * Calling this triggers a repaint.
         *
         * @param points Shared pointer to weather point items.
         */
        void setWeatherPoints(
            std::shared_ptr<const std::vector<viewmodel::WeatherPointItem>>
                points);

        /**
         * @brief Sets the city label data.
         *
         * Coordinates are normalized to [0,1].
         * Calling this triggers a repaint.
         *
         * @param labels Shared pointer to city label items.
         */
        void setCityLabels(
            std::shared_ptr<const std::vector<viewmodel::CityLabelItem>>
                labels);

        /**
         * @brief Sets the map aspect ratio (width / height).
         *
         * This value is used by heightForWidth() to maintain correct
         * proportions.
         *
         * @param aspect The aspect ratio of the map.
         */
        void setMapAspect(double aspect);

        /**
         * @brief Returns the preferred size of the widget.
         *
         * The height is based on a fraction of the screen height
         * (INITIAL_WINDOW_SCALE), and the width is computed from the current
         * aspect ratio.
         */
        QSize sizeHint() const override;

        /**
         * @brief Enables Qt's height-for-width layout behavior.
         *
         * This ensures that the widget's height is always computed from its
         * width using the map's aspect ratio.
         */
        bool hasHeightForWidth() const override;

        /**
         * @brief Computes the height required for a given width.
         *
         * @param w The width provided by the layout system.
         * @return The height computed as w / aspect.
         */
        int heightForWidth(int w) const override;

      protected:
        /**
         * @brief Paints the entire map and all weather layers.
         *
         * This method delegates to:
         *  - drawMap()
         *  - drawCityLabels()
         *  - drawWeatherIconsAndTemperatures()
         *  - drawWindArrows()
         */
        void paintEvent(QPaintEvent *event) override;

      private:
        /**
         * @brief Internal scaling structure used for converting normalized
         *        coordinates to widget pixel coordinates.
         */
        struct Scale {
            double sx; ///< Horizontal scale factor
            double sy; ///< Vertical scale factor
        };

        /**
         * @brief Computes the scale factors based on widget size and aspect
         * ratio.
         *
         * Ensures that the map is centered and scaled uniformly.
         */
        Scale computeScale() const;

        void drawMap(QPainter &painter);
        void drawCityLabels(QPainter &painter);
        void drawWeatherIconsAndTemperatures(QPainter &painter);
        void drawWindArrows(QPainter &painter);

        /**
         * @brief Draws a weather icon centered at the given position.
         */
        void drawWeatherIcon(QPainter &painter, const QPointF &pos,
                             viewmodel::WeatherIcon icon);

        /**
         * @brief Returns (and lazily loads) the pixmap for a given weather
         * icon.
         */
        const QPixmap &weatherIconPixmap(viewmodel::WeatherIcon icon);

        // --- Data ---
        std::shared_ptr<const std::vector<std::vector<QPointF>>> m_polygons;
        std::shared_ptr<const std::vector<viewmodel::WeatherPointItem>>
            m_weatherPoints;
        std::shared_ptr<const std::vector<viewmodel::CityLabelItem>>
            m_cityLabels;

        double m_mapAspect = 1.0; // Current map aspect ratio (width / height)

        static constexpr int WEATHER_ICON_COUNT =
            static_cast<int>(viewmodel::WeatherIcon::Unknown) + 1;

        std::array<QPixmap, WEATHER_ICON_COUNT> m_weatherIconPixmaps;
        std::array<bool, WEATHER_ICON_COUNT> m_weatherIconPixmapLoaded = {};
    };

} // namespace view
