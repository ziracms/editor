#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>

namespace Ui {
class MessageDialog;
}

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageDialog(QWidget *parent = nullptr);
    ~MessageDialog();
    void setLabelText(QString text);
    void setHeaderText(QString text);
    int exec() override;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateGeometry();
    void slideIn();
private:
    Ui::MessageDialog *ui;
};

#endif // MESSAGEDIALOG_H
