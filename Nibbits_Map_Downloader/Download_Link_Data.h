#ifndef DOWNLOAD_LINK_DATA_H
#define DOWNLOAD_LINK_DATA_H

#include <QString>

struct Download_Link_Data {
    QString downloadLink;
    QString fileName;
    QString fileType;
    bool validForMeleeMode;
    int numPlayers;
};

#endif // DOWNLOAD_LINK_DATA_H
