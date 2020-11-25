/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "editprojectdialog.h"
#include <QPushButton>
#include "helper.h"
#include "scroller.h"
#include "settings.h"
#include "virtualinput.h"

EditProjectDialog::EditProjectDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::EditProjectDialog())
{
    ui->setupUi(this);
    setModal(true);

    connect(ui->editProjectDialogNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged(QString)));

    ui->editProjectDialogPathLabel->setVisible(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    ui->buttonBox->setContentsMargins(0, 0, 20, 0);
    ui->editProjectDialogHeaderLabel->setStyleSheet(DIALOG_HEADER_STYLESHEET);

    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    // scrolling by gesture
    if (Settings::get("enable_android_gestures") == "yes") {
        Scroller::enableGestures(ui->editProjectScrollArea);
    }
    if (Settings::get("auto_show_virtual_keyboard") == "yes") {
        VirtualInput::registerDialog(this);
    }
    setWindowState( windowState() | Qt::WindowMaximized);
    #endif
}

EditProjectDialog::~EditProjectDialog()
{
    delete ui;
}

void EditProjectDialog::nameChanged(QString name)
{
    checkName(name);
}

void EditProjectDialog::checkName(QString name)
{
    if (name.trimmed().size() == 0) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void EditProjectDialog::setPath(QString path)
{
    ui->editProjectDialogResultLabel->setText(path);
}

void EditProjectDialog::setName(QString name)
{
    ui->editProjectDialogNameLineEdit->setText(name);
}

QString EditProjectDialog::getName()
{
    QString name = ui->editProjectDialogNameLineEdit->text();
    name = name.trimmed();
    return name;
}

void EditProjectDialog::setLintEnabled(bool enabled)
{
    ui->editProjectDialogLintCheckbox->setChecked(enabled);
}

bool EditProjectDialog::getLintEnabled()
{
    return ui->editProjectDialogLintCheckbox->isChecked();
}

void EditProjectDialog::setCSEnabled(bool enabled)
{
    ui->editProjectDialogCSCheckbox->setChecked(enabled);
}

bool EditProjectDialog::getCSEnabled()
{
    return ui->editProjectDialogCSCheckbox->isChecked();
}

void EditProjectDialog::focusName()
{
    ui->editProjectDialogNameLineEdit->setFocus();
}
