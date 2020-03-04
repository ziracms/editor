#include "docktitlebar.h"
#include "helper.h"

const int DOCK_TITLE_BAR_SIZE = 5;

DockTitleBar::DockTitleBar(QWidget *parent) : QWidget(parent)
{

}

QSize DockTitleBar::sizeHint() const
{
    return QSize(DOCK_TITLE_BAR_SIZE, DOCK_TITLE_BAR_SIZE);
}

QSize DockTitleBar::minimumSizeHint() const
{
    return QSize(DOCK_TITLE_BAR_SIZE, DOCK_TITLE_BAR_SIZE);
}
