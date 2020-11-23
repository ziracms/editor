#include "spellchecker.h"
#include "settings.h"

SpellChecker::SpellChecker(): spellChecker(nullptr){}

SpellChecker& SpellChecker::instance()
{
    static SpellChecker _instance;
    return _instance;
}

SpellCheckerInterface * SpellChecker::getSpellChecker()
{
    return spellChecker;
}

SpellCheckerInterface * SpellChecker::load()
{
    if (spellChecker != nullptr) delete spellChecker;
    spellChecker = Helper::loadSpellChecker(QString::fromStdString(Settings::get("plugins_path")));
    return spellChecker;
}
