/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "completepopup.h"
#include <QScrollBar>
#include "icon.h"

//const int COMPLETE_POPUP_MIN_WIDTH = 150;
//const int COMPLETE_POPUP_MAX_WIDTH = 400;
//const int COMPLETE_POPUP_MAX_HEIGHT = 160;
const int COMPLETE_POPUP_ITEM_EXTRA_SPACE = 40;
const int COMPLETE_POPUP_ICON_SIZE = 10;
const int COMPLETE_POPUP_MAX_VISIBLE_ROWS_COUNT = 5;
const int ITEMS_LIMIT = 100;

CompletePopup::CompletePopup(QWidget * parent) : QListWidget(parent)
{
    setVisible(false);
    /*
    setMaximumWidth(COMPLETE_POPUP_MAX_WIDTH);
    setMaximumHeight(COMPLETE_POPUP_MAX_HEIGHT);
    setMinimumWidth(COMPLETE_POPUP_MIN_WIDTH);
    */
    setFocusProxy(parent);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setIconSize(QSize(COMPLETE_POPUP_ICON_SIZE, COMPLETE_POPUP_ICON_SIZE));
    setContentsMargins(0,0,0,0);
    setSpacing(0);
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
    newItem->setIcon(Icon::get("right", QIcon(":/icons/item.png")));
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
    int rowCo = model()->rowCount();
    /*
    if (rowCo>0) {
        QFontMetrics fm(font());
        for (int i=0; i<rowCo; i++) {
            QListWidgetItem * itm = item(i);
            QString txt = itm->text();
            int iw = fm.width(txt) + iconSize().width() + COMPLETE_POPUP_ITEM_EXTRA_SPACE;
            if (iw > width) width = iw;
        }
        if (width > COMPLETE_POPUP_MAX_WIDTH) width = COMPLETE_POPUP_MAX_WIDTH;
        height = rowCo * fm.height() + COMPLETE_POPUP_ITEM_EXTRA_SPACE;
        if (height > COMPLETE_POPUP_MAX_HEIGHT) height = COMPLETE_POPUP_MAX_HEIGHT;
        setCurrentRow(0);
    }
    */
    if (rowCo>0) {
        setCurrentRow(0);
        width = sizeHintForColumn(0) + frameWidth() * 2;
        width += COMPLETE_POPUP_ITEM_EXTRA_SPACE; // for right margin
        int co = COMPLETE_POPUP_MAX_VISIBLE_ROWS_COUNT;
        if (rowCo < co) co = rowCo;
        height = co * sizeHintForRow(0) + frameWidth() * 2;
        if (width < viewWidth) setMaximumWidth(width);
        else setMaximumWidth(viewWidth);
        if (height < viewHeight) setMaximumHeight(height);
        else setMaximumHeight(viewHeight);
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
        y = viewTop + cursTop - height;
    }
    if (y < 0) {
        y = 0;
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
