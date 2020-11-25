#ifndef SCROLLER_H
#define SCROLLER_H

#include <QAbstractItemView>
#include <QTimer>

extern const char * SCROLLER_DISABLE_TIMER_PROPERTY;

class Scroller : public QObject
{
    Q_OBJECT
public:
    static Scroller& instance();
    static void enableGestures(QObject * object, bool horizontal = true, bool vertical = true);
    static void disableGestures(QObject * object);
    static void reset();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void _enableGestures(QObject * object, bool horizontal = true, bool vertical = true);
    void _disableGestures(QObject * object);
    void _reset();
private:
    Scroller();
    int mouseX;
    int mouseY;
    QPointF mousePoint;
    bool isActive;
    bool isTriggered;
    QObject * activeObject;
    QTimer timer;
private slots:
    void trigger();
};

#endif // SCROLLER_H
