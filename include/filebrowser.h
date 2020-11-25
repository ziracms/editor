/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "settings.h"
#include <QTreeWidget>
#include <QLineEdit>
#include <QEvent>
#include <QMenu>
#include <QTimer>

class FileBrowser : public QObject
{
    Q_OBJECT
public:
    FileBrowser(QTreeWidget * widget, QLineEdit * line);
    void showCreateFileDialog(QString startDir = "");
    void showCreateFolderDialog(QString startDir = "");
    void showRenameDialog(QString startPath);
    void showCreateProjectDialog(bool phpLintEnabled, bool phpCSEnabled, QString startDir = "");
    void showEditProjectDialog(QString title, QString path, bool phpLintEnabled, bool phpCSEnabled);
    void openProject(QString startDir = "");
    void refreshFileBrowserDirectory(QString directory);
    void rebuildFileBrowserTree(QString path);
    QString getRootPath();
    QString getHomeDir();
    void focus();
    bool isFocused();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void initFileBrowser(QString homeDir = "");
    void buildFileBrowserTree(QString startDir, QTreeWidgetItem * parent = nullptr);
    void fbCreateNewItemRequested(QTreeWidgetItem * item, QString actionName);
    void fbEditItemRequested(QTreeWidgetItem * item, QString actionName);
    void fbDeleteRequested(QTreeWidgetItem * item);
    void fbReloadItem(QTreeWidgetItem * item);
    void fbCopyItem(QTreeWidgetItem * item);
    void fbCutItem(QTreeWidgetItem * item);
    void fbPasteItem(QTreeWidgetItem * item);
    void fileBrowserRemoveEmptyItems();
    void fileBrowserContextMenuRequested(QTreeWidgetItem * item);
private:
    QTreeWidget * treeWidget;
    QLineEdit * pathLine;
    QString fbpath;
    QString fbcopypath;
    QString fbcutpath;
    QTreeWidgetItem * fbcopyitem;
    QTreeWidgetItem * fbcutitem;
    QMenu menu;
    QString fileBrowserHomeDir;
    bool acceptEnter;
    bool editMode;
    QTimer mousePressTimer;
public slots:
    void contextMenu();
private slots:
    void fileBrowserExpanded(QTreeWidgetItem * item);
    void fileBrowserCollapsed(QTreeWidgetItem * item);
    void fileBrowserDoubleClicked(QTreeWidgetItem * item, int column);
    void fileBrowserDoubleClickFile(QTreeWidgetItem * item, int column);
    void fileBrowserPathReturnPressed();
    void fileBrowserContextMenuRequested(QPoint p);
    void fileBrowserItemChanged(QTreeWidgetItem * item, int col);
    void fileBrowserItemSelectionChanged();
    void upActionTriggered(bool checked);
    void homeActionTriggered(bool checked);
    void triggerContextMenu();
signals:
    void openFile(QString);
    void fileCreated(QString);
    void folderCreated(QString);
    void fileOrFolderRenamed(QString, QString);
    void fileDeleted(QString);
    void projectCreateRequested(QString, QString, bool, bool);
    void projectEditRequested(QString, QString, bool, bool);
    void projectOpenRequested(QString);
    void showMessage(QString text);
    void showError(QString text);
};

#endif // FILEBROWSER_H
