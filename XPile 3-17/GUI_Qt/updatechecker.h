#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(const QString &currentVersion, QObject *parent = nullptr);
    void checkNow();

signals:
    void updateAvailable(const QString &newVersion, const QString &downloadUrl);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
    QString m_currentVersion;
};
