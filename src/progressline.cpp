#include "progressline.h"
#include <QPainter>
#include <QTimer>
#include "helper.h"

const int PROGRESS_LINE_HEIGHT = 2;
const int PROGRESS_PARTS_COUNT = 1;
const int PROGRESS_MAX = 100;
const int UPDATE_MILLISECONDS = 10;

ProgressLine::ProgressLine(QWidget *parent) : QWidget(parent)
{
    progressColor = QColor(QString::fromStdString(Settings::get("progress_color")));
    progress = 0;
    active = false;
    wantStop = false;
    hide();
}

void ProgressLine::paintEvent(QPaintEvent */*event*/)
{
    double pieceW = geometry().width() / PROGRESS_PARTS_COUNT;
    double progressW = pieceW / PROGRESS_MAX;

    double deltaW = 0;
    if (progress <= PROGRESS_MAX / 2) {
        deltaW = progress * (progressW / 2);
    } else {
        deltaW = (PROGRESS_MAX - progress) * (progressW / 2);
    }

    double deltaX = 0;
    if (progress > PROGRESS_MAX / 2) {
        deltaX = (progress - PROGRESS_MAX / 2) * (progressW / 2);
    }

    QPainter painter(this);
    for (int i=0; i< PROGRESS_PARTS_COUNT; i++) {
        int x = static_cast<int>(pieceW * i + progress * progressW + deltaX);
        int w = static_cast<int>(progressW + deltaW);
        if (x + w > geometry().width()) w = geometry().width() - x;
        painter.fillRect(x, 0, w, geometry().height(), progressColor);
    }
}

void ProgressLine::updateGeometry(int x, int y, int w)
{
    setGeometry(x, y - PROGRESS_LINE_HEIGHT, w, PROGRESS_LINE_HEIGHT);
}

void ProgressLine::updateProgress()
{
    progress++;
    if (progress >= PROGRESS_MAX) {
        progress = 0;
        if (wantStop) active = false;
    }
    update();
    if (active) QTimer::singleShot(UPDATE_MILLISECONDS, this, SLOT(updateProgress()));
    else hide();
}

void ProgressLine::activate()
{
    if (active) {
        wantStop = false;
        return;
    }
    active = true;
    wantStop = false;
    show();
    raise();
    updateProgress();
}

void ProgressLine::deactivate()
{
    wantStop = true;
}
