#include "virtualinput.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QInputMethodEvent>
#include <QTextBlock>
#include "helper.h"

/*
#if defined(Q_OS_ANDROID)
#include <QtAndroidExtras/QtAndroid>
#endif
*/

const QString WIDGET_OBJECT_NAME = "virtualInputWidget";
const QString INPUT_OBJECT_NAME = "virtualInput";

const int PADDING_TOP = 10;
const int PADDING_BOTTOM = 10;
const int PADDING_LEFT = 20;
const int PADDING_RIGHT = 20;
const int OFFSET_X = 50;
const int OFFSET_Y = 40;
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

void VirtualInput::registerDialog(QDialog *dialog, bool withPlainTextEdits)
{
    /*
    #if defined(Q_OS_ANDROID)
    if(QtAndroid::androidSdkVersion() >= 21) return;
    #endif
    */
    instance()._registerDialog(dialog, withPlainTextEdits);
}

void VirtualInput::_registerDialog(QDialog *dialog, bool withPlainTextEdits)
{
    dialog->installEventFilter(this);
    QWidget * widget = new QWidget(dialog);
    widget->setObjectName(WIDGET_OBJECT_NAME);
    QHBoxLayout * layout = new QHBoxLayout();
    widget->setLayout(layout);
    layout->setContentsMargins(PADDING_LEFT, PADDING_TOP, PADDING_RIGHT, PADDING_BOTTOM);
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

    if (withPlainTextEdits) {
        QList<QPlainTextEdit *> plainTextEditList = dialog->findChildren<QPlainTextEdit *>();
        for (QPlainTextEdit * plainTextEdit : plainTextEditList) {
            if (plainTextEdit->objectName().size() == 0) continue;
            plainTextEdit->setInputMethodHints(Qt::ImhNoPredictiveText);
            plainTextEdit->installEventFilter(this);
        }
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
    filterDialogEvent(watched, event);
    filterLineEditEvent(watched, event);
    filterPlainTextEditEvent(watched, event);
    return false;
}

void VirtualInput::filterDialogEvent(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        QDialog * dialog = qobject_cast<QDialog *>(watched);
        if (dialog == nullptr) return;
        QLineEdit * virtualInput = dialog->findChild<QLineEdit *>(INPUT_OBJECT_NAME);
        if (virtualInput == nullptr) return;
        QWidget * widget = qobject_cast<QWidget *>(virtualInput->parent());
        if (!widget->isVisible()) return;
        updateGeometry(dialog, widget);
    }
}

void VirtualInput::filterLineEditEvent(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyRelease || event->type() == QEvent::InputMethodQuery) {
        QLineEdit * lineEdit = qobject_cast<QLineEdit *>(watched);
        if (lineEdit == nullptr) return;
        QDialog * dialog = findParentDialog(lineEdit);
        if (dialog == nullptr) return;
        QLineEdit * virtualInput = dialog->findChild<QLineEdit *>(INPUT_OBJECT_NAME);
        if (virtualInput == nullptr) return;
        QWidget * widget = qobject_cast<QWidget *>(virtualInput->parent());
        if (widget == nullptr) return;
        if (event->type() == QEvent::InputMethodQuery && !widget->isVisible()) return;

        if (timer.isActive()) {
            timer.stop();
            timer.start();
        }
        if (lineEdit != virtualInput) {
            virtualInput->blockSignals(true);
            virtualInput->setText(lineEdit->text());
            virtualInput->blockSignals(false);
            if (!widget->isVisible()) {
                showWidget(widget);
                updateGeometry(dialog, widget);
            }
            activeInput = lineEdit;
        } else {
            QLineEdit * activeLineEdit = qobject_cast<QLineEdit *>(activeInput);
            if (activeLineEdit != nullptr) {
                activeLineEdit->blockSignals(true);
                activeLineEdit->setText(virtualInput->text());
                activeLineEdit->blockSignals(false);
            } else {
                QPlainTextEdit * activePlainTextEdit = qobject_cast<QPlainTextEdit *>(activeInput);
                if (activePlainTextEdit != nullptr) {
                    activePlainTextEdit->blockSignals(true);
                    QTextCursor textCursor = activePlainTextEdit->textCursor();
                    textCursor.movePosition(QTextCursor::StartOfBlock);
                    textCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    textCursor.insertText(virtualInput->text());
                    activePlainTextEdit->blockSignals(false);
                }
            }
        }
    }
}

void VirtualInput::filterPlainTextEditEvent(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::KeyRelease || event->type() == QEvent::InputMethodQuery) {
        QPlainTextEdit * pTextEdit = qobject_cast<QPlainTextEdit *>(watched);
        if (pTextEdit == nullptr) return;
        QDialog * dialog = findParentDialog(pTextEdit);
        if (dialog == nullptr) return;
        QLineEdit * virtualInput = dialog->findChild<QLineEdit *>(INPUT_OBJECT_NAME);
        if (virtualInput == nullptr) return;
        QWidget * widget = qobject_cast<QWidget *>(virtualInput->parent());
        if (widget == nullptr) return;
        if (event->type() == QEvent::InputMethodQuery && !widget->isVisible()) return;

        if (timer.isActive()) {
            timer.stop();
            timer.start();
        }
        virtualInput->blockSignals(true);
        virtualInput->setText(pTextEdit->textCursor().block().text());
        virtualInput->blockSignals(false);
        if (!widget->isVisible()) {
            showWidget(widget);
            updateGeometry(dialog, widget);
        }
        activeInput = pTextEdit;
    }
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
