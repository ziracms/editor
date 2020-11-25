#include "welcome.h"
#include <QTimer>
#include "scroller.h"
#include "helper.h"
#include "settings.h"

Welcome::Welcome(bool light, QWidget *parent) : QWidget(parent),
    ui(new Ui::WelcomeScreen())
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
    if (Settings::get("enable_android_gestures") == "yes") {
        Scroller::enableGestures(ui->welcomeScrollArea);
    }
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

void Welcome::onCreateProjectPressed()
{
    emit createProject();
}

void Welcome::onOpenProjectPressed()
{
    emit openProject();
}
