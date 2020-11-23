#include "terminal.h"
#include "settings.h"

Terminal::Terminal(): terminal(nullptr){}

TerminalInterface * Terminal::instance()
{
    static Terminal _instance;
    if (_instance.pluginsPath.isNull() || _instance.pluginsPath != QString::fromStdString(Settings::get("plugins_path"))) {
        if (_instance.terminal != nullptr) delete _instance.terminal;
        _instance.pluginsPath = QString::fromStdString(Settings::get("plugins_path"));
        _instance.terminal = Helper::loadTerminalPlugin(_instance.pluginsPath);
    }
    return _instance.terminal;
}
