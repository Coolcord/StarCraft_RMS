#include "Main_Window.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Main_Window w(nullptr, a.applicationDirPath());
    w.show();
    return a.exec();
}
