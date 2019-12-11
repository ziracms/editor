#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include "settings.h"

class Popup : public QWidget
{
    Q_OBJECT
public:
    explicit Popup(Settings * settings, QWidget *parent = nullptr);
    ~Popup();
    QSize sizeHint() const override;
    void displayText(int x, int y, QString text);
    void displayError(int x, int y, QString text);
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void display(int x, int y, QString text);
    QHBoxLayout * hLayout;
    QLabel * imgLabel;
    QLabel * textLabel;
private:
    bool animationInProgress;
signals:

public slots:
    void hide();
    void animateIn();
    void animateOut();
};

#endif // POPUP_H
