TARGET  = QPlastiqueStyle
PLUGIN_TYPE = styles
PLUGIN_CLASS_NAME = QPlastiqueStylePlugin
load(qt_plugin)

QT = core core-private gui gui-private widgets

HEADERS += qplastiquestyle.h \
    qplastiquestyledark.h \
    qhexstring_p.h \
    qstylecache_p.h \
    qstylehelper_p.h

SOURCES += qplastiquestyle.cpp \
    qplastiquestyledark.cpp \
    qstylehelper.cpp \
    plugin.cpp

OTHER_FILES += plastique.json
