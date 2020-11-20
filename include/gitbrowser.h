#ifndef GITBROWSER_H
#define GITBROWSER_H

#include "settings.h"
#include <QTreeWidget>
#include <QPushButton>
#include <QTimer>

class GitBrowser : public QObject
{
    Q_OBJECT
public:
    GitBrowser(QTreeWidget * widget, Settings * settings);
    void build(QString output);
    void clear();
    void focus();
    bool isFocused();
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
    QTimer mousePressTimer;
public slots:
    void contextMenu();
private slots:
    void gitBrowserContextMenuRequested(QPoint p);
    void gitBrowserItemSelectionChanged();
signals:
    void addRequested(QString path);
    void resetRequested(QString path);
    void commitRequested();
};

#endif // GITBROWSER_H
