#pragma once
#include "domain/CoordinateNormalizer.h"
#include "shared/BoundingBox.h"
#include <vector>

namespace domain {

    // Normalized representation of map polygons with latitude/longitude bounds
    struct NormalizedMap {
        std::vector<std::vector<domain::NormalizedPoint>> polygons;
        shared::BoundingBox bbox;
    };

} // namespace domain
