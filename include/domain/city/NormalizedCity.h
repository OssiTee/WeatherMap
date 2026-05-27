#pragma once
#include <string>

namespace domain {

    // Represents a city with normalized [0,1] coordinates.
    struct NormalizedCity {
        std::string name;
        double xNorm;
        double yNorm;
    };

} // namespace domain
