#include "updatechecker.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

UpdateChecker::UpdateChecker(const QString &currentVersion, QObject *parent)
    : QObject(parent), m_currentVersion(currentVersion)
{
    connect(&m_nam, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onReplyFinished);
}

void UpdateChecker::checkNow()
{
    QNetworkRequest req(QUrl("https://api.github.com/repos/albazzaztariq/UniLogic/releases/latest"));
    req.setHeader(QNetworkRequest::UserAgentHeader, "UniLogic-ProjectManager/1.0");
    m_nam.get(req);
}

void UpdateChecker::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError)
        return; // Silently ignore network errors

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString tagName = obj["tag_name"].toString();
    QString htmlUrl = obj["html_url"].toString();

    // Strip leading 'v' for comparison
    QString remoteVer = tagName.startsWith('v') ? tagName.mid(1) : tagName;

    if (!remoteVer.isEmpty() && remoteVer != m_currentVersion)
        emit updateAvailable(remoteVer, htmlUrl);
}
