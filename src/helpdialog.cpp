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

const QString TPL_QT_VERSION = "<div><center>Qt %1</center></div>";
const QString TPL_APPLICATION_NAME = "<div><center><big><b>%1</b></big></center></div>";
const QString TPL_APPLICATION_VERSION = "<div><center>%1</center></div>";
const QString TPL_ORGANIZATION = "<div><center>(C) 2019 %1 (%2)</center></div>";
const QString TPL_BLANK = "<div>&nbsp;</div>";
const QString TPL_LOGO_IMAGE = "<div><center><img src=\":/image/zira.png\" width=\"150\" height=\"150\" /></center></div>";

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog),
    pressed(0)
{
    ui->setupUi(this);

    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    setWindowState( windowState() | Qt::WindowMaximized);
    #endif
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

    pressed = 10;
}

void HelpDialog::faqContent()
{
    QFile f(":/help/faq");
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    QString text = in.readAll();
    f.close();

    setWindowTitle(tr("FAQ"));
    ui->helpLabel->setText(text);

    pressed = 10;
}

void HelpDialog::aboutContent()
{
    QString text = TPL_LOGO_IMAGE+TPL_APPLICATION_NAME.arg(APPLICATION_NAME);
    text += TPL_APPLICATION_VERSION.arg(APPLICATION_VERSION);
    text += TPL_BLANK;

    QFile f(":/help/about");
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    text += in.readAll();
    f.close();

    text += TPL_BLANK;
    text += TPL_ORGANIZATION.arg(PROJECT_NAME).arg(GITHUB_EDITOR_URL);
    text += TPL_BLANK;
    text += TPL_QT_VERSION.arg(qVersion());

    setWindowTitle(tr("About"));
    ui->helpLabel->setText(text);

    pressed = 0;
}

void HelpDialog::mousePressEvent(QMouseEvent */*event*/)
{
    // easter egg
    pressed++;
    if (pressed == 10) {
        QString text = ui->helpLabel->text();
        int width = ui->helpLabel->geometry().width();
        QString ah = "3c6469763e3c63656e7465723e417574686f723a2042616b68616469722052616b68696d626165762e3c2f63656e7465723e3c2f6469763e";
        QString lh = "3c6469763e3c63656e7465723e546173686b656e742c20557a62656b697374616e2e3c2f63656e7465723e3c2f6469763e";
        QString gh = "3c6469763e3c63656e7465723e4772656574696e677320746f20616c6c2074686f736520626f726e20696e207468652055535352213c2f63656e7465723e3c2f6469763e";
        text += TPL_BLANK;
        text += QByteArray::fromHex(gh.toLocal8Bit());
        text += QByteArray::fromHex(ah.toLocal8Bit());
        text += QByteArray::fromHex(lh.toLocal8Bit());
        ui->helpLabel->setText(text);
        ui->helpLabel->setFixedWidth(width);
    }
}
