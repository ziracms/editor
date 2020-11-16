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
    void focus();
protected:
    void enableGestures();
    void disableGestures();
private:
    Ui::WelcomeScreen * ui;
    bool isGesturesEnabled;
signals:
    void openProject();
    void createProject();
public slots:
private slots:
    void onOpenProjectPressed();
    void onCreateProjectPressed();
};

#endif // WELCOME_H
