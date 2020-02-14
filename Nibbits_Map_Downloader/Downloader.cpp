#include "Downloader.h"
#include "HTML_String_Helper.h"
#include <QDir>
#include <QTextStream>

const static QString STRING_BASE_URL = "http://sc.nibbits.com";

Downloader::Downloader(QObject *parent, const QString &startingURL, const QString &downloadFolder) : QObject(parent) {
    this->parent = parent;
    this->startingPageURL = startingURL;
    this->currentPageURL = startingURL;
    this->downloadFolder = downloadFolder;
    this->downloadLinks = new QQueue<Download_Link_Data>();
    this->htmlStringHelper = new HTML_String_Helper();
    this->manager = new QNetworkAccessManager(parent);
    connect(this->manager, &QNetworkAccessManager::networkAccessibleChanged, this, &Downloader::Network_Accessible_Changed);
    connect(this->manager, &QNetworkAccessManager::sslErrors, this, &Downloader::SSL_Errors);
}

Downloader::~Downloader() {
    delete this->downloadLinks;
    delete this->htmlStringHelper;
    delete this->manager;
    this->manager = nullptr;
}

void Downloader::Download_Maps() {
    if (!this->currentPageURL.startsWith(STRING_BASE_URL) || !this->currentPageURL.contains("&p=")) {
        qCritical() << "Invalid URL!";
        return;
    }
    if (!QDir(this->downloadFolder).exists()) {
        qCritical() << "Cannot open the downloads folder!";
        qCritical() << this->downloadFolder;
        return;
    }
    QNetworkReply *reply = this->manager->get(QNetworkRequest(QUrl(this->currentPageURL)));
    connect(reply, &QNetworkReply::finished, this, &Downloader::Read_Download_Links);
}

void Downloader::Read_Download_Links() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    if (reply->error() != QNetworkReply::NoError) return;
    QByteArray data = reply->readAll();
    QTextStream stream(&data);
    Download_Link_Data downloadLinkData;
    downloadLinkData.numPlayers = 0;
    downloadLinkData.fileName = QString();
    downloadLinkData.fileType = QString();
    downloadLinkData.downloadLink = QString();
    while (!stream.atEnd()) {
        //Read Max Players
        QString line = stream.readLine();
        if (line.contains("<span class=\"players\">(")) {
            bool valid = false;
            int tmp = 0;
            tmp = line.split("<span class=\"players\">(").last().split(")").first().toInt(&valid);
            if (valid) downloadLinkData.numPlayers = tmp;
        }

        //Read File Type
        if (line.contains("class=\"filetype-")) {
            downloadLinkData.fileType = line.split("class=\"filetype-").at(1).split("\"").first().toLower();
        }

        //Read File Name
        if (line.contains("title=\"Download ")) {
            QString fileName = line.split("title=\"Download ").at(1).split("\">").first();
            downloadLinkData.fileName = this->Fix_File_Name(fileName, downloadLinkData.numPlayers, downloadLinkData.fileType);
        }

        //Read Download Link
        if (line.contains("<a href=\"nibbler://sc.nibbits.com/maps/get/")) {
            QStringList strings = line.split("<a href=\"nibbler://sc.nibbits.com");
            for (QString string : strings) {
                if (string.startsWith("/maps/get")) {
                    downloadLinkData.downloadLink = STRING_BASE_URL+string.split("\"").first();
                    this->downloadLinks->append(downloadLinkData);
                    downloadLinkData.numPlayers = 0;
                    downloadLinkData.fileName = QString();
                    downloadLinkData.fileType = QString();
                    downloadLinkData.downloadLink = QString();
                }
            }
        }
    }

    if (!this->downloadLinks->isEmpty()) {
        this->Process_Next_Download_Link();
    } else {
        qInfo() << "";
        qInfo() << "Downloads complete!";
    }
}

void Downloader::Map_Download_Finished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    if (reply->error() != QNetworkReply::NoError) return;
    QString locationRedirect = reply->header(QNetworkRequest::LocationHeader).toString();
    if (!locationRedirect.isEmpty()) {
        qInfo().noquote() << locationRedirect;
        QString locationRedirectLower = locationRedirect.toLower();
        if (locationRedirectLower.endsWith(".scm") || locationRedirectLower.endsWith(".scx")) {
            this->downloadLinks->front().fileName = this->Fix_File_Name(locationRedirect.split("/").last(), this->downloadLinks->front().numPlayers, this->downloadLinks->front().fileType); //update the file name
        }
        qInfo() << "Downloading" << this->downloadLinks->front().fileName;
        QNetworkReply *reply = this->manager->get(QNetworkRequest(QUrl(locationRedirect)));
        connect(reply, &QNetworkReply::finished, this, &Downloader::Map_Download_Finished);
        return;
    }

    QByteArray data = reply->readAll();
    QFile file(this->downloadFolder+"/"+this->downloadLinks->front().fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate) || file.write(data) != data.size()) {
        file.close();
        file.remove();
        qCritical() << "Unable to write" << this->downloadLinks->front().fileName << "!";
        return;
    }
    file.close();

    this->downloadLinks->pop_front();
    if (!this->downloadLinks->isEmpty()) {
        this->Process_Next_Download_Link();
    } else {
        if (this->Get_Next_Page()) this->Download_Maps();
    }
}

void Downloader::Network_Accessible_Changed(QNetworkAccessManager::NetworkAccessibility accessible) {
    Q_UNUSED(accessible);
    qInfo() << "Network_Accessible_Changed";
}

void Downloader::SSL_Errors(QNetworkReply *reply, const QList<QSslError> &errors) {
    Q_UNUSED(reply);
    Q_UNUSED(errors);
    qInfo() << "SSL_Errors";
}

void Downloader::Process_Next_Download_Link() {
    assert(!this->downloadLinks->isEmpty());
    qInfo() << "";
    qInfo().noquote() << this->downloadLinks->front().downloadLink;

    QNetworkReply *reply = this->manager->get(QNetworkRequest(QUrl(this->downloadLinks->front().downloadLink)));
    connect(reply, &QNetworkReply::finished, this, &Downloader::Map_Download_Finished);
}

bool Downloader::Get_Next_Page() {
    bool valid = false;
    int page = this->currentPageURL.split("&p=").last().split("&").first().toInt(&valid);
    if (!valid) return false;
    this->currentPageURL.remove("&p="+QString::number(page));
    this->currentPageURL += "&p="+QString::number(page+1);
    qInfo() << "";
    qInfo() << "********************";
    qInfo().noquote() << "Page:" << page+1;
    qInfo() << "********************";
    return true;
}

QString Downloader::Fix_File_Name(QString fileName, int numPlayers, const QString &fileType) {
    //Remove whitespace at the beginning of the file name
    while (fileName.startsWith(" ")) fileName.remove(0, 1);

    //Remove the old player count
    if (fileName.size() > 3 && numPlayers > 0) {
        if (fileName.startsWith("(") && fileName.at(2) == ")") {
            bool valid = false;
            QString(fileName.at(1)).toInt(&valid);
            if (valid) fileName.remove(0, 3);
        }
    }

    //Remove whitespace at the beginning of the file name again
    while (fileName.startsWith(" ")) fileName.remove(0, 1);

    //Fix the file extension
    QString fileNameLower = fileName.toLower();
    if (fileNameLower.endsWith(".scx") || fileNameLower.endsWith(".scm")) fileName.chop(4);
    if (numPlayers > 0) fileName = this->htmlStringHelper->Convert_From_HTML_Name("("+QString::number(numPlayers)+")"+fileName+"."+fileType);
    else fileName = this->htmlStringHelper->Convert_From_HTML_Name(fileName+"."+fileType);
    return fileName;
}
