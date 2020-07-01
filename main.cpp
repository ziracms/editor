/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include "helper.h"

const QString SCALE_AUTO_SETTINGS_VAR = "scale_auto";
const QString SCALE_FACTOR_SETTINGS_VAR = "scale_factor";

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // disable text handles in Android (copy, paste menu)
    #if defined(Q_OS_ANDROID)
    qputenv("QT_QPA_NO_TEXT_HANDLES", QByteArray("1"));
    #endif

    int exitCode = 0;
    do {
        double scaleFactor = 1.;
        bool scaleAuto = true;
        QSettings qSettings;
        if (qSettings.contains(SCALE_AUTO_SETTINGS_VAR)) {
            QVariant scaleAutoVar = qSettings.value(SCALE_AUTO_SETTINGS_VAR, "yes");
            if (scaleAutoVar.toString() == "no") scaleAuto = false;
        }
        if (qSettings.contains(SCALE_FACTOR_SETTINGS_VAR)) {
            QVariant scaleFactorPercent = qSettings.value(SCALE_FACTOR_SETTINGS_VAR, "100");
            scaleFactor = scaleFactorPercent.toDouble() / 100;
        }
        if (!scaleAuto && scaleFactor >= 1. && scaleFactor <= 4.) {
            qputenv("QT_SCALE_FACTOR", Helper::doubleToStr(scaleFactor).toLatin1());
        } else {
            qunsetenv("QT_SCALE_FACTOR");
            qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", QByteArray("1"));
        }
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        exitCode = a.exec();
    } while(MainWindow::WANT_RESTART);
    return exitCode;
}
