/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "renamedialog.h"
#include <QPushButton>
#include <QFileDialog>
#include "helper.h"
#include "scroller.h"
#include "settings.h"
#include "virtualinput.h"

RenameDialog::RenameDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog())
{
    ui->setupUi(this);
    setModal(true);

    connect(ui->renameDialogFileLineEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));

    ui->renameDialogPathLabel->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->buttonBox->setContentsMargins(0, 0, 20, 0);
    ui->renameDialogHeaderLabel->setStyleSheet(DIALOG_HEADER_STYLESHEET);

    directory = "";

    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    // scrolling by gesture
    if (Settings::get("enable_android_gestures") == "yes") {
        Scroller::enableGestures(ui->renameScrollArea);
    }
    if (Settings::get("auto_show_virtual_keyboard") == "yes") {
        VirtualInput::registerDialog(this);
    }
    setWindowState( windowState() | Qt::WindowMaximized);
    #endif
}

RenameDialog::~RenameDialog()
{
    delete ui;
}

void RenameDialog::nameChanged(QString path)
{
    QString directory = getDirectory();
    if (path.size() > 0 && directory.size() > 0) {
        ui->renameDialogResultLabel->setText(directory + QDir::separator() + path);
        ui->renameDialogPathLabel->setVisible(true);
    } else {
        ui->renameDialogResultLabel->setText("");
        ui->renameDialogPathLabel->setVisible(false);
    }
    checkPath(directory, path);
}

void RenameDialog::checkPath(QString directory, QString name)
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

void RenameDialog::setDirectory(QString path)
{
    directory = path;
}

QString RenameDialog::getDirectory()
{
    return directory;
}

void RenameDialog::setName(QString path)
{
    ui->renameDialogFileLineEdit->setText(path);
}

QString RenameDialog::getName()
{
    QString filename = ui->renameDialogFileLineEdit->text();
    if (filename.size() > 0 && filename.mid(0, 1) == QDir::separator()) filename = filename.mid(1);
    return filename;
}

QString RenameDialog::getPath()
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

void RenameDialog::focusName()
{
    ui->renameDialogFileLineEdit->setFocus();
}
