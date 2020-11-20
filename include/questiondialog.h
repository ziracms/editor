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
    int exec() override;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometry();
    void slideIn();
private:
    Ui::QuestionDialog *ui;
};

#endif // QUESTIONDIALOG_H
