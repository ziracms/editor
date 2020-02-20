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

    connect(ui->welcomeOpenProjectButton, SIGNAL(pressed()), parent, SLOT(on_actionOpenProject_triggered()));
    connect(ui->welcomeCreateProjectButton, SIGNAL(pressed()), parent, SLOT(on_actionNewProject_triggered()));

    hide();
}

Welcome::~Welcome()
{
    delete ui;
}
