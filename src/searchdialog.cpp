/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "searchdialog.h"
#include <QFileDialog>
#include "helper.h"

SearchDialog::SearchDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog())
{
    ui->setupUi(this);
    setModal(true);

    connect(ui->searchDialogDirectoryLineEdit, SIGNAL(textChanged(QString)), this, SLOT(directoryChanged(QString)));
    connect(ui->searchDialogTextLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
    connect(ui->searchDialogChooseButton, SIGNAL(pressed()), this, SLOT(chooseButtonPressed()));

    ui->buttonBox->setContentsMargins(0, 0, 20, 0);
    ui->searchDialogHeaderLabel->setProperty("abstract_label", true);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::directoryChanged(QString path)
{
    checkPath(path, getText());
}

void SearchDialog::textChanged(QString text)
{
    checkPath(getDirectory(), text);
}

void SearchDialog::checkPath(QString directory, QString text)
{
    if (directory.size() == 0 || text.trimmed().size() == 0 ||
        !Helper::folderExists(directory)
    ) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void SearchDialog::chooseButtonPressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), getDirectory(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.size() > 0) {
        setDirectory(dir);
    }
}

void SearchDialog::setDirectory(QString path)
{
    ui->searchDialogDirectoryLineEdit->setText(path);
}

QString SearchDialog::getDirectory()
{
    QString directory = ui->searchDialogDirectoryLineEdit->text();
    if (directory.size() > 0 && directory.mid(directory.size()-1, 1) == "/") directory = directory.mid(0, directory.size()-1);
    return directory;
}

void SearchDialog::setText(QString text)
{
    ui->searchDialogTextLineEdit->setText(text);
}

QString SearchDialog::getText()
{
    return ui->searchDialogTextLineEdit->text().trimmed();
}

void SearchDialog::setExtensions(QString text)
{
    ui->searchDialogExtensionsLineEdit->setText(text);
}

QString SearchDialog::getExtensions()
{
    return ui->searchDialogExtensionsLineEdit->text();
}

void SearchDialog::setCaseOption(bool checked)
{
    ui->searchDialogCaseCheckbox->setChecked(checked);
}

bool SearchDialog::getCaseOption()
{
    return ui->searchDialogCaseCheckbox->isChecked();
}

void SearchDialog::setWordOption(bool checked)
{
    ui->searchDialogWordCheckbox->setChecked(checked);
}

bool SearchDialog::getWordOption()
{
    return ui->searchDialogWordCheckbox->isChecked();
}

void SearchDialog::setRegexpOption(bool checked)
{
    ui->searchDialogRegexpCheckbox->setChecked(checked);
}

bool SearchDialog::getRegexpOption()
{
    return ui->searchDialogRegexpCheckbox->isChecked();
}

void SearchDialog::focusDirectory()
{
    ui->searchDialogDirectoryLineEdit->setFocus();
}

void SearchDialog::focusText()
{
    ui->searchDialogTextLineEdit->setFocus();
}
