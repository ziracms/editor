#include "include/questiondialog.h"
#include "ui_questiondialog.h"

QuestionDialog::QuestionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuestionDialog)
{
    ui->setupUi(this);
}

QuestionDialog::~QuestionDialog()
{
    delete ui;
}

void QuestionDialog::setLabelText(QString text)
{
    ui->textLabel->setText(text);
}

void QuestionDialog::setHeaderText(QString text)
{
    ui->headerLabel->setText(text);
}
