#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QString>
#include <QStringList>

class Readable_Config_File;

class Settings_Manager {
public:
    Settings_Manager(const QString &applicationLocation);
    ~Settings_Manager();
    bool Save();
    bool Load();
    void Load_Defaults();

    QStringList Get_Directories();
    QString Get_Output_Folder();
    QString Get_Last_Path_For_Get_Next_Map();
    bool Get_Get_Next_Map_Instead_Of_Random();
    bool Get_Check_Player_Count();
    int Get_Number_Of_Random_Maps();
    int Get_Min_Number_Of_Players();
    int Get_Max_Number_Of_Players();

    void Set_Directories(const QStringList &directories);
    void Set_Output_Folder(const QString &outputFolder);
    void Set_Last_Path_For_Get_Next_Map(const QString &lastPathForGetNextMap);
    void Set_Get_Next_Map_Instead_Of_Random(bool getNextMapInsteadOfRandom);
    void Set_Check_Player_Count(bool checkPlayerCount);
    void Set_Number_Of_Random_Maps(int numberOfRandomMaps);
    void Set_Min_Number_Of_Players(int minNumberOfPlayers);
    void Set_Max_Number_Of_Players(int maxNumberOfPlayers);

private:
    Readable_Config_File *configFile;
    QString applicationLocation;
    QString configFileLocation;
    QStringList directories;
    QString outputFolder;
    QString lastPathForGetNextMap;
    bool getNextMapInsteadOfRandom;
    bool checkPlayerCount;
    int numberOfRandomMaps;
    int minNumberOfPlayers;
    int maxNumberOfPlayers;
};

#endif // SETTINGS_MANAGER_H
