#include "Settings_Manager.h"
#include "../../C_Common_Code/Qt/Readable_Config_File/Readable_Config_File.h"

Settings_Manager::Settings_Manager(const QString &applicationLocation) {
    this->applicationLocation = applicationLocation;
    this->configFileLocation = applicationLocation+"/RMS.cfg";
    this->configFile = new Readable_Config_File();
    this->Load_Defaults();
}

Settings_Manager::~Settings_Manager() {
    this->configFile->Discard_And_Close();
    delete this->configFile;
    this->configFile = nullptr;
}

bool Settings_Manager::Save() {
    if (!this->configFile->Open_Without_Loading(this->configFileLocation)) return false;
    QString directoryList;
    for (QString directory : this->directories) {
        if (!directory.isEmpty()) directoryList += directory+";";
    }
    directoryList.chop(1);

    if (!this->configFile->Set_Value("Directories", directoryList)) return false;
    if (!this->configFile->Set_Value("Output_Folder", this->outputFolder)) return false;
    if (!this->configFile->Set_Value("Last_Path_For_Get_Next_Map", this->lastPathForGetNextMap)) return false;
    if (!this->configFile->Set_Value("Get_Next_Map_Instead_Of_Random", this->getNextMapInsteadOfRandom)) return false;
    if (!this->configFile->Set_Value("Check_Player_Count", this->checkPlayerCount)) return false;
    if (!this->configFile->Set_Value("Number_Of_Random_Maps", this->numberOfRandomMaps)) return false;
    if (!this->configFile->Set_Value("Min_Number_Of_Players", this->minNumberOfPlayers)) return false;
    if (!this->configFile->Set_Value("Max_Number_Of_Players", this->maxNumberOfPlayers)) return false;
    return this->configFile->Save_And_Close();
}

bool Settings_Manager::Load() {
    this->Load_Defaults();
    if (!this->configFile->Open(this->configFileLocation)) return false;
    QString directoryList;
    this->configFile->Get_Value("Directories", directoryList);
    this->configFile->Get_Value("Output_Folder", this->outputFolder);
    this->configFile->Get_Value("Last_Path_For_Get_Next_Map", this->lastPathForGetNextMap);
    this->configFile->Get_Value("Get_Next_Map_Instead_Of_Random", this->getNextMapInsteadOfRandom);
    this->configFile->Get_Value("Check_Player_Count", this->checkPlayerCount);
    this->configFile->Get_Value("Number_Of_Random_Maps", this->numberOfRandomMaps);
    this->configFile->Get_Value("Min_Number_Of_Players", this->minNumberOfPlayers);
    this->configFile->Get_Value("Max_Number_Of_Players", this->maxNumberOfPlayers);
    this->directories = directoryList.split(';');
    return true;
}

void Settings_Manager::Load_Defaults() {
    this->directories = QStringList();
    this->outputFolder = QString();
    this->lastPathForGetNextMap = QString();
    this->getNextMapInsteadOfRandom = false;
    this->checkPlayerCount = true;
    this->numberOfRandomMaps = 0;
    this->minNumberOfPlayers = 2;
    this->maxNumberOfPlayers = 4;
}

QStringList Settings_Manager::Get_Directories() {
    return this->directories;
}

QString Settings_Manager::Get_Output_Folder() {
    return this->outputFolder;
}

QString Settings_Manager::Get_Last_Path_For_Get_Next_Map() {
    return this->lastPathForGetNextMap;
}

bool Settings_Manager::Get_Get_Next_Map_Instead_Of_Random() {
    return this->getNextMapInsteadOfRandom;
}

bool Settings_Manager::Get_Check_Player_Count() {
    return this->checkPlayerCount;
}

int Settings_Manager::Get_Number_Of_Random_Maps() {
    return this->numberOfRandomMaps;
}

int Settings_Manager::Get_Min_Number_Of_Players() {
    return this->minNumberOfPlayers;
}

int Settings_Manager::Get_Max_Number_Of_Players() {
    return this->maxNumberOfPlayers;
}

void Settings_Manager::Set_Directories(const QStringList &directories) {
    this->directories = directories;
}

void Settings_Manager::Set_Output_Folder(const QString &outputFolder) {
    this->outputFolder = outputFolder;
}

void Settings_Manager::Set_Last_Path_For_Get_Next_Map(const QString &lastPathForGetNextMap) {
    this->lastPathForGetNextMap = lastPathForGetNextMap;
}

void Settings_Manager::Set_Get_Next_Map_Instead_Of_Random(bool getNextMapInsteadOfRandom) {
    this->getNextMapInsteadOfRandom = getNextMapInsteadOfRandom;
}

void Settings_Manager::Set_Check_Player_Count(bool checkPlayerCount) {
    this->checkPlayerCount = checkPlayerCount;
}

void Settings_Manager::Set_Number_Of_Random_Maps(int numberOfRandomMaps) {
    this->numberOfRandomMaps = numberOfRandomMaps;
}

void Settings_Manager::Set_Min_Number_Of_Players(int minNumberOfPlayers) {
    this->minNumberOfPlayers = minNumberOfPlayers;
}

void Settings_Manager::Set_Max_Number_Of_Players(int maxNumberOfPlayers) {
    this->maxNumberOfPlayers = maxNumberOfPlayers;
}
