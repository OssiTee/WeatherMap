#include "viewmodel/map/MapLoaderViewModel.h"

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
        : QObject(parent)
        , m_service(std::shared_ptr<domain::IMapService>(std::move(service))) {
        static bool metaRegistered = registerMapShapeMetaType();
        (void)metaRegistered;

        connect(&m_watcher,
                &QFutureWatcher<shared::Result<domain::NormalizedMap>>::finished,
                this, &MapLoaderViewModel::onMapLoadFinished);
    }

    // Load map geometry from the service asynchronously and emit the
    // normalized polygon data back to the UI thread.
    void MapLoaderViewModel::loadMap() {
        SPDLOG_INFO("Starting map loading...");

        if (m_watcher.isRunning()) {
            return;
        }

        auto service = m_service;

        auto future = QtConcurrent::run([service = std::move(service)]() {
            return service->loadMap();
        });

        m_watcher.setFuture(std::move(future));
    }

    void MapLoaderViewModel::onMapLoadFinished() {
        auto result = m_watcher.result();

        if (result.isError()) {
            SPDLOG_ERROR("Failed to load map shape: {}",
                         result.errorMessage());
            emit errorOccurred(QString::fromStdString(result.errorMessage()));
            return;
        }

        const auto &map = result.value();

        // Convert domain::NormalizedPoint → QPointF on the UI thread right
        // before publishing the final immutable payload.
        auto qpolys = convertToQtPoints(map.polygons);

        auto qpolysPtr =
            std::make_shared<const std::vector<std::vector<QPointF>>>(
                std::move(qpolys));

        emit mapShapeReady(qpolysPtr, map.bbox);
    }

} // namespace viewmodel
