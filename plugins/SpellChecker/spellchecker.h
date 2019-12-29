#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QtPlugin>
#include <QTextCodec>
#include "spellcheckerinterface.h"

#include <hunspell/hunspell.hxx>

class SpellChecker : public SpellCheckerInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID SpellCheckerInterface_iid FILE "SpellChecker.json")
    Q_INTERFACES(SpellCheckerInterface)
#endif // QT_VERSION >= 0x050000

public:
    SpellChecker(QObject *parent = nullptr);
    ~SpellChecker() override;
    bool check(QString & word) override;
    QStringList suggest(QString & word) override;
    QString getDir() override;
private:
    Hunspell * hunspell;
    Hunspell * hunspellFallback;
    QTextCodec * defaultCodec;
    QTextCodec * fallbackCodec;
};

#endif // SPELLCHECKER_H
