/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    int exitCode = 0;
    do {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        exitCode = a.exec();
    } while(exitCode == MainWindow::EXIT_CODE_RESTART);
    return exitCode;
}
