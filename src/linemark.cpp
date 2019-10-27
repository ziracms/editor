/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "linemark.h"

LineMark::LineMark(Editor * codeEditor) : QWidget(codeEditor)
{
    editor = codeEditor;
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
}

QSize LineMark::sizeHint() const {
    return QSize(editor->lineMarkAreaWidth(), 0);
}

void LineMark::paintEvent(QPaintEvent *event)
{
    editor->lineMarkAreaPaintEvent(event);
}

void LineMark::addMark(int line, QString text)
{
    marks[line] = text.toStdString();
}

int LineMark::getMark(int line, QString & text)
{
    marksIterator = marks.find(line);
    if (marksIterator != marks.end()) {
        text = QString::fromStdString(marksIterator->second);
        return marksIterator->first;
    }
    return -1;
}

void LineMark::clearMarks()
{
    marks.clear();
}

void LineMark::addError(int line, QString text)
{
    errors[line] = text.toStdString();
}

int LineMark::getError(int line, QString & text)
{
    errorsIterator = errors.find(line);
    if (errorsIterator != errors.end()) {
        text = QString::fromStdString(errorsIterator->second);
        return errorsIterator->first;
    }
    return -1;
}

void LineMark::clearErrors()
{
    errors.clear();
}

void LineMark::addWarning(int line, QString text)
{
    warnings[line] = text.toStdString();
}

int LineMark::getWarning(int line, QString & text)
{
    warningsIterator = warnings.find(line);
    if (warningsIterator != warnings.end()) {
        text = QString::fromStdString(warningsIterator->second);
        return warningsIterator->first;
    }
    return -1;
}

void LineMark::clearWarnings()
{
    warnings.clear();
}

void LineMark::clear()
{
    clearMarks();
    clearErrors();
    clearWarnings();
}

void LineMark::mouseMoveEvent(QMouseEvent *event)
{
    editor->showLineMark(event->y());
}

void LineMark::mousePressEvent(QMouseEvent *event)
{
    editor->addLineMark(event->y());
}

int LineMark::getErrorsCount() {
    return errors.size();
}

int LineMark::getWarningsCount() {
    return warnings.size();
}

int LineMark::getMarksCount() {
    return marks.size();
}
