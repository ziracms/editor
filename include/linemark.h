/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef LINEMARK_H
#define LINEMARK_H

#include <QWidget>
#include "editor.h"

class LineMark : public QWidget
{
    Q_OBJECT
public:
    explicit LineMark(Editor * codeEditor);
    QSize sizeHint() const override;
    void addMark(int line, QString text = "");
    int getMark(int line, QString & text);
    void clearMarks();
    void addError(int line, QString text = "");
    int getError(int line, QString & text);
    void clearErrors();
    void addWarning(int line, QString text = "");
    int getWarning(int line, QString & text);
    void clearWarnings();
    void clear();
    int getErrorsCount();
    int getWarningsCount();
    int getMarksCount();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    Editor * editor;
    std::unordered_map<int, std::string> marks;
    std::unordered_map<int, std::string>::iterator marksIterator;
    std::unordered_map<int, std::string> errors;
    std::unordered_map<int, std::string>::iterator errorsIterator;
    std::unordered_map<int, std::string> warnings;
    std::unordered_map<int, std::string>::iterator warningsIterator;
signals:

public slots:
};

#endif // LINEMARK_H
