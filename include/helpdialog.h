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
    void shortcutsContent();
    void aboutContent();
private:
    Ui::HelpDialog *ui;
};

#endif // HELPDIALOG_H
