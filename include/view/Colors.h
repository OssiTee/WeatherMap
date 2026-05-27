#pragma once
#include <QColor>

namespace view {

    struct Colors {
        // Temerature text color
        static inline QColor Temperature() { return QColor(30, 80, 200); }

        // City label color
        static inline QColor CityLabel() { return Qt::black; }

        // Wind arrow color
        static inline QColor WindArrow() { return QColor(0, 0, 120); }

        // Map fill color
        static inline QColor MapFill() { return QColor(230, 230, 230); }

        // Map border color
        static inline QColor MapBorder() { return Qt::black; }
    };

} // namespace view
