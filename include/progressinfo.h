#ifndef PROGRESSINFO_H
#define PROGRESSINFO_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include "settings.h"

class ProgressInfo : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressInfo(QWidget *parent = nullptr);
    void setText(QString text);
    void activate();
    void deactivate();
    void updateGeometry(int x, int y, int w, int h);
private:
    QHBoxLayout * layout;
    QLabel * label;
    QToolButton * btn;
signals:
    void cancelTriggered();
public slots:
    void cancelPressed();
};

#endif // PROGRESSINFO_H
