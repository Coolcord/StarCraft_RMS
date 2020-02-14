#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkProxy>

class HTML_String_Helper;

class Downloader : public QObject {
    Q_OBJECT

public:
    Downloader(QObject *parent, const QString &startingPageURL, const QString &downloadFolder);
    ~Downloader();

signals:

public slots:
    void Download_Maps();

private slots:
    void Read_Download_Links();
    void Map_Download_Finished();
    void Network_Accessible_Changed(QNetworkAccessManager::NetworkAccessibility accessible);
    void SSL_Errors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    void Process_Next_Download_Link();
    bool Get_Next_Page();

    QObject *parent;
    HTML_String_Helper *htmlStringHelper;
    QString startingPageURL;
    QString currentPageURL;
    QString downloadFolder;
    QStringList downloadLinks;
    QStringList fileNames;
    QNetworkAccessManager *manager;
};

#endif // DOWNLOADER_H
