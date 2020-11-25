/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "createfolderdialog.h"
#include <QPushButton>
#include <QFileDialog>
#include "helper.h"
#include "scroller.h"
#include "settings.h"

CreateFolderDialog::CreateFolderDialog(QWidget * parent):
    QDialog(parent),
    ui(new Ui::CreateFolderDialog())
{
    ui->setupUi(this);
    setModal(true);

    connect(ui->createFolderDialogDirectoryLineEdit, SIGNAL(textChanged(QString)), this, SLOT(directoryChanged(QString)));
    connect(ui->createFolderDialogFileLineEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));
    connect(ui->createFolderDialogChooseButton, SIGNAL(pressed()), this, SLOT(chooseButtonPressed()));

    ui->createFolderDialogPathLabel->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->buttonBox->setContentsMargins(0, 0, 20, 0);
    ui->createFolderDialogHeaderLabel->setStyleSheet(DIALOG_HEADER_STYLESHEET);

    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    // scrolling by gesture
    if (Settings::get("enable_android_gestures") == "yes") {
        Scroller::enableGestures(ui->createFolderScrollArea);
    }
    setWindowState( windowState() | Qt::WindowMaximized);
    #endif
}

CreateFolderDialog::~CreateFolderDialog()
{
    delete ui;
}

void CreateFolderDialog::directoryChanged(QString path)
{
    QString filename = getName();
    if (path.size() > 0 && filename.size() > 0) {
        ui->createFolderDialogResultLabel->setText(path + QDir::separator() + filename);
    } else {
        ui->createFolderDialogResultLabel->setText("");
    }
    checkPath(path, filename);
}

void CreateFolderDialog::nameChanged(QString path)
{
    QString directory = getDirectory();
    if (path.size() > 0 && directory.size() > 0) {
        ui->createFolderDialogResultLabel->setText(directory + QDir::separator() + path);
        ui->createFolderDialogPathLabel->setVisible(true);
    } else {
        ui->createFolderDialogResultLabel->setText("");
        ui->createFolderDialogPathLabel->setVisible(false);
    }
    checkPath(directory, path);
}

void CreateFolderDialog::chooseButtonPressed()
{
    //QString dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), getDirectory(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString dir = Helper::getExistingDirectory(this, tr("Choose directory"), getDirectory());
    if (dir.size() > 0) {
        setDirectory(dir);
    }
}

void CreateFolderDialog::checkPath(QString directory, QString name)
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

void CreateFolderDialog::setDirectory(QString path)
{
    ui->createFolderDialogDirectoryLineEdit->setText(path);
}

QString CreateFolderDialog::getDirectory()
{
    QString directory = ui->createFolderDialogDirectoryLineEdit->text();
    if (directory.size() > 0 && directory.mid(directory.size()-1, 1) == QDir::separator()) directory = directory.mid(0, directory.size()-1);
    return directory;
}

void CreateFolderDialog::setName(QString path)
{
    ui->createFolderDialogFileLineEdit->setText(path);
}

QString CreateFolderDialog::getName()
{
    QString filename = ui->createFolderDialogFileLineEdit->text();
    if (filename.size() > 0 && filename.mid(0, 1) == QDir::separator()) filename = filename.mid(1);
    return filename;
}

QString CreateFolderDialog::getPath()
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

void CreateFolderDialog::focusDirectory()
{
    ui->createFolderDialogDirectoryLineEdit->setFocus();
}

void CreateFolderDialog::focusName()
{
    ui->createFolderDialogFileLineEdit->setFocus();
}
