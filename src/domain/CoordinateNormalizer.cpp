#include "domain/CoordinateNormalizer.h"

namespace domain {

    inline NormalizedPoint
    CoordinateNormalizer::normalizePoint(double lat, double lon,
                                         const shared::BoundingBox &box,
                                         double latSpan, double lonSpan) {
        NormalizedPoint p;
        p.x = (lon - box.minLon) / lonSpan;
        p.y = 1.0 - ((lat - box.minLat) / latSpan);
        return p;
    }

    NormalizedPoint
    CoordinateNormalizer::normalize(double lat, double lon,
                                    const shared::BoundingBox &box) {
        NormalizedPoint p;

        const double lonSpan = box.maxLon - box.minLon;
        const double latSpan = box.maxLat - box.minLat;

        // Avoid division by zero if the bounding box is degenerate.
        if (lonSpan <= 0.0 || latSpan <= 0.0) {
            p.x = 0.0;
            p.y = 0.0;
            return p;
        }

        return normalizePoint(lat, lon, box, latSpan, lonSpan);
    }

    std::vector<std::vector<NormalizedPoint>>
    CoordinateNormalizer::normalizePolygons(
        const std::vector<std::vector<LatLon>> &polys,
        const shared::BoundingBox &box) {

        std::vector<std::vector<NormalizedPoint>> result;
        result.reserve(polys.size());

        const double lonSpan = box.maxLon - box.minLon;
        const double latSpan = box.maxLat - box.minLat;

        // Avoid division by zero if the bounding box is degenerate.
        if (lonSpan <= 0.0 || latSpan <= 0.0) {
            return {};
        }

        for (const auto &poly : polys) {
            std::vector<NormalizedPoint> out;
            out.reserve(poly.size());

            for (const auto &p : poly) {
                // Normalize to [0,1]
                auto normalized =
                    normalizePoint(p.lat, p.lon, box, latSpan, lonSpan);
                out.emplace_back(normalized.x, normalized.y);
            }

            result.push_back(std::move(out));
        }

        return result;
    }

} // namespace domain
