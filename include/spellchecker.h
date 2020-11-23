#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "helper.h"

class SpellChecker
{
public:
    static SpellCheckerInterface * instance();
private:
    SpellChecker();
    SpellCheckerInterface * spellChecker;
    QString pluginsPath;
};

#endif // SPELLCHECKER_H
