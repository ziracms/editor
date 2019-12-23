#ifndef GITBROWSER_H
#define GITBROWSER_H

#include "settings.h"
#include <QTreeWidget>
#include <QPushButton>

class GitBrowser : public QObject
{
    Q_OBJECT
public:
    GitBrowser(QTreeWidget * widget, Settings * settings);
    void build(QString output);
    void clear();
protected:
    void gbAddRequested(QTreeWidgetItem * item);
    void gbResetRequested(QTreeWidgetItem * item);
    void gbCommitRequested();
private:
    QTreeWidget * treeWidget;
    QColor errorColor;
    QColor msgColor;
private slots:
    void gitBrowserContextMenuRequested(QPoint p);
signals:
    void addRequested(QString path);
    void resetRequested(QString path);
    void commitRequested();
};

#endif // GITBROWSER_H
