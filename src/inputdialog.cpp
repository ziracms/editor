#include "inputdialog.h"
#include "ui_inputdialog.h"

InputDialog::InputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputDialog)
{
    ui->setupUi(this);
    ui->descriptionLabel->setVisible(false);
}

InputDialog::~InputDialog()
{
    delete ui;
}


void InputDialog::setLabelText(QString text)
{
    ui->textLabel->setText(text);
}

void InputDialog::setHeaderText(QString text)
{
    ui->headerLabel->setText(text);
}

void InputDialog::setDescriptionText(QString text)
{
    ui->descriptionLabel->setText(text);
    ui->descriptionLabel->setVisible(true);
}

void InputDialog::setTextValue(QString text)
{
    ui->lineEdit->setText(text);
}

QString InputDialog::textValue()
{
    return ui->lineEdit->text();
}
