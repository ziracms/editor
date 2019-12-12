#include "popup.h"
#include <QTimer>
#include <QPropertyAnimation>
#include <QFontDatabase>
#include <QPainter>
#include <QPaintEvent>
#include "math.h"

const int WIDGET_MIN_WIDTH = 600;
const int WIDGET_MIN_HEIGHT = 60;
const int IMAGE_WIDTH = 150;
const int PADDING = 10;
const int BORDER = 2;

const int ANIMATION_DURATION = 200;
const int ANIMATION_OFFSET = 150;
const int HIDE_DELAY = 4000;

Popup::Popup(Settings * settings, QWidget *parent) : QWidget(parent)
{
    setMinimumWidth(WIDGET_MIN_WIDTH);
    setMinimumHeight(WIDGET_MIN_HEIGHT);

    std::string tooltipBgColorStr = settings->get("editor_tooltip_bg_color");
    std::string tooltipColorStr = settings->get("editor_tooltip_color");
    std::string widgetBorderColorStr = settings->get("editor_widget_border_color");

    QFont sysFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setMargin(0);
    hLayout->setSpacing(0);

    imgLabel = new QLabel();
    QPixmap pm(":/image/vanko1");
    imgLabel->setPixmap(pm);
    imgLabel->setScaledContents(false);
    imgLabel->setFixedWidth(IMAGE_WIDTH);
    imgLabel->setAlignment(Qt::AlignVCenter);
    hLayout->addWidget(imgLabel);

    textLabel = new QLabel();
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignTop);
    textLabel->setMinimumWidth(WIDGET_MIN_WIDTH - IMAGE_WIDTH);
    textLabel->setFont(sysFont);
    textLabel->setContentsMargins(0, 0, 0, 0);
    textLabel->setMargin(PADDING);
    textLabel->setStyleSheet("background:"+QString::fromStdString(tooltipBgColorStr)+";color:"+QString::fromStdString(tooltipColorStr)+";");
    hLayout->addWidget(textLabel);

    hLayout->addStretch();
    setStyleSheet("background:"+QString::fromStdString(widgetBorderColorStr)+";border:"+QString::number(BORDER)+"px solid "+QString::fromStdString(widgetBorderColorStr)+";");
    hide();
}

Popup::~Popup()
{

}

QSize Popup::sizeHint() const {
    return QSize(WIDGET_MIN_WIDTH, WIDGET_MIN_HEIGHT);
}

void Popup::mousePressEvent(QMouseEvent * /*e*/)
{
    hide();
}

void Popup::hide()
{
    setVisible(false);
    animationInProgress = false;
}

void Popup::animateIn()
{
    QRect rect = geometry();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(ANIMATION_DURATION);
    animation->setStartValue(QRect(rect.x(), rect.y()-ANIMATION_OFFSET, rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));

    QEasingCurve easing(QEasingCurve::OutCubic);
    animation->setEasingCurve(easing);

    animation->start();
}

void Popup::animateOut()
{
    QRect rect = geometry();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(ANIMATION_DURATION);
    animation->setStartValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.x(), rect.y()-ANIMATION_OFFSET, rect.width(), rect.height()));

    QEasingCurve easing(QEasingCurve::InCubic);
    animation->setEasingCurve(easing);

    animation->start();
}

void Popup::display(int x, int y, QString text)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    QRect rect = geometry();
    rect.setTop(y);
    rect.setLeft(x);
    setGeometry(rect);
    textLabel->setText(text);

    QFontMetrics fm(textLabel->font());
    int w = geometry().width() - IMAGE_WIDTH - 2*PADDING;
    int tw = fm.width(text);
    if (tw > w) {
        int l = static_cast<int>(ceil(static_cast<double>(tw) / static_cast<double>(w)));
        setFixedHeight(fm.height()*l+2*PADDING+2*BORDER);
    } else {
        setFixedHeight(fm.height()+2*PADDING+2*BORDER);
    }

    setVisible(true);
    raise();

    if (!animationInProgress) {
        animationInProgress = true;
        animateIn();
        QTimer::singleShot(HIDE_DELAY-2*ANIMATION_DURATION, this, SLOT(animateOut()));
        QTimer::singleShot(HIDE_DELAY, this, SLOT(hide()));
    }
}

void Popup::displayText(int x, int y, QString text)
{
    QPixmap pm(":/image/vanko2");
    imgLabel->setPixmap(pm);

    display(x, y, text);
}

void Popup::displayError(int x, int y, QString text)
{
    QPixmap pm(":/image/vanko1");
    imgLabel->setPixmap(pm);

    display(x, y, text);
}
