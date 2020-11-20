#include "include/questiondialog.h"
#include "ui_questiondialog.h"
#include <QScreen>
#include "helper.h"

QuestionDialog::QuestionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuestionDialog)
{
    ui->setupUi(this);

    if (parent != nullptr) {
        parent->installEventFilter(this);
    }
}

QuestionDialog::~QuestionDialog()
{
    delete ui;
}

int QuestionDialog::exec()
{
    updateGeometry();
    slideIn();
    return QDialog::exec();
}

void QuestionDialog::updateGeometry()
{
    QScreen * screen = QGuiApplication::primaryScreen();
    setGeometry(0, 0, screen->availableGeometry().width(), geometry().height());
}

bool QuestionDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Resize) {
        updateGeometry();
    }
    return false;
}

void QuestionDialog::slideIn()
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

void QuestionDialog::setLabelText(QString text)
{
    ui->textLabel->setText(text);
}

void QuestionDialog::setHeaderText(QString text)
{
    ui->headerLabel->setText(text);
}
