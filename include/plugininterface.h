#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QObject>

extern const QString PLUGINS_DIR;

class PluginInterface : public QObject
{
public:
    PluginInterface(QObject *parent = nullptr);
    virtual ~PluginInterface();
    virtual QString getDir() = 0;
};

#endif // PLUGININTERFACE_H
