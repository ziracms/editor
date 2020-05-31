#ifndef TERMINALINTERFACE_H
#define TERMINALINTERFACE_H

#include "plugininterface.h"
#include <QFont>

extern const QString TERMINAL_PLUGIN_NAME;

class TerminalInterface : public PluginInterface
{
public:
    TerminalInterface(QObject *parent = nullptr);
    virtual ~TerminalInterface();
    virtual QWidget * getWidget() = 0;
    virtual void setFont(QFont font) = 0;
    virtual void exec(QString text) = 0;
    virtual void changeDir(QString path) = 0;
    virtual void copy() = 0;
    virtual void paste() = 0;
};

#define TerminalInterface_iid "com.github.ziracms.editor.TerminalInterface"
Q_DECLARE_INTERFACE(TerminalInterface, TerminalInterface_iid)

#endif // TERMINALINTERFACE_H
