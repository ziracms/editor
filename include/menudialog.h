#ifndef MENUDIALOG_H
#define MENUDIALOG_H

#include <QDialog>
#include "ui_menudialog.h"
#include <QMenuBar>
#include <QPropertyAnimation>

class MenuDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MenuDialog(QMenuBar * menuBar, QWidget *parent = nullptr);
    ~MenuDialog();
    void build();
    void build(QList<QAction *> actions);
    int exec() override;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometry();
    void addItem(QListWidgetItem * item);
    void animateIn();
    void setupMenu();
    void buildSubMenu(QMenu * menu);
private:
    Ui::MenuDialog *ui;
    QMenuBar * menuBar;
    QMenu * mainMenu;
    QMenu * subMenu;
    QAction * backAction;
    QList<QAction *> actionsList;
    QAction * action;
    QPropertyAnimation *animationIn;
    bool animationInProgress;
private slots:
    void onItemClicked(QListWidgetItem * item);
    void animationInFinished();
    void topLevelItemTriggered(bool checked);
    void backItemTriggered(bool checked);
    void contextMenuItemTriggered(bool checked);
    void preferencesItemTriggered(bool checked);
    void exitItemTriggered(bool checked);
signals:
    void showContextMenu();
    void showPreferences();
    void quit();
};

#endif // MENUDIALOG_H
