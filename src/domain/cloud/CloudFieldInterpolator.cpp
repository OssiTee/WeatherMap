#include "domain/cloud/CloudFieldInterpolator.h"

#include <algorithm>
#include <cmath>

namespace {

    constexpr double MIN_CLOUD_PERCENT = 0.0;
    constexpr double MAX_CLOUD_PERCENT = 100.0;

    inline size_t fieldIndex(int x, int y, int width) {
        // Row-major index into a flattened 2D field.
        return static_cast<size_t>(y) * static_cast<size_t>(width) +
               static_cast<size_t>(x);
    }

} // namespace

namespace domain {

    std::vector<double> CloudFieldInterpolator::buildWeightedCloudField(
        const std::vector<NormalizedCloudPoint> &points, int width, int height,
        double sigma) {
        // Invalid inputs produce no field so callers can fail gracefully.
        if (points.empty() || width < 2 || height < 2 || sigma <= 0.0) {
            return {};
        }

        // Precompute the Gaussian coefficient used for all distance weights.
        const double invTwoSigma2 = 1.0 / (2.0 * sigma * sigma);

        std::vector<double> field(static_cast<size_t>(width) *
                                  static_cast<size_t>(height));

        for (int y = 0; y < height; ++y) {
            const double yNorm =
                static_cast<double>(y) / static_cast<double>(height - 1);

            for (int x = 0; x < width; ++x) {
                const double xNorm =
                    static_cast<double>(x) / static_cast<double>(width - 1);

                double weightedCloud = 0.0;
                double weightSum = 0.0;

                for (const auto &p : points) {
                    const double dx = xNorm - p.xNorm;
                    const double dy = yNorm - p.yNorm;
                    const double dist2 = dx * dx + dy * dy;

                    // Nearby samples dominate, distant samples fade smoothly.
                    const double w = std::exp(-dist2 * invTwoSigma2);
                    weightedCloud += w *
                                     std::clamp(p.cloudCoverPercent,
                                                MIN_CLOUD_PERCENT,
                                                MAX_CLOUD_PERCENT);
                    weightSum += w;
                }

                const size_t idx = fieldIndex(x, y, width);
                if (weightSum <= 0.0) {
                    field[idx] = 0.0;
                    continue;
                }

                field[idx] = weightedCloud / weightSum;
            }
        }

        return field;
    }

} // namespace domain
