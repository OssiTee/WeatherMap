#include "viewmodel/city/CityLabelsViewModel.h"
#include <cassert>
#include <spdlog/spdlog.h>

namespace viewmodel {

    // Construct the city labels ViewModel and register the queued signal
    // meta type needed for passing shared pointers across threads.
    CityLabelsViewModel::CityLabelsViewModel(
        std::unique_ptr<domain::ICityService> service)
        : m_service(std::move(service)) {
        if (!m_service) {
            SPDLOG_ERROR(
                "CityLabelsViewModel constructed with nullptr service");
            assert(false && "ICityService pointer must not be null");
        }

        qRegisterMetaType<std::shared_ptr<const std::vector<CityLabelItem>>>(
            "std::shared_ptr<const std::vector<viewmodel::CityLabelItem>>");
    }

    void CityLabelsViewModel::setBoundingBox(const shared::BoundingBox &box) {
        m_bbox = box;
        m_hasBBox = true;
    }

    // Generate city labels from the service data and normalize them to the
    // current bounding box.
    void CityLabelsViewModel::updateLabels() {
        if (!m_hasBBox) {
            return;
        }
        auto cityResult = m_service->cities(m_bbox);

        if (!cityResult.isSuccess()) {
            emit errorOccurred(
                QString::fromStdString(cityResult.errorMessage()));
            return;
        }

        const auto &cities = cityResult.value();

        auto cityLabels = std::make_shared<std::vector<CityLabelItem>>();
        cityLabels->reserve(cities.size());

        for (const auto &c : cities) {
            CityLabelItem item;
            item.name = QString::fromStdString(c.name);
            item.x = c.xNorm;
            item.y = c.yNorm;

            cityLabels->push_back(std::move(item));
        }

        m_labels = cityLabels;
        emit labelsReady(std::move(cityLabels));
    }

} // namespace viewmodel
