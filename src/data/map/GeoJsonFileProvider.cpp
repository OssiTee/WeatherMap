#include "data/map/GeoJsonFileProvider.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
#include <spdlog/spdlog.h>

namespace {

    //! @brief Helper: Update bounding box with a new point
    static inline void updateBounds(double lat, double lon, double &minLat,
                                    double &maxLat, double &minLon,
                                    double &maxLon) {
        minLat = std::min(minLat, lat);
        maxLat = std::max(maxLat, lat);
        minLon = std::min(minLon, lon);
        maxLon = std::max(maxLon, lon);
    }

    //! @brief Helper: Parse a Polygon geometry
    static void parsePolygon(const QJsonArray &rings,
                             std::vector<std::vector<domain::LatLon>> &polygons,
                             double &minLat, double &maxLat, double &minLon,
                             double &maxLon) {
        if (rings.isEmpty()) {
            return;
        }

        QJsonArray outer = rings.at(0).toArray();
        std::vector<domain::LatLon> poly;
        poly.reserve(outer.size());

        for (const auto &v : outer) {
            QJsonArray p = v.toArray();
            if (p.size() < 2) {
                continue;
            }

            double lon = p.at(0).toDouble();
            double lat = p.at(1).toDouble();

            poly.push_back({lat, lon});
            updateBounds(lat, lon, minLat, maxLat, minLon, maxLon);
        }

        if (!poly.empty()) {
            polygons.push_back(std::move(poly));
        }
    }

    //! @brief Helper: Parse a MultiPolygon geometry
    static void
    parseMultiPolygon(const QJsonArray &polys,
                      std::vector<std::vector<domain::LatLon>> &polygons,
                      double &minLat, double &maxLat, double &minLon,
                      double &maxLon) {
        for (const auto &polyVal : polys) {
            QJsonArray rings = polyVal.toArray();
            if (rings.isEmpty()) {
                continue;
            }

            parsePolygon(rings, polygons, minLat, maxLat, minLon, maxLon);
        }
    }

    //! @brief Helper: Parse a geometry object (Polygon or MultiPolygon)
    static void
    parseGeometry(const QJsonObject &geom,
                  std::vector<std::vector<domain::LatLon>> &polygons,
                  double &minLat, double &maxLat, double &minLon,
                  double &maxLon) {
        const QString type = geom["type"].toString();
        const QJsonArray coords = geom["coordinates"].toArray();

        if (type == "Polygon") {
            parsePolygon(coords, polygons, minLat, maxLat, minLon, maxLon);
        } else if (type == "MultiPolygon") {
            parseMultiPolygon(coords, polygons, minLat, maxLat, minLon, maxLon);
        }
    }

} // namespace

namespace data {

    //! @brief Main API: Load shape from GeoJSON
    GeoJsonFileProvider::GeoJsonFileProvider(QString resourcePath)
        : m_resourcePath(std::move(resourcePath)) {}

    shared::Result<domain::IMapProvider::RawMapData>
    GeoJsonFileProvider::loadShape() {
        std::vector<std::vector<domain::LatLon>> polygons;

        // Initialize bounding box
        double minLat = std::numeric_limits<double>::max();
        double maxLat = std::numeric_limits<double>::lowest();
        double minLon = std::numeric_limits<double>::max();
        double maxLon = std::numeric_limits<double>::lowest();

        // Read file
        QFile file(m_resourcePath);
        if (!file.open(QIODevice::ReadOnly)) {
            std::string error =
                "Failed to open " + m_resourcePath.toStdString();
            SPDLOG_ERROR(error);
            return shared::Result<domain::IMapProvider::RawMapData>::error(
                error);
        }

        QByteArray data = file.readAll();
        file.close();

        // Parse JSON
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);

        if (err.error != QJsonParseError::NoError) {
            std::string error =
                "JSON parse error: " + err.errorString().toStdString();
            SPDLOG_ERROR(error);
            return shared::Result<domain::IMapProvider::RawMapData>::error(
                error);
        }

        if (!doc.isObject()) {
            std::string error = "Invalid JSON: root is not an object";
            SPDLOG_ERROR(error);
            return shared::Result<domain::IMapProvider::RawMapData>::error(
                error);
        }

        QJsonObject root = doc.object();

        // Expect FeatureCollection
        if (root["type"].toString() != "FeatureCollection") {
            std::string error = "Invalid GeoJSON: not a FeatureCollection";
            SPDLOG_ERROR(error);
            return shared::Result<domain::IMapProvider::RawMapData>::error(
                error);
        }

        QJsonArray features = root["features"].toArray();

        // Parse each feature
        for (const auto &f : features) {
            if (!f.isObject()) {
                continue;
            }

            QJsonObject feature = f.toObject();
            if (!feature.contains("geometry"))
                continue;

            QJsonObject geom = feature["geometry"].toObject();
            parseGeometry(geom, polygons, minLat, maxLat, minLon, maxLon);
        }

        if (polygons.empty()) {
            std::string error = "No polygons found in GeoJSON";
            SPDLOG_ERROR(error);
            return shared::Result<domain::IMapProvider::RawMapData>::error(
                error);
        }

        domain::IMapProvider::RawMapData result{std::move(polygons), minLat,
                                                maxLat, minLon, maxLon};
        SPDLOG_INFO("Successfully loaded {} polygons from {}",
                    result.polygons.size(), m_resourcePath.toStdString());
        return shared::Result<domain::IMapProvider::RawMapData>::success(
            std::move(result));
    }

} // namespace data
