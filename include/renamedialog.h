/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef RENAMEFILEDIALOG_H
#define RENAMEFILEDIALOG_H

#include "ui_rename.h"
#include <QDialog>

class RenameDialog : public QDialog
{
    Q_OBJECT
public:
    RenameDialog(QWidget * parent);
    ~RenameDialog() override;
    void setDirectory(QString path);
    void setName(QString path);
    QString getDirectory();
    QString getName();
    QString getPath();
    void focusName();
protected:
    void checkPath(QString directory, QString name);
private:
    Ui::RenameDialog * ui;
    QString directory;
private slots:
    void nameChanged(QString path);
};

#endif // RENAMEFILEDIALOG_H
