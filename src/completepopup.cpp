/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "completepopup.h"
#include <QScrollBar>

const int COMPLETE_POPUP_MIN_WIDTH = 150;
const int COMPLETE_POPUP_MAX_WIDTH = 400;
const int COMPLETE_POPUP_MAX_HEIGHT = 160;

const int ITEMS_LIMIT = 100;

CompletePopup::CompletePopup(QWidget * parent) : QListWidget(parent)
{
    setVisible(false);
    setMaximumWidth(COMPLETE_POPUP_MAX_WIDTH);
    setMaximumHeight(COMPLETE_POPUP_MAX_HEIGHT);
    setMinimumWidth(COMPLETE_POPUP_MIN_WIDTH);
    setFocusProxy(parent);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

    textStartPos = -1;
}

void CompletePopup::setTextStartPos(int pos)
{
    textStartPos = pos;
}

int CompletePopup::getTextStartPos()
{
    return textStartPos;
}

void CompletePopup::clearItems()
{
    clear();
    textStartPos = -1;
}

void CompletePopup::addItem(QString str, QString data, QString delimiter)
{
    if (count() >= ITEMS_LIMIT) return;
    QListWidgetItem * newItem = new QListWidgetItem;
    QVariant itemData(data);
    newItem->setData(Qt::UserRole, itemData);
    newItem->setText(str);
    newItem->setIcon(style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton));
    if (data.size()>0 && data[0]=="(") {
        if (delimiter.size() > 0 && data.indexOf(delimiter) >= 0) {
            data.replace(delimiter, "\n"+str);
        }
        newItem->setToolTip(str+" "+data);
    } else {
        newItem->setToolTip(str);
    }
    insertItem(count(), newItem);
}

void CompletePopup::showPopup(int cursLeft, int cursTop, int viewLeft, int viewTop, int viewWidth, int viewHeight, int blockHeight)
{
    setVisible(true);
    int width = geometry().width(), height = geometry().height();
    if (count()>0) {
        int w = sizeHintForColumn(0);
        int h = sizeHintForRow(0);
        if (w > 0 && h > 0) {
            int vSw = verticalScrollBar()->width();
            if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) vSw = 0;
            int hSh = horizontalScrollBar()->height();
            if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) hSh = 0;
            width = w+vSw;
            if (width > COMPLETE_POPUP_MAX_WIDTH) width = COMPLETE_POPUP_MAX_WIDTH;
            height = ((count()+1) * h)+hSh;
            if (height > COMPLETE_POPUP_MAX_HEIGHT) height = COMPLETE_POPUP_MAX_HEIGHT;
        }
        setCurrentRow(0);
    }
    int x = viewLeft + cursLeft;
    if (x + width > viewLeft + viewWidth) {
        x = viewLeft + cursLeft - width;
    }
    if (x < 0) {
        x = 0;
    }
    int  y = viewTop + cursTop + blockHeight;
    if (y + height > viewTop + viewHeight) {
        int _y = viewTop + cursTop - height;
        if (_y >= 0) y = _y;
    }
    if (width > 0 && height > 0) {
        setGeometry(x, y , width, height);
    } else {
        move(x, y);
    }
}

void CompletePopup::hidePopup()
{
    setVisible(false);
}

void CompletePopup::onItemClicked(QListWidgetItem * item)
{
    QVariant itemData = item->data(Qt::UserRole);
    emit itemDataClicked(item->text(), itemData.toString());
}

void CompletePopup::selectNextItem()
{
    int total = count();
    if (total < 2) return;
    int current = currentRow();
    current++;
    if (current >= total) current = 0;
    setCurrentRow(current);
}

void CompletePopup::selectPreviousItem()
{
    int total = count();
    if (total < 2) return;
    int current = currentRow();
    current--;
    if (current < 0) current = total-1;
    setCurrentRow(current);
}

void CompletePopup::chooseCurrentItem()
{
    QListWidgetItem * item = currentItem();
    onItemClicked(item);
}

int CompletePopup::limit()
{
    return ITEMS_LIMIT;
}
