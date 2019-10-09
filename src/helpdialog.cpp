/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "helpdialog.h"
#include "ui_helpdialog.h"
#include <QFile>
#include <QTextStream>
#include "helper.h"

const QString TPL_APPLICATION_NAME = "<div><center><big><b>%1</b></big></center></div>";
const QString TPL_APPLICATION_VERSION = "<div><center>%1</center></div>";
const QString TPL_ORGANIZATION = "<div>(C) 2019 %1.</div>";
const QString TPL_BLANK = "<div>&nbsp;</div>";

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);
}

HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::shortcutsContent()
{
    QFile f(":/help/editor_shortcuts");
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    QString text = in.readAll();
    f.close();

    setWindowTitle(tr("Shortcuts"));
    ui->helpLabel->setText(text);
}

void HelpDialog::aboutContent()
{
    QString text = TPL_APPLICATION_NAME.arg(APPLICATION_NAME);
    text += TPL_APPLICATION_VERSION.arg(APPLICATION_VERSION);
    text += TPL_BLANK;

    QFile f(":/help/about");
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    text += in.readAll();
    f.close();

    text += TPL_BLANK;
    text += TPL_ORGANIZATION.arg(ORGANIZATION_NAME);

    setWindowTitle(tr("About"));
    ui->helpLabel->setText(text);
}
