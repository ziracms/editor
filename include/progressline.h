#ifndef PROGRESSLINE_H
#define PROGRESSLINE_H

#include <QWidget>
#include "settings.h"

class ProgressLine : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressLine(Settings * settings, QWidget *parent = nullptr);
    void updateGeometry(int x, int y, int w);
    void activate();
    void deactivate();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QColor progressColor;
    int progress;
    bool active;
    bool wantStop;
signals:

public slots:
    void updateProgress();
};

#endif // PROGRESSLINE_H
