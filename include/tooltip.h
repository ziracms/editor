/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QLabel>

class Tooltip : public QLabel
{
    Q_OBJECT
public:
    Tooltip();
    ~Tooltip();
protected:
    void mousePressEvent(QMouseEvent *e) override;
};

#endif // TOOLTIP_H
