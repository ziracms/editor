#include "welcome.h"
#include <QTimer>
#include <QScroller>
#include "helper.h"

#if defined(Q_OS_ANDROID)
const int ANDROID_PUSH_BUTTONS_DELAY = 100;
#endif

Welcome::Welcome(bool light, QWidget *parent) : QWidget(parent),
    ui(new Ui::WelcomeScreen()),
    isGesturesEnabled(false)
{
    ui->setupUi(this);

    ui->welcomeLabel->setStyleSheet("background: none");
    ui->welcomeLabel->setText("");
    if (light) {
        ui->welcomeLabel->setPixmap(QPixmap(":/styles/light/zira.png"));
    } else {
        ui->welcomeLabel->setPixmap(QPixmap(":/styles/dark/zira.png"));
    }

    ui->welcomeBottomLabel->setText("ver."+APPLICATION_VERSION);

    #if defined(Q_OS_ANDROID)
    ui->welcomeLabelLayout->setContentsMargins(0, 0, 0, 10);
    // scrolling by gesture
    enableGestures();
    #endif

    connect(ui->welcomeOpenProjectButton, SIGNAL(pressed()), this, SLOT(onOpenProjectPressed()));
    connect(ui->welcomeCreateProjectButton, SIGNAL(pressed()), this, SLOT(onCreateProjectPressed()));

    hide();
}

Welcome::~Welcome()
{
    delete ui;
}

void Welcome::focus()
{
    ui->welcomeOpenProjectButton->setFocus();
}

void Welcome::enableGestures()
{
    QScroller::grabGesture(ui->welcomeScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    isGesturesEnabled = true;
}

void Welcome::disableGestures()
{
    QScroller::ungrabGesture(ui->welcomeScrollArea->viewport());
    isGesturesEnabled = false;
}

void Welcome::onCreateProjectPressed()
{
    #if defined(Q_OS_ANDROID)
    // temporarily disable gestures and call it again after delay
    if (isGesturesEnabled) {
        disableGestures();
        QTimer::singleShot(ANDROID_PUSH_BUTTONS_DELAY, this, SLOT(onCreateProjectPressed()));
        return;
    }
    #endif
    emit createProject();
    #if defined(Q_OS_ANDROID)
    enableGestures();
    #endif
}

void Welcome::onOpenProjectPressed()
{
    #if defined(Q_OS_ANDROID)
    // temporarily disable gestures and call it again after delay
    if (isGesturesEnabled) {
        disableGestures();
        QTimer::singleShot(ANDROID_PUSH_BUTTONS_DELAY, this, SLOT(onOpenProjectPressed()));
        return;
    }
    #endif
    emit openProject();
    #if defined(Q_OS_ANDROID)
    enableGestures();
    #endif
}
