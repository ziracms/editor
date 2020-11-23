#include "spellchecker.h"
#include "settings.h"

SpellChecker::SpellChecker(): spellChecker(nullptr){}

SpellCheckerInterface * SpellChecker::instance()
{
    static SpellChecker _instance;
    if (_instance.pluginsPath.isNull() || _instance.pluginsPath != QString::fromStdString(Settings::get("plugins_path"))) {
        if (_instance.spellChecker != nullptr) delete _instance.spellChecker;
        _instance.pluginsPath = QString::fromStdString(Settings::get("plugins_path"));
        _instance.spellChecker = Helper::loadSpellChecker(_instance.pluginsPath);
    }
    return _instance.spellChecker;
}
