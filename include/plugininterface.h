#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QObject>

extern const QString PLUGINS_DEFAULT_FOLDER_NAME;

class PluginInterface : public QObject
{
public:
    PluginInterface(QObject *parent = nullptr);
    virtual ~PluginInterface();
    virtual void initialize(QString path = "") = 0;
    virtual QString getDirName() = 0;
};

#endif // PLUGININTERFACE_H
