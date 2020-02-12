#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>

const static int MAX_BATCH_SIZE = 200;

bool Flatten_Directory(const QString &directory) {
    QDirIterator iter(directory, QStringList() << "*.scm" << "*.scx", QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        QString filePath = iter.next();
        QString newPath = directory+"/"+QFileInfo(filePath).fileName();
        if (filePath != newPath) {
            QFile(newPath).remove();
            QFile file(filePath);
            if (!file.rename(newPath)) return false;
        }
    }
    QDirIterator dirIter(directory, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIter.hasNext()) {
        QString dirPath = dirIter.next();
        if (dirPath != directory) QDir(dirPath).removeRecursively();
    }
    return true;
}

bool Move_Next_Batch_Into_Folder(const QString &directory, const QString &newDirectory, int &numMoved) {
    QDirIterator iter(directory, QStringList() << "*.scm" << "*.scx", QDir::Files);
    numMoved = 0;
    while (iter.hasNext() && numMoved < MAX_BATCH_SIZE) {
        QString filePath = iter.next();
        QString newPath = newDirectory+"/"+QFileInfo(filePath).fileName();
        if (filePath != newPath) {
            QFile(newPath).remove();
            QFile file(filePath);
            if (!file.rename(newPath)) return false;
            ++numMoved;
        }
    }
    return true;
}

bool Sort_Folder_Into_Pages(const QString &directory) {
    QDir dir(directory);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    if (dir.count() < MAX_BATCH_SIZE) return true; //nothing to do
    int numMoved = 0;
    for (int i = 1; i < 100; ++i) {
        QString newDirName;
        if (i < 10) newDirName = "Page 0"+QString::number(i);
        else newDirName = "Page "+QString::number(i);
        QString newDir = directory+"/"+newDirName;
        QDir(directory).mkdir(newDirName);
        if (!Move_Next_Batch_Into_Folder(directory, directory+"/"+newDirName, numMoved)) {
            qCritical().noquote() << "Sort Failed!";
            return false;
        }
        if (numMoved == 0) {
            QDir(newDir).removeRecursively();
            break;
        }
    }
    return true;
}

bool Is_File_Name_A_Number(QString fileName) {
    bool isNumber = false;
    fileName = fileName.toLower();
    QString(fileName.at(0)).toInt(&isNumber);
    if (!isNumber && fileName.at(0).toLatin1() == '(' && fileName.size() > 1) {
        QString(fileName.at(1)).toInt(&isNumber);
    } else {
        isNumber = false;
    }
    return isNumber;
}

bool Sort_Folder_By_Letter(const QString &directory, const QString &letter) {
    qInfo() << "Sorting folder by letter" << letter;
    QDir(directory).mkdir("_"+letter+"_");
    QString newDirectory = directory+"/_"+letter+"_";
    QDirIterator iter(directory, QStringList() << "*.scm" << "*.scx", QDir::Files);
    char c = letter.toLower().at(0).toLatin1();
    while (iter.hasNext()) {
        QString filePath = iter.next();
        if (filePath.isEmpty()) continue;
        QString fileNameLower = QFileInfo(filePath).fileName().toLower();
        switch (c) {
        default:
            if (!fileNameLower.startsWith(letter.toLower())) continue;
            break;
        case '0':
            if (!Is_File_Name_A_Number(fileNameLower)) continue;
            break;
        case '!':
            break;
        }
        QString newPath = newDirectory+"/"+QFileInfo(filePath).fileName();
        if (filePath != newPath) {
            QFile(newPath).remove();
            QFile file(filePath);
            if (!file.rename(newPath)) return false;
        }
    }
    return Sort_Folder_Into_Pages(newDirectory);
}

bool Sort_Folder_By_Letters(const QString &directory) {
    if (!Sort_Folder_By_Letter(directory, "A")) return false;
    if (!Sort_Folder_By_Letter(directory, "B")) return false;
    if (!Sort_Folder_By_Letter(directory, "C")) return false;
    if (!Sort_Folder_By_Letter(directory, "D")) return false;
    if (!Sort_Folder_By_Letter(directory, "E")) return false;
    if (!Sort_Folder_By_Letter(directory, "F")) return false;
    if (!Sort_Folder_By_Letter(directory, "G")) return false;
    if (!Sort_Folder_By_Letter(directory, "H")) return false;
    if (!Sort_Folder_By_Letter(directory, "I")) return false;
    if (!Sort_Folder_By_Letter(directory, "J")) return false;
    if (!Sort_Folder_By_Letter(directory, "K")) return false;
    if (!Sort_Folder_By_Letter(directory, "L")) return false;
    if (!Sort_Folder_By_Letter(directory, "M")) return false;
    if (!Sort_Folder_By_Letter(directory, "N")) return false;
    if (!Sort_Folder_By_Letter(directory, "O")) return false;
    if (!Sort_Folder_By_Letter(directory, "P")) return false;
    if (!Sort_Folder_By_Letter(directory, "Q")) return false;
    if (!Sort_Folder_By_Letter(directory, "R")) return false;
    if (!Sort_Folder_By_Letter(directory, "S")) return false;
    if (!Sort_Folder_By_Letter(directory, "T")) return false;
    if (!Sort_Folder_By_Letter(directory, "U")) return false;
    if (!Sort_Folder_By_Letter(directory, "V")) return false;
    if (!Sort_Folder_By_Letter(directory, "W")) return false;
    if (!Sort_Folder_By_Letter(directory, "X")) return false;
    if (!Sort_Folder_By_Letter(directory, "Y")) return false;
    if (!Sort_Folder_By_Letter(directory, "Z")) return false;
    if (!Sort_Folder_By_Letter(directory, "0")) return false;
    if (!Sort_Folder_By_Letter(directory, "!")) return false;
    return true;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    //Show help
    if (argc != 3) {
        qInfo().noquote() << argv[0] << "<directoryToSort>" << "<useLettersFirst>";
        qInfo().noquote() << "<useLettersFirst> must be either true or false";
        return 0;
    }

    //Parse directory
    QString sortDirectory = argv[1];
    sortDirectory.replace('\\', '/');
    if (!QFileInfo(sortDirectory).exists()) {
        qCritical().noquote() << "The specified directory does not exist!";
        return 1;
    }

    //Parse Use Letters
    bool useLetters = false;
    QString useLettersFirst = argv[2];
    useLettersFirst = useLettersFirst.toLower();
    if (useLettersFirst == "true") {
        useLetters = true;
    } else if (useLettersFirst == "false") {
        useLetters = false;
    } else {
        qCritical().noquote() << "<useLettersFirst> must be either true or false!";
        return 1;
    }

    //Flatten the directory
    qInfo() << "Flattening directory...";
    if (!Flatten_Directory(sortDirectory)) {
        qCritical().noquote() << "Unable to flatten the directory!";
        return 1;
    }

    if (useLetters) {
        if (!Sort_Folder_By_Letters(sortDirectory)) qCritical().noquote() << "Sorting failed!";
    } else {
        qInfo() << "Sorting pages...";
        Sort_Folder_Into_Pages(sortDirectory);
    }

    return 0;
}
