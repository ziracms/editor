/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef BREADCRUMBS_H
#define BREADCRUMBS_H

#include <QWidget>
#include "editor.h"

class Breadcrumbs : public QWidget
{
    Q_OBJECT
public:
    explicit Breadcrumbs(Editor * codeEditor);
    QSize sizeHint() const override;
    void setText(QString txt);
    QString getText();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
private:
    Editor * editor;
    QString text;
signals:

public slots:
};

#endif // BREADCRUMBS_H
