#ifndef SNIPPETS_H
#define SNIPPETS_H

#include <QHash>
#include "settings.h"

class Snippets
{
public:
    static Snippets& instance();
    static void load();
    static QString parse(QString data, QString prefix, QString indent, int & moveCursorBack, int & setSelectStartFromEnd, int & setSelectLength, int & setMultiSelectStartFromEnd, int & setMultiSelectLength);

    QHash<QString, QString> phpSnippets;
    QHash<QString, QString> jsSnippets;
    QHash<QString, QString> cssSnippets;
    QHash<QString, QString> htmlSnippets;
protected:
    void _load();
private:
    Snippets();
};

#endif // SNIPPETS_H
