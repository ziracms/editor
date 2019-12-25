#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "editor.h"

extern const int ANNOTATION_LEFT_MARGIN;
extern const int ANNOTATION_RIGHT_MARGIN;

class Annotation : public QWidget
{
    Q_OBJECT
public:
    explicit Annotation(Editor * editor, Settings * settings);
    QSize sizeHint() const override;
    void setText(QString text);
    QString getText();
    void setSize(int w, int h);
    void fadeIn();
    void fadeOut();
protected:
    void wheelEvent(QWheelEvent *event) override;
private:
    Editor * editor;
    QHBoxLayout * hLayout;
    QLabel * imgLabel;
    QLabel * txtLabel;
    QPropertyAnimation *animationIn;
    QPropertyAnimation *animationOut;
    QGraphicsOpacityEffect *opacityEffect;
    bool animationInProgress;
signals:

public slots:
    void animationInFinished();
    void animationOutFinished();
};

#endif // ANNOTATION_H
