/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "linemap.h"
#include "helper.h"

LineMap::LineMap(Editor * codeEditor) : QWidget(codeEditor)
{
    editor = codeEditor;
    //setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
}

QSize LineMap::sizeHint() const {
    return QSize(editor->lineMapAreaWidth(), 0);
}

void LineMap::paintEvent(QPaintEvent *event)
{
    editor->lineMapAreaPaintEvent(event);
}

void LineMap::addMark(int line)
{
    marks.append(line);
}

QVector<int> LineMap::getMarks()
{
    return marks;
}

void LineMap::clearMarks()
{
    marks.clear();
}

void LineMap::addError(int line)
{
    errors.append(line);
}

QVector<int> LineMap::getErrors()
{
    return errors;
}

void LineMap::clearErrors()
{
    errors.clear();
}

void LineMap::addWarning(int line)
{
    warnings.append(line);
}

QVector<int> LineMap::getWarnings()
{
    return warnings;
}

void LineMap::clearWarnings()
{
    warnings.clear();
}

void LineMap::clear()
{
    clearMarks();
    clearErrors();
    clearWarnings();
}

void LineMap::mousePressEvent(QMouseEvent *event)
{
    editor->scrollLineMap(event->y());
}

void LineMap::mouseMoveEvent(QMouseEvent *event)
{
    editor->showLineMap(event->y());
}
