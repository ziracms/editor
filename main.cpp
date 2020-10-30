/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "mainwindow.h"
#include <QApplication>
#include "settings.h"
#include "helper.h"

int main(int argc, char *argv[])
{
    Helper::setApplicationAttributes();
    int exitCode = 0;
    do {
        Settings::initApplicationScaling();
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        exitCode = a.exec();
    } while(MainWindow::WANT_RESTART);
    return exitCode;
}
