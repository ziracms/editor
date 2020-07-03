#ifndef ICON_H
#define ICON_H

#include <QWidget>
#include <QMenuBar>

class Icon
{
public:
    Icon();
    static void applyActionIcons(QMenuBar * menuBar, QString prefix);
    static QIcon get(QString actionName, QIcon defaultIcon = QIcon());
    static void reset();
    static QString prefix;
protected:
    static void iterateMenuActions(QMenu * menu);
};

#endif // ICON_H
