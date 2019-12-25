#include "annotation.h"
#include <QScrollBar>

const int ANIMATION_DURATION = 200;

Annotation::Annotation(Editor * editor, Settings * settings) :
    QWidget(editor),
    editor(editor)
{
    setCursor(Qt::IBeamCursor);
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

    animationInProgress = false;
    QEasingCurve easing(QEasingCurve::InCubic);

    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);

    animationIn = new QPropertyAnimation(opacityEffect, "opacity");
    animationIn->setDuration(ANIMATION_DURATION);
    animationIn->setStartValue(0);
    animationIn->setEndValue(1);
    animationIn->setEasingCurve(easing);
    connect(animationIn, SIGNAL(finished()), this, SLOT(animationInFinished()));

    animationOut = new QPropertyAnimation(opacityEffect, "opacity");
    animationOut->setDuration(ANIMATION_DURATION);
    animationOut->setStartValue(1);
    animationOut->setEndValue(0);
    animationOut->setEasingCurve(easing);
    connect(animationOut, SIGNAL(finished()), this, SLOT(animationOutFinished()));

    hide();
}

QSize Annotation::sizeHint() const {
    return QSize(0, 0);
}

void Annotation::fadeIn()
{
    if (animationInProgress) return;
    show();
    animationInProgress = true;
    animationIn->start();
}

void Annotation::fadeOut()
{
    if (animationInProgress) return;
    show();
    animationInProgress = true;
    animationOut->start();
}

void Annotation::animationInFinished()
{
    animationInProgress = false;
}

void Annotation::animationOutFinished()
{
    animationInProgress = false;
    hide();
}

void Annotation::wheelEvent(QWheelEvent */*event*/)
{
    if (isVisible()) hide();
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
