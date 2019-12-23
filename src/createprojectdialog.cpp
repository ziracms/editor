/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "createprojectdialog.h"
#include <QPushButton>
#include <QFileDialog>
#include "helper.h"

CreateProjectDialog::CreateProjectDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::CreateProjectDialog())
{
    ui->setupUi(this);
    setModal(true);

    connect(ui->createProjectDialogDirectoryLineEdit, SIGNAL(textChanged(QString)), this, SLOT(directoryChanged(QString)));
    connect(ui->createProjectDialogNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->createProjectDialogDirectoryButton, SIGNAL(pressed()), this, SLOT(chooseButtonPressed()));

    ui->createProjectDialogPathLabel->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->buttonBox->setContentsMargins(0, 0, 20, 0);
    ui->createProjectDialogHeaderLabel->setProperty("abstract_label", true);
}

CreateProjectDialog::~CreateProjectDialog()
{
    delete ui;
}

void CreateProjectDialog::directoryChanged(QString path)
{
    QString name = getName();
    if (path.size() > 0) {
        ui->createProjectDialogResultLabel->setText(path);
        ui->createProjectDialogPathLabel->setVisible(true);
    } else {
        ui->createProjectDialogResultLabel->setText("");
        ui->createProjectDialogPathLabel->setVisible(false);
    }
    checkPath(path, name);
}

void CreateProjectDialog::nameChanged(QString name)
{
    QString directory = getDirectory();
    checkPath(directory, name);
}

void CreateProjectDialog::chooseButtonPressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose project source directory"), getDirectory(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.size() > 0) {
        setDirectory(dir);
    }
}

void CreateProjectDialog::checkPath(QString directory, QString name)
{
    if (directory.size() == 0 || name.trimmed().size() == 0 ||
        !Helper::folderExists(directory)
    ) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void CreateProjectDialog::setDirectory(QString path)
{
    ui->createProjectDialogDirectoryLineEdit->setText(path);
}

QString CreateProjectDialog::getDirectory()
{
    QString directory = ui->createProjectDialogDirectoryLineEdit->text();
    if (directory.size() > 0 && directory.mid(directory.size()-1, 1) == QDir::separator()) directory = directory.mid(0, directory.size()-1);
    return directory;
}

void CreateProjectDialog::setName(QString name)
{
    ui->createProjectDialogNameLineEdit->setText(name);
}

QString CreateProjectDialog::getName()
{
    QString name = ui->createProjectDialogNameLineEdit->text();
    name = name.trimmed();
    return name;
}

void CreateProjectDialog::setLintEnabled(bool enabled)
{
    ui->createProjectDialogLintCheckbox->setChecked(enabled);
}

bool CreateProjectDialog::getLintEnabled()
{
    return ui->createProjectDialogLintCheckbox->isChecked();
}

void CreateProjectDialog::setCSEnabled(bool enabled)
{
    ui->createProjectDialogCSCheckbox->setChecked(enabled);
}

bool CreateProjectDialog::getCSEnabled()
{
    return ui->createProjectDialogCSCheckbox->isChecked();
}

QString CreateProjectDialog::getPath()
{
    QString directory = getDirectory();
    if (directory.size() > 0 && directory.mid(directory.size()-1, 1) == QDir::separator()) directory = directory.mid(0, directory.size()-1);
    if (directory.size() > 0) {
        return directory;
    }
    return "";
}

void CreateProjectDialog::focusDirectory()
{
    ui->createProjectDialogDirectoryLineEdit->setFocus();
}

void CreateProjectDialog::focusName()
{
    ui->createProjectDialogNameLineEdit->setFocus();
}
