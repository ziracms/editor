#-------------------------------------------------
#
# Project created by QtCreator 2019-01-28T15:56:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    src/mainwindow.cpp \
    src/helper.cpp \
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
    src/filebrowser.cpp \
    src/editortabs.cpp \
    src/parsephp.cpp \
    src/parsejs.cpp \
    src/parsecss.cpp \
    src/parse.cpp \
    src/navigator.cpp \
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
    src/helpdialog.cpp

HEADERS += \
    include/mainwindow.h \
    include/helper.h \
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
    include/filebrowser.h \
    include/editortabs.h \
    include/parsephp.h \
    include/parsejs.h \
    include/parsecss.h \
    include/parse.h \
    include/navigator.h \
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
    include/helpdialog.h

FORMS += \
    ui/mainwindow.ui \
    ui/createfile.ui \
    ui/createfolder.ui \
    ui/createproject.ui \
    ui/search.ui \
    ui/editproject.ui \
    ui/settings.ui \
    ui/helpdialog.ui

RESOURCES += \
    qrc/highlight.qrc \
    qrc/syntax.qrc \
    qrc/image.qrc \
    qrc/help.qrc \
    qrc/style.qrc
