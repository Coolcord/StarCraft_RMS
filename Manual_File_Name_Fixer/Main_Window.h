#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class Main_Window; }
QT_END_NAMESPACE

class Main_Window : public QDialog {
    Q_OBJECT

public:
    Main_Window(QWidget *parent, const QString &applicationLocation);
    ~Main_Window();

private slots:
    void on_btnPath_clicked();
    void on_leNewName_textChanged(const QString &arg1);
    void on_btnApplyAndFindNext_clicked();

    void on_btnRefresh_clicked();

    void on_btnShowInExplorer_clicked();

private:
    void Find_Next();

    Ui::Main_Window *ui;
    QString applicationLocation;
    QString lastFilePath;
};
#endif // MAIN_WINDOW_H
