#ifndef WELCOME_H
#define WELCOME_H

#include <QWidget>
#include "ui_welcome.h"

class Welcome : public QWidget
{
    Q_OBJECT
public:
    explicit Welcome(bool light, QWidget *parent = nullptr);
    ~Welcome();
    void connectButtons(QWidget * mainWnd);
    void focus();
private:
    Ui::WelcomeScreen * ui;
signals:

public slots:
};

#endif // WELCOME_H
