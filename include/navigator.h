/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QObject>
#include <QTreeWidget>
#include "settings.h"
#include "parsephp.h"
#include "parsejs.h"
#include "parsecss.h"

class Navigator : public QObject
{
    Q_OBJECT
public:
    explicit Navigator(QTreeWidget * widget, Settings * settings);
    void build(ParsePHP::ParseResult result);
    void build(ParseJS::ParseResult result);
    void build(ParseCSS::ParseResult result);
    void clear();
protected:

private:
    QTreeWidget * treeWidget;
signals:
    void showLine(int line);
private slots:
    void navigatorDoubleClicked(QTreeWidgetItem * item, int column);
    void navigatorExpanded(QTreeWidgetItem * item);
    void navigatorCollapsed(QTreeWidgetItem * item);
};

#endif // NAVIGATOR_H
