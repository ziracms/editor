/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef EDITPROJECTDIALOG_H
#define EDITPROJECTDIALOG_H

#include "ui_editproject.h"
#include <QDialog>

class EditProjectDialog : public QDialog
{
    Q_OBJECT
public:
    EditProjectDialog(QWidget * parent);
    ~EditProjectDialog();
    void setPath(QString path);
    void setName(QString name);
    void setLintEnabled(bool enabled);
    void setCSEnabled(bool enabled);
    QString getName();
    bool getLintEnabled();
    bool getCSEnabled();
    void focusName();
protected:
    void checkName(QString name);
private:
    Ui::EditProjectDialog * ui;
private slots:
    void nameChanged(QString name);
};

#endif // EDITPROJECTDIALOG_H
