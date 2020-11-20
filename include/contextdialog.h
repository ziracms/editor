#ifndef CONTEXTDIALOG_H
#define CONTEXTDIALOG_H

#include <QDialog>
#include <QPropertyAnimation>
#include "ui_contextdialog.h"

class ContextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContextDialog(QWidget *parent = nullptr);
    ~ContextDialog();
    void build(QList<QAction *> actions);
    int exec() override;
    QAction * getAction();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometry();
    void addItem(QListWidgetItem * item);
    void animateIn();
private:
    Ui::ContextDialog *ui;
    QList<QAction *> actionsList;
    QWidget * parentWidget;
    QAction * action;
    QPropertyAnimation *animationIn;
    bool animationInProgress;
private slots:
    void onItemClicked(QListWidgetItem * item);
    void animationInFinished();
};

#endif // CONTEXTDIALOG_H
