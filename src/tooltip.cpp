/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "tooltip.h"

Tooltip::Tooltip()
{

}

void Tooltip::mousePressEvent(QMouseEvent * /*e*/)
{
    hide();
}
