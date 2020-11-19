#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include "ui_inputdialog.h"

class InputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputDialog(QWidget *parent = nullptr);
    ~InputDialog();
    void setLabelText(QString text);
    void setHeaderText(QString text);
    void setDescriptionText(QString text);
    void setTextValue(QString text);
    QString textValue();
private:
    Ui::InputDialog *ui;
};

#endif // INPUTDIALOG_H
