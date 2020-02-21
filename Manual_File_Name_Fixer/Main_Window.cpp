#include "Main_Window.h"
#include "ui_Main_Window.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QProcess>

const static QString STRING_STATUS = "Status: ";

Main_Window::Main_Window(QWidget *parent, const QString &applicationLocation)
    : QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::Main_Window)
{
    this->applicationLocation = applicationLocation;
    this->lastFilePath = QString();
    ui->setupUi(this);
}

Main_Window::~Main_Window() {
    delete ui;
}

void Main_Window::on_btnPath_clicked() {
    QString startingFolder = this->applicationLocation;
    QString previousFolder = this->ui->lePath->text();
    if (!previousFolder.isEmpty()) startingFolder = previousFolder;

    //Open the folder
    QString folder = QFileDialog::getExistingDirectory(this, "Select a folder", startingFolder, QFileDialog::ShowDirsOnly);
    if (!folder.isEmpty()) this->ui->lePath->setText(folder);
    this->Find_Next();
}

void Main_Window::on_leNewName_textChanged(const QString &arg1) {
    QString fileName = arg1;
    if (fileName.size() > 31) {
        this->ui->lblStatus->setText(STRING_STATUS+"Exceeded by "+QString::number(fileName.size()-31)+" characters");
    } else {
        if (QFileInfo(this->lastFilePath+"/"+arg1).exists()) {
            this->ui->lblStatus->setText(STRING_STATUS+"That file name is in use!");
        } else {
            this->ui->lblStatus->setText(STRING_STATUS+"OK");
        }
    }
}

void Main_Window::on_btnApplyAndFindNext_clicked() {
    QString oldFileName = this->ui->lePreviousName->text();
    QString newFileName = this->ui->leNewName->text();
    if (oldFileName != newFileName) {
        if (!QFile::rename(this->lastFilePath+"/"+oldFileName, this->lastFilePath+"/"+newFileName)) {
            this->ui->lblStatus->setText(STRING_STATUS+"Unable to rename "+oldFileName+"!");
            return;
        }
    }
    this->Find_Next();
}

void Main_Window::Find_Next() {
    QDirIterator iter(this->ui->lePath->text(), QStringList() << "*.scm" << "*.scx", QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        QString filePath = iter.next();
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        if (fileName.size() > 31) {
            this->ui->lePreviousName->setText(fileName);
            this->ui->leNewName->setText(fileName);
            this->lastFilePath = fileInfo.path();
            this->ui->lblStatus->setText(STRING_STATUS+"Exceeded by "+QString::number(fileName.size()-31)+" characters");
            return;
        }
    }
    this->ui->lePreviousName->setText(QString());
    this->ui->leNewName->setText(QString());
    this->lastFilePath = QString();
    this->ui->lblStatus->setText(STRING_STATUS+"No issues found!");
}

void Main_Window::on_btnRefresh_clicked() {
    this->Find_Next();
}

void Main_Window::on_btnShowInExplorer_clicked() {
    QString file = this->ui->lePreviousName->text();
    QString nativePath = QDir::toNativeSeparators(this->lastFilePath+"/"+file);
    QProcess::startDetached(QString("explorer /select, \""+nativePath+"\""));
}
