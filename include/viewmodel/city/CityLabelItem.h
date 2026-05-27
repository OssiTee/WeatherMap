#pragma once
#include <QString>

namespace viewmodel {

    /**
     * @brief Item for city labels displayed on the map.
     */
    struct CityLabelItem {
        QString name; // City name to display
        double x;     // Normalized x-coordinate [0,1]
        double y;     // Normalized y-coordinate [0,1]
    };

} // namespace viewmodel