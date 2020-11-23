#ifndef TERMINAL_H
#define TERMINAL_H

#include "helper.h"

class Terminal
{
public:
    static Terminal& instance();
    TerminalInterface * load();
    TerminalInterface * getTerminal();
private:
    Terminal();
    TerminalInterface * terminal;
};

#endif // TERMINAL_H
