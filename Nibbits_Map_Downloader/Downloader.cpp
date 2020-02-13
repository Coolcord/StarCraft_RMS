#include "Downloader.h"
#include <QTextStream>

const static QString STRING_BASE_URL = "http://sc.nibbits.com";

Downloader::Downloader(QObject *parent, const QString &startingURL, const QString &downloadFolder) : QObject(parent) {
    this->parent = parent;
    this->startingURL = startingURL;
    this->currentURL = startingURL;
    this->downloadFolder = downloadFolder;
    this->downloadLinks = QStringList();
    this->fileNames = QStringList();
    this->manager = new QNetworkAccessManager(parent);
    connect(this->manager, &QNetworkAccessManager::networkAccessibleChanged, this, &Downloader::Network_Accessible_Changed);
    connect(this->manager, &QNetworkAccessManager::sslErrors, this, &Downloader::SSL_Errors);
}

Downloader::~Downloader() {
    delete this->manager;
    this->manager = nullptr;
}

void Downloader::Download_Maps() {
    if (!this->currentURL.startsWith(STRING_BASE_URL) || !this->currentURL.contains("&p=")) {
        qCritical() << "Invalid URL!";
        return;
    }
    QNetworkReply *reply = this->manager->get(QNetworkRequest(QUrl(this->currentURL)));
    connect(reply, &QNetworkReply::finished, this, &Downloader::Get_Download_Links);
}

void Downloader::Get_Download_Links() {
    QStringList foundURLs;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    if (reply->error() != QNetworkReply::NoError) return;
    QByteArray data = reply->readAll();
    QTextStream stream(&data);
    int lastMaxPlayers = 0;
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
                    foundURLs.append(downloadURL);
                    this->downloadLinks.append(downloadURL);
                }
            }
        }

        //Read File Name
        if (line.contains("title=\"Download ")) {
            QStringList strings = line.split("title=\"Download ").last().split("\">").at(1).split("</a>").first().split(" (");
            QString fileName;
            for (int i = 0; i < strings.size()-1; ++i) {
                fileName += strings.at(i)+" (";
            }
            fileName.chop(2);

            //Add the player count
            if (fileName.size() > 3) {
                if (fileName.startsWith("(") && fileName.at(2) == ")") {
                    bool valid = false;
                    QString(fileName.at(1)).toInt(&valid);
                    if (valid) fileName.remove(0, 3);
                }
            }
            fileName = "("+QString::number(lastMaxPlayers)+")"+fileName;
            this->fileNames.append(fileName);
        }
    }

    if (foundURLs.isEmpty()) {
        this->Process_Download_Links();
    } else {
        for (QString url : foundURLs) { qInfo() << url; }
        if (this->Get_Next_Page()) this->Download_Maps();
    }
}

void Downloader::Process_Download_Links() {
    if (this->downloadLinks.size() != this->fileNames.size()) {
        qCritical() << "Some download links did not read as expected! Aborting downloads!";
        return;
    }

    //TODO: WRITE THIS!!!
    qInfo() << "Processing download links... this isn't completed yet!";
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

bool Downloader::Get_Next_Page() {
    bool valid = false;
    int page = this->currentURL.split("&p=").last().split("&").first().toInt(&valid);
    if (!valid) return false;
    this->currentURL.remove("&p="+QString::number(page));
    this->currentURL += "&p="+QString::number(page+1);
    return true;
}
