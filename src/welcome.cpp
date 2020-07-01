#include "welcome.h"
#include "helper.h"

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
    #endif

    hide();
}

Welcome::~Welcome()
{
    delete ui;
}

void Welcome::connectButtons(QWidget *mainWnd)
{
    connect(ui->welcomeOpenProjectButton, SIGNAL(pressed()), mainWnd, SLOT(on_actionOpenProject_triggered()));
    connect(ui->welcomeCreateProjectButton, SIGNAL(pressed()), mainWnd, SLOT(on_actionNewProject_triggered()));
}
