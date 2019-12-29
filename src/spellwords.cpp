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
}
