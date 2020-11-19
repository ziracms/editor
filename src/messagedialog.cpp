#include "include/messagedialog.h"
#include "ui_messagedialog.h"

MessageDialog::MessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageDialog)
{
    ui->setupUi(this);
}

MessageDialog::~MessageDialog()
{
    delete ui;
}

void MessageDialog::setLabelText(QString text)
{
    ui->textLabel->setText(text);
}

void MessageDialog::setHeaderText(QString text)
{
    ui->headerLabel->setText(text);
}
