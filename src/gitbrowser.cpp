#include "gitbrowser.h"
#include <QDir>
#include <QMenu>
#include <QShortcut>
#include <QKeyEvent>
#include "scroller.h"
#include "icon.h"
#include "helper.h"

const QString GB_ACTION_NAME_ADD = "add";
const QString GB_ACTION_NAME_RESET = "reset";
const QString GB_ACTION_NAME_COMMIT = "commit";

GitBrowser::GitBrowser(QTreeWidget * widget):
    treeWidget(widget), mousePressTimer(this)
{
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(gitBrowserContextMenuRequested(QPoint)));
    connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(gitBrowserItemSelectionChanged()));

    QString errorColorStr = QString::fromStdString(Settings::get("git_output_error_color"));
    errorColor = QColor(errorColorStr);
    QString msgColorStr = QString::fromStdString(Settings::get("git_output_message_color"));
    msgColor = QColor(msgColorStr);

    treeWidget->installEventFilter(this);
    #if defined(Q_OS_ANDROID)
    treeWidget->viewport()->installEventFilter(this); // for context menu
    // scrolling by gesture
    Scroller::enableGestures(treeWidget, false);
    #endif

    QString shortcutContextMenuStr = QString::fromStdString(Settings::get("shortcut_context_menu"));
    QShortcut * shortcutContextMenu = new QShortcut(QKeySequence(shortcutContextMenuStr), treeWidget);
    shortcutContextMenu->setContext(Qt::WidgetShortcut);
    connect(shortcutContextMenu, SIGNAL(activated()), this, SLOT(contextMenu()));

    mousePressTimer.setInterval(1000);
    mousePressTimer.setSingleShot(true);
    connect(&mousePressTimer, SIGNAL(timeout()), this, SLOT(contextMenu()));
}

void GitBrowser::clear()
{
    treeWidget->clear();
}

void GitBrowser::focus()
{
    treeWidget->setFocus();
    QList<QTreeWidgetItem *> items = treeWidget->selectedItems();
    if (items.count() == 0 && treeWidget->topLevelItemCount() > 0)  {
        QTreeWidgetItem * item = treeWidget->topLevelItem(0);
        if (item != nullptr) {
            treeWidget->setCurrentItem(item);
        }
    }
}

bool GitBrowser::isFocused()
{
    return treeWidget->hasFocus();
}

void GitBrowser::build(QString output)
{
    clear();
    if (output.size() == 0) return;
    QStringList lines = output.split("\n");
    for (QString line : lines) {
        if (line.size() == 0) continue;
        bool staged = line.mid(0, 1) != " " && line.mid(0, 1) != "?";
        line = line.trimmed();
        QStringList lineParts = line.split(QRegularExpression("\\s+"));
        if (lineParts.size() == 4 && lineParts.at(2) == "->") {
            lineParts.replace(1, lineParts.at(3));
            lineParts.removeLast();
            lineParts.removeLast();
        }
        if (lineParts.size() != 2) continue;
        QString status = lineParts.at(0).trimmed();
        QString path = lineParts.at(1).trimmed();
        QString name = path;
        if (name.count('/') >= 2) {
            int p1 = name.indexOf('/');
            int p2 = name.lastIndexOf('/');
            if (p1 > 0 && p2 < name.length()-1 && p2 > p1) {
                name = name.mid(0, p1+1) + "..." + name.mid(p2);
            }
        }
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, name);
        item->setText(1, status);
        /* QTreeWidgetItem::setTextColor is deprecated */
        /*
        if (!staged) item->setTextColor(1, errorColor);
        else item->setTextColor(1, msgColor);
        */
        if (!staged) item->setForeground(1, errorColor);
        else item->setForeground(1, msgColor);
        item->setToolTip(0, path);
        item->setData(0, Qt::UserRole, QVariant(path));
        treeWidget->addTopLevelItem(item);
    }
    treeWidget->resizeColumnToContents(1);
}

void GitBrowser::gitBrowserContextMenuRequested(QPoint /*p*/)
{
    //QTreeWidgetItem * item = treeWidget->itemAt(p);
    QTreeWidgetItem * item = treeWidget->currentItem();
    gitBrowserContextMenuRequested(item);
}

void GitBrowser::gitBrowserContextMenuRequested(QTreeWidgetItem * item)
{
    QString path = "";
    if (item != nullptr) path = item->data(0, Qt::UserRole).toString();

    QMenu menu(treeWidget);

    QAction * addAction = menu.addAction(Icon::get("actionGitAdd", QIcon(":icons/plus.png")), tr("Add"));
    addAction->setData(QVariant(GB_ACTION_NAME_ADD));
    if (item == nullptr) addAction->setEnabled(false);

    QAction * resetAction = menu.addAction(Icon::get("actionGitReset", QIcon(":icons/minus.png")), tr("Reset"));
    resetAction->setData(QVariant(GB_ACTION_NAME_RESET));
    if (item == nullptr) resetAction->setEnabled(false);

    menu.addSeparator();

    QAction * commitAction = menu.addAction(Icon::get("actionGitCommit", QIcon(":icons/ok.png")), tr("Commit"));
    commitAction->setData(QVariant(GB_ACTION_NAME_COMMIT));

    QAction * selectedAction = nullptr;
    #if defined(Q_OS_ANDROID)
    Scroller::reset();
    selectedAction = Helper::contextMenuToDialog(&menu, treeWidget);
    #else
    //QAction * selectedAction = menu.exec(treeWidget->viewport()->mapToGlobal(p));
    QPoint p = QCursor::pos();
    selectedAction = menu.exec(p);
    #endif
    if (selectedAction == nullptr) return;

    QString actionName = selectedAction->data().toString();
    if (actionName.size() == 0) return;

    if (actionName == GB_ACTION_NAME_ADD && item != nullptr) gbAddRequested(item);
    if (actionName == GB_ACTION_NAME_RESET && item != nullptr) gbResetRequested(item);
    if (actionName == GB_ACTION_NAME_COMMIT) gbCommitRequested();
}

void GitBrowser::gbAddRequested(QTreeWidgetItem * item)
{
    QString path;
    path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    emit addRequested(path);
}

void GitBrowser::gbResetRequested(QTreeWidgetItem * item)
{
    QString path;
    path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    emit resetRequested(path);
}

void GitBrowser::gbCommitRequested()
{
    emit commitRequested();
}

void GitBrowser::gitBrowserItemSelectionChanged()
{
    if (mousePressTimer.isActive()) {
        mousePressTimer.stop();
        mousePressTimer.start();
    }
}

bool GitBrowser::eventFilter(QObject *watched, QEvent *event)
{
    QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
    bool shift = false, ctrl = false;
    if (keyEvent->modifiers() & Qt::ShiftModifier) shift = true;
    if (keyEvent->modifiers() & Qt::ControlModifier) ctrl = true;
    // add on enter
    if(watched == treeWidget && event->type() == QEvent::KeyPress) {
        if (keyEvent->key() == Qt::Key_Return && !ctrl && !shift) {
            QTreeWidgetItem * item = treeWidget->currentItem();
            gbAddRequested(item);
        }
    }
    // context menu for Android
    if(watched == treeWidget->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent != nullptr && mouseEvent->buttons() == Qt::LeftButton) {
            mousePressTimer.start();
        }
    }
    if(watched == treeWidget->viewport() && event->type() == QEvent::MouseButtonRelease) {
        if (mousePressTimer.isActive()) mousePressTimer.stop();
    }
    return false;
}

void GitBrowser::contextMenu()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    gitBrowserContextMenuRequested(item);
}
