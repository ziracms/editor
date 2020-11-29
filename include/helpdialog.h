/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include "ui_helpdialog.h"
#include <QDialog>

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog() override;
    void faqContent();
    void aboutContent();
protected:
    void mousePressEvent(QMouseEvent *event) override;
private:
    Ui::HelpDialog *ui;
    int pressed;
};

#endif // HELPDIALOG_H
