#include "contextdialog.h"
#include <QScrollBar>
#include <QScroller>
#include <QScreen>
#include "icon.h"
#include "helper.h"

const int ICON_SIZE = 64;
const int ANIMATION_DURATION = 100;

ContextDialog::ContextDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContextDialog),
    parent(parent)
{
    ui->setupUi(this);

    ui->contextListWidget->setFocusProxy(this);
    ui->contextListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->contextListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->contextListWidget->setContentsMargins(0,0,0,0);
    ui->contextListWidget->setSpacing(0);
    connect(ui->contextListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

    QScroller::grabGesture(ui->contextListWidget->viewport(), QScroller::LeftMouseButtonGesture);
    ui->contextListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    action = nullptr;
    animationInProgress = false;
    QEasingCurve easingIn(QEasingCurve::OutCubic);
    animationIn = new QPropertyAnimation(this, "geometry");
    animationIn->setDuration(ANIMATION_DURATION);
    animationIn->setEasingCurve(easingIn);
    connect(animationIn, SIGNAL(finished()), this, SLOT(animationInFinished()));
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

int ContextDialog::show()
{
    int width = ui->contextListWidget->geometry().width();
    int height = ui->contextListWidget->geometry().height();
    int co = ui->contextListWidget->model()->rowCount();
    if (co>0) {
        setVisible(true);
        #if defined(Q_OS_ANDROID)
        QScreen * screen = QGuiApplication::primaryScreen();
        if (width < screen->availableGeometry().width() / 2) width = screen->availableGeometry().width() / 2;
        if (height < screen->availableGeometry().height()) height = screen->availableGeometry().height();
        ui->contextListWidget->setFixedWidth(width);
        ui->contextListWidget->setFixedHeight(height);
        setGeometry(0, 0, width, height);
        animateIn();
        #else
        width = ui->contextListWidget->sizeHintForColumn(0) + ui->contextListWidget->frameWidth() * 2;
        if (ui->contextListWidget->verticalScrollBar()->isVisible()) width += ui->contextListWidget->verticalScrollBar()->width();
        height = co * ui->contextListWidget->sizeHintForRow(0) + ui->contextListWidget->frameWidth() * 2;
        if (width > parent->geometry().width()) width = parent->geometry().width();
        if (height > parent->geometry().height()) height = parent->geometry().height();
        ui->contextListWidget->setFixedWidth(width);
        ui->contextListWidget->setFixedHeight(height);
        setFocus();
        #endif
        setVisible(false);
    }
    return exec();

}

void ContextDialog::onItemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < actionsList.size()) {
        action = actionsList.at(index);
        if (!action->isSeparator() && action->isEnabled()) {
            close();
            parent->setFocus();
            action->activate(QAction::Trigger);
        } else {
            action = nullptr;
        }
    }
}

void ContextDialog::focusOutEvent(QFocusEvent *e)
{
    if (action == nullptr) close();
    QDialog::focusOutEvent(e);
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
