#ifndef SCROLLER_H
#define SCROLLER_H

#include <QAbstractItemView>

class Scroller : public QObject
{
    Q_OBJECT
public:
    static Scroller& instance();
    static void enableGestures(QObject * object, bool horizontal = true, bool vertical = true);
    static void disableGestures(QObject * object);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void _enableGestures(QObject * object, bool horizontal = true, bool vertical = true);
    void _disableGestures(QObject * object);
private:
    Scroller();
    int mouseX;
    int mouseY;
};

#endif // SCROLLER_H
