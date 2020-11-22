#include "popup.h"
#include <QTimer>
#include <QFontDatabase>
#include <QPainter>
#include <QPaintEvent>
#include "math.h"
#include "helper.h"

const int WIDGET_MIN_WIDTH = 600;
const int WIDGET_MIN_HEIGHT = 60;
const int PADDING = 10;
const int BORDER = 1;

const int ANIMATION_DURATION = 200;
const int ANIMATION_OFFSET = 150;
const int HIDE_DELAY = 4000;

Popup::Popup(QWidget *parent) : QWidget(parent)
{
    setMinimumWidth(WIDGET_MIN_WIDTH);
    setMinimumHeight(WIDGET_MIN_HEIGHT);

    bgColorStr = Settings::get("popup_bg_color");
    errorBgColorStr = Settings::get("popup_error_bg_color");
    colorStr = Settings::get("popup_color");

    QFont popupFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    std::string fontFamily = Settings::get("editor_font_family");
    std::string fontSize = Settings::get("editor_font_size");
    if (fontFamily.size() > 0) {
        popupFont.setStyleHint(QFont::Monospace);
        popupFont.setFamily(QString::fromStdString(fontFamily));
    }
    popupFont.setPointSize(std::stoi(fontSize));
    popupFont.setStyleName("");

    hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setMargin(0);
    hLayout->setSpacing(0);

    textLabel = new QLabel();
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignTop);
    textLabel->setMinimumWidth(WIDGET_MIN_WIDTH);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    textLabel->setFont(popupFont);
    textLabel->setContentsMargins(0, 0, 0, 0);
    textLabel->setMargin(PADDING);
    textLabel->setStyleSheet("background:"+QString::fromStdString(bgColorStr)+";color:"+QString::fromStdString(colorStr)+";");
    hLayout->addWidget(textLabel);

    hLayout->addStretch();
    setStyleSheet("background:"+QString::fromStdString(bgColorStr)+";border:"+QString::number(BORDER)+"px solid "+QString::fromStdString(bgColorStr)+";");

    animationInProgress = false;
    QEasingCurve easing(QEasingCurve::OutCubic);

    animationIn = new QPropertyAnimation(this, "geometry");
    animationIn->setDuration(ANIMATION_DURATION);
    animationIn->setEasingCurve(easing);
    connect(animationIn, SIGNAL(finished()), this, SLOT(animationInFinished()));

    animationOut = new QPropertyAnimation(this, "geometry");
    animationOut->setDuration(ANIMATION_DURATION);
    animationOut->setEasingCurve(easing);
    connect(animationOut, SIGNAL(finished()), this, SLOT(animationOutFinished()));

    hide();
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
    if (animationInProgress) return;
    animationInProgress = true;
    if (!isVisible()) setVisible(true);
    raise();
    QRect rect = geometry();
    animationIn->setStartValue(QRect(rect.x(), rect.y()-ANIMATION_OFFSET, rect.width(), rect.height()));
    animationIn->setEndValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
    animationIn->start();
}

void Popup::animateOut()
{
    if (!isVisible()) return;
    if (animationInProgress) return;
    animationInProgress = true;
    QRect rect = geometry();
    animationOut->setStartValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
    animationOut->setEndValue(QRect(rect.x(), rect.y()-ANIMATION_OFFSET, rect.width(), rect.height()));
    animationOut->start();
}

void Popup::animationInFinished()
{
    animationInProgress = false;
    QTimer::singleShot(HIDE_DELAY, this, SLOT(animateOut()));
}

void Popup::animationOutFinished()
{
    animationInProgress = false;
    hide();
}

void Popup::display(int x, int y, QString text)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    setMaximumWidth(16777215);
    setMinimumWidth(WIDGET_MIN_WIDTH);
    textLabel->setMaximumWidth(16777215);
    textLabel->setMinimumWidth(WIDGET_MIN_WIDTH);
    int width = geometry().width();
    QWidget * wnd = Helper::getWindowWidget();
    if (wnd != nullptr) {
        int sw = wnd->geometry().width();
        if (x + width > sw && x > 0) {
            x = 0;
        }
        if (x + width > sw && x == 0) {
            setMaximumWidth(sw);
            setMinimumWidth(sw);
            textLabel->setMinimumWidth(sw);
            textLabel->setMaximumWidth(sw);
        }
    }

    QRect rect = geometry();
    rect.setTop(y);
    rect.setLeft(x);
    setGeometry(rect);
    textLabel->setText(text);

    QFontMetrics fm(textLabel->font());
    int w = textLabel->geometry().width() - 2*PADDING;
    /* QFontMetrics::width is deprecated */
    /*
    int tw = fm.width(text);
    */
    int tw = fm.horizontalAdvance(text);
    if (tw > w) {
        int l = static_cast<int>(ceil(static_cast<double>(tw) / static_cast<double>(w)));
        setFixedHeight(fm.height()*l+2*PADDING+2*BORDER);
    } else {
        setFixedHeight(fm.height()+2*PADDING+2*BORDER);
    }

    animateIn();
}

void Popup::displayText(int x, int y, QString text)
{
    textLabel->setStyleSheet("background:"+QString::fromStdString(bgColorStr)+";color:"+QString::fromStdString(colorStr)+";");
    setStyleSheet("background:"+QString::fromStdString(bgColorStr)+";border:"+QString::number(BORDER)+"px solid "+QString::fromStdString(bgColorStr)+";");
    display(x, y, text);
}

void Popup::displayError(int x, int y, QString text)
{
    textLabel->setStyleSheet("background:"+QString::fromStdString(errorBgColorStr)+";color:"+QString::fromStdString(colorStr)+";");
    setStyleSheet("background:"+QString::fromStdString(errorBgColorStr)+";border:"+QString::number(BORDER)+"px solid "+QString::fromStdString(errorBgColorStr)+";");
    display(x, y, text);
}
