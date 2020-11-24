#include "scroller.h"
#include <QAbstractItemView>
#include <QScrollArea>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTextEdit>
#include <QTreeWidget>
#include "helper.h"

const char * PROPERTY_HORIZONTAL_SCROLL = "horizontal";
const char * PROPERTY_VERTICAL_SCROLL = "vertical";

const char * SCROLLER_DISABLE_TIMER_PROPERTY = "disable-timer";

const int ACTIVATE_DELTA = 20;
const int TRIGGER_DELAY = 500;

Scroller::Scroller(): mouseX(-1), mouseY(-1), isActive(false), isTriggered(false), activeObject(nullptr) {
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(trigger()));
}

Scroller& Scroller::instance()
{
    static Scroller _instance;
    return _instance;
}

void Scroller::enableGestures(QObject *object, bool horizontal, bool vertical)
{
    instance()._enableGestures(object, horizontal, vertical);
}

void Scroller::_enableGestures(QObject *object, bool horizontal, bool vertical)
{
    if (!horizontal && !vertical) return;
    QAbstractScrollArea * scrollArea = qobject_cast<QAbstractScrollArea *>(object);
    if (scrollArea == nullptr) return;
    scrollArea->viewport()->installEventFilter(this);
    scrollArea->setProperty(PROPERTY_HORIZONTAL_SCROLL, QVariant(horizontal));
    scrollArea->setProperty(PROPERTY_VERTICAL_SCROLL, QVariant(vertical));
    QAbstractItemView * itemView = qobject_cast<QAbstractItemView *>(object);
    if (itemView != nullptr) {
        itemView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        itemView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }
}

void Scroller::disableGestures(QObject *object)
{
    instance()._disableGestures(object);
}

void Scroller::_disableGestures(QObject *object)
{
    QAbstractScrollArea * scrollArea = qobject_cast<QAbstractScrollArea *>(object);
    if (scrollArea == nullptr) return;
    scrollArea->viewport()->removeEventFilter(this);
}

void Scroller::reset()
{
    instance()._reset();
}

void Scroller::_reset()
{
    mouseX = -1;
    mouseY = -1;
    isActive = false;
    isTriggered = false;
    activeObject = nullptr;
    if (timer.isActive()) timer.stop();
}

bool Scroller::eventFilter(QObject *watched, QEvent *event)
{
    QAbstractScrollArea * scrollArea = qobject_cast<QAbstractScrollArea *>(watched->parent());
    if (scrollArea == nullptr) return false;
    QVariant disableTimerV = scrollArea->property(SCROLLER_DISABLE_TIMER_PROPERTY);
    bool disableTimer = disableTimerV.isValid() ? disableTimerV.toBool() : false;
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr && mouseEvent->buttons() == Qt::LeftButton) {
            mouseX = mouseEvent->globalX();
            mouseY = mouseEvent->globalY();
            isActive = false;
            if (!isTriggered && !disableTimer) {
                activeObject = scrollArea;
                if (timer.isActive()) timer.stop();
                timer.start(TRIGGER_DELAY);
                return true;
            }
        }
    }
    if(event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr) {
            isTriggered = false;
            mouseX = -1;
            mouseY = -1;
            if (timer.isActive()) {
                timer.stop();
                if (!isActive) {
                    timer.start(0);
                    return true;
                }
            }
            if (isActive) {
                isActive = false;
                if (!disableTimer) return true;
            }
        }
    }
    if(event->type() == QEvent::MouseMove) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr && mouseEvent->buttons() == Qt::LeftButton && !isTriggered) {
            QVariant horizontalV = scrollArea->property(PROPERTY_HORIZONTAL_SCROLL);
            bool horizontal = horizontalV.isValid() ? horizontalV.toBool() : false;
            QVariant verticalV = scrollArea->property(PROPERTY_VERTICAL_SCROLL);
            bool vertical = verticalV.isValid() ? verticalV.toBool() : false;
            if (horizontal && mouseX >= 0) {
                int deltaX = mouseEvent->globalX() - mouseX;
                if (isActive || std::abs(deltaX) >= ACTIVATE_DELTA) {
                    if (isActive && scrollArea->horizontalScrollBar()->isVisible()) {
                        int sliderPos = scrollArea->horizontalScrollBar()->sliderPosition() - deltaX;
                        if (sliderPos < scrollArea->horizontalScrollBar()->minimum()) sliderPos = scrollArea->horizontalScrollBar()->minimum();
                        if (sliderPos > scrollArea->horizontalScrollBar()->maximum()) sliderPos = scrollArea->horizontalScrollBar()->maximum();
                        scrollArea->horizontalScrollBar()->setSliderPosition(sliderPos);
                    } else {
                        isActive = true;
                    }
                    mouseX = mouseEvent->globalX();
                }
            }
            if (vertical && mouseY >= 0) {
                int deltaY = mouseEvent->globalY() - mouseY;
                if (isActive || std::abs(deltaY) >= ACTIVATE_DELTA) {
                    if (isActive && scrollArea->verticalScrollBar()->isVisible()) {
                        int sliderPos = scrollArea->verticalScrollBar()->sliderPosition() - deltaY;
                        if (sliderPos < scrollArea->verticalScrollBar()->minimum()) sliderPos = scrollArea->verticalScrollBar()->minimum();
                        if (sliderPos > scrollArea->verticalScrollBar()->maximum()) sliderPos = scrollArea->verticalScrollBar()->maximum();
                        scrollArea->verticalScrollBar()->setSliderPosition(sliderPos);
                    } else {
                        isActive = true;
                    }
                    mouseY = mouseEvent->globalY();
                }
            }
        }
    }
    return false;
}

void Scroller::trigger()
{
    if (isActive) return;
    QAbstractScrollArea * scrollArea = qobject_cast<QAbstractScrollArea *>(activeObject);
    if (scrollArea == nullptr) return;
    isTriggered = true;
    QMouseEvent pressEvent(QEvent::MouseButtonPress, scrollArea->mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(scrollArea->viewport(), &pressEvent);
    if (timer.interval() == 0) {
        QMouseEvent moveEvent(QEvent::MouseMove, scrollArea->mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(scrollArea->viewport(), &moveEvent);

        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, scrollArea->mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(scrollArea->viewport(), &releaseEvent);
    }
}
