#include "inputdialog.h"
#include "ui_inputdialog.h"
#include <QScreen>
#include "helper.h"

InputDialog::InputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputDialog)
{
    ui->setupUi(this);
    ui->descriptionLabel->setVisible(false);

    if (parent != nullptr) {
        parent->installEventFilter(this);
    }
}

InputDialog::~InputDialog()
{
    delete ui;
}

int InputDialog::exec()
{
    updateGeometry();
    slideIn();
    return QDialog::exec();
}

void InputDialog::updateGeometry()
{
    QScreen * screen = QGuiApplication::primaryScreen();
    setGeometry(0, 0, screen->availableGeometry().width(), geometry().height());
}

bool InputDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Resize) {
        updateGeometry();
    }
    return false;
}

void InputDialog::slideIn()
{
    setVisible(true);
    QEasingCurve easingIn(QEasingCurve::OutCubic);
    QPropertyAnimation * animationIn = new QPropertyAnimation(this, "geometry");
    animationIn->setDuration(ANDROID_DIALOG_ANIMATION_DURATION);
    animationIn->setEasingCurve(easingIn);
    QRect rect = geometry();
    animationIn->setStartValue(QRect(rect.x(), rect.y()-rect.height(), rect.width(), rect.height()));
    animationIn->setEndValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
    animationIn->start(QAbstractAnimation::DeleteWhenStopped);
    setVisible(false);
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

void InputDialog::setTextEchoMode(QLineEdit::EchoMode mode)
{
    ui->lineEdit->setEchoMode(mode);
}

QString InputDialog::textValue()
{
    return ui->lineEdit->text();
}
