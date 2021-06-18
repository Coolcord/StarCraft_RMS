#include "Main_Window.h"
#include "ui_Main_Window.h"
#include "../RMS/Settings_Manager.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

const QString STRING_WINDOW_TITLE = "StarCraft RMS";
const QString STRING_WINDOW_TITLE_SHORT = "RMS";

Main_Window::Main_Window(QWidget *parent, const QString &applicationLocation)
    : QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::Main_Window)
{
    this->applicationLocation = applicationLocation;
    ui->setupUi(this);
    this->setWindowTitle(STRING_WINDOW_TITLE);
    this->saveSettingsOnClose = true;
    this->instanceRunning = false;
    this->settingsManager = new Settings_Manager(applicationLocation);
    this->Load();
}

Main_Window::~Main_Window() {
    delete this->settingsManager;
    delete ui;
}

void Main_Window::on_sbMinNumberOfPlayers_valueChanged(int arg1) {
    if (arg1 > this->ui->sbMaxNumberOfPlayers->value()) this->ui->sbMaxNumberOfPlayers->setValue(arg1);
}

void Main_Window::on_sbMaxNumberOfPlayers_valueChanged(int arg1) {
    if (arg1 < this->ui->sbMinNumberOfPlayers->value()) this->ui->sbMinNumberOfPlayers->setValue(arg1);
}

void Main_Window::on_btnAddFolder_clicked() {
    //Open the folder
    QString folder = QFileDialog::getExistingDirectory(this, "Select a folder", this->Get_Starting_Folder(), QFileDialog::ShowDirsOnly);
    if (!folder.isEmpty() && this->ui->listFoldersToScan->findItems(folder, Qt::MatchExactly).isEmpty()) {
        this->ui->listFoldersToScan->addItem(folder);
    }
}

void Main_Window::on_btnRemove_clicked() {
    for (QListWidgetItem *item : this->ui->listFoldersToScan->selectedItems()) {
        this->ui->listFoldersToScan->takeItem(this->ui->listFoldersToScan->row(item));
    }
}


void Main_Window::on_btnSaveAndRun_clicked() {
    if (this->instanceRunning) return;
    this->instanceRunning = true;
    if (!this->Save()) { this->Show_Save_Failed_Message(); this->instanceRunning = false; return; }
    QString programLocation = this->applicationLocation+"/RMS-CLI.exe";
    if (!QFileInfo(programLocation).exists()) {
        QMessageBox::critical(this, STRING_WINDOW_TITLE_SHORT, "RMS-CLI.exe is not in the program directory!", QMessageBox::Ok);
        this->instanceRunning = false;
        return;
    }

    //Set the arguments
    QProcess process;
    QStringList arguments;
    arguments.append("--force");
    process.setProgram(programLocation);
    process.setArguments(arguments);
    process.start(process.program(), process.arguments());
    process.waitForFinished(-1);
    bool noErrors = false;
    if (process.exitStatus() == QProcess::NormalExit) {
        if (process.exitCode() == 0) noErrors = true;
    }
    if (noErrors) QMessageBox::information(this, STRING_WINDOW_TITLE_SHORT, "Random maps selected!", QMessageBox::Ok);
    else QMessageBox::critical(this, STRING_WINDOW_TITLE_SHORT, "Failed to select maps!", QMessageBox::Ok);
    this->Load();
    this->instanceRunning = false;
}

void Main_Window::on_btnSaveAndClose_clicked() {
    if (this->Save()) {
        this->saveSettingsOnClose = false;
        this->close();
    } else {
        this->Show_Save_Failed_Message();
    }
}

void Main_Window::on_btnClose_clicked() {
    this->saveSettingsOnClose = false;
    this->close();
}

bool Main_Window::Save() {
    QStringList directories;
    for (int i = 0; i < this->ui->listFoldersToScan->count(); ++i) {
        directories.append(this->ui->listFoldersToScan->item(i)->text());
    }
    this->settingsManager->Set_Directories(directories);
    this->settingsManager->Set_Output_Folder(this->ui->leOutputFolder->text());
    this->settingsManager->Set_Last_Path_For_Get_Next_Map(this->ui->leLastMapPath->text());
    this->settingsManager->Set_Select_New_Maps_When_StarCraft_Launches(this->ui->cbSelectNewMapsWhenStarCraftLaunches->isChecked());
    this->settingsManager->Set_Get_Next_Map_Instead_Of_Random(this->ui->cbGetNextMapInsteadOfRandom->isChecked());
    this->settingsManager->Set_Check_Player_Count(this->ui->cbCheckPlayerCount->isChecked());
    this->settingsManager->Set_Number_Of_Random_Maps(this->ui->sbNumberOfRandomMaps->value());
    this->settingsManager->Set_Min_Number_Of_Players(this->ui->sbMinNumberOfPlayers->value());
    this->settingsManager->Set_Max_Number_Of_Players(this->ui->sbMaxNumberOfPlayers->value());
    return this->settingsManager->Save();
}

bool Main_Window::Load() {
    this->settingsManager->Load();
    this->ui->listFoldersToScan->clear();
    QStringList directories = this->settingsManager->Get_Directories();
    for (QString directory : directories) {
        if (!directory.isEmpty()) this->ui->listFoldersToScan->addItem(directory);
    }
    this->ui->leOutputFolder->setText(this->settingsManager->Get_Output_Folder());
    this->ui->leLastMapPath->setText(this->settingsManager->Get_Last_Path_For_Get_Next_Map());
    this->ui->cbSelectNewMapsWhenStarCraftLaunches->setChecked(this->settingsManager->Get_Select_New_Maps_When_StarCraft_Launches());
    this->ui->cbGetNextMapInsteadOfRandom->setChecked(this->settingsManager->Get_Get_Next_Map_Instead_Of_Random());
    this->ui->cbCheckPlayerCount->setChecked(this->settingsManager->Get_Check_Player_Count());
    this->ui->sbNumberOfRandomMaps->setValue(this->settingsManager->Get_Number_Of_Random_Maps());
    this->ui->sbMinNumberOfPlayers->setValue(this->settingsManager->Get_Min_Number_Of_Players());
    this->ui->sbMaxNumberOfPlayers->setValue(this->settingsManager->Get_Max_Number_Of_Players());
    this->Enable_Last_Map_Path(this->settingsManager->Get_Get_Next_Map_Instead_Of_Random());
    this->Enable_Player_Number_Check(this->settingsManager->Get_Check_Player_Count());
    return true;
}

void Main_Window::Enable_Player_Number_Check(bool enabled) {
    this->ui->lblMinNumberOfPlayers->setEnabled(enabled);
    this->ui->sbMinNumberOfPlayers->setEnabled(enabled);
    this->ui->lblMaxNumberOfPlayers->setEnabled(enabled);
    this->ui->sbMaxNumberOfPlayers->setEnabled(enabled);
}

void Main_Window::Enable_Last_Map_Path(bool enabled) {
    this->ui->lblLastMapPath->setEnabled(enabled);
    this->ui->leLastMapPath->setEnabled(enabled);
    this->ui->btnLastMapPath->setEnabled(enabled);
}

QString Main_Window::Get_Starting_Folder() {
    QString startingFolder;
    if (this->ui->listFoldersToScan->selectedItems().count() > 0) {
        QListWidgetItem *item = this->ui->listFoldersToScan->selectedItems().first();
        if (item) startingFolder = this->ui->listFoldersToScan->currentItem()->text();
    }
    if (startingFolder.isEmpty()) {
        if (this->ui->listFoldersToScan->count() > 0) startingFolder = this->ui->listFoldersToScan->item(0)->text();
    }

    //Get the parent folder if possible
    if (!startingFolder.isEmpty()) {
        QDir dir(startingFolder);
        dir.cdUp();
        startingFolder = dir.path();
    } else {
        startingFolder = this->applicationLocation;
    }
    return startingFolder;
}

void Main_Window::Show_Save_Failed_Message() {
    QMessageBox::critical(this, STRING_WINDOW_TITLE_SHORT, "Failed to save the config file!", QMessageBox::Ok);
}

void Main_Window::on_btnOutputFolder_clicked() {
    QString startingFolder = this->ui->leOutputFolder->text();
    if (startingFolder.isEmpty()) startingFolder = this->Get_Starting_Folder();

    //Open the folder
    QString folder = QFileDialog::getExistingDirectory(this, "Select a folder", startingFolder, QFileDialog::ShowDirsOnly);
    if (!folder.isEmpty() && this->ui->listFoldersToScan->findItems(folder, Qt::MatchExactly).isEmpty()) {
        this->ui->leOutputFolder->setText(folder);
    }
}

void Main_Window::on_cbCheckPlayerCount_toggled(bool checked) {
    this->Enable_Player_Number_Check(checked);
}

void Main_Window::on_Main_Window_finished() {
    if (this->saveSettingsOnClose) this->Save();
}

void Main_Window::on_btnLastMapPath_clicked() {
    QString startingFolder = QFileInfo(this->ui->leLastMapPath->text()).path();
    if (startingFolder.isEmpty()) startingFolder = this->Get_Starting_Folder();

    //Open the folder
    QString file = QFileDialog::getOpenFileName(this, "Select a map", startingFolder, "StarCraft Maps (*.scm *.scx)");
    if (!file.isEmpty() && this->ui->listFoldersToScan->findItems(file, Qt::MatchExactly).isEmpty()) {
        this->ui->leLastMapPath->setText(file);
    }
}

void Main_Window::on_cbGetNextMapInsteadOfRandom_toggled(bool checked) {
    this->Enable_Last_Map_Path(checked);
}
