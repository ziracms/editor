/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "createfiledialog.h"
#include <QPushButton>
#include <QFileDialog>
#include "helper.h"

CreateFileDialog::CreateFileDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::CreateFileDialog())
{
    ui->setupUi(this);
    setModal(true);

    connect(ui->createFileDialogDirectoryLineEdit, SIGNAL(textChanged(QString)), this, SLOT(directoryChanged(QString)));
    connect(ui->createFileDialogFileLineEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->createFileDialogChooseButton, SIGNAL(pressed()), this, SLOT(chooseButtonPressed()));

    ui->createFileDialogPathLabel->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->buttonBox->setContentsMargins(0, 0, 20, 0);
    ui->createFileDialogHeaderLabel->setProperty("abstract_label", true);

    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    setWindowState( windowState() | Qt::WindowMaximized);
    #endif
}

CreateFileDialog::~CreateFileDialog()
{
    delete ui;
}

void CreateFileDialog::directoryChanged(QString path)
{
    QString filename = getName();
    if (path.size() > 0 && filename.size() > 0) {
        ui->createFileDialogResultLabel->setText(path + QDir::separator() + filename);
    } else {
        ui->createFileDialogResultLabel->setText("");
    }
    checkPath(path, filename);
}

void CreateFileDialog::nameChanged(QString path)
{
    QString directory = getDirectory();
    if (path.size() > 0 && directory.size() > 0) {
        ui->createFileDialogResultLabel->setText(directory + QDir::separator() + path);
        ui->createFileDialogPathLabel->setVisible(true);
    } else {
        ui->createFileDialogResultLabel->setText("");
        ui->createFileDialogPathLabel->setVisible(false);
    }
    checkPath(directory, path);
}

void CreateFileDialog::chooseButtonPressed()
{
    //QString dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), getDirectory(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString dir = Helper::getExistingDirectory(this, tr("Choose directory"), getDirectory());
    if (dir.size() > 0) {
        setDirectory(dir);
    }
}

void CreateFileDialog::checkPath(QString directory, QString name)
{
    if (directory.size() == 0 || name.size() == 0 ||
        !Helper::fileOrFolderExists(directory) ||
        Helper::fileOrFolderExists(directory + QDir::separator() + name)
    ) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void CreateFileDialog::setDirectory(QString path)
{
    ui->createFileDialogDirectoryLineEdit->setText(path);
}

QString CreateFileDialog::getDirectory()
{
    QString directory = ui->createFileDialogDirectoryLineEdit->text();
    if (directory.size() > 0 && directory.mid(directory.size()-1, 1) == QDir::separator()) directory = directory.mid(0, directory.size()-1);
    return directory;
}

void CreateFileDialog::setName(QString path)
{
    ui->createFileDialogFileLineEdit->setText(path);
}

QString CreateFileDialog::getName()
{
    QString filename = ui->createFileDialogFileLineEdit->text();
    if (filename.size() > 0 && filename.mid(0, 1) == QDir::separator()) filename = filename.mid(1);
    return filename;
}

QString CreateFileDialog::getPath()
{
    QString directory = getDirectory();
    QString filename = getName();
    if (directory.size() > 0 && directory.mid(directory.size()-1, 1) == QDir::separator()) directory = directory.mid(0, directory.size()-1);
    if (filename.size() > 0 && filename.mid(0, 1) == QDir::separator()) filename = filename.mid(1);
    if (directory.size() > 0 && filename.size() > 0) {
        return directory + QDir::separator() + filename;
    }
    return "";
}

void CreateFileDialog::focusDirectory()
{
    ui->createFileDialogDirectoryLineEdit->setFocus();
}

void CreateFileDialog::focusName()
{
    ui->createFileDialogFileLineEdit->setFocus();
}
