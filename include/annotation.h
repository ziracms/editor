#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QWidget>
#include "editor.h"

class Annotation : public QWidget
{
    Q_OBJECT
public:
    explicit Annotation(Editor * editor, Settings * settings);
    QSize sizeHint() const override;
    void setText(QString text);
    QString getText();
    void setSize(int w, int h);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    Editor * editor;
    QLabel * label;
signals:

public slots:
};

#endif // ANNOTATION_H
