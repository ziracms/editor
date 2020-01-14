/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "tooltip.h"
#include <QPainter>
#include <QPaintEvent>

const int TOOLTIP_PADDING = 7;

Tooltip::Tooltip(Settings * settings)
{
    std::string tooltipBorderColorStr = settings->get("editor_tooltip_border_color");
    std::string tooltipBgColorStr = settings->get("editor_tooltip_bg_color");
    std::string tooltipColorStr = settings->get("editor_tooltip_color");

    tooltipBorderColor = QColor(tooltipBorderColorStr.c_str());
    tooltipBgColor = QColor(tooltipBgColorStr.c_str());
    tooltipColor = QColor(tooltipColorStr.c_str());

    setWindowFlag(Qt::ToolTip);
    setContentsMargins(TOOLTIP_PADDING, TOOLTIP_PADDING, TOOLTIP_PADDING, TOOLTIP_PADDING);
    setAutoFillBackground(true);
    QPalette tooltipPalette = palette();
    tooltipPalette.setColor(QPalette::Window, tooltipBgColor);
    tooltipPalette.setColor(QPalette::WindowText, tooltipColor);
    setPalette(tooltipPalette);
    setWordWrap(true);
    hide();
}

void Tooltip::mousePressEvent(QMouseEvent * /*e*/)
{
    hide();
}

void Tooltip::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(tooltipBorderColor);
    painter.drawRect(0, 0, geometry().width()-1, geometry().height()-1);

    QLabel::paintEvent(event);
}
