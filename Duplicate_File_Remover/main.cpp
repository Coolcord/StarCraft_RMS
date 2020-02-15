#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QString>

void Increment_Num_Files_Processed(int &numFilesProcessed) {
    ++numFilesProcessed;
    if (numFilesProcessed%1000 == 0 && numFilesProcessed > 0) {
        qInfo().noquote() << "Processed" << numFilesProcessed << "files...";
    }
}

QString Get_Checksum(const QString &fileLocation) {
    QFile file(fileLocation);
    if (!file.exists()) return QString();
    if (!file.open(QIODevice::ReadOnly)) return QString();
    QString checksum = QString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha512).toHex().toUpper());
    file.close();
    return checksum;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    if (argc != 3) {
        qInfo() << argv[0] << "<source_directory>" << "<compare_directory>";
        qInfo() << "Files will only be deleted from the compare directory.";
        qInfo() << "Files in the source directory will be deleted if the source directory is in the compare directory";
        return 0;
    }

    //Read the source and compare directories
    QString sourceDirectory = QString(argv[1]);
    QString compareDirectory = QString(argv[2]);
    if (!QDir(sourceDirectory).exists()) {
        qCritical() << "Source directory does not exist!";
        return 1;
    }
    if (!QDir(compareDirectory).exists()) {
        qCritical() << "Compare directory does not exist!";
        return 1;
    }
    sourceDirectory.replace("\\", "/");
    compareDirectory.replace("\\", "/");

    //Get the checksums of everything in the source directory
    qInfo() << "Scanning source directory...";
    qInfo().noquote() << sourceDirectory;
    QSet<QString> checksums;
    int numFilesProcessed = -1;
    QDirIterator sourceFilesIter(sourceDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (sourceFilesIter.hasNext()) {
        QString fileLocation = sourceFilesIter.next();
        if (QFileInfo(fileLocation).path().contains(compareDirectory)) continue;
        Increment_Num_Files_Processed(numFilesProcessed);
        QString checksum = Get_Checksum(fileLocation);
        if (!checksum.isEmpty()) checksums.insert(checksum);
    }

    //Find and delete duplicates in the compare directory
    qInfo() << "";
    qInfo() << "Scanning compare directory...";
    qInfo().noquote() << compareDirectory;
    QDirIterator compareDirectoryIter(compareDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (compareDirectoryIter.hasNext()) {
        QString fileLocation = compareDirectoryIter.next();
        Increment_Num_Files_Processed(numFilesProcessed);
        QString checksum = Get_Checksum(fileLocation);
        if (checksum.isEmpty()) continue;
        if (checksums.contains(checksum)) {
            qInfo() << "Removing" << QFileInfo(fileLocation).fileName();
            QFile(fileLocation).remove();
        } else {
            checksums.insert(checksum);
        }
    }

    qInfo() << "";
    qInfo() << "All done!";
    return 0;
}
