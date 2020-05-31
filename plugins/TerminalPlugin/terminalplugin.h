#ifndef TERMINALPLUGIN_H
#define TERMINALPLUGIN_H

#include "terminalinterface.h"
#include "qtermwidget5/qtermwidget.h"

class TerminalPlugin : public TerminalInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID TerminalInterface_iid FILE "TerminalPlugin.json")
    Q_INTERFACES(TerminalInterface)
#endif // QT_VERSION >= 0x050000

public:
    explicit TerminalPlugin(QObject *parent = nullptr);
    void initialize(QString path) override;
    QString getDirName() override;
    QWidget * getWidget() override;
    void setFont(QFont font) override;
    void exec(QString text) override;
    void changeDir(QString path) override;
    void copy() override;
    void paste() override;
private:
    QTermWidget * console;
};

#endif // TERMINALPLUGIN_H
