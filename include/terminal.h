#ifndef TERMINAL_H
#define TERMINAL_H

#include "helper.h"

class Terminal
{
public:
    static TerminalInterface * instance();
private:
    Terminal();
    TerminalInterface * terminal;
    QString pluginsPath;
};

#endif // TERMINAL_H
