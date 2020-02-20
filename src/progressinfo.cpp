#include "progressinfo.h"
#include <QFontDatabase>

const int BORDER = 1;
const int PADDING = 5;

ProgressInfo::ProgressInfo(Settings * settings, QWidget *parent) : QWidget(parent)
{
    std::string bgColorStr = settings->get("popup_bg_color");
    std::string colorStr = settings->get("popup_color");

    QFont popupFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    std::string fontFamily = settings->get("editor_font_family");
    std::string fontSize = settings->get("editor_tooltip_font_size");
    if (fontFamily.size() > 0) {
        popupFont.setStyleHint(QFont::Monospace);
        popupFont.setFamily(QString::fromStdString(fontFamily));
    }
    popupFont.setPointSize(std::stoi(fontSize));
    popupFont.setStyleName("");

    QFontMetrics fm(popupFont);
    int height = fm.height();

    setMinimumHeight(height);

    layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetMaximumSize);

    label = new QLabel();
    label->setWordWrap(false);
    label->setMinimumHeight(height);
    label->setTextFormat(Qt::PlainText);
    label->setFont(popupFont);
    label->setContentsMargins(PADDING, 0, PADDING, 0);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setStyleSheet("background:"+QString::fromStdString(bgColorStr)+";color:"+QString::fromStdString(colorStr)+";");

    layout->addWidget(label);

    btn = new QToolButton();
    btn->setIcon(QIcon(":/icons/close.png"));
    btn->setMinimumWidth(height);
    btn->setMinimumHeight(height);
    btn->setToolTip(tr("Cancel"));
    btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addWidget(btn);

    setLayout(layout);

    connect(btn, SIGNAL(pressed()), this, SLOT(cancelPressed()));
    setStyleSheet("background:"+QString::fromStdString(bgColorStr)+";border:"+QString::number(BORDER)+"px solid "+QString::fromStdString(bgColorStr)+";");

    hide();
}

void ProgressInfo::setText(QString text)
{
    label->setText(text);
}

void ProgressInfo::activate()
{
    show();
    raise();
}

void ProgressInfo::deactivate()
{
    hide();
}

void ProgressInfo::cancelPressed()
{
    emit cancelTriggered();
}

void ProgressInfo::updateGeometry(int x, int y, int w, int h)
{
    setGeometry(x, y, w, h);
}
