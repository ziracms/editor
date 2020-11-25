#include "virtualinput.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QInputMethodEvent>
#include "helper.h"

/*
#if defined(Q_OS_ANDROID)
#include <QtAndroidExtras/QtAndroid>
#endif
*/

const QString INPUT_OBJECT_NAME = "virtualInput";

const int PADDING = 20;
const int OFFSET_X = 10;
const int OFFSET_Y = 10;
const int INPUT_MIN_HEIGHT = 30;
const int UPDATE_INTERVAL = 1000;

VirtualInput::VirtualInput() {
    activeObject = nullptr;
    activeInput = nullptr;
    timer.setInterval(UPDATE_INTERVAL);
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateState()));
}

VirtualInput& VirtualInput::instance()
{
    static VirtualInput _instance;
    return _instance;
}

void VirtualInput::registerDialog(QDialog *dialog)
{
    /*
    #if defined(Q_OS_ANDROID)
    if(QtAndroid::androidSdkVersion() >= 21) return;
    #endif
    */
    instance()._registerDialog(dialog);
}

void VirtualInput::_registerDialog(QDialog *dialog)
{
    dialog->installEventFilter(this);
    QWidget * widget = new QWidget(dialog);
    QHBoxLayout * layout = new QHBoxLayout();
    widget->setLayout(layout);
    layout->setContentsMargins(PADDING, PADDING, PADDING, PADDING);
    QLabel * label = new QLabel();
    label->setText(tr("Input:"));
    layout->addWidget(label);
    QLineEdit * lineEdit = new QLineEdit();
    lineEdit->setMinimumHeight(INPUT_MIN_HEIGHT);
    lineEdit->setObjectName(INPUT_OBJECT_NAME);
    layout->addWidget(lineEdit);
    widget->hide();

    QList<QLineEdit *> lineEditList = dialog->findChildren<QLineEdit *>();
    for (QLineEdit * lineEdit : lineEditList) {
        if (lineEdit->objectName().size() == 0 || lineEdit->objectName() == "qt_spinbox_lineedit") continue;
        lineEdit->setInputMethodHints(Qt::ImhNoPredictiveText);
        lineEdit->installEventFilter(this);
    }

    connect(dialog, SIGNAL(destroyed(QObject*)), this, SLOT(onDialogDestroy(QObject*)));
}

void VirtualInput::updateGeometry(QDialog * dialog, QWidget * widget)
{
    if (dialog == nullptr || widget == nullptr || widget->layout() == nullptr) return;
    int top = OFFSET_Y, left = OFFSET_X;
    int width = dialog->width() - (2 * OFFSET_X);
    int height = widget->layout()->sizeHint().height();
    widget->setGeometry(left, top, width, height);

}

QDialog * VirtualInput::findParentDialog(QObject *widget)
{
    if (widget == nullptr) return nullptr;
    QObject * parent = widget->parent();
    QDialog * dialog = qobject_cast<QDialog *>(parent);
    if (dialog == nullptr) return findParentDialog(parent);
    return dialog;
}

void VirtualInput::showWidget(QWidget *widget)
{
    activeObject = widget;
    widget->show();
    widget->raise();
    if (!timer.isActive()) timer.start();
}

void VirtualInput::hideWidget(QWidget *widget)
{
    activeObject = nullptr;
    activeInput = nullptr;
    widget->hide();
    if (timer.isActive()) timer.stop();
}

bool VirtualInput::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        QDialog * dialog = qobject_cast<QDialog *>(watched);
        if (dialog == nullptr) return false;
        QLineEdit * virtualInput = dialog->findChild<QLineEdit *>(INPUT_OBJECT_NAME);
        if (virtualInput == nullptr) return false;
        QWidget * widget = qobject_cast<QWidget *>(virtualInput->parent());
        if (!widget->isVisible()) return false;
        updateGeometry(dialog, widget);
        return false;
    }
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyRelease || event->type() == QEvent::InputMethodQuery) {
        QLineEdit * lineEdit = qobject_cast<QLineEdit *>(watched);
        if (lineEdit == nullptr) return false;
        QDialog * dialog = findParentDialog(lineEdit);
        if (dialog == nullptr) return false;
        QLineEdit * virtualInput = dialog->findChild<QLineEdit *>(INPUT_OBJECT_NAME);
        if (virtualInput == nullptr) return false;
        QWidget * widget = qobject_cast<QWidget *>(virtualInput->parent());
        if (widget == nullptr) return false;
        if (event->type() == QEvent::InputMethodQuery && !widget->isVisible()) return false;

        if (timer.isActive()) {
            timer.stop();
            timer.start();
        }
        if (lineEdit != virtualInput) {
            virtualInput->setText(lineEdit->text());
            if (!widget->isVisible()) {
                showWidget(widget);
                updateGeometry(dialog, widget);
            }
            activeInput = lineEdit;
        } else {
            QLineEdit * activeLineEdit = qobject_cast<QLineEdit *>(activeInput);
            if (activeLineEdit != nullptr) {
                activeLineEdit->setText(virtualInput->text());
            }
        }

    }
    return false;
}

void VirtualInput::updateState()
{
    QWidget * widget = qobject_cast<QWidget *>(activeObject);
    if (widget == nullptr) return;
    if (!QApplication::inputMethod()->isVisible()) {
        hideWidget(widget);
    }
}

void VirtualInput::onDialogDestroy(QObject*)
{
    activeObject = nullptr;
    activeInput = nullptr;
    if (timer.isActive()) timer.stop();
}
