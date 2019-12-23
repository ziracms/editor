#include "gitbrowser.h"
#include <QDir>
#include <QMenu>

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
}

void GitBrowser::clear()
{
    treeWidget->clear();
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

void GitBrowser::gitBrowserContextMenuRequested(QPoint p)
{
    QTreeWidgetItem * item = treeWidget->itemAt(p);
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

    QAction * selectedAction = menu.exec(treeWidget->viewport()->mapToGlobal(p));
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
