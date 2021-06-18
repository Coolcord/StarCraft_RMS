#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QProcess>

const static bool ALLOW_FILE_DELETION = true;
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

bool Get_Number_Of_Players(int &numPlayers, const QString &scenarioLocation) {
    const QByteArray SEARCH_DATA = QByteArray::fromHex(QString("4F574E520C000000").toLatin1());

    QFile scenario(scenarioLocation);
    if (!scenario.open(QIODevice::ReadOnly)) return false;
    QByteArray bytes = scenario.readAll();
    scenario.close();
    int ownerSection = -1;
    for (int i = 0, j = 0; i < bytes.size() && ownerSection < 0; ++i) {
        if (bytes.at(i) == SEARCH_DATA.at(j)) {
            ++j;
            if (j == SEARCH_DATA.size()) {
                ownerSection = i+1;

                //Attempt to read this section
                if (ownerSection == -1 || ownerSection+11 > bytes.size()) return false;
                numPlayers = 0;
                const char HUMAN_PLAYER_BYTE = static_cast<char>(0x06);
                for (int k = 0; k < 12; ++k) {
                    if (bytes.at(ownerSection+k) == HUMAN_PLAYER_BYTE) ++numPlayers;
                }
                if (numPlayers > 0) {
                    return true; //read was successful!
                } else {
                    j = 0;
                    continue; //section is invalid! Just ignore it and continue!
                }
            }
        } else {
            j = 0;
        }
    }
    return false;
}

bool Get_Number_Of_Players(const QString &tmpDir, int &numPlayers) {
    numPlayers = -1;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/scenario.chk")) return true;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/File00000000.xxx")) return true;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/File00000001.xxx")) return true;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/File00000002.xxx")) return true;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/File00000003.xxx")) return true;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/File00000004.xxx")) return true;
    if (Get_Number_Of_Players(numPlayers, tmpDir+"/File00000005.xxx")) return true;
    numPlayers = -1;
    return false;
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

        int numPlayers = -1;
        if (!Get_Number_Of_Players(tmpDir, numPlayers) || numPlayers == -1) {
            qCritical() << "Unable to read the scenario file for" << fileName << "!";
            if (!fileName.startsWith("FAILED-")) QFile::rename(filePath+"/"+fileName, filePath+"/FAILED-"+fileName);
            continue;
        }

        //Remove the player count from the old file name
        QString oldFileName = fileName;
        QString playerCount = "("+QString::number(numPlayers)+")";
        while (fileName.size() > 4 && fileName.at(0).toLatin1() == '(') {
            if (fileName.at(3).toLatin1() == ')') {
                bool isNumber = false;
                QString(fileName.at(1)).toInt(&isNumber, 10);
                if (!isNumber) break;
                QString(fileName.at(2)).toInt(&isNumber, 10);
                if (!isNumber) break;
                fileName.remove(0, 4); //drop the first 4 characters of the name
            } else if (fileName.at(2).toLatin1() == ')') {
                bool isNumber = false;
                QString(fileName.at(1)).toInt(&isNumber, 10);
                if (!isNumber) break;
                fileName.remove(0, 3); //drop the first 3 characters of the name
            } else if (fileName.startsWith("FAILED-")) {
                fileName.remove(0, 7);
            } else {
                break;
            }
        }

        //Rename the map file if necessary
        QString newFileName = playerCount+fileName;
        if (oldFileName != newFileName) {
            qInfo() << "Adding player count to" << fileName;
            QString newFileName = playerCount+fileName;
            if (!QFile::rename(fileLocation, filePath+"/"+newFileName)) {
                if (ALLOW_FILE_DELETION) {
                    if (QFileInfo(filePath+"/"+newFileName).exists()) {
                        qCritical() << "Removing duplicate file" << fileName;
                        QFile::remove(fileLocation);
                    }
                } else {
                    qCritical() << "Unable to rename file" << fileName;
                    return 1;
                }
            }
        }
    }

    QDir(tmpDir).removeRecursively();
    qInfo() << "";
    qInfo() << "All done!";
    return 0;
}
