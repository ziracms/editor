#include "terminalplugin.h"

TerminalPlugin::TerminalPlugin(QObject *parent)
    : TerminalInterface(parent)
{
}

void TerminalPlugin::initialize(QString path)
{
    console = new QTermWidget(0);
    console->setEnvironment(QStringList() << "TERM=konsole-256color");
    console->setColorScheme("Linux"); // "GreenOnBlack", "Linux", "SolarizedLight", "BlackOnWhite", "DarkPastels", "BlackOnRandomLight", "WhiteOnBlack", "BlackOnLightYellow", "Solarized", "BreezeModified"
    console->setWorkingDirectory(path);
}

QString TerminalPlugin::getDirName()
{
    return TERMINAL_PLUGIN_NAME;
}

QWidget * TerminalPlugin::getWidget()
{
    return console;
}

void TerminalPlugin::copy()
{
    console->copyClipboard();
}

void TerminalPlugin::paste()
{
    console->pasteClipboard();
}

void TerminalPlugin::setFont(QFont font)
{
    console->setTerminalFont(font);
}

void TerminalPlugin::exec(QString text)
{
    console->sendText(text+"\n");
}

void TerminalPlugin::changeDir(QString path)
{
    console->changeDir(path);
}

void TerminalPlugin::startShell()
{
    console->startShellProgram();
}
