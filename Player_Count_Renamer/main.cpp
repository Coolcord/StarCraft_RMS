#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QProcess>

const static QString STRING_TMP_DIR_NAME = "playerCountRenamerTMP";

void Increment_Num_Files_Processed(int &numFilesProcessed) {
    ++numFilesProcessed;
    if (numFilesProcessed%1000 == 0 && numFilesProcessed > 0) {
        qInfo().noquote() << "Processed" << numFilesProcessed << "files...";
    }
}

bool Copy_File(const QString &sourceFile, const QString &destinationFolder) {
    QFile file(sourceFile);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QString fileName = QFileInfo(sourceFile).fileName();
    QFile newFile(destinationFolder+"/"+fileName);
    if (!newFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) { file.close(); return false; }
    bool success = newFile.write(file.readAll()) == file.size();
    newFile.close();
    file.close();
    return success;
}

bool Recreate_TMP_Directory(const QString &applicationLocation) {
    QDir(applicationLocation+"/"+STRING_TMP_DIR_NAME).removeRecursively();
    return QDir(applicationLocation).mkdir(STRING_TMP_DIR_NAME);
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    //Read the directory provided by the user
    if (argc != 2) {
        qInfo() << argv[0] << "<directory>";
        return 0;
    }
    QString directory = argv[1];
    if (!QDir(directory).exists()) {
        qCritical() << "The specified directory does not exist!";
        return 1;
    }
    QString mpqEditorLocation = a.applicationDirPath()+"/MPQEditor.exe";
    if (!QFileInfo(mpqEditorLocation).exists()) {
        qCritical() << "Ladik's MPQ Editor is required!";
        qCritical() << "";
        qCritical() << "MPQEditor.exe must be placed in the same directory as" << argv[0];
        return 1;
    }

    //Make a tmp directory to extract files to
    QString applicationLocation = a.applicationDirPath();
    Recreate_TMP_Directory(applicationLocation);
    QString tmpDir = applicationLocation+"/"+STRING_TMP_DIR_NAME;

    int numFilesProcessed = -1;
    QByteArray searchData = QByteArray::fromHex(QString("4F574E520C000000").toLatin1());
    QDirIterator filesIter(directory, QDir::Files, QDirIterator::Subdirectories);
    while (filesIter.hasNext()) {
        Increment_Num_Files_Processed(numFilesProcessed);

        //Copy the map to the tmp directory
        QString fileLocation = filesIter.next();
        QFileInfo fileInfo = QFileInfo(fileLocation);
        QString filePath = fileInfo.path();
        QString fileName = fileInfo.fileName();
        if (!Recreate_TMP_Directory(applicationLocation)) { qCritical() << "Unable to create the tmp directory!"; return 1; }
        if (!Copy_File(fileLocation, tmpDir)) { qCritical() << "Unable to copy" << fileName << "to the tmp directory!"; return 1; }

        //Extract the map files
        QProcess process;
        QStringList arguments;
        process.setWorkingDirectory(tmpDir);
        process.setProgram(mpqEditorLocation);
        arguments << "extract" << tmpDir+"/"+fileName;
        process.setArguments(arguments);
        process.start(process.program(), process.arguments());
        process.waitForFinished(-1);

        //Find the "OWNR" section in the scenario file
        QString scenarioLocation = tmpDir+"/scenario.chk";
        if (!QFileInfo(scenarioLocation).exists()) scenarioLocation = tmpDir+"/File00000000.xxx";
        if (!QFileInfo(scenarioLocation).exists()) scenarioLocation = tmpDir+"/File00000001.xxx";
        if (!QFileInfo(scenarioLocation).exists()) scenarioLocation = tmpDir+"/File00000002.xxx";
        if (!QFileInfo(scenarioLocation).exists()) scenarioLocation = tmpDir+"/File00000003.xxx";
        if (!QFileInfo(scenarioLocation).exists()) scenarioLocation = tmpDir+"/File00000004.xxx";
        QFile scenario(scenarioLocation);
        if (!scenario.open(QIODevice::ReadOnly)) { qCritical() << "Unable to open the scenario file for" << fileName << "!"; continue; }
        QByteArray bytes = scenario.readAll();
        scenario.close();
        int ownerSection = -1;
        for (int i = 0, j = 0; i < bytes.size() && ownerSection < 0; ++i) {
            if (bytes.at(i) == searchData.at(j)) {
                ++j;
                if (j == searchData.size()) ownerSection = i+1;
            } else {
                j = 0;
            }
        }
        if (ownerSection == -1 || ownerSection+11 > bytes.size()) {
            qCritical() << "Unable to locate the OWNR section in" << fileName << "!"; continue;
        }

        //Read the number of players
        int numPlayers = 0;
        const char HUMAN_PLAYER_BYTE = static_cast<char>(0x06);
        for (int i = 0; i < 12; ++i) {
            if (bytes.at(ownerSection+i) == HUMAN_PLAYER_BYTE) ++numPlayers;
        }

        //Remove the player count from the old file name
        QString playerCount = "("+QString::number(numPlayers)+")";
        while (fileName.size() > 3 && fileName.at(0).toLatin1() == '(' && fileName.at(2).toLatin1() == ')') {
            bool isNumber = false;
            QString(fileName.at(1)).toInt(&isNumber, 10);
            if (!isNumber) break;
            fileName.remove(0, 3); //drop the first 3 characters of the name
        }

        //Rename the map file if necessary
        QString newFileName = playerCount+fileName;
        if (fileName != newFileName) {
            qInfo() << "Adding player count to" << fileName;
            QString newFileName = playerCount+fileName;
            if (!QFile::rename(fileLocation, filePath+"/"+newFileName)) {
                qCritical() << "Cannot rename" << fileName;
                return 1;
            }
        }
    }

    QDir(tmpDir).removeRecursively();
    qInfo() << "";
    qInfo() << "All done!";
    return 0;
}
