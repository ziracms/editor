/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "filebrowser.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QStandardPaths>
#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFileDialog>
#include <QShortcut>
#include <QScroller>
#include "helper.h"
#include "createfiledialog.h"
#include "createfolderdialog.h"
#include "createprojectdialog.h"
#include "editprojectdialog.h"
#include "project.h"
#include "icon.h"

const QString FB_ACTION_NAME_CREATE_FILE = "fb_new_file";
const QString FB_ACTION_NAME_CREATE_FOLDER = "fb_new_folder";
const QString FB_ACTION_NAME_DELETE = "fb_delete";
const QString FB_ACTION_NAME_RENAME = "fb_rename";
const QString FB_ACTION_NAME_RELOAD = "fb_reload";
const QString FB_ACTION_NAME_OPEN = "fb_open";
const QString FB_ACTION_NAME_COPY = "fb_copy";
const QString FB_ACTION_NAME_CUT = "fb_cut";
const QString FB_ACTION_NAME_PASTE = "fb_paste";

FileBrowser::FileBrowser(QTreeWidget * widget, QLineEdit * line, Settings * settings):
    treeWidget(widget), pathLine(line), menu(widget), mousePressTimer(this)
{
    fbpath = ""; fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
    fileBrowserHomeDir = QString::fromStdString(settings->get("file_browser_home"));
    initFileBrowser(fileBrowserHomeDir);
    menu.hide();
    acceptEnter = true;
    editMode = false;

    QString shortcutContextMenuStr = QString::fromStdString(settings->get("shortcut_context_menu"));
    QShortcut * shortcutContextMenu = new QShortcut(QKeySequence(shortcutContextMenuStr), treeWidget);
    shortcutContextMenu->setContext(Qt::WidgetShortcut);
    connect(shortcutContextMenu, SIGNAL(activated()), this, SLOT(contextMenu()));

    mousePressTimer.setInterval(1000);
    mousePressTimer.setSingleShot(true);
    connect(&mousePressTimer, SIGNAL(timeout()), this, SLOT(contextMenu()));

    treeWidget->setFocus();
}

void FileBrowser::initFileBrowser(QString homeDir)
{
    if (homeDir.size() == 0) {
        QStringList stddirs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (stddirs.size()>0) homeDir = stddirs.at(0);
    }
    if (homeDir.size()==0) homeDir = ".";
    if (!Helper::isQtVersionLessThan(5, 12, 0)) {
        QAction * upAction = pathLine->addAction(Icon::get("up", QIcon(":icons/levelup.png")), QLineEdit::TrailingPosition);
        connect(upAction, SIGNAL(triggered(bool)), this, SLOT(upActionTriggered(bool)));
        QAction * homeAction = pathLine->addAction(Icon::get("home", QIcon(":icons/home.png")), QLineEdit::LeadingPosition);
        connect(homeAction, SIGNAL(triggered(bool)), this, SLOT(homeActionTriggered(bool)));
    }
    buildFileBrowserTree(homeDir);
    connect(treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(fileBrowserExpanded(QTreeWidgetItem*)));
    connect(treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(fileBrowserCollapsed(QTreeWidgetItem*)));
    connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(fileBrowserDoubleClicked(QTreeWidgetItem*,int)));
    connect(pathLine, SIGNAL(returnPressed()), this, SLOT(fileBrowserPathReturnPressed()));
    connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(fileBrowserItemChanged(QTreeWidgetItem*,int)));
    connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(fileBrowserItemSelectionChanged()));
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(fileBrowserContextMenuRequested(QPoint)));
    treeWidget->installEventFilter(this);
    #if defined(Q_OS_ANDROID)
    treeWidget->viewport()->installEventFilter(this); // for context menu
    // scrolling by gesture
    QScroller::grabGesture(treeWidget->viewport(), QScroller::LeftMouseButtonGesture);
    treeWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    #endif
    pathLine->installEventFilter(this);
}

void FileBrowser::buildFileBrowserTree(QString startDir, QTreeWidgetItem * parent)
{
    if (parent == nullptr) {
        fbcopyitem = nullptr; fbcutitem = nullptr;
    }
    if (startDir.size() > 1 && startDir.at(startDir.size()-1) == "/") startDir = startDir.mid(0, startDir.size()-1);
    QFileInfo startDirInfo(startDir);
    if (!startDirInfo.exists() || !startDirInfo.isReadable() || !startDirInfo.isDir()) return;
    QDirIterator it(startDir, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    QStringList dirsList, filesList;
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fInfo(path);
        if (!fInfo.exists() || !fInfo.isReadable()) continue;
        if (fInfo.isDir() && (fInfo.fileName() == ".git" || fInfo.fileName() == PROJECT_SUBDIR || fInfo.fileName() == ".idea" || fInfo.fileName() == ".vscode" || fInfo.fileName() == "nbproject")) continue;
        if (fInfo.isDir()) dirsList.append(path);
        else filesList.append(path);
    }
    if (dirsList.size() > 0) dirsList.sort(Qt::CaseInsensitive);
    for (int i=0; i<dirsList.size(); i++) {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        QString path = dirsList.at(i);
        QFileInfo fInfo(path);
        item->setText(0, fInfo.fileName());
        //item->setIcon(0, treeWidget->style()->standardIcon(QStyle::SP_DirIcon));
        item->setIcon(0, Icon::get("folder", QIcon(":icons/folder.png")));
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        item->setToolTip(0, path);
        item->setData(0, Qt::UserRole, QVariant(path));
        if (parent == nullptr) {
            treeWidget->addTopLevelItem(item);
        } else {
            parent->addChild(item);
        }
    }
    if (filesList.size() > 0) filesList.sort(Qt::CaseInsensitive);
    for (int i=0; i<filesList.size(); i++) {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        QString path = filesList.at(i);
        QFileInfo fInfo(path);
        item->setText(0, fInfo.fileName());
        //item->setIcon(0, treeWidget->style()->standardIcon(QStyle::SP_FileIcon));
        item->setIcon(0, Icon::get("file", QIcon(":icons/file.png")));
        item->setToolTip(0, path);
        item->setData(0, Qt::UserRole, QVariant(path));
        if (parent == nullptr) {
            treeWidget->addTopLevelItem(item);
        } else {
            parent->addChild(item);
        }
        if (fbcopypath.size() > 0 && fbcopypath == path) fbcopyitem = item;
        else if (fbcutpath.size() > 0 && fbcutpath == path) fbcutitem = item;
    }
    treeWidget->resizeColumnToContents(0);
    if (parent == nullptr) {
        fbpath = startDirInfo.absoluteFilePath();
        pathLine->setText(fbpath);
    }
}

void FileBrowser::rebuildFileBrowserTree(QString path)
{
    if (path.size() == 0) return;
    treeWidget->clear();
    buildFileBrowserTree(path);
}

void FileBrowser::fileBrowserExpanded(QTreeWidgetItem * item)
{
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isDir()) return;
    buildFileBrowserTree(path, item);
}

void FileBrowser::fileBrowserCollapsed(QTreeWidgetItem * item)
{
    while(item->childCount() > 0) {
        QTreeWidgetItem * child = item->child(item->childCount()-1);
        if (child->childCount() > 0) {
            fileBrowserCollapsed(child);
        }
        if (fbcopyitem == child) fbcopyitem = nullptr;
        if (fbcutitem == child) fbcutitem = nullptr;
        item->removeChild(child);
        delete child;
    }
}

void FileBrowser::fileBrowserDoubleClicked(QTreeWidgetItem * item, int column)
{
    if (item == nullptr) return;
    if (column != 0) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isWritable()) return;
    if (fInfo.isFile()) emit openFile(path);
    else if (fInfo.isDir()) rebuildFileBrowserTree(path);
}

void FileBrowser::fileBrowserPathReturnPressed()
{
    QString path = pathLine->text();
    if (path.size() == 0) {
        QStringList stddirs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (stddirs.size()>0) path = stddirs.at(0);
    }
    if (path.size()==0) path = ".";
    rebuildFileBrowserTree(path);
}

void FileBrowser::upActionTriggered(bool)
{
    QString path = pathLine->text();
    int p = path.lastIndexOf("/");
    if (p >= 0) {
        path = path.mid(0, p);
        if (path.size() == 0) path = "/";
        pathLine->setText(path);
        fileBrowserPathReturnPressed();
    }
}

void FileBrowser::homeActionTriggered(bool)
{
    QStringList stddirs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (stddirs.size()>0) {
        pathLine->setText(stddirs.at(0));
        fileBrowserPathReturnPressed();
    }
}

void FileBrowser::fileBrowserContextMenuRequested(QPoint /*p*/)
{
    //QTreeWidgetItem * item = treeWidget->itemAt(p);
    QTreeWidgetItem * item = treeWidget->currentItem();
    fileBrowserContextMenuRequested(item);
}

void FileBrowser::fileBrowserContextMenuRequested(QTreeWidgetItem * item)
{
    QString path;
    if (item != nullptr) path = item->data(0, Qt::UserRole).toString();
    else path = fbpath;
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable()) return;

    bool disabled = !fInfo.isWritable();
    bool isFolder = fInfo.isDir();

    menu.clear();
    menu.setFont(QApplication::font());

    //QAction * openAction = menu.addAction(treeWidget->style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Open"));
    QAction * openAction = menu.addAction(Icon::get("actionOpenFile", QIcon(":icons/document-open.png")), tr("Open"));
    openAction->setData(QVariant(FB_ACTION_NAME_OPEN));
    if (disabled || item == nullptr) openAction->setDisabled(true);

    //QAction * reloadAction = menu.addAction(treeWidget->style()->standardIcon(QStyle::SP_BrowserReload), tr("Refresh"));
    QAction * reloadAction = menu.addAction(Icon::get("actionRefresh", QIcon(":icons/view-refresh.png")), tr("Refresh"));
    reloadAction->setData(QVariant(FB_ACTION_NAME_RELOAD));
    if (disabled || !isFolder || item == nullptr) reloadAction->setDisabled(true);

    menu.addSeparator();

    //QAction * newFileAction = menu.addAction(treeWidget->style()->standardIcon(QStyle::SP_FileIcon), tr("Create new file"));
    QAction * newFileAction = menu.addAction(Icon::get("actionNewFile", QIcon(":icons/document-new.png")), tr("Create new file"));
    newFileAction->setData(QVariant(FB_ACTION_NAME_CREATE_FILE));
    if (disabled || !isFolder) newFileAction->setDisabled(true);

    //QAction * newFolderAction = menu.addAction(treeWidget->style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Create new folder"));
    QAction * newFolderAction = menu.addAction(Icon::get("actionNewFolder", QIcon(":icons/folder-new.png")), tr("Create new folder"));
    newFolderAction->setData(QVariant(FB_ACTION_NAME_CREATE_FOLDER));
    if (disabled || !isFolder) newFolderAction->setDisabled(true);

    //QAction * renameAction = menu.addAction(treeWidget->style()->standardIcon(QStyle::SP_CommandLink), tr("Rename"));
    QAction * renameAction = menu.addAction(Icon::get("actionEdit", QIcon(":icons/edit.png")), tr("Rename"));
    renameAction->setData(QVariant(FB_ACTION_NAME_RENAME));
    if (disabled || item == nullptr) renameAction->setDisabled(true);

    menu.addSeparator();

    QAction * copyAction = menu.addAction(Icon::get("actionCopy", QIcon(":icons/edit-copy.png")), tr("Copy"));
    copyAction->setData(QVariant(FB_ACTION_NAME_COPY));
    if (disabled || isFolder || item == nullptr) copyAction->setDisabled(true);

    QAction * cutAction = menu.addAction(Icon::get("actionCut", QIcon(":icons/edit-cut.png")), tr("Cut"));
    cutAction->setData(QVariant(FB_ACTION_NAME_CUT));
    if (disabled || isFolder || item == nullptr) cutAction->setDisabled(true);

    QAction * pasteAction = menu.addAction(Icon::get("actionPaste", QIcon(":icons/edit-paste.png")), tr("Paste"));
    pasteAction->setData(QVariant(FB_ACTION_NAME_PASTE));
    if (disabled || !isFolder || (fbcopypath.size() == 0 && fbcutpath.size() == 0) || item == nullptr) pasteAction->setDisabled(true);

    menu.addSeparator();

    //QAction * deleteAction = menu.addAction(treeWidget->style()->standardIcon(QStyle::SP_TrashIcon), tr("Delete"));
    QAction * deleteAction = menu.addAction(Icon::get("actionDelete", QIcon(":icons/edit-delete.png")), tr("Delete"));
    deleteAction->setData(QVariant(FB_ACTION_NAME_DELETE));
    if (disabled || item == nullptr) deleteAction->setDisabled(true);

    acceptEnter = false;

    //QAction * selectedAction = menu.exec(treeWidget->viewport()->mapToGlobal(p));
    QPoint p = QCursor::pos();
    QAction * selectedAction = menu.exec(p);
    menu.hide();
    if (selectedAction == nullptr) {
        acceptEnter = true;
        return;
    }

    QString actionName = selectedAction->data().toString();
    if (actionName.size() == 0) {
        acceptEnter = true;
        return;
    }

    if (item != nullptr) {
        if (actionName == FB_ACTION_NAME_CREATE_FILE || actionName == FB_ACTION_NAME_CREATE_FOLDER) fbCreateNewItemRequested(item, actionName);
        if (actionName == FB_ACTION_NAME_RENAME) fbEditItemRequested(item, actionName);
        if (actionName == FB_ACTION_NAME_DELETE) fbDeleteRequested(item);
        if (actionName == FB_ACTION_NAME_OPEN) fileBrowserDoubleClicked(item, 0);
        if (actionName == FB_ACTION_NAME_RELOAD) fbReloadItem(item);
        if (actionName == FB_ACTION_NAME_COPY) fbCopyItem(item);
        if (actionName == FB_ACTION_NAME_CUT) fbCutItem(item);
        if (actionName == FB_ACTION_NAME_PASTE) fbPasteItem(item);
    } else {
        if (actionName == FB_ACTION_NAME_CREATE_FILE) showCreateFileDialog(path);
        if (actionName == FB_ACTION_NAME_CREATE_FOLDER) showCreateFolderDialog(path);
    }

    acceptEnter = true;
}

void FileBrowser::fbCreateNewItemRequested(QTreeWidgetItem * item, QString actionName)
{
    if (item == nullptr) return;
    if (actionName != FB_ACTION_NAME_CREATE_FILE && actionName != FB_ACTION_NAME_CREATE_FOLDER) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isWritable() || !fInfo.isDir()) return;

    editMode = true;

    if (!item->isExpanded()) treeWidget->expandItem(item);
    QTreeWidgetItem * tmpitem = new QTreeWidgetItem();
    tmpitem->setData(0, Qt::UserRole, QVariant(""));
    tmpitem->setData(0, Qt::UserRole+1, QVariant(actionName));
    tmpitem->setFlags(item->flags() | Qt::ItemIsEditable);
    if (actionName == FB_ACTION_NAME_CREATE_FILE) {
        tmpitem->setText(0, "");
        //tmpitem->setIcon(0, treeWidget->style()->standardIcon(QStyle::SP_FileIcon));
        tmpitem->setIcon(0, Icon::get("file", QIcon(":icons/file.png")));
        item->addChild(tmpitem);
    } else if (actionName == FB_ACTION_NAME_CREATE_FOLDER) {
        tmpitem->setText(0, "");
        //tmpitem->setIcon(0, treeWidget->style()->standardIcon(QStyle::SP_DirIcon));
        tmpitem->setIcon(0, Icon::get("folder", QIcon(":icons/folder.png")));
        item->insertChild(0, tmpitem);
    }
    treeWidget->editItem(tmpitem, 0);
    treeWidget->scrollToItem(tmpitem);
}

void FileBrowser::fbEditItemRequested(QTreeWidgetItem * item, QString actionName)
{
    if (item == nullptr) return;
    if (actionName != FB_ACTION_NAME_RENAME) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isWritable()) return;

    editMode = true;

    item->setData(0, Qt::UserRole+1, QVariant(actionName));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    treeWidget->editItem(item, 0);
}

void FileBrowser::fbDeleteRequested(QTreeWidgetItem * item)
{
    if (item == nullptr) return;
    QTreeWidgetItem * parent = item->parent();
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isWritable()) return;
    if (fInfo.isDir() &&
        Helper::showQuestion(tr("Delete"), tr("Do you really want to delete folder \"%1\" ?").arg(fInfo.fileName()))
        //QMessageBox::question(treeWidget, tr("Delete"), tr("Do you really want to delete folder \"%1\" ?").arg(fInfo.fileName()), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok
    ) {
        if (!Helper::deleteFolder(path)) {
            // folder is not empty ?
            if (Helper::showQuestion(tr("Delete"), tr("Folder is not empty. Delete all files in \"%1\" ?").arg(fInfo.fileName()))) {
                if (!Helper::deleteFolderRecursivly(path)) {
                    Helper::showMessage(QObject::tr("Could not delete folder."));
                } else {
                    // reload
                    if (parent != nullptr) fbReloadItem(parent);
                    else rebuildFileBrowserTree(fbpath);
                }
            }
        } else {
            // reload
            if (parent != nullptr) fbReloadItem(parent);
            else rebuildFileBrowserTree(fbpath);
        }
    } else if (fInfo.isFile() &&
               Helper::showQuestion(tr("Delete"), tr("Do you really want to delete file \"%1\" ?").arg(fInfo.fileName()))
               //QMessageBox::question(treeWidget, tr("Delete"), tr("Do you really want to delete file \"%1\" ?").arg(fInfo.fileName()), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok
    ) {
        if (!Helper::deleteFile(path)) {
            Helper::showMessage(QObject::tr("Could not delete file."));
        } else {
            // reload
            if (parent != nullptr) fbReloadItem(parent);
            else rebuildFileBrowserTree(fbpath);
            // close open tab
            emit fileDeleted(path);
        }
    }
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::fileBrowserItemChanged(QTreeWidgetItem * item, int col)
{
    if (menu.isVisible()) {
        menu.hide();
        acceptEnter = true;
    }
    if (item == nullptr || col != 0) return;
    QTreeWidgetItem * parent = item->parent();

    QString path;
    if (parent != nullptr) path = parent->data(0, Qt::UserRole).toString();
    else path = fbpath;
    if (path.size() == 0) return;
    QString actionName = item->data(0, Qt::UserRole+1).toString();
    if (actionName.size() == 0) return;

    QString oldPath = item->data(0, Qt::UserRole).toString();
    QString newName = item->text(0);
    QString newPath = path + "/" + newName;
    if (newName.size() > 0 && newPath != oldPath) {
        if (!Helper::fileOrFolderExists(newPath)) {
            bool success = false;
            if (actionName == FB_ACTION_NAME_CREATE_FILE) success = Helper::createFile(newPath);
            else if (actionName == FB_ACTION_NAME_CREATE_FOLDER) success = Helper::createDir(newPath);
            else if (actionName == FB_ACTION_NAME_RENAME) success = Helper::renameFileOrFolder(oldPath, newPath);
            if (!success) {
                Helper::showMessage(QObject::tr("Could not create new file or folder."));
            } else {
                if (actionName == FB_ACTION_NAME_CREATE_FILE) emit fileCreated(newPath);
                if (actionName == FB_ACTION_NAME_CREATE_FOLDER) emit folderCreated(newPath);
                if (actionName == FB_ACTION_NAME_RENAME) emit fileOrFolderRenamed(oldPath, newPath);
            }
        } else {
            Helper::showMessage(QObject::tr("File or folder with such name already exists."));
        }
        // reload
        if (parent != nullptr) fbReloadItem(parent);
        else rebuildFileBrowserTree(path);
    }
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::fileBrowserItemSelectionChanged()
{
    fileBrowserRemoveEmptyItems();
    editMode = false;
}

void FileBrowser::fileBrowserRemoveEmptyItems()
{
    QTreeWidgetItemIterator it(treeWidget);
    while(*it) {
        if ((*it)->text(0) == "") {
            QTreeWidgetItem * parent = (*it)->parent();
            if (parent != nullptr) fbReloadItem(parent);
            else rebuildFileBrowserTree(fbpath);
            break;
        }
        it++;
    }
}

void FileBrowser::fbReloadItem(QTreeWidgetItem * item)
{
    treeWidget->blockSignals(true);
    if (item->isExpanded()) treeWidget->collapseItem(item);
    fileBrowserCollapsed(item);
    treeWidget->expandItem(item);
    fileBrowserExpanded(item);
    treeWidget->blockSignals(false);
}

void FileBrowser::fbCopyItem(QTreeWidgetItem * item)
{
    if (item == nullptr) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isFile()) return;
    fbcopyitem = item;
    fbcopypath = path;
    fbcutitem = nullptr;
    fbcutpath = "";
}

void FileBrowser::fbCutItem(QTreeWidgetItem * item)
{
    if (item == nullptr) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isFile()) return;
    fbcutitem = item;
    fbcutpath = path;
    fbcopyitem = nullptr;
    fbcopypath = "";
}

void FileBrowser::fbPasteItem(QTreeWidgetItem * item)
{
    if (item == nullptr) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.size() == 0) return;
    QFileInfo dInfo(path);
    if (!dInfo.exists() || !dInfo.isReadable() || !dInfo.isWritable() || !dInfo.isDir()) return;
    if (fbcopypath.size() > 0) {
        QFileInfo fInfo(fbcopypath);
        if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isFile()) return;
        QString newPath = path + "/" + fInfo.fileName();
        if (!Helper::fileOrFolderExists(newPath)) {
            if (!Helper::copyFile(fbcopypath, newPath)) {
                QMessageBox msgBox;
                msgBox.setText(QObject::tr("Could not create new file or folder."));
                msgBox.exec();
            } else {
                // reload
                fbReloadItem(item);
            }
            fbcopypath = ""; fbcutpath = "";
            fbcopyitem = nullptr; fbcutitem = nullptr;
        } else {
            Helper::showMessage(QObject::tr("File or folder with such name already exists."));
        }
    } else if (fbcutpath.size() > 0) {
        QFileInfo fInfo(fbcutpath);
        if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isFile()) return;
        QString newPath = path + "/" + fInfo.fileName();
        if (!Helper::fileOrFolderExists(newPath)) {
            if (!Helper::renameFile(fbcutpath, newPath)) {
                Helper::showMessage(QObject::tr("Could not create new file or folder."));
            } else {
                // reload
                fbReloadItem(item);
                if (fbcutitem != nullptr) {
                    QTreeWidgetItem * parent = fbcutitem->parent();
                    if (parent != nullptr) fbReloadItem(parent);
                    else rebuildFileBrowserTree(fbpath);
                }
                emit fileDeleted(fbcutpath);
            }
            fbcopypath = ""; fbcutpath = "";
            fbcopyitem = nullptr; fbcutitem = nullptr;
        } else {
            Helper::showMessage(QObject::tr("File or folder with such name already exists."));
        }
    }
}

void FileBrowser::showCreateFileDialog(QString startDir)
{
    if (startDir.size() == 0) startDir = fbpath;
    treeWidget->clearFocus();
    CreateFileDialog dialog(treeWidget);
    dialog.setDirectory(startDir);
    dialog.focusName();
    if (dialog.exec() != QDialog::Accepted) return;
    QString directory = dialog.getDirectory();
    QString path = dialog.getPath();
    if (directory.size() == 0 || path.size() == 0) return;
    if (!Helper::fileOrFolderExists(path)) {
        if (!Helper::createFile(path)) {
            Helper::showMessage(QObject::tr("Could not create new file or folder."));
        } else {
            emit fileCreated(path);
            // reload
            refreshFileBrowserDirectory(directory);
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("File or folder with such name already exists."));
        msgBox.exec();
    }
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::showCreateFolderDialog(QString startDir)
{
    if (startDir.size() == 0) startDir = fbpath;
    treeWidget->clearFocus();
    CreateFolderDialog dialog(treeWidget);
    dialog.setDirectory(startDir);
    dialog.focusName();
    if (dialog.exec() != QDialog::Accepted) return;
    QString directory = dialog.getDirectory();
    QString path = dialog.getPath();
    if (directory.size() == 0 || path.size() == 0) return;
    if (!Helper::fileOrFolderExists(path)) {
        if (!Helper::createDir(path)) {
            Helper::showMessage(QObject::tr("Could not create new file or folder."));
        } else {
            emit folderCreated(path);
            // reload
            refreshFileBrowserDirectory(directory);
        }
    } else {
        Helper::showMessage(QObject::tr("File or folder with such name already exists."));
    }
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::showCreateProjectDialog(bool phpLintEnabled, bool phpCSEnabled, QString startDir)
{
    if (startDir.size() == 0) startDir = fbpath;
    treeWidget->clearFocus();
    CreateProjectDialog dialog(treeWidget);
    dialog.setDirectory(startDir);
    dialog.setLintEnabled(phpLintEnabled);
    dialog.setCSEnabled(phpCSEnabled);
    dialog.focusName();
    if (dialog.exec() != QDialog::Accepted) return;
    QString name = dialog.getName();
    QString path = dialog.getPath();
    bool lintEnabled = dialog.getLintEnabled();
    bool csEnabled = dialog.getCSEnabled();
    if (name.size() == 0 || path.size() == 0) return;
    if (Helper::folderExists(path)) {
        emit projectCreateRequested(name, path, lintEnabled, csEnabled);
    } else {
        Helper::showMessage(QObject::tr("Folder with such name not found."));
    }
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::showEditProjectDialog(QString title, QString path, bool phpLintEnabled, bool phpCSEnabled)
{
    if (path.size() == 0 || !Helper::folderExists(path)) return;
    treeWidget->clearFocus();
    EditProjectDialog dialog(treeWidget);
    dialog.setName(title);
    dialog.setPath(path);
    dialog.setLintEnabled(phpLintEnabled);
    dialog.setCSEnabled(phpCSEnabled);
    dialog.focusName();
    if (dialog.exec() != QDialog::Accepted) return;
    QString name = dialog.getName();
    bool lintEnabled = dialog.getLintEnabled();
    bool csEnabled = dialog.getCSEnabled();
    if (name.size() == 0) return;
    emit projectEditRequested(name, path, lintEnabled, csEnabled);
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::openProject(QString startDir)
{
    if (startDir.size() == 0) startDir = getHomeDir();
    if (startDir.size() == 0) startDir = fbpath;
    treeWidget->clearFocus();
    //QString path = QFileDialog::getExistingDirectory(treeWidget, tr("Select project source directory"), startDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString path = Helper::getExistingDirectory(treeWidget, tr("Select project source directory"), startDir);
    if (path.size() == 0) return;
    if (Helper::folderExists(path)) {
        emit projectOpenRequested(path);
    } else {
        Helper::showMessage(QObject::tr("Folder with such name not found."));
    }
    fbcopypath = ""; fbcutpath = "";
    fbcopyitem = nullptr; fbcutitem = nullptr;
}

void FileBrowser::refreshFileBrowserDirectory(QString directory)
{
    QTreeWidgetItem * item = nullptr;
    QTreeWidgetItemIterator it(treeWidget);
    while(*it) {
        QString itemPath = (*it)->data(0, Qt::UserRole).toString();
        if (itemPath == directory) {
            item = (*it);
            break;
        }
        it++;
    }
    if (item != nullptr) fbReloadItem(item);
    else if (directory == fbpath) rebuildFileBrowserTree(fbpath);
}

QString FileBrowser::getRootPath()
{
    return fbpath;
}

QString FileBrowser::getHomeDir()
{
    return fileBrowserHomeDir;
}

void FileBrowser::focus()
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

bool FileBrowser::isFocused()
{
    return treeWidget->hasFocus();
}

bool FileBrowser::eventFilter(QObject *watched, QEvent *event)
{
    QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
    bool shift = false, ctrl = false;
    if (keyEvent->modifiers() & Qt::ShiftModifier) shift = true;
    if (keyEvent->modifiers() & Qt::ControlModifier) ctrl = true;
    // refresh file browser selected item
    if(watched == treeWidget && event->type() == QEvent::KeyRelease) {
        if (!editMode && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Escape || (keyEvent->key() == Qt::Key_Tab && !shift))) {
            fileBrowserRemoveEmptyItems();
        }
    }
    // context menu
    if(watched == treeWidget && event->type() == QEvent::KeyPress) {
        if (keyEvent->key() == Qt::Key_Return && !ctrl && !shift && acceptEnter && !editMode) {
            QTreeWidgetItem * item = treeWidget->currentItem();
            fileBrowserDoubleClicked(item, 0);
        } else if (keyEvent->key() == Qt::Key_Return && editMode) {
            editMode = false;
            treeWidget->setFocus(); // workaround for Android
        } else if (keyEvent->key() == Qt::Key_Up) {
            if (treeWidget->topLevelItemCount() == 0 || treeWidget->currentItem() == treeWidget->topLevelItem(0)) {
                pathLine->setFocus();
            }
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
    // focus
    if(watched == pathLine && event->type() == QEvent::KeyPress) {
        if (keyEvent->key() == Qt::Key_Down) {
            focus();
        }
    }
    return false;
}

void FileBrowser::contextMenu()
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    fileBrowserContextMenuRequested(item);
}
