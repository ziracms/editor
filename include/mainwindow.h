/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QTreeWidgetItem>
#include <QThread>
#include <QToolButton>
#include <QTabWidget>
#include <QSplitter>
#include "settings.h"
#include "highlightwords.h"
#include "completewords.h"
#include "spellwords.h"
#include "parserworker.h"
#include "filebrowser.h"
#include "editortabs.h"
#include "navigator.h"
#include "gitbrowser.h"
#include "helpwords.h"
#include "project.h"
#include "git.h"
#include "spellcheckerinterface.h"
#include "terminalinterface.h"
#include "quickaccess.h"
#include "progressline.h"
#include "progressinfo.h"
#include "popup.h"
#include "tabslist.h"
#include "welcome.h"
#include "types.h"
#include "style.h"

namespace Ui {
    class MainWindow;
    class CreateFileDialog;
    class CreateFolderDialog;
    class CreateProjectDialog;
    class EditProjectDialog;
    class SearchDialog;
    class SettingsDialog;
    class HelpDialog;
    class WelcomeScreen;
}

Q_DECLARE_METATYPE(ParsePHP::ParseResult)
Q_DECLARE_METATYPE(ParseJS::ParseResult)
Q_DECLARE_METATYPE(ParseCSS::ParseResult)
Q_DECLARE_METATYPE(WordsMapList)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    static bool WANT_RESTART;
protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void setWindowTitleText(QString text);
    void disableActionsForEmptyTabs();
    void enableActionsForOpenTabs();
    void disableActionsForEmptyProject();
    void enableActionsForOpenProject();
    QString getTmpDirPath();
    void clearMessagesTabText();
    void addMessagesTabText(QString text);
    void clearHelpTabSource();
    void setHelpTabSource(QString path);
    void scrollMessagesTabToTop();
    void parseTab();
    void parseTabSplit();
    void reloadWords();
    QString getGitWorkingDir();
    void runServersCommand(QString command, QString pwd, QString description);
    void compileSass(QString src, QString dst);
    void applyThemeColors(QString pluginsDir, bool light, bool applyFont);
    void updateTabsListButton();
    void resetLastSearchParams();
    void showWelcomeScreen();
    void hideWelcomeScreen();
    Editor * getActiveEditor();
    QString getCurrentTabFilename();
public slots:
    void setStatusBarText(QString text);
    void editorShowLine(int line);
    void editorShowLineSymbol(int line, int symbol);
    void restartApp();
    void on_actionOpenProject_triggered();
    void on_actionNewProject_triggered();
private slots:
    void on_actionOpenFile_triggered();
    void on_actionNewFile_triggered();
    void on_actionNewFolder_triggered();
    void on_actionUpdateProject_triggered();
    void on_actionRescanProject_triggered();
    void on_actionCloseProject_triggered();
    void on_actionEditProject_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAll_triggered();
    void on_actionSaveAs_triggered();
    void on_actionClose_triggered();
    void on_actionQuit_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void on_actionFindReplace_triggered();
    void on_actionColorPicker_triggered();
    void on_actionSearchInFiles_triggered();
    void on_actionGitStatus_triggered();
    void on_actionGitLog_triggered();
    void on_actionGitDiffTree_triggered();
    void on_actionGitDiffAll_triggered();
    void on_actionGitDiffCurrent_triggered();
    void on_actionGitDiffAllCommit_triggered();
    void on_actionGitDiffCurrentCommit_triggered();
    void on_actionGitDiscardChanges_triggered();
    void on_actionGitCancelCommit_triggered();
    void on_actionGitDiscardCommit_triggered();
    void on_actionGitRevert_triggered();
    void on_actionGitResetAll_triggered();
    void on_actionGitResetCurrent_triggered();
    void on_actionGitAddAll_triggered();
    void on_actionGitAddCurrent_triggered();
    void on_actionGitCommit_triggered(bool add = false);
    void on_actionGitPush_triggered();
    void on_actionGitPull_triggered();
    void on_actionStartServers_triggered();
    void on_actionStopServers_triggered();
    void on_actionServersStatus_triggered();
    void on_actionShowHideSidebar_triggered();
    void on_actionShowHideToolbar_triggered();
    void on_actionShowHideOutput_triggered();
    void on_actionDisplayDockButtons_triggered();
    void on_actionQuickAccess_triggered();
    void on_actionSettings_triggered();
    void on_actionHelpShortcuts_triggered();
    void on_actionHelpAbout_triggered();
    void on_actionHelpContact_triggered();
    void on_actionHelpDonate_triggered();
    void on_actionHelpZiraCMS_triggered();
    void on_actionHelpFAQ_triggered();
    void on_actionCompileSass_triggered();
    void on_actionExecuteFile_triggered();
    void on_actionExecuteSelection_triggered();
    void on_actionSplitTab_triggered();
    void on_actionOpenContextMenu_triggered();
    void fileBrowserOpen(QString file);
    void focusTreeTriggered();
    void previousTabTriggered();
    void nextTabTriggered();
    void editorSearchInFilesRequested(QString text);
    void editorActionsChanged();
    void editorUndoRedoChanged();
    void editorBackForwardChanged();
    void sidebarProgressChanged(int v);
    void editorFilenameChanged(QString name);
    void editorTabOpened(int index);
    void editorTabSplitOpened(int index);
    void editorTabSwitched(int index);
    void editorTabSplitSwitched(int index);
    void editorTabClosed(int index);
    void editorTabSplitClosed(int index);
    void editorModifiedStateChanged(bool m);
    void editorSaved(int index);
    void editorSplitSaved(int index);
    void editorReady(int index);
    void editorSplitReady(int index);
    void parseLintFinished(int tabIndex, QStringList errorTexts, QStringList errorLines, QString output);
    void execPHPFinished(int tabIndex, QString output);
    void parsePHPCSFinished(int tabIndex, QStringList errorTexts, QStringList errorLines);
    void parseMixedFinished(int tabIndex, ParsePHP::ParseResult result);
    void parseJSFinished(int tabIndex, ParseJS::ParseResult result);
    void parseCSSFinished(int tabIndex, ParseCSS::ParseResult result);
    void parseProjectFinished(bool success = true, bool isModified = true);
    void projectCreateRequested(QString name, QString path, bool lintEnabled, bool csEnabled);
    void projectEditRequested(QString name, QString path, bool lintEnabled, bool csEnabled);
    void projectOpenRequested(QString path);
    void openTabsRequested(QStringList files, bool initHighlight);
    void gotoTabLinesRequested(QList<int> lines);
    void showTodoRequested(QString text);
    void switchToTabRequested(int index);
    void closeAllTabsRequested(void);
    void projectLoadOnStart(void);
    void openFromArgs(void);
    void editorShowDeclaration(QString name);
    void editorShowHelp(QString name);
    void helpBrowserAnchorClicked(QUrl url);
    void messagesBrowserAnchorClicked(QUrl url);
    void editorParsePHPRequested(int index, QString text);
    void editorParseJSRequested(int index, QString text);
    void editorParseCSSRequested(int index, QString text);
    void menuEditOnShow();
    void menuViewOnShow();
    void menuToolsOnShow();
    void searchInFilesFound(QString file, QString lineText, int line, int symbol);
    void searchInFilesFinished();
    void searchListItemDoubleClicked(QListWidgetItem * item);
    void outputDockLocationChanged(Qt::DockWidgetArea area);
    void sidebarDockLocationChanged(Qt::DockWidgetArea area);
    void toolbarOrientationChanged(Qt::Orientation orientation);
    void workerMessage(QString text);
    void runGitCommand(QString path, QString command, QStringList attrs, bool outputResult = true, bool silent = false);
    void gitCommandFinished(QString command, QString output, bool outputResult = true);
    void serversCommandFinished(QString output);
    void sassCommandFinished(QString output);
    void editorFocused();
    void editorSplitFocused();
    void showQAPanel();
    void hideQAPanel();
    void quickAccessRequested(QString file, int line);
    void quickFindRequested(QString text);
    void showPopupText(QString text);
    void showPopupError(QString text);
    void gitTabRefreshRequested();
    void gitTabAddAndCommitRequested();
    void gitTabAddRequested(QString path);
    void gitTabResetRequested(QString path);
    void gitAnnotationRequested(QString path);
    void gitDiffUnifiedRequested(QString path);
    void sidebarActionTriggered(bool checked);
    void outputActionTriggered(bool checked);
    void activateProgressLine();
    void deactivateProgressLine();
    void activateProgressInfo(QString text);
    void updateProgressInfo(QString text);
    void deactivateProgressInfo();
    void editorTabsResize();
    //void editorPaneResize();
    void tabsListTriggered();
    void tabsListSelected(int index);
    void terminalCopy();
    void terminalPaste();
    void showTerminal();
    void startTerminal();
    void outputTabSwitched(int index);
    void inputMethodVisibleChanged();
    void checkScaleFactor();
private:
    Ui::MainWindow *ui;
    Settings * settings;
    HighlightWords * highlightWords;
    CompleteWords * completeWords;
    HelpWords * helpWords;
    SpellWords * spellWords;
    ParserWorker * parserWorker;
    QThread parserThread;
    FileBrowser * filebrowser;
    Navigator * navigator;
    GitBrowser * gitBrowser;
    EditorTabs * editorTabs;
    Project * project;
    Git * git;
    SpellCheckerInterface * spellChecker;
    TerminalInterface * terminal;
    QToolButton * tabsListButton;
    QSplitter * editorsSplitter;
    QString outputMsgErrorTpl;
    QString outputMsgWarningTpl;
    int outputMsgCount;
    QRegularExpression phpManualHeaderExpr;
    QRegularExpression phpManualBreadcrumbsExpr;
    QString lastSearchText;
    QString lastSearchExtensions;
    QStringList lastSearchExcludeDirs;
    bool lastSearchOptionCase;
    bool lastSearchOptionWord;
    bool lastSearchOptionRegexp;
    QColor searchResultsColor;
    QColor outputColor;
    QColor outputBgColor;
    QString colorSheme;
    QString theme;
    QString customThemesPath;
    bool parsePHPLintEnabled;
    bool parsePHPCSEnabled;
    bool parsePHPEnabled;
    bool parseJSEnabled;
    bool parseCSSEnabled;
    bool gitCommandsEnabled;
    bool serverCommandsEnabled;
    QuickAccess * qa;
    Popup * popup;
    ProgressLine * progressLine;
    ProgressInfo * progressInfo;
    TabsList * tabsList;
    QStringList args;
    bool tmpDisableParser;
    Welcome * welcomeScreen;
    int terminalTabIndex;
    QTabWidget * tabWidgetSplit;
    EditorTabs * editorTabsSplit;
    bool isSplitActive;
    QHash<QString,int> filesHistory;
signals:
    void disableWorker();
    void parseLint(int tabIndex, QString path);
    void execPHP(int tabIndex, QString path);
    void execSelection(int tabIndex, QString text);
    void parsePHPCS(int tabIndex, QString path);
    void parseMixed(int tabIndex, QString content);
    void parseJS(int tabIndex, QString content);
    void parseCSS(int tabIndex, QString content);
    void parseProject(QString path);
    void searchInFiles(QString searchDirectory, QString searchText, QString searchExtensions, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp, QStringList excludeDirs);
    void gitCommand(QString path, QString command, QStringList attrs, bool outputResult = true, bool silent = false);
    void serversCommand(QString command, QString pwd);
    void sassCommand(QString src, QString dst);
    void quickFind(QString dir, QString text, WordsMapList words, QStringList wordPrefixes);
};

#endif // MAINWINDOW_H
