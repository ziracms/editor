#include "menudialog.h"
#include "ui_menudialog.h"
#include <QScroller>
#include <QScrollBar>
#include <QScreen>
#include "icon.h"
#include "helper.h"

const char * MENU_TOP_LEVEL_ITEM_PROPERTY = "toplevel";
const int MENU_WIDTH_EXTRA_SPACE = 40;

MenuDialog::MenuDialog(QMenuBar * menuBar, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MenuDialog),
    menuBar(menuBar)
{
    ui->setupUi(this);
    setupMenu();

    ui->listWidget->setFocusProxy(this);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setContentsMargins(0,0,0,0);
    ui->listWidget->setSpacing(0);
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

    QScroller::grabGesture(ui->listWidget->viewport(), QScroller::LeftMouseButtonGesture);
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

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

MenuDialog::~MenuDialog()
{
    delete ui;
}

void MenuDialog::setupMenu()
{
    mainMenu = new QMenu(this);
    subMenu = new QMenu(this);
    backAction = new QAction(this);

    QAction * fileAction = mainMenu->addAction(tr("File"));
    fileAction->setIcon(Icon::get("actionNewFile", QIcon(":/icons/document-new.png")));
    fileAction->setData(QVariant("menuFile"));
    fileAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(fileAction, SIGNAL(triggered(bool)), this, SLOT(topLevelItemTriggered(bool)));

    QAction * editAction = mainMenu->addAction(tr("Edit"));
    editAction->setIcon(Icon::get("actionEdit", QIcon(":/icons/edit.png")));
    editAction->setData(QVariant("menuEdit"));
    editAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(editAction, SIGNAL(triggered(bool)), this, SLOT(topLevelItemTriggered(bool)));

    QAction * gitAction = mainMenu->addAction(tr("Git"));
    gitAction->setIcon(Icon::get("actionGitCommit", QIcon(":/icons/ok.png")));
    gitAction->setData(QVariant("menuGit"));
    gitAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(gitAction, SIGNAL(triggered(bool)), this, SLOT(topLevelItemTriggered(bool)));

    QAction * toolsAction = mainMenu->addAction(tr("Tools"));
    toolsAction->setIcon(Icon::get("actionSettings", QIcon(":/icons/configure.png")));
    toolsAction->setData(QVariant("menuTools"));
    toolsAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(toolsAction, SIGNAL(triggered(bool)), this, SLOT(topLevelItemTriggered(bool)));

    QAction * viewAction = mainMenu->addAction(tr("View"));
    viewAction->setIcon(Icon::get("actionSplitTab", QIcon(":/icons/split.png")));
    viewAction->setData(QVariant("menuView"));
    viewAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(viewAction, SIGNAL(triggered(bool)), this, SLOT(topLevelItemTriggered(bool)));

    QAction * contextMenuAction = new QAction(this);
    contextMenuAction->setText(tr("Context menu"));
    contextMenuAction->setIcon(Icon::get("actionMenu", QIcon(":/icons/list.png")));
    contextMenuAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 0);
    connect(contextMenuAction, SIGNAL(triggered(bool)), this, SLOT(contextMenuItemTriggered(bool)));
    mainMenu->addAction(contextMenuAction);

    QAction * preferencesAction = new QAction(this);
    preferencesAction->setText(tr("Preferences"));
    preferencesAction->setIcon(Icon::get("actionSettings", QIcon(":/icons/configure.png")));
    preferencesAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 0);
    connect(preferencesAction, SIGNAL(triggered(bool)), this, SLOT(preferencesItemTriggered(bool)));
    mainMenu->addAction(preferencesAction);

    QAction * helpAction = mainMenu->addAction(tr("Help"));
    helpAction->setIcon(Icon::get("actionHelpAbout", QIcon(":/icons/help-info.png")));
    helpAction->setData(QVariant("menuHelp"));
    helpAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(helpAction, SIGNAL(triggered(bool)), this, SLOT(topLevelItemTriggered(bool)));

    QAction * exitAction = new QAction(this);
    exitAction->setText(tr("Exit"));
    exitAction->setIcon(Icon::get("actionUndo", QIcon(":/icons/edit-undo.png")));
    exitAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 0);
    connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(exitItemTriggered(bool)));
    mainMenu->addAction(exitAction);

    backAction->setText(tr("Back"));
    backAction->setIcon(Icon::get("actionUndo", QIcon(":/icons/edit-undo.png")));
    backAction->setProperty(MENU_TOP_LEVEL_ITEM_PROPERTY, 1);
    connect(backAction, SIGNAL(triggered(bool)), this, SLOT(backItemTriggered(bool)));
}

void MenuDialog::build() {
    build(mainMenu->actions());
}

void MenuDialog::build(QList<QAction *> actions) {
    ui->listWidget->clear();
    subMenu->clear();
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

void MenuDialog::addItem(QListWidgetItem * item)
{
    ui->listWidget->addItem(item);
}

int MenuDialog::exec()
{
    setVisible(true);
    updateGeometry();
    animateIn();
    setVisible(false);
    return QDialog::exec();

}

void MenuDialog::onItemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < actionsList.size()) {
        action = actionsList.at(index);
        if (!action->isSeparator() && action->isEnabled()) {
            QVariant prop = action->property(MENU_TOP_LEVEL_ITEM_PROPERTY);
            if (!prop.isValid() || prop.toInt() != 1) {
                close();
            }
            action->activate(QAction::Trigger);
        }
    }
}

void MenuDialog::topLevelItemTriggered(bool)
{
    QAction * action = qobject_cast<QAction *>(sender());
    if (action != nullptr) {
        subMenu->clear();
        subMenu->addAction(backAction);
        QString name = action->data().toString();
        QMenu * menu = menuBar->findChild<QMenu *>(name);
        if (menu == nullptr) return;
        menu->aboutToShow();
        buildSubMenu(menu);
        build(subMenu->actions());
        updateGeometry();
    }
}

void MenuDialog::buildSubMenu(QMenu * menu)
{
    QList<QAction *> actions = menu->actions();
    for (QAction * action : actions) {
        if (action->isSeparator()) continue;
        QMenu * menu = action->menu();
        if (menu != nullptr) {
            buildSubMenu(menu);
            continue;
        }
        subMenu->addAction(action);
    }
}

void MenuDialog::backItemTriggered(bool)
{
    build();
    updateGeometry();
}

void MenuDialog::contextMenuItemTriggered(bool)
{
    emit showContextMenu();
}

void MenuDialog::preferencesItemTriggered(bool)
{
    emit showPreferences();
}

void MenuDialog::exitItemTriggered(bool)
{
    emit quit();
}

void MenuDialog::animateIn()
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

void MenuDialog::animationInFinished()
{
    animationInProgress = false;
}

void MenuDialog::updateGeometry()
{
    int width = 0;
    int rowsCo = ui->listWidget->model()->rowCount();
    for (int i=0; i<rowsCo; i++) {
        QModelIndex modelIndex = ui->listWidget->model()->index(i, 0);
        int w = ui->listWidget->sizeHintForIndex(modelIndex).width();
        if (w > width) width = w;
    }
    if (width == 0) width = ui->listWidget->sizeHintForColumn(0);
    width += ui->listWidget->frameWidth() * 2;
    if (ui->listWidget->verticalScrollBar()->isVisible()) width += ui->listWidget->verticalScrollBar()->width();
    width += MENU_WIDTH_EXTRA_SPACE;
    QScreen * screen = QGuiApplication::primaryScreen();
    if (width > screen->availableGeometry().width()) width = screen->availableGeometry().width();
    int height = screen->availableGeometry().height();
    ui->listWidget->setFixedWidth(width);
    ui->listWidget->setFixedHeight(height);
    setMinimumWidth(width);
    setMinimumHeight(height);
    setGeometry(0, 0, width, height);
}

bool MenuDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Resize) {
        updateGeometry();
    }
    return false;
}

