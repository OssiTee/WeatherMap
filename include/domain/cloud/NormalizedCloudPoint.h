#pragma once

namespace domain {

    /**
     * @brief Cloud coverage value in normalized map coordinates.
     */
    struct NormalizedCloudPoint {
        double xNorm = 0.0;
        double yNorm = 0.0;
        double cloudCoverPercent = 0.0;
    };

} // namespace domain
