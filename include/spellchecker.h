#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "helper.h"

class SpellChecker
{
public:
    static SpellChecker& instance();
    SpellCheckerInterface * load();
    SpellCheckerInterface * getSpellChecker();
private:
    SpellChecker();
    SpellCheckerInterface * spellChecker;
};

#endif // SPELLCHECKER_H
