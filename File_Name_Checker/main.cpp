#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    if (argc != 2) {
        qInfo() << argv[0] << "<directory>";
        return 0;
    }

    //Read the source and compare directories
    QString directory = QString(argv[1]);
    if (!QDir(directory).exists()) {
        qCritical() << "Source directory does not exist!";
        return 1;
    }
    directory.replace("\\", "/");

    QFile file(a.applicationDirPath()+"/dump.txt");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qCritical() << "Unable to open dump.txt for writing!";
        return 1;
    }
    QTextStream stream(&file);
    int numFiles = 0;
    QDirIterator iter(directory, QStringList() << "*.scm" << "*.scx", QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        QString filePath = iter.next();
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        if (fileName.size() > 31) {
            int amount = fileName.size()-31;
            QString output = "\"" + fileName + "\" by " + QString::number(amount) + " characters";
            qInfo().noquote() << output;
            stream << output << endl;
            ++numFiles;
        }
    }
    QString output = "Number of files affected: " + QString::number(numFiles);
    qInfo().noquote() << output;
    qInfo() << "Results stored in dump.txt";
    stream << output << endl;
    stream.flush();
    file.close();
    return 0;
}
