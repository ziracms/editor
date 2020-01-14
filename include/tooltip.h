/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QLabel>
#include "settings.h"

class Tooltip : public QLabel
{
    Q_OBJECT
public:
    Tooltip(Settings * settings);
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;
private:
    QColor tooltipBorderColor;
    QColor tooltipBgColor;
    QColor tooltipColor;
};

#endif // TOOLTIP_H
