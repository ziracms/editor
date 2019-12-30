#include "spellwords.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QTimer>

const int LOAD_DELAY = 250; // should not be less then PROJECT_LOAD_DELAY

SpellWords::SpellWords()
{
    QTimer::singleShot(LOAD_DELAY, this, SLOT(load()));
}

void SpellWords::load()
{
    loadWords();
}

void SpellWords::reload()
{
    reset();
    load();
}

void SpellWords::reset()
{
    words.clear();
}

void SpellWords::loadWords()
{
    QString k;

    // spell words
    QFile pf(":/spell/words");
    pf.open(QIODevice::ReadOnly);
    QTextStream pin(&pf);
    while (!pin.atEnd()) {
        k = pin.readLine();
        if (k == "") continue;
        words[k.toStdString()] = k.toStdString();
    }
    pf.close();

    // spell words
    QFile pcf(":/spell/wordsCS");
    pcf.open(QIODevice::ReadOnly);
    QTextStream pcin(&pcf);
    while (!pcin.atEnd()) {
        k = pcin.readLine();
        if (k == "") continue;
        wordsCS[k.toStdString()] = k.toStdString();
    }
    pcf.close();
}
