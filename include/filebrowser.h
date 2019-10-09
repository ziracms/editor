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

class FileBrowser : public QObject
{
    Q_OBJECT
public:
    FileBrowser(QTreeWidget * widget, QLineEdit * line, Settings * settings);
    ~FileBrowser();
    void showCreateFileDialog(QString startDir = "");
    void showCreateFolderDialog(QString startDir = "");
    void showCreateProjectDialog(bool phpLintEnabled, bool phpCSEnabled, QString startDir = "");
    void showEditProjectDialog(QString title, QString path, bool phpLintEnabled, bool phpCSEnabled);
    void openProject(QString startDir = "");
    void refreshFileBrowserDirectory(QString directory);
    void rebuildFileBrowserTree(QString path);
    QString getRootPath();
    QString getHomeDir();
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
private:
    QTreeWidget * treeWidget;
    QLineEdit * pathLine;
    QString fbpath;
    QString fbcopypath;
    QString fbcutpath;
    QTreeWidgetItem * fbcopyitem;
    QTreeWidgetItem * fbcutitem;
    QString fileBrowserHomeDir;
private slots:
    void fileBrowserExpanded(QTreeWidgetItem * item);
    void fileBrowserCollapsed(QTreeWidgetItem * item);
    void fileBrowserDoubleClicked(QTreeWidgetItem * item, int column);
    void fileBrowserPathReturnPressed();
    void fileBrowserContextMenuRequested(QPoint p);
    void fileBrowserItemChanged(QTreeWidgetItem * item, int col);
    void fileBrowserItemSelectionChanged();
    void upActionTriggered(bool checked);
signals:
    void openFile(QString);
    void fileCreated(QString);
    void folderCreated(QString);
    void fileOrFolderRenamed(QString, QString);
    void fileDeleted(QString);
    void projectCreateRequested(QString, QString, bool, bool);
    void projectEditRequested(QString, QString, bool, bool);
    void projectOpenRequested(QString);
};

#endif // FILEBROWSER_H
