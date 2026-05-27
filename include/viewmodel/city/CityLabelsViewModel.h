#pragma once

#include "CityLabelItem.h"
#include "domain/city/ICityService.h"
#include "shared/BoundingBox.h"

#include <QObject>
#include <vector>

namespace viewmodel {

    /**
     * @class CityLabelsViewModel
     * @brief Produces normalized city label positions for the UI.
     *
     * Responsibilities:
     *  - Retrieve city data from the domain CityService.
     *  - Produce UI-friendly CityLabelItem objects.
     *  - Emit labelsReady() when new labels are available.
     *  - Emit errorOccurred() if city data cannot be loaded.
     */
    class CityLabelsViewModel : public QObject {
        Q_OBJECT

      public:
        /**
         * @brief Constructs the ViewModel with a CityService dependency.
         *
         * @param service Unique pointer to an ICityService implementation.
         *                Must not be null. Ownership is transferred to the
         * ViewModel.
         */
        explicit CityLabelsViewModel(
            std::unique_ptr<domain::ICityService> service);

        /**
         * @brief Sets the geographic bounding box used for coordinate
         * normalization.
         */
        void setBoundingBox(const shared::BoundingBox &box);

        /**
         * @brief Rebuilds the city label list using the current bounding box.
         *
         * If the underlying CityService reports an error, errorOccurred() is
         * emitted and no labels are produced.
         */
        void updateLabels();

        /**
         * @brief Returns the processed city labels.
         */
        const std::vector<CityLabelItem> &labels() const {
            static const std::vector<CityLabelItem> empty;
            return m_labels ? *m_labels : empty;
        }

      signals:
        /**
         * @brief Emitted when new city label data is available.
         */
        void
        labelsReady(std::shared_ptr<const std::vector<CityLabelItem>> labels);

        /**
         * @brief Emitted when city data cannot be loaded or processed.
         *
         * @param message Human-readable error message.
         */
        void errorOccurred(const QString &message);

      private:
        std::unique_ptr<domain::ICityService> m_service;
        shared::BoundingBox m_bbox;
        bool m_hasBBox = false;

        std::shared_ptr<const std::vector<CityLabelItem>> m_labels;
    };

} // namespace viewmodel

Q_DECLARE_METATYPE(std::shared_ptr<const std::vector<viewmodel::CityLabelItem>>)
