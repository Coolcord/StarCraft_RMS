#include <QCoreApplication>
#include "Downloader.h"
#include <QString>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    if (argc != 3) {
        qInfo().noquote() << argv[0] << "<url> <download folder>";
        qInfo().noquote() << "The URL MUST end with &p=# where # is a number!";
        return 1;
    }

    Downloader downloader(nullptr, argv[1], argv[2]);
    qInfo() << "Requesting links...";
    downloader.Download_Maps();

    return a.exec();
}
