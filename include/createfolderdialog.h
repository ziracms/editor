/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef CREATEFOLDERDIALOG_H
#define CREATEFOLDERDIALOG_H

#include "ui_createfolder.h"
#include <QDialog>

class CreateFolderDialog : public QDialog
{
    Q_OBJECT
public:
    CreateFolderDialog(QWidget * parent);
    ~CreateFolderDialog() override;
    void setDirectory(QString path);
    void setName(QString path);
    QString getDirectory();
    QString getName();
    QString getPath();
    void focusDirectory();
    void focusName();
protected:
    void checkPath(QString directory, QString name);
private:
    Ui::CreateFolderDialog * ui;
private slots:
    void directoryChanged(QString path);
    void nameChanged(QString path);
    void chooseButtonPressed(void);
};

#endif // CREATEFOLDERDIALOG_H
