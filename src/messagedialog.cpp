#include "include/messagedialog.h"
#include "ui_messagedialog.h"
#include <QScreen>
#include "helper.h"

MessageDialog::MessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageDialog)
{
    ui->setupUi(this);

    if (parent != nullptr) {
        parent->installEventFilter(this);
    }
}

MessageDialog::~MessageDialog()
{
    delete ui;
}

int MessageDialog::exec()
{
    updateGeometry();
    slideIn();
    return QDialog::exec();
}

void MessageDialog::updateGeometry()
{
    QScreen * screen = QGuiApplication::primaryScreen();
    setGeometry(0, 0, screen->availableGeometry().width(), geometry().height());
}

bool MessageDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Resize) {
        updateGeometry();
    }
    return false;
}

void MessageDialog::slideIn()
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

void MessageDialog::setLabelText(QString text)
{
    ui->textLabel->setText(text);
}

void MessageDialog::setHeaderText(QString text)
{
    ui->headerLabel->setText(text);
}
