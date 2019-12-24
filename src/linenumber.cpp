/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "linenumber.h"

LineNumber::LineNumber(Editor * codeEditor) : QWidget(codeEditor)
{
    editor = codeEditor;
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
}

QSize LineNumber::sizeHint() const {
    return QSize(editor->lineNumberAreaWidth(), 0);
}

void LineNumber::paintEvent(QPaintEvent *event)
{
    editor->lineNumberAreaPaintEvent(event);
}

void LineNumber::mouseMoveEvent(QMouseEvent *event)
{
    editor->showLineNumber(event->y());
}
