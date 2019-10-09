/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef LINEMAP_H
#define LINEMAP_H

#include <QWidget>
#include "editor.h"

class LineMap : public QWidget
{
    Q_OBJECT
public:
    explicit LineMap(Editor * codeEditor);
    QSize sizeHint() const override;
    void addMark(int line);
    QVector<int> getMarks();
    void clearMarks();
    void addError(int line);
    QVector<int> getErrors();
    void clearErrors();
    void addWarning(int line);
    QVector<int> getWarnings();
    void clearWarnings();
    void clear();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    Editor * editor;
    QVector<int> marks;
    QVector<int> errors;
    QVector<int> warnings;
signals:

public slots:
};

#endif // LINEMAP_H
