#include "data/city/CityLoader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace data {

    // CityLoader reads a JSON city list from the configured resource path.
    // The JSON must contain a top-level "cities" array with objects that
    // include name, lat, and lon fields.
    CityLoader::CityLoader(QString resourcePath)
        : m_resourcePath(std::move(resourcePath)) {}

    shared::Result<std::vector<domain::ICityLoader::RawCity>>
    CityLoader::loadCities() {
        QFile file(m_resourcePath);

        if (!file.open(QIODevice::ReadOnly)) {
            return shared::Result<std::vector<domain::ICityLoader::RawCity>>::
                error("Failed to open city data file: " +
                      m_resourcePath.toStdString());
        }

        QByteArray bytes = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(bytes);

        if (!doc.isObject()) {
            return shared::Result<std::vector<domain::ICityLoader::RawCity>>::
                error("City JSON is not an object");
        }

        auto arr = doc.object()["cities"].toArray();
        std::vector<domain::ICityLoader::RawCity> result;
        result.reserve(arr.size());

        for (const auto &item : arr) {
            auto obj = item.toObject();

            domain::ICityLoader::RawCity c;
            c.name = obj["name"].toString().toStdString();
            c.latitude = obj["lat"].toDouble();
            c.longitude = obj["lon"].toDouble();

            result.push_back(std::move(c));
        }

        return shared::Result<std::vector<domain::ICityLoader::RawCity>>::
            success(std::move(result));
    }

} // namespace data
