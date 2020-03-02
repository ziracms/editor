/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef EDITORTABS_H
#define EDITORTABS_H

#include <QObject>
#include "editor.h"

class EditorTabs : public QObject
{
    Q_OBJECT
public:
    EditorTabs(SpellCheckerInterface * spellChecker, QTabWidget * widget, Settings * settings, HighlightWords * highlightWords, CompleteWords * completeWords, HelpWords * helpWords, SpellWords * spellWords);
    void createTab(QString filepath, bool initHighlight = true);
    bool closeWindowAllowed();
    Editor * getActiveEditor();
    QStringList getOpenTabFiles();
    QList<int> getOpenTabLines();
    int getCurrentTabIndex();
    QString getCurrentTabFilename();
    void closeSaved();
    void setActiveTab(int index);
    void setTabLines(QList<int> lines);
    void initHighlighters();
    QRect getGeometry();
    QRect getGeometryGlobal();
    QRect getGeometryMappedTo(QWidget * parent);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    Editor * getTabEditor(int index);
    QString getTabNameFromPath(QString filepath);
    void fileBrowserFileRenamed(QString oldpath, QString newpath);
    void fileBrowserFolderRenamed(QString oldpath, QString newpath);
private:
    Editor * editor;
    SpellCheckerInterface * spellChecker;
    QTabWidget * tabWidget;
    Settings * settings;
    HighlightWords * highlightWords;
    CompleteWords * completeWords;
    HelpWords * helpWords;
    SpellWords * spellWords;

    bool blockSig;
signals:
    void statusBarText(QString);
    void editorFilenameChanged(QString);
    void tabOpened(int index);
    void tabSwitched(int index);
    void tabClosed(int index);
    void modifiedStateChanged(bool m);
    void editorSaved(int index);
    void editorReady(int index);
    void editorShowDeclaration(QString name);
    void editorShowHelp(QString name);
    void editorParsePHPRequested(int index, QString text);
    void editorParseJSRequested(int index, QString text);
    void editorParseCSSRequested(int index, QString text);
    void editorUndoRedoChanged();
    void editorBackForwardChanged();
    void editorSearchInFilesRequested(QString text);
    void updateProject(void);
    void editorFocused(void);
    void editorBreadcrumbsClick(void);
    void editorShowPopupTextRequested(QString text);
    void editorShowPopupErrorRequested(QString text);
    void gitTabRefreshRequested();
    void editorTabsResize();
    void editorPaneResize();
public slots:
    void openFile(QString filepath, bool initHighlight = true);
    void fileBrowserCreated(QString path);
    void fileBrowserRenamed(QString oldpath, QString newpath);
    void fileBrowserDeleted(QString path);
    void open(QString dir = "");
    void save();
    void saveAll();
    void saveAs();
    void close();
    void closeTab(int index);
private slots:
    void ready(int index);
    void switchTab(int index);
    void movedTab(int from, int to);
    void editorModifiedStateChanged(int index, bool m);
    void statusBarTextChangeRequested(int index, QString text);
    void filenameChanged(int index, QString name);
    void saved(int index);
    void reloaded(int index);
    void showDeclaration(int index, QString name);
    void showHelp(int index, QString name);
    void parsePHP(int index, QString text);
    void parseJS(int index, QString text);
    void parseCSS(int index, QString text);
    void undoRedoChanged(int index);
    void backForwardChanged(int index);
    void searchInFiles(QString text);
    void searchInFilesRequested();
    void focusIn(int index);
    void breadcrumbsClick(int index);
    void warning(int index, QString slug, QString text);
    void showPopupText(int index, QString text);
    void showPopupError(int index, QString text);
};

#endif // EDITORTABS_H
