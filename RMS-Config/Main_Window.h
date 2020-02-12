#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Main_Window; }
QT_END_NAMESPACE

class Settings_Manager;

class Main_Window : public QDialog {
    Q_OBJECT

public:
    Main_Window(QWidget *parent, const QString &applicationLocation);
    ~Main_Window();

private slots:
    void on_sbMinNumberOfPlayers_valueChanged(int arg1);
    void on_sbMaxNumberOfPlayers_valueChanged(int arg1);
    void on_btnAddFolder_clicked();
    void on_btnRemove_clicked();
    void on_btnSaveAndRun_clicked();
    void on_btnSaveAndClose_clicked();
    void on_btnClose_clicked();
    void on_btnOutputFolder_clicked();
    void on_cbCheckPlayerCount_toggled(bool checked);
    void on_Main_Window_finished();

private:
    bool Save();
    bool Load();
    void Enable_Player_Number_Check(bool enabled);
    QString Get_Starting_Folder();
    void Show_Save_Failed_Message();

    Ui::Main_Window *ui;
    Settings_Manager *settingsManager;
    QString applicationLocation;
    bool saveSettingsOnClose;
    bool instanceRunning;
};
#endif // MAIN_WINDOW_H
