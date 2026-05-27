#pragma once

#include <QByteArray>
#include <QString>

namespace data {

    /**
     * @class NetworkClient
     * @brief Simple synchronous HTTP client for performing GET requests.
     *
     * NetworkClient provides a minimal wrapper around Qt's
     * QNetworkAccessManager to perform blocking HTTP GET requests. It is
     * intentionally lightweight and contains no retry logic, caching, or error
     * reporting beyond returning an empty QByteArray on failure.
     *
     * Responsibilities:
     *  - Execute a synchronous HTTP GET request
     *  - Return the raw response body as QByteArray
     *  - Return an empty QByteArray if the request fails
     *
     * This class is used by WeatherRepository but can be reused for any
     * simple network retrieval task. It is designed to be easily mockable
     * in unit tests.
     */
    class NetworkClient {
      public:
        /**
         * @brief Performs a blocking HTTP GET request.
         *
         * The function waits for the network reply to finish using a local
         * QEventLoop. If the request succeeds, the response body is returned.
         * If the request fails, an empty QByteArray is returned.
         *
         * @param url Fully qualified URL to fetch.
         * @return QByteArray containing the response body, or empty on error.
         */
        QByteArray get(const QString &url) const;
    };

} // namespace data
