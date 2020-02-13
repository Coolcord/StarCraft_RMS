#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkProxy>

class Downloader : public QObject {
    Q_OBJECT

public:
    Downloader(QObject *parent, const QString &startingURL, const QString &downloadFolder);
    ~Downloader();

signals:

public slots:
    void Download_Maps();

private slots:
    void Get_Download_Links();
    void Process_Download_Links();
    void Network_Accessible_Changed(QNetworkAccessManager::NetworkAccessibility accessible);
    void SSL_Errors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    bool Get_Next_Page();

    QObject *parent;
    QString startingURL;
    QString currentURL;
    QString downloadFolder;
    QStringList downloadLinks;
    QStringList fileNames;
    QNetworkAccessManager *manager;
};

#endif // DOWNLOADER_H
