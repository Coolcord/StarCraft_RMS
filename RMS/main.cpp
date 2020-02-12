#include <QCoreApplication>
#include "../../C_Common_Code/Qt/Random/Random.h"
#include "Settings_Manager.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QTextStream>
#include <QTime>
#include <assert.h>

QString Read_Spot(const QString &outputFolder) {
    QFile file(outputFolder+"/RMS_spot.cfg");
    if (!file.open(QIODevice::ReadOnly)) return QString();
    QTextStream stream(&file);
    QString filePath = stream.readLine();
    file.close();
    return filePath;
}

bool Write_Spot(const QString &outputFolder, const QString &filePath) {
    QFile file(outputFolder+"/RMS_spot.cfg");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) return false;
    QTextStream stream(&file);
    stream << filePath << endl;
    stream.flush();
    file.close();
    return true;
}

bool Copy_Map_Into_Output_Folder(const QString &inputFilePath, const QString &outputFolder) {
    QFile inputFile(inputFilePath);
    int fileSize = inputFile.size();
    if (!inputFile.open(QIODevice::ReadOnly)) return false;
    QString fileName = QFileInfo(inputFilePath).fileName();
    QFile outputFile(outputFolder+"/"+fileName);
    if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qCritical().noquote() << "Unable to open" << outputFolder+"/"+fileName;
        inputFile.close();
        return false;
    }
    if (outputFile.write(inputFile.readAll()) != fileSize) {
        qCritical().noquote() << "Unable to write" << outputFolder+"/"+fileName;
        inputFile.close();
        outputFile.close();
        outputFile.remove();
        return false;
    }
    inputFile.close();
    outputFile.close();
    qInfo().noquote() << fileName;
    if (!Write_Spot(outputFolder, inputFilePath)) {
        qCritical().noquote() << "Unable to write spot!";
        return true; //copy succeeded, but spot write didn't. Just treat this as a warning
    }
    return true;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Random::Get_Instance().Seed(QString::number(QTime::currentTime().msecsSinceStartOfDay()), 1);
    Settings_Manager settings(a.applicationDirPath());
    settings.Load();
    if (settings.Get_Directories().isEmpty()) {
        qCritical().noquote() << "There are no directories to scan!";
        return 1; //nothing to do
    }

    //Get the Output Folder and clear all previous Random Maps
    QString outputFolder = settings.Get_Output_Folder();
    if (outputFolder.isEmpty() || !QFileInfo(outputFolder).exists()) {
        qCritical().noquote() << "Output folder does not exist!";
        return 1;
    }
    QDirIterator outputIter(settings.Get_Output_Folder(), QStringList() << "*.scm" << "*.scx", QDir::Files, QDirIterator::Subdirectories);
    while (outputIter.hasNext()) QFile(outputIter.next()).remove();

    //Get the Possible Maps
    QStringList possibleMaps;
    QStringList directoriesToScan = settings.Get_Directories();
    QSet<QString> scannedMaps;
    for (QString directory : directoriesToScan) {
        if (directory == outputFolder) continue; //skip the output folder
        QDirIterator filesIter(directory, QStringList() << "*.scm" << "*.scx", QDir::Files, QDirIterator::Subdirectories);
        while (filesIter.hasNext()) {
            QString filePath = filesIter.next();
            assert(!filePath.isEmpty());
            if (scannedMaps.contains(filePath)) continue; //avoid duplicates
            else scannedMaps.insert(filePath);
            if (settings.Get_Check_Player_Count()) {
                //File name must start with "(#)" where # is a number
                QString fileName = filesIter.fileName();
                if (fileName.size() < 3) continue;
                if (!fileName.startsWith("(")) continue;
                bool validNumber = false;
                int numPlayers = QString(fileName.at(1)).toInt(&validNumber);
                if (!validNumber) continue;
                if (numPlayers < settings.Get_Min_Number_Of_Players()) continue;
                if (numPlayers > settings.Get_Max_Number_Of_Players()) continue;
                if (fileName.at(2) != ')') continue;
                possibleMaps.append(filePath);
            } else {
                possibleMaps.append(filePath);
            }
        }
    }

    //Get Random Maps
    if (possibleMaps.isEmpty()) return 0; //nothing to do
    int numRandomMaps = 0;
    if (settings.Get_Get_Next_Map_Instead_Of_Random()) { //get next maps
        //Get the starting position
        int currentIndex = 0;
        QString lastFile = Read_Spot(outputFolder);
        if (!lastFile.isEmpty()) {
            currentIndex = -1;
            int index = 0;
            for (QStringList::iterator iter = possibleMaps.begin(); iter != possibleMaps.end() && currentIndex != -1; ++iter, ++index) {
                if (*iter == lastFile) currentIndex = index;
            }
            if (currentIndex == -1) currentIndex = 0; //not found, so just restart
        }

        //Copy the maps in order
        int startingIndex = currentIndex;
        bool seenStart = false;
        assert(currentIndex >= 0);
        while (numRandomMaps < settings.Get_Number_Of_Random_Maps()) {
            QString filePath = possibleMaps.at(currentIndex);
            if (Copy_Map_Into_Output_Folder(filePath, outputFolder)) ++numRandomMaps;
            ++currentIndex;
            if (currentIndex > possibleMaps.size()) currentIndex = 0;
            if (seenStart) break; //don't wrap around more than once
            if (currentIndex == startingIndex) seenStart = true;
        }
    } else { //get random maps
        while (numRandomMaps < settings.Get_Number_Of_Random_Maps() && !possibleMaps.isEmpty()) {
            //Choose a random map
            int index = Random::Get_Instance().Get_Num(0, possibleMaps.size()-1);
            QString filePath = possibleMaps.at(index);
            possibleMaps.removeAt(index);
            if (Copy_Map_Into_Output_Folder(filePath, outputFolder)) ++numRandomMaps;
        }
    }
    if (numRandomMaps > 0) {
        return 0;
    } else {
        qCritical().noquote() << "Nothing was selected!";
        return 1;
    }
}
