#include "terminal.h"
#include "settings.h"

Terminal::Terminal(): terminal(nullptr){}

Terminal& Terminal::instance()
{
    static Terminal _instance;
    return _instance;
}

TerminalInterface * Terminal::getTerminal()
{
    return terminal;
}

TerminalInterface * Terminal::load()
{
    if (terminal != nullptr) delete terminal;
    terminal = Helper::loadTerminalPlugin(QString::fromStdString(Settings::get("plugins_path")));
    return terminal;
}
