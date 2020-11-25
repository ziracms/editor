#-------------------------------------------------
#
# Project created by QtCreator 2019-01-28T15:56:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

android {
    QT += androidextras
}

TARGET = ZiraEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += include

SOURCES += \
    main.cpp \
    src/colordialog.cpp \
    src/contextdialog.cpp \
    src/docktitlebar.cpp \
    src/filedialog.cpp \
    src/fileiconprovider.cpp \
    src/icon.cpp \
    src/inputdialog.cpp \
    src/mainwindow.cpp \
    src/helper.cpp \
    src/menudialog.cpp \
    src/messagedialog.cpp \
    src/progressinfo.cpp \
    src/progressline.cpp \
    src/questiondialog.cpp \
    src/scroller.cpp \
    src/settings.cpp \
    src/editor.cpp \
    src/linenumber.cpp \
    src/highlightdata.cpp \
    src/highlight.cpp \
    src/completepopup.cpp \
    src/search.cpp \
    src/linemark.cpp \
    src/linemap.cpp \
    src/highlightwords.cpp \
    src/completewords.cpp \
    src/editortab.cpp \
    src/parserworker.cpp \
    src/createfiledialog.cpp \
    src/createfolderdialog.cpp \
    src/renamedialog.cpp \
    src/filebrowser.cpp \
    src/editortabs.cpp \
    src/parsephp.cpp \
    src/parsejs.cpp \
    src/parsecss.cpp \
    src/parse.cpp \
    src/navigator.cpp \
    src/snippets.cpp \
    src/spellchecker.cpp \
    src/style.cpp \
    src/tabslist.cpp \
    src/terminal.cpp \
    src/terminalinterface.cpp \
    src/tooltip.cpp \
    src/breadcrumbs.cpp \
    src/helpwords.cpp \
    src/createprojectdialog.cpp \
    src/project.cpp \
    src/searchdialog.cpp \
    src/git.cpp \
    src/servers.cpp \
    src/editprojectdialog.cpp \
    src/quickaccess.cpp \
    src/settingsdialog.cpp \
    src/helpdialog.cpp \
    src/popup.cpp \
    src/gitbrowser.cpp \
    src/annotation.cpp \
    src/spellcheckerinterface.cpp \
    src/plugininterface.cpp \
    src/spellwords.cpp \
    src/virtualinput.cpp \
    src/welcome.cpp

HEADERS += \
    include/colordialog.h \
    include/contextdialog.h \
    include/docktitlebar.h \
    include/filedialog.h \
    include/fileiconprovider.h \
    include/icon.h \
    include/inputdialog.h \
    include/mainwindow.h \
    include/helper.h \
    include/menudialog.h \
    include/messagedialog.h \
    include/progressinfo.h \
    include/progressline.h \
    include/questiondialog.h \
    include/scroller.h \
    include/settings.h \
    include/editor.h \
    include/linenumber.h \
    include/highlightdata.h \
    include/highlight.h \
    include/completepopup.h \
    include/search.h \
    include/linemark.h \
    include/linemap.h \
    include/highlightwords.h \
    include/completewords.h \
    include/editortab.h \
    include/parserworker.h \
    include/createfiledialog.h \
    include/createfolderdialog.h \
    include/renamedialog.h \
    include/filebrowser.h \
    include/editortabs.h \
    include/parsephp.h \
    include/parsejs.h \
    include/parsecss.h \
    include/parse.h \
    include/navigator.h \
    include/snippets.h \
    include/spellchecker.h \
    include/style.h \
    include/tabslist.h \
    include/terminal.h \
    include/terminalinterface.h \
    include/tooltip.h \
    include/breadcrumbs.h \
    include/helpwords.h \
    include/createprojectdialog.h \
    include/project.h \
    include/searchdialog.h \
    include/git.h \
    include/servers.h \
    include/editprojectdialog.h \
    include/quickaccess.h \
    include/types.h \
    include/settingsdialog.h \
    include/helpdialog.h \
    include/popup.h \
    include/gitbrowser.h \
    include/annotation.h \
    include/spellcheckerinterface.h \
    include/plugininterface.h \
    include/spellwords.h \
    include/virtualinput.h \
    include/welcome.h

FORMS += \
    ui/contextdialog.ui \
    ui/inputdialog.ui \
    ui/mainwindow.ui \
    ui/createfile.ui \
    ui/createfolder.ui \
    ui/menudialog.ui \
    ui/messagedialog.ui \
    ui/questiondialog.ui \
    ui/rename.ui \
    ui/createproject.ui \
    ui/search.ui \
    ui/editproject.ui \
    ui/settings.ui \
    ui/helpdialog.ui \
    ui/welcome.ui

RESOURCES += \
    qrc/fonts.qrc \
    qrc/highlight.qrc \
    qrc/syntax.qrc \
    qrc/image.qrc \
    qrc/help.qrc \
    qrc/style.qrc \
    qrc/spell.qrc

equals(QT_MAJOR_VERSION, 5): lessThan(QT_MINOR_VERSION, 13) {
    DISTFILES += \
        android/Qt5_12/AndroidManifest.xml \
        android/Qt5_12/build.gradle \
        android/Qt5_12/gradle/wrapper/gradle-wrapper.jar \
        android/Qt5_12/gradle/wrapper/gradle-wrapper.properties \
        android/Qt5_12/gradlew \
        android/Qt5_12/gradlew.bat \
        android/Qt5_12/res/values/libs.xml

    contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
        ANDROID_PACKAGE_SOURCE_DIR = \
            $$PWD/android/Qt5_12
    }

    contains(ANDROID_TARGET_ARCH,arm64-v8a) {
        ANDROID_PACKAGE_SOURCE_DIR = \
            $$PWD/android/Qt5_12
    }
}

equals(QT_MAJOR_VERSION, 5): greaterThan(QT_MINOR_VERSION, 12) {
    DISTFILES += \
        android/Qt5_14/AndroidManifest.xml \
        android/Qt5_14/build.gradle \
        android/Qt5_14/gradle/wrapper/gradle-wrapper.jar \
        android/Qt5_14/gradle/wrapper/gradle-wrapper.properties \
        android/Qt5_14/gradlew \
        android/Qt5_14/gradlew.bat \
        android/Qt5_14/res/values/libs.xml

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android/Qt5_14
}
