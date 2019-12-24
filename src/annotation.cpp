#include "annotation.h"
#include <QScrollBar>

Annotation::Annotation(Editor * editor, Settings * settings) :
    QWidget(editor),
    editor(editor)
{
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);

    label = new QLabel(this);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(false);
    label->setAlignment(Qt::AlignRight);
    label->setMinimumWidth(0);
    label->setFont(editor->font());
    label->setContentsMargins(0, 0, 0, 0);
    label->setMargin(0);
    label->setMouseTracking(true);

    QString colorStr = QString::fromStdString(settings->get("highlight_single_line_comment_color"));
    label->setStyleSheet("background:none;color:"+colorStr+";");

    hide();
}

QSize Annotation::sizeHint() const {
    return QSize(0, 0);
}

void Annotation::mouseMoveEvent(QMouseEvent */*event*/)
{
    hide();
}

void Annotation::setText(QString text)
{
    label->setText(text);
}

QString Annotation::getText()
{
    return label->text();
}

void Annotation::setSize(int w, int h)
{
    label->setFixedSize(w, h);
    setFixedSize(w, h);
}
