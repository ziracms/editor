#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include "settings.h"

class Popup : public QWidget
{
    Q_OBJECT
public:
    explicit Popup(Settings * settings, QWidget *parent = nullptr);
    QSize sizeHint() const override;
    void displayText(int x, int y, QString text);
    void displayError(int x, int y, QString text);
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void display(int x, int y, QString text);
    QHBoxLayout * hLayout;
    QLabel * textLabel;
    std::string bgColorStr;
    std::string errorBgColorStr;
    std::string colorStr;
private:
    QPropertyAnimation *animationIn;
    QPropertyAnimation *animationOut;
    bool animationInProgress;
signals:

public slots:
    void hide();
    void animateIn();
    void animateOut();
    void animationInFinished();
    void animationOutFinished();
};

#endif // POPUP_H
