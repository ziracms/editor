#ifndef SPELLCHECKERINTERFACE_H
#define SPELLCHECKERINTERFACE_H

#include "plugininterface.h"

extern const QString SPELLCHECKER_PLUGIN_NAME;

class SpellCheckerInterface : public PluginInterface
{
public:
    SpellCheckerInterface(QObject *parent = nullptr);
    virtual ~SpellCheckerInterface();
    virtual bool check(QString & word) = 0;
    virtual QStringList suggest(QString & word) = 0;
};

#define SpellCheckerInterface_iid "com.github.ziracms.editor.SpellCheckerInterface"
Q_DECLARE_INTERFACE(SpellCheckerInterface, SpellCheckerInterface_iid)

#endif // SPELLCHECKERINTERFACE_H
