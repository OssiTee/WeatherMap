#include "viewmodel/map/MapLoaderViewModel.h"

#include <QMetaObject>
#include <QMetaType>
#include <QtConcurrent>
#include <memory>
#include <spdlog/spdlog.h>

namespace {

    // Converts domain::NormalizedPoint → QPointF
    std::vector<std::vector<QPointF>> convertToQtPoints(
        const std::vector<std::vector<domain::NormalizedPoint>> &polys) {
        std::vector<std::vector<QPointF>> qpolys;
        qpolys.reserve(polys.size());

        for (const auto &poly : polys) {
            std::vector<QPointF> qpoly;
            qpoly.reserve(poly.size());

            for (const auto &p : poly) {
                qpoly.emplace_back(p.x, p.y);
            }

            qpolys.push_back(std::move(qpoly));
        }

        return qpolys;
    }

    // Register the map shape meta type once so it can be used with queued
    // signal delivery across threads.
    bool registerMapShapeMetaType() {
        qRegisterMetaType<
            std::shared_ptr<const std::vector<std::vector<QPointF>>>>(
            "std::shared_ptr<const std::vector<std::vector<QPointF>>>");
        return true;
    }

} // unnamed namespace

namespace viewmodel {

    // Construct the map loader and ensure signal meta type registration.
    MapLoaderViewModel::MapLoaderViewModel(
        std::unique_ptr<domain::IMapService> service, QObject *parent)
        : QObject(parent), m_service(std::move(service)) {
        static bool metaRegistered = registerMapShapeMetaType();
        (void)metaRegistered;
    }

    // Load map geometry from the service asynchronously and emit the
    // normalized polygon data back to the UI thread.
    void MapLoaderViewModel::loadMap() {
        SPDLOG_INFO("Starting map loading...");

        auto _ = QtConcurrent::run([this]() {
            auto result = m_service->loadMap();

            if (result.isError()) {
                auto msg = QString::fromStdString(result.errorMessage());
                SPDLOG_ERROR("Failed to load map shape: {}",
                             result.errorMessage());

                QMetaObject::invokeMethod(
                    this, [this, msg]() { emit errorOccurred(msg); },
                    Qt::QueuedConnection);
                return;
            }

            const auto &map = result.value();

            // Convert domain::NormalizedPoint → QPointF
            auto qpolys = convertToQtPoints(map.polygons);

            auto qpolysPtr =
                std::make_shared<const std::vector<std::vector<QPointF>>>(
                    std::move(qpolys));

            QMetaObject::invokeMethod(
                this,
                [this, qpolysPtr, box = map.bbox]() {
                    emit mapShapeReady(qpolysPtr, box);
                },
                Qt::QueuedConnection);
        });
    }

} // namespace viewmodel
