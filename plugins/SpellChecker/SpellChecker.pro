#-------------------------------------------------
#
# Project created by QtCreator 2019-12-27T14:30:14
#
#-------------------------------------------------

QT       += core gui

TARGET = SpellChecker
TEMPLATE = lib
CONFIG += plugin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../ZiraEditor/include

SOURCES += \
        ../ZiraEditor/src/plugininterface.cpp \
        ../ZiraEditor/src/spellcheckerinterface.cpp \
        spellchecker.cpp
HEADERS += \
        ../ZiraEditor/include/plugininterface.h \
        ../ZiraEditor/include/spellcheckerinterface.h \
        spellchecker.h
DISTFILES += SpellChecker.json 


unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += hunspell
