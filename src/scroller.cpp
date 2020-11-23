#include "scroller.h"
#include <QAbstractItemView>
#include <QScrollArea>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include "helper.h"

const char * PROPERTY_HORIZONTAL_SCROLL = "horizontal";
const char * PROPERTY_VERTICAL_SCROLL = "vertical";

Scroller::Scroller(): mouseX(-1), mouseY(-1){}

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

bool Scroller::eventFilter(QObject *watched, QEvent *event)
{
    QAbstractScrollArea * scrollArea = qobject_cast<QAbstractScrollArea *>(watched->parent());
    if (scrollArea == nullptr) return false;
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr && mouseEvent->buttons() == Qt::LeftButton) {
            mouseX = mouseEvent->globalX();
            mouseY = mouseEvent->globalY();
        }
    }
    if(event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr && mouseEvent->buttons() == Qt::LeftButton) {
            mouseX = -1;
            mouseY = -1;
        }
    }
    if(event->type() == QEvent::MouseMove) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr && mouseEvent->buttons() == Qt::LeftButton) {
            QVariant horizontalV = scrollArea->property(PROPERTY_HORIZONTAL_SCROLL);
            bool horizontal = horizontalV.isValid() ? horizontalV.toBool() : false;
            QVariant verticalV = scrollArea->property(PROPERTY_VERTICAL_SCROLL);
            bool vertical = verticalV.isValid() ? verticalV.toBool() : false;
            if (horizontal && mouseX >= 0 && scrollArea->horizontalScrollBar()->isVisible()) {
                int deltaX = mouseEvent->globalX() - mouseX;
                if (deltaX != 0) {
                    int sliderPos = scrollArea->horizontalScrollBar()->sliderPosition() - deltaX;
                    if (sliderPos < scrollArea->horizontalScrollBar()->minimum()) sliderPos = scrollArea->horizontalScrollBar()->minimum();
                    if (sliderPos > scrollArea->horizontalScrollBar()->maximum()) sliderPos = scrollArea->horizontalScrollBar()->maximum();
                    scrollArea->horizontalScrollBar()->setSliderPosition(sliderPos);
                    mouseX = mouseEvent->globalX();
                }
            }
            if (vertical && mouseY >= 0 && scrollArea->verticalScrollBar()->isVisible()) {
                int deltaY = mouseEvent->globalY() - mouseY;
                if (deltaY != 0) {
                    int sliderPos = scrollArea->verticalScrollBar()->sliderPosition() - deltaY;
                    if (sliderPos < scrollArea->verticalScrollBar()->minimum()) sliderPos = scrollArea->verticalScrollBar()->minimum();
                    if (sliderPos > scrollArea->verticalScrollBar()->maximum()) sliderPos = scrollArea->verticalScrollBar()->maximum();
                    scrollArea->verticalScrollBar()->setSliderPosition(sliderPos);
                    mouseY = mouseEvent->globalY();
                }
            }
        }
    }
    return false;
}
