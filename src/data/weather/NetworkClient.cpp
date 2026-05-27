#include "data/weather/NetworkClient.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace data {

    // Perform a blocking HTTP GET request with a manual timeout
    // (Qt6-compatible)
    QByteArray NetworkClient::get(const QString &url) const {
        QNetworkAccessManager manager;
        QNetworkRequest request(url);

        // Issue asynchronous GET request
        QNetworkReply *reply = manager.get(request);

        // Local event loop to wait synchronously
        QEventLoop loop;
        QTimer timer;

        timer.setSingleShot(true);
        timer.setInterval(5000); // 5s timeout

        // Abort the reply if timeout occurs
        QObject::connect(&timer, &QTimer::timeout, [&]() { reply->abort(); });

        // Quit loop when reply finishes (success or error)
        QObject::connect(reply, &QNetworkReply::finished, &loop,
                         &QEventLoop::quit);

        timer.start();
        loop.exec();

        // Handle network errors or timeout
        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return {};
        }

        // Read full response body
        QByteArray data = reply->readAll();
        reply->deleteLater();
        return data;
    }

} // namespace data
