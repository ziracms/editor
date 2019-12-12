/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef CREATEFILEDIALOG_H
#define CREATEFILEDIALOG_H

#include "ui_createfile.h"
#include <QDialog>

class CreateFileDialog : public QDialog
{
    Q_OBJECT
public:
    CreateFileDialog(QWidget * parent);
    ~CreateFileDialog() override;
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
    Ui::CreateFileDialog * ui;
private slots:
    void directoryChanged(QString path);
    void nameChanged(QString path);
    void chooseButtonPressed(void);
};

#endif // CREATEFILEDIALOG_H
