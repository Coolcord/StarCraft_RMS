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
    this->downloadLinks = QStringList();
    this->fileNames = QStringList();
    this->htmlStringHelper = new HTML_String_Helper();
    this->manager = new QNetworkAccessManager(parent);
    connect(this->manager, &QNetworkAccessManager::networkAccessibleChanged, this, &Downloader::Network_Accessible_Changed);
    connect(this->manager, &QNetworkAccessManager::sslErrors, this, &Downloader::SSL_Errors);
}

Downloader::~Downloader() {
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
    int lastMaxPlayers = 0;
    QString lastFileType = "scx";
    while (!stream.atEnd()) {
        //Read Max Players
        QString line = stream.readLine();
        if (line.contains("<span class=\"players\">(")) {
            bool valid = false;
            int tmp = 0;
            tmp = line.split("<span class=\"players\">(").last().split(")").first().toInt(&valid);
            if (valid) lastMaxPlayers = tmp;
        }

        //Read Download Link
        if (line.contains("<a href=\"nibbler://sc.nibbits.com/maps/get/")) {
            QStringList strings = line.split("<a href=\"nibbler://sc.nibbits.com");
            for (QString string : strings) {
                if (string.startsWith("/maps/get")) {
                    QString downloadURL = STRING_BASE_URL+string.split("\"").first();
                    this->downloadLinks.append(downloadURL);
                }
            }
        }

        //Read File Type
        if (line.contains("class=\"filetype-")) {
            lastFileType = line.split("class=\"filetype-").at(1).split("\"").first().toLower();
        }

        //Read File Name
        if (line.contains("title=\"Download ")) {
            QString fileName = line.split("title=\"Download ").at(1).split("\">").first();

            //Add the player count
            if (fileName.size() > 3) {
                if (fileName.startsWith("(") && fileName.at(2) == ")") {
                    bool valid = false;
                    QString(fileName.at(1)).toInt(&valid);
                    if (valid) fileName.remove(0, 3);
                }
            }

            //Remove whitespace at the beginning of the file name
            while (fileName.startsWith(" ")) {
                fileName.remove(0, 1);
            }

            //Fix the file extension
            QString fileNameLower = fileName.toLower();
            if (fileNameLower.endsWith(".scx") || fileNameLower.endsWith(".scm")) fileName.chop(4);
            fileName = this->htmlStringHelper->Convert_From_HTML_Name("("+QString::number(lastMaxPlayers)+")"+fileName+"."+lastFileType);
            this->fileNames.append(fileName);
        }
    }

    if (!this->downloadLinks.isEmpty()) {
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
        QNetworkReply *reply = this->manager->get(QNetworkRequest(QUrl(locationRedirect)));
        connect(reply, &QNetworkReply::finished, this, &Downloader::Map_Download_Finished);
        return;
    }

    QByteArray data = reply->readAll();
    QFile file(this->downloadFolder+"/"+this->fileNames.front());
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate) || file.write(data) != data.size()) {
        file.close();
        file.remove();
        qCritical() << "Unable to write" << this->fileNames.front() << "!";
        return;
    }
    file.close();

    this->downloadLinks.removeFirst();
    this->fileNames.pop_front();
    if (!this->downloadLinks.isEmpty()) {
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
    if (this->downloadLinks.size() != this->fileNames.size()) {
        qCritical() << "Some download links did not read as expected on the following page:";
        qCritical() << this->currentPageURL;
        qCritical() << "";
        qCritical() << "Aborting downloads!";
        this->downloadLinks.clear();
        this->fileNames.clear();
        return;
    }
    assert(!this->downloadLinks.isEmpty() && !this->fileNames.isEmpty());
    qInfo() << "";
    qInfo().nospace() << "Downloading " << this->fileNames.front();
    qInfo().noquote() << this->downloadLinks.front();

    QNetworkReply *reply = this->manager->get(QNetworkRequest(QUrl(this->downloadLinks.front())));
    connect(reply, &QNetworkReply::finished, this, &Downloader::Map_Download_Finished);
}

bool Downloader::Get_Next_Page() {
    bool valid = false;
    int page = this->currentPageURL.split("&p=").last().split("&").first().toInt(&valid);
    if (!valid) return false;
    this->currentPageURL.remove("&p="+QString::number(page));
    this->currentPageURL += "&p="+QString::number(page+1);
    return true;
}
