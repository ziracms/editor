#include "annotation.h"
#include <QScrollBar>

const int ANNOTATION_LEFT_MARGIN = 20;
const int ANNOTATION_RIGHT_MARGIN = 10;

const int ANIMATION_DURATION = 200;

Annotation::Annotation(Editor * editor, Settings * settings) :
    QWidget(editor),
    editor(editor)
{
    setCursor(Qt::IBeamCursor);
    setMouseTracking(true);

    hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setMargin(0);
    hLayout->setSpacing(0);

    imgLabel = new QLabel();
    QPixmap pm(":/icons/annotation.png");
    imgLabel->setPixmap(pm);
    imgLabel->setScaledContents(true);
    imgLabel->setCursor(Qt::ArrowCursor);
    imgLabel->setObjectName("annotationIcon");

    hLayout->addWidget(imgLabel);

    txtLabel = new QLabel();
    txtLabel->setTextFormat(Qt::PlainText);
    txtLabel->setWordWrap(false);
    txtLabel->setAlignment(Qt::AlignRight);
    txtLabel->setMinimumWidth(0);
    txtLabel->setFont(editor->font());
    txtLabel->setContentsMargins(0, 0, 0, 0);
    txtLabel->setMargin(0);
    txtLabel->setMouseTracking(true);

    hLayout->addWidget(txtLabel);

    imgLabel->setStyleSheet("#annotationIcon{background:none;}");

    QString colorStr = QString::fromStdString(settings->get("annotation_color"));
    txtLabel->setStyleSheet("background:none;color:"+colorStr+";");

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
    txtLabel->setText(text);
    imgLabel->setToolTip(text);
}

QString Annotation::getText()
{
    return txtLabel->text();
}

void Annotation::setSize(int w, int h)
{
    if (w < h) w = h;
    imgLabel->setFixedSize(h, h);
    txtLabel->setFixedSize(w - h, h);
    setFixedSize(w, h);
}
