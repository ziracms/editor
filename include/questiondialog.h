#ifndef QUESTIONDIALOG_H
#define QUESTIONDIALOG_H

#include <QDialog>

namespace Ui {
class QuestionDialog;
}

class QuestionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuestionDialog(QWidget *parent = nullptr);
    ~QuestionDialog();
    void setLabelText(QString text);
    void setHeaderText(QString text);
private:
    Ui::QuestionDialog *ui;
};

#endif // QUESTIONDIALOG_H
