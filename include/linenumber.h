/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef LINENUMBER_H
#define LINENUMBER_H

#include <QWidget>
#include "editor.h"

class LineNumber : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumber(Editor * editor);
    ~LineNumber();
    QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    Editor * editor;
signals:

public slots:
};

#endif // LINENUMBER_H
