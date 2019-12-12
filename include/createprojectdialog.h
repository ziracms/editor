/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef CREATEPROJECTDIALOG_H
#define CREATEPROJECTDIALOG_H

#include "ui_createproject.h"
#include <QDialog>

class CreateProjectDialog : public QDialog
{
    Q_OBJECT
public:
    CreateProjectDialog(QWidget * parent);
    ~CreateProjectDialog() override;
    void setDirectory(QString path);
    void setName(QString name);
    void setLintEnabled(bool enabled);
    void setCSEnabled(bool enabled);
    QString getDirectory();
    QString getName();
    QString getPath();
    bool getLintEnabled();
    bool getCSEnabled();
    void focusDirectory();
    void focusName();
protected:
    void checkPath(QString directory, QString name);
private:
    Ui::CreateProjectDialog * ui;
private slots:
    void directoryChanged(QString path);
    void nameChanged(QString name);
    void chooseButtonPressed(void);
};

#endif // CREATEPROJECTDIALOG_H
