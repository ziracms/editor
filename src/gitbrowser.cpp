#include "gitbrowser.h"
#include <QDir>
#include <QMenu>
#include <QShortcut>
#include <QKeyEvent>

const QString GB_ACTION_NAME_ADD = "add";
const QString GB_ACTION_NAME_RESET = "reset";
const QString GB_ACTION_NAME_COMMIT = "commit";

GitBrowser::GitBrowser(QTreeWidget * widget, Settings * settings):
    treeWidget(widget)
{
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(gitBrowserContextMenuRequested(QPoint)));

    QString errorColorStr = QString::fromStdString(settings->get("git_output_error_color"));
    errorColor = QColor(errorColorStr);
    QString msgColorStr = QString::fromStdString(settings->get("git_output_message_color"));
    msgColor = QColor(msgColorStr);

    treeWidget->installEventFilter(this);

    QString shortcutContextMenuStr = QString::fromStdString(settings->get("shortcut_context_menu"));
    QShortcut * shortcutContextMenu = new QShortcut(QKeySequence(shortcutContextMenuStr), treeWidget);
    shortcutContextMenu->setContext(Qt::WidgetShortcut);
    connect(shortcutContextMenu, SIGNAL(activated()), this, SLOT(contextMenu()));
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
        if (!staged) item->setTextColor(1, errorColor);
        else item->setTextColor(1, msgColor);
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

    QAction * addAction = menu.addAction(QIcon(":icons/plus.png"), tr("Add"));
    addAction->setData(QVariant(GB_ACTION_NAME_ADD));
    if (item == nullptr) addAction->setEnabled(false);

    QAction * resetAction = menu.addAction(QIcon(":icons/minus.png"), tr("Reset"));
    resetAction->setData(QVariant(GB_ACTION_NAME_RESET));
    if (item == nullptr) resetAction->setEnabled(false);

    menu.addSeparator();

    QAction * commitAction = menu.addAction(QIcon(":icons/ok.png"), tr("Commit"));
    commitAction->setData(QVariant(GB_ACTION_NAME_COMMIT));

    //QAction * selectedAction = menu.exec(treeWidget->viewport()->mapToGlobal(p));
    QPoint p = QCursor::pos();
    QAction * selectedAction = menu.exec(p);
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
    return false;
}

void GitBrowser::contextMenu()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    gitBrowserContextMenuRequested(item);
}
