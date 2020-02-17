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
    bool eventFilter(QObject *watched, QEvent *event) override;
    void gbAddRequested(QTreeWidgetItem * item);
    void gbResetRequested(QTreeWidgetItem * item);
    void gbCommitRequested();
    void gitBrowserContextMenuRequested(QTreeWidgetItem * item);
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
