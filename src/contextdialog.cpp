#include "contextdialog.h"
#include <QScrollBar>
#include <QScreen>
#include "icon.h"
#include "helper.h"
#include "scroller.h"
#include "settings.h"

const int MENU_WIDTH_EXTRA_SPACE = 40;

ContextDialog::ContextDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContextDialog),
    parentWidget(parent)
{
    ui->setupUi(this);

    ui->contextListWidget->setFocusProxy(this);
    ui->contextListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->contextListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->contextListWidget->setContentsMargins(0,0,0,0);
    ui->contextListWidget->setSpacing(0);
    connect(ui->contextListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

    if (Settings::get("enable_android_gestures") == "yes") {
        Scroller::enableGestures(ui->contextListWidget);
    }

    action = nullptr;
    animationInProgress = false;
    QEasingCurve easingIn(QEasingCurve::OutCubic);
    animationIn = new QPropertyAnimation(this, "geometry");
    animationIn->setDuration(ANDROID_DIALOG_ANIMATION_DURATION);
    animationIn->setEasingCurve(easingIn);
    connect(animationIn, SIGNAL(finished()), this, SLOT(animationInFinished()));

    if (parent != nullptr) {
        parent->installEventFilter(this);
    }
}

ContextDialog::~ContextDialog()
{
    delete ui;
}

QAction * ContextDialog::getAction()
{
    return action;
}

void ContextDialog::build(QList<QAction *> actions) {
    ui->contextListWidget->clear();
    actionsList = actions;
    action = nullptr;
    for (int i=0; i<actionsList.size(); i++) {
        QAction * action = actionsList.at(i);
        if (action->isSeparator()) continue;
        QListWidgetItem * item = new QListWidgetItem();
        if (!action->isEnabled()) {
            Qt::ItemFlags flags = item->flags();
            flags = flags  & ~Qt::ItemIsEnabled;
            item->setFlags(flags);
        }
        QIcon icon = action->icon();
        if (icon.isNull()) icon.addFile(":/icons/blank.png", QSize(ICON_SIZE, ICON_SIZE), QIcon::Normal);
        icon.addFile(":/icons/blank.png", QSize(ICON_SIZE, ICON_SIZE), QIcon::Disabled);
        item->setIcon(icon);
        QString text = action->text().replace("&","");
        int p = text.indexOf("\t");
        if (p > 0) text = text.mid(0, p);
        item->setText(text);
        item->setData(Qt::UserRole, QVariant(i));
        addItem(item);
    }
}

void ContextDialog::addItem(QListWidgetItem * item)
{
    ui->contextListWidget->addItem(item);
}

int ContextDialog::exec()
{
    setVisible(true);
    updateGeometry();
    animateIn();
    setVisible(false);
    return QDialog::exec();

}

void ContextDialog::onItemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < actionsList.size()) {
        action = actionsList.at(index);
        if (!action->isSeparator() && action->isEnabled()) {
            done(QDialog::Accepted);
            if (parentWidget != nullptr) {
                parentWidget->setFocus();
            }
            action->activate(QAction::Trigger);
        }
    }
}

void ContextDialog::animateIn()
{
    if (animationInProgress) return;
    animationInProgress = true;
    if (!isVisible()) setVisible(true);
    raise();
    QRect rect = geometry();
    animationIn->setStartValue(QRect(rect.x()-rect.width(), rect.y(), rect.width(), rect.height()));
    animationIn->setEndValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
    animationIn->start();
}

void ContextDialog::animationInFinished()
{
    animationInProgress = false;
}

void ContextDialog::updateGeometry()
{
    int width = 0;
    int rowsCo = ui->contextListWidget->model()->rowCount();
    for (int i=0; i<rowsCo; i++) {
        QModelIndex modelIndex = ui->contextListWidget->model()->index(i, 0);
        int w = ui->contextListWidget->sizeHintForIndex(modelIndex).width();
        if (w > width) width = w;
    }
    if (width == 0) width = ui->contextListWidget->sizeHintForColumn(0);
    width += ui->contextListWidget->frameWidth() * 2;
    if (ui->contextListWidget->verticalScrollBar()->isVisible()) width += ui->contextListWidget->verticalScrollBar()->width();
    width += MENU_WIDTH_EXTRA_SPACE;
    QScreen * screen = QGuiApplication::primaryScreen();
    if (width > screen->availableGeometry().width()) width = screen->availableGeometry().width();
    int height = screen->availableGeometry().height();
    ui->contextListWidget->setFixedWidth(width);
    ui->contextListWidget->setFixedHeight(height);
    setMinimumWidth(width);
    setMinimumHeight(height);
    setGeometry(0, 0, width, height);
}

bool ContextDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Resize) {
        updateGeometry();
    }
    return false;
}
