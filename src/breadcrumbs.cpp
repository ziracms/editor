/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "breadcrumbs.h"

Breadcrumbs::Breadcrumbs(Editor * codeEditor) : QWidget(codeEditor)
{
    editor = codeEditor;
    setCursor(Qt::PointingHandCursor);
}

Breadcrumbs::~Breadcrumbs()
{

}

QSize Breadcrumbs::sizeHint() const {
    return QSize(0, editor->breadcrumbsHeight());
}

void Breadcrumbs::paintEvent(QPaintEvent *event)
{
    editor->breadcrumbsPaintEvent(event);
}

void Breadcrumbs::mousePressEvent(QMouseEvent * /*e*/)
{
    emit editor->breadcrumbsClick(editor->getTabIndex());
}

void Breadcrumbs::setText(QString txt)
{
    text = txt;
}

QString Breadcrumbs::getText()
{
    return text;
}
