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
    void setTextEchoMode(QLineEdit::EchoMode mode);
    QString textValue();
    int exec() override;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometry();
    void slideIn();
private:
    Ui::InputDialog *ui;
};

#endif // INPUTDIALOG_H
