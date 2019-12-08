/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helper.h"
#include "settings.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDirIterator>
#include <QMimeData>
#include <QShortcut>
#include <QTimer>
#include <QTextCursor>
#include <QColorDialog>
#include <QInputDialog>
#include <QTextStream>
#include <QDesktopServices>
#include "editortab.h"
#include "searchdialog.h"
#include "servers.h"
#include "settingsdialog.h"
#include "helpdialog.h"

const int OUTPUT_TAB_MESSAGES_INDEX = 0;
const int OUTPUT_TAB_HELP_INDEX = 1;
const int OUTPUT_TAB_SEARCH_INDEX = 2;
const int OUTPUT_TAB_GIT_INDEX = 3;
const int OUTPUT_TAB_TODO_INDEX = 4;

const int PROJECT_LOAD_DELAY = 500;

const std::string PHP_MANUAL_ENCODING = "UTF-8";

int const MainWindow::EXIT_CODE_RESTART = -123456789;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<ParsePHP::ParseResult>();
    qRegisterMetaType<ParseJS::ParseResult>();
    qRegisterMetaType<ParseCSS::ParseResult>();
    qRegisterMetaType<WordsMapList>();

    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);

    settings = new Settings();
    settings->load();
    connect(settings, SIGNAL(restartApp()), this, SLOT(restartApp()));

    theme = QString::fromStdString(settings->get("theme"));
    colorSheme = QString::fromStdString(settings->get("color_scheme"));
    if (colorSheme == COLOR_SCHEME_DARK) settings->applyDarkColors();
    else settings->applyLightColors();

    ui->setupUi(this);
    setAcceptDrops(true);

    disableActionsForEmptyTabs();
    disableActionsForEmptyProject();

    // restore window geometry & state
    QSettings windowSettings;
    restoreGeometry(windowSettings.value("main_window_geometry").toByteArray());
    restoreState(windowSettings.value("main_window_state").toByteArray());

    // load words
    highlightWords = new HighlightWords(settings);
    completeWords = new CompleteWords(highlightWords);
    helpWords = new HelpWords();

    // statusbar progress
    progressBar = new QProgressBar;
    progressBar->setMaximumWidth(150);
    progressBar->setRange(0, 100);
    progressBar->hide();
    statusBar()->addPermanentWidget(progressBar);

    // editor tabs
    editorTabs = new EditorTabs(ui->tabWidget, settings, highlightWords, completeWords, helpWords);
    connect(editorTabs, SIGNAL(statusBarText(QString)), this, SLOT(setStatusBarText(QString)));
    connect(editorTabs, SIGNAL(progressChange(int)), this, SLOT(progressChanged(int)));
    connect(editorTabs, SIGNAL(editorFilenameChanged(QString)), this, SLOT(editorFilenameChanged(QString)));
    connect(editorTabs, SIGNAL(tabOpened(int)), this, SLOT(editorTabOpened(int)));
    connect(editorTabs, SIGNAL(tabSwitched(int)), this, SLOT(editorTabSwitched(int)));
    connect(editorTabs, SIGNAL(tabClosed(int)), this, SLOT(editorTabClosed(int)));
    connect(editorTabs, SIGNAL(modifiedStateChanged(bool)), this, SLOT(editorModifiedStateChanged(bool)));
    connect(editorTabs, SIGNAL(editorSaved(int)), this, SLOT(editorSaved(int)));
    connect(editorTabs, SIGNAL(editorReady(int)), this, SLOT(editorReady(int)));
    connect(editorTabs, SIGNAL(editorShowDeclaration(QString)), this, SLOT(editorShowDeclaration(QString)));
    connect(editorTabs, SIGNAL(editorShowHelp(QString)), this, SLOT(editorShowHelp(QString)));
    connect(editorTabs, SIGNAL(editorParsePHPRequested(int,QString)), this, SLOT(editorParsePHPRequested(int,QString)));
    connect(editorTabs, SIGNAL(editorParseJSRequested(int,QString)), this, SLOT(editorParseJSRequested(int,QString)));
    connect(editorTabs, SIGNAL(editorParseCSSRequested(int,QString)), this, SLOT(editorParseCSSRequested(int,QString)));
    connect(editorTabs, SIGNAL(editorUndoRedoChanged()), this, SLOT(editorUndoRedoChanged()));
    connect(editorTabs, SIGNAL(editorBackForwardChanged()), this, SLOT(editorBackForwardChanged()));
    connect(editorTabs, SIGNAL(editorSearchInFilesRequested(QString)), this, SLOT(editorSearchInFilesRequested(QString)));
    connect(editorTabs, SIGNAL(updateProject()), this, SLOT(on_actionUpdateProject_triggered()));
    connect(editorTabs, SIGNAL(editorFocused()), this, SLOT(editorFocused()));
    connect(editorTabs, SIGNAL(editorBreadcrumbsClick()), this, SLOT(on_actionQuickAccess_triggered()));

    // filebrowser
    filebrowser = new FileBrowser(ui->fileBrowserTreeWidget, ui->fileBrowserPathLine, settings);
    connect(filebrowser, SIGNAL(openFile(QString)), editorTabs, SLOT(openFile(QString)));
    connect(filebrowser, SIGNAL(fileCreated(QString)), editorTabs, SLOT(fileBrowserCreated(QString)));
    connect(filebrowser, SIGNAL(fileOrFolderRenamed(QString, QString)), editorTabs, SLOT(fileBrowserRenamed(QString, QString)));
    connect(filebrowser, SIGNAL(fileDeleted(QString)), editorTabs, SLOT(fileBrowserDeleted(QString)));
    connect(filebrowser, SIGNAL(projectCreateRequested(QString,QString, bool, bool)), this, SLOT(projectCreateRequested(QString, QString, bool, bool)));
    connect(filebrowser, SIGNAL(projectEditRequested(QString,QString, bool, bool)), this, SLOT(projectEditRequested(QString, QString, bool, bool)));
    connect(filebrowser, SIGNAL(projectOpenRequested(QString)), this, SLOT(projectOpenRequested(QString)));

    // navigator
    navigator = new Navigator(ui->navigatorTreeWidget, settings);
    connect(navigator, SIGNAL(showLine(int)), this, SLOT(editorShowLine(int)));

    // hide sidebar progressbar
    ui->sidebarProgressBarWrapperWidget->setVisible(false);

    // project class
    project = new Project();
    connect(project, SIGNAL(openTabsRequested(QStringList, bool)), this, SLOT(openTabsRequested(QStringList, bool)));
    connect(project, SIGNAL(gotoTabLinesRequested(QList<int>)), this, SLOT(gotoTabLinesRequested(QList<int>)));
    connect(project, SIGNAL(switchToTabRequested(int)), this, SLOT(switchToTabRequested(int)));
    connect(project, SIGNAL(closeAllTabsRequested()), this, SLOT(closeAllTabsRequested()));
    connect(project, SIGNAL(showTodoRequested(QString)), this, SLOT(showTodoRequested(QString)));

    // git
    git = new Git(settings);
    connect(git, SIGNAL(runGitCommand(QString,QString,QStringList)), this, SLOT(runGitCommand(QString,QString,QStringList)));

    // quick access widget
    qa = new QuickAccess(settings, this);
    connect(qa, SIGNAL(quickAccessRequested(QString,int)), this, SLOT(quickAccessRequested(QString,int)));
    connect(qa, SIGNAL(quickFindRequested(QString)), this, SLOT(quickFindRequested(QString)));

    // enable php lint & cs
    parsePHPLintEnabled = false;
    std::string parsePHPLintEnabledStr = settings->get("parser_enable_php_lint");
    if (parsePHPLintEnabledStr == "yes") parsePHPLintEnabled = true;
    parsePHPCSEnabled = false;
    std::string parsePHPCSEnabledStr = settings->get("parser_enable_php_cs");
    if (parsePHPCSEnabledStr == "yes") parsePHPCSEnabled = true;
    parsePHPEnabled = false;
    std::string parsePHPEnabledStr = settings->get("parser_enable_parse_php");
    if (parsePHPEnabledStr == "yes") parsePHPEnabled = true;
    parseJSEnabled = false;
    std::string parseJSEnabledStr = settings->get("parser_enable_parse_js");
    if (parseJSEnabledStr == "yes") parseJSEnabled = true;
    parseCSSEnabled = false;
    std::string parseCSSEnabledStr = settings->get("parser_enable_parse_css");
    if (parseCSSEnabledStr == "yes") parseCSSEnabled = true;
    gitCommandsEnabled = false;
    std::string gitCommandsEnabledStr = settings->get("parser_enable_git");
    if (gitCommandsEnabledStr == "yes") gitCommandsEnabled = true;
    serverCommandsEnabled = false;
    std::string serverCommandsEnabledStr = settings->get("parser_enable_servers");
    if (serverCommandsEnabledStr == "yes") serverCommandsEnabled = true;

    // parser
    parserWorker = new ParserWorker(settings);
    parserWorker->moveToThread(&parserThread);
    connect(&parserThread, &QThread::finished, parserWorker, &QObject::deleteLater);
    connect(this, SIGNAL(disableWorker()), parserWorker, SLOT(disable()));
    connect(this, SIGNAL(parseLint(int,QString)), parserWorker, SLOT(lint(int,QString)));
    connect(this, SIGNAL(parsePHPCS(int,QString)), parserWorker, SLOT(phpcs(int,QString)));
    connect(this, SIGNAL(parseMixed(int,QString)), parserWorker, SLOT(parseMixed(int,QString)));
    connect(this, SIGNAL(parseJS(int,QString)), parserWorker, SLOT(parseJS(int,QString)));
    connect(this, SIGNAL(parseCSS(int,QString)), parserWorker, SLOT(parseCSS(int,QString)));
    connect(this, SIGNAL(parseProject(QString)), parserWorker, SLOT(parseProject(QString)));
    connect(this, SIGNAL(searchInFiles(QString,QString,QString,bool,bool,bool)), parserWorker, SLOT(searchInFiles(QString,QString,QString,bool,bool,bool)));
    connect(this, SIGNAL(gitCommand(QString, QString, QStringList)), parserWorker, SLOT(gitCommand(QString, QString, QStringList)));
    connect(this, SIGNAL(serversCommand(QString, QString)), parserWorker, SLOT(serversCommand(QString,QString)));
    connect(this, SIGNAL(sassCommand(QString, QString)), parserWorker, SLOT(sassCommand(QString,QString)));
    connect(this, SIGNAL(quickFind(QString, QString, WordsMapList, QStringList)), parserWorker, SLOT(quickFind(QString, QString, WordsMapList, QStringList)));
    connect(parserWorker, SIGNAL(lintFinished(int,QStringList,QStringList,QString)), this, SLOT(parseLintFinished(int,QStringList,QStringList,QString)));
    connect(parserWorker, SIGNAL(phpcsFinished(int,QStringList,QStringList)), this, SLOT(parsePHPCSFinished(int,QStringList,QStringList)));
    connect(parserWorker, SIGNAL(parseMixedFinished(int,ParsePHP::ParseResult)), this, SLOT(parseMixedFinished(int,ParsePHP::ParseResult)));
    connect(parserWorker, SIGNAL(parseJSFinished(int,ParseJS::ParseResult)), this, SLOT(parseJSFinished(int,ParseJS::ParseResult)));
    connect(parserWorker, SIGNAL(parseCSSFinished(int,ParseCSS::ParseResult)), this, SLOT(parseCSSFinished(int,ParseCSS::ParseResult)));
    connect(parserWorker, SIGNAL(parseProjectFinished()), this, SLOT(parseProjectFinished()));
    connect(parserWorker, SIGNAL(parseProjectProgress(int)), this, SLOT(sidebarProgressChanged(int)));
    connect(parserWorker, SIGNAL(searchInFilesFound(QString,QString,int,int)), this, SLOT(searchInFilesFound(QString,QString,int,int)));
    connect(parserWorker, SIGNAL(searchInFilesFinished()), this, SLOT(searchInFilesFinished()));
    connect(parserWorker, SIGNAL(message(QString)), this, SLOT(workerMessage(QString)));
    connect(parserWorker, SIGNAL(gitCommandFinished(QString)), this, SLOT(gitCommandFinished(QString)));
    connect(parserWorker, SIGNAL(serversCommandFinished(QString)), this, SLOT(serversCommandFinished(QString)));
    connect(parserWorker, SIGNAL(sassCommandFinished(QString)), this, SLOT(sassCommandFinished(QString)));
    connect(parserWorker, SIGNAL(quickFound(QString,QString,QString,int)), qa, SLOT(quickFound(QString,QString,QString,int)));
    parserThread.start();

    // message templates
    QString outputMsgErrorColor = QString::fromStdString(settings->get("messages_error_color"));
    QString outputMsgWarningColor = QString::fromStdString(settings->get("messages_warning_color"));
    outputMsgErrorTpl = "<p style=\"color:"+outputMsgErrorColor+"\"><a href=\"%1\">["+tr("Line")+": %1]</a> %2</p>";
    outputMsgWarningTpl = "<p style=\"color:"+outputMsgWarningColor+"\"><a href=\"%1\">["+tr("Line")+": %1]</a> %2</p>";

    outputMsgCount = 0;
    ui->messagesBrowser->setOpenLinks(false);
    ui->messagesBrowser->setOpenExternalLinks(false);
    ui->messagesBrowser->document()->setDefaultStyleSheet("a { text-decoration: none; }");
    connect(ui->messagesBrowser, SIGNAL(anchorClicked(QUrl)), this, SLOT(messagesBrowserAnchorClicked(QUrl)));

    phpManualHeaderExpr = QRegularExpression("<div [^>]*class=\"navbar navbar-fixed-top\"[^>]*>[\\s]*<div[^>]*>.+?</div>[\\s]*</div>", QRegularExpression::DotMatchesEverythingOption);
    phpManualBreadcrumbsExpr = QRegularExpression("<div [^>]*id=\"breadcrumbs\"[^>]*>.+?</div>", QRegularExpression::DotMatchesEverythingOption);

    ui->helpBrowser->setOpenLinks(false);
    ui->helpBrowser->setOpenExternalLinks(false);
    ui->helpBrowser->document()->setDefaultStyleSheet("a { text-decoration: none; }");
    connect(ui->helpBrowser, SIGNAL(anchorClicked(QUrl)), this, SLOT(helpBrowserAnchorClicked(QUrl)));

    lastSearchText = "";
    lastSearchExtensions = "";
    connect(ui->searchListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(searchListItemDoubleClicked(QListWidgetItem*)));

    // todo tab is disabled by default
    ui->todoTab->setEnabled(false);

    connect(ui->menuEdit, SIGNAL(aboutToShow()), this, SLOT(menuEditOnShow()));
    connect(ui->menuView, SIGNAL(aboutToShow()), this, SLOT(menuViewOnShow()));
    connect(ui->menuTools, SIGNAL(aboutToShow()), this, SLOT(menuToolsOnShow()));

    if (ui->mainToolBar->orientation() == Qt::Vertical) {
        ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    }
    connect(ui->mainToolBar, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(toolbarOrientationChanged(Qt::Orientation)));

    ui->outputTabWidget->setFocusPolicy(Qt::NoFocus);
    if (dockWidgetArea(ui->outputDockWidget) == Qt::RightDockWidgetArea) {
        ui->outputTabWidget->setTabPosition(QTabWidget::East);
    } else if (dockWidgetArea(ui->outputDockWidget) == Qt::LeftDockWidgetArea) {
        ui->outputTabWidget->setTabPosition(QTabWidget::West);
    } else {
        ui->outputTabWidget->setTabPosition(QTabWidget::North);
    }
    connect(ui->outputDockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(outputDockLocationChanged(Qt::DockWidgetArea)));

    ui->sidebarTabWidget->setFocusPolicy(Qt::NoFocus);
    if (dockWidgetArea(ui->sidebarDockWidget) == Qt::RightDockWidgetArea) {
        ui->sidebarTabWidget->setTabPosition(QTabWidget::East);
    } else {
        ui->sidebarTabWidget->setTabPosition(QTabWidget::West);
    }
    connect(ui->sidebarDockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(sidebarDockLocationChanged(Qt::DockWidgetArea)));

    searchResultsColor = QColor(QString::fromStdString(settings->get("search_results_color")));
    outputColor = QColor(QString::fromStdString(settings->get("output_color")));
    outputBgColor = QColor(QString::fromStdString(settings->get("output_bg_color")));

    // output tabs font
    QFont outputFont;
    std::string fontFamily = settings->get("editor_font_family");
    std::string fontSize = settings->get("editor_font_size");
    if (fontFamily=="") {
        QFont sysFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        outputFont.setFamily(sysFont.family());
    } else {
        outputFont.setStyleHint(QFont::Monospace);
        outputFont.setFamily(QString::fromStdString(fontFamily));
    }
    outputFont.setPointSize(std::stoi(fontSize));
    ui->messagesBrowser->setFont(outputFont);
    ui->helpBrowser->setFont(outputFont);
    ui->searchListWidget->setFont(outputFont);
    ui->outputEdit->setFont(outputFont);
    ui->todoEdit->setFont(outputFont);

    QPalette outputPalette;
    outputPalette.setColor(QPalette::Base, outputBgColor);
    outputPalette.setColor(QPalette::Text, outputColor);
    ui->messagesBrowser->setPalette(outputPalette);
    ui->helpBrowser->setPalette(outputPalette);
    ui->searchListWidget->setPalette(outputPalette);
    ui->outputEdit->setPalette(outputPalette);
    ui->todoEdit->setPalette(outputPalette);

    // settings
    if (!gitCommandsEnabled) {
        QList<QAction *> gitActions = ui->menuGit->actions();
        foreach (QAction * action, gitActions) {
            action->setEnabled(false);
        }
    }
    if (!serverCommandsEnabled) {
        QList<QAction *> toolsActions = ui->menuTools->actions();
        foreach (QAction * action, toolsActions) {
            if (action->objectName() == "actionServersStatus" ||
                action->objectName() == "actionStartServers" ||
                action->objectName() == "actionStopServers"
            ) {
                action->setEnabled(false);
            }
        }
    }
    if (!parsePHPEnabled) {
        QList<QAction *> createActions = ui->menuCreate->actions();
        foreach (QAction * action, createActions) {
            if (action->objectName() == "actionNewProject"
            ) {
                action->setEnabled(false);
            }
        }
        QList<QAction *> fileActions = ui->menuFile->actions();
        foreach (QAction * action, fileActions) {
            if (action->objectName() == "actionOpenProject" ||
                action->objectName() == "actionUpdateProject" ||
                action->objectName() == "actionRescanProject" ||
                action->objectName() == "actionEditProject"
            ) {
                action->setEnabled(false);
            }
        }
    }

    setWindowTitleText("");
    args = QCoreApplication::arguments();
    if (args.length() <= 1) {
        QTimer::singleShot(PROJECT_LOAD_DELAY, this, SLOT(projectLoadOnStart()));
    } else {
        QTimer::singleShot(PROJECT_LOAD_DELAY, this, SLOT(openFromArgs()));
    }

    // shortcuts
    QString shortcutSidebarStr = QString::fromStdString(settings->get("shortcut_sidebar"));
    QShortcut * shortcutSidebar = new QShortcut(QKeySequence(shortcutSidebarStr), this);
    connect(shortcutSidebar, SIGNAL(activated()), this, SLOT(on_actionShowHideSidebar_triggered()));

    QString shortcutToobarStr = QString::fromStdString(settings->get("shortcut_toolbar"));
    QShortcut * shortcutToobar = new QShortcut(QKeySequence(shortcutToobarStr), this);
    connect(shortcutToobar, SIGNAL(activated()), this, SLOT(on_actionShowHideToolbar_triggered()));

    QString shortcutOutputStr = QString::fromStdString(settings->get("shortcut_output"));
    QShortcut * shortcutOutput = new QShortcut(QKeySequence(shortcutOutputStr), this);
    connect(shortcutOutput, SIGNAL(activated()), this, SLOT(on_actionShowHideOutput_triggered()));

    QString shortcutQuickAccessStr = QString::fromStdString(settings->get("shortcut_quick_access"));
    QShortcut * shortcutQuickAccess = new QShortcut(QKeySequence(shortcutQuickAccessStr), this);
    connect(shortcutQuickAccess, SIGNAL(activated()), this, SLOT(on_actionQuickAccess_triggered()));

    QString shortcutQuickAccessAltStr = QString::fromStdString(settings->get("shortcut_quick_access_alt"));
    QShortcut * shortcutQuickAccessAlt = new QShortcut(QKeySequence(shortcutQuickAccessAltStr), this);
    connect(shortcutQuickAccessAlt, SIGNAL(activated()), this, SLOT(on_actionQuickAccess_triggered()));

    // styles
    applyThemeColors();
}

MainWindow::~MainWindow()
{
    parserThread.quit();
    parserThread.wait();
    delete filebrowser;
    delete navigator;
    delete editorTabs;
    delete project;
    delete git;
    delete settings;
    delete highlightWords;
    delete completeWords;
    delete helpWords;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // check modified
    if (!editorTabs->closeWindowAllowed()) {
        event->ignore();
        return;
    }
    emit disableWorker();
    // save project
    project->save(editorTabs->getOpenTabFiles(), editorTabs->getOpenTabLines(), editorTabs->getCurrentTabIndex(), ui->todoEdit->toPlainText());
    settings->save();
    // save wnd geometry & state
    QSettings windowSettings;
    windowSettings.setValue("main_window_geometry", saveGeometry());
    windowSettings.setValue("main_window_state", saveState());
    windowSettings.setValue("project_path", project->getPath());
    QMainWindow::closeEvent(event);
}

void MainWindow::menuEditOnShow()
{
    editorActionsChanged();
}

void MainWindow::menuViewOnShow()
{
    QList<QAction *> viewActions = ui->menuView->actions();
    foreach (QAction * action, viewActions) {
        if (action->objectName() == "actionShowHideSidebar") {
            if (ui->sidebarDockWidget->isVisible()) action->setChecked(true);
            else action->setChecked(false);
        } else if (action->objectName() == "actionShowHideToolbar") {
            if (ui->mainToolBar->isVisible()) action->setChecked(true);
            else action->setChecked(false);
        } else if (action->objectName() == "actionShowHideOutput") {
            if (ui->outputDockWidget->isVisible()) action->setChecked(true);
            else action->setChecked(false);
        }
    }
}

void MainWindow::menuToolsOnShow()
{
    bool sassEnabled = false;
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor != nullptr && !textEditor->isModified()) {
        QString ext = textEditor->getFileExtension().toLower();
        if (ext == "scss" || ext == "sass") {
            sassEnabled = true;
        }
    }
    QList<QAction *> toolsActions = ui->menuTools->actions();
    foreach (QAction * action, toolsActions) {
        if (action->objectName() == "actionCompileSass") {
            action->setEnabled(sassEnabled);
        }
    }
}

void MainWindow::editorUndoRedoChanged()
{
    editorActionsChanged();
}

void MainWindow::editorBackForwardChanged()
{
    editorActionsChanged();
}

void MainWindow::editorActionsChanged()
{
    bool undo = false, redo = false, back = false, forward = false;
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor != nullptr) {
        undo = textEditor->isUndoable();
        redo = textEditor->isRedoable();
        back = textEditor->isBackable();
        forward = textEditor->isForwadable();
    }
    QList<QAction *> editActions = ui->menuEdit->actions();
    foreach (QAction * action, editActions) {
        if (action->objectName() == "actionUndo") {
            action->setEnabled(undo);
        } else if (action->objectName() == "actionRedo") {
            action->setEnabled(redo);
        } else if (action->objectName() == "actionBack") {
            action->setEnabled(back);
        } else if (action->objectName() == "actionForward") {
            action->setEnabled(forward);
        }
    }
}

void MainWindow::disableActionsForEmptyTabs()
{
    QList<QAction *> fileActions = ui->menuFile->actions();
    foreach (QAction * action, fileActions) {
        if (action->objectName() == "actionSave" ||
            action->objectName() == "actionSaveAll" ||
            action->objectName() == "actionSaveAs" ||
            action->objectName() == "actionClose"
        ) {
            action->setEnabled(false);
        }
    }
    QList<QAction *> editActions = ui->menuEdit->actions();
    foreach (QAction * action, editActions) {
        if (action->objectName() == "actionUndo" ||
            action->objectName() == "actionRedo" ||
            action->objectName() == "actionBack" ||
            action->objectName() == "actionForward" ||
            action->objectName() == "actionFindReplace"
        ) {
            action->setEnabled(false);
        }
    }
    if (gitCommandsEnabled) {
        QList<QAction *> gitActions = ui->menuGit->actions();
        foreach (QAction * action, gitActions) {
            if (action->objectName() == "actionGitDiffCurrent" ||
                action->objectName() == "actionGitAddCurrent" ||
                action->objectName() == "actionGitResetCurrent" ||
                action->objectName() == "actionGitDiffCurrentCommit"
            ) {
                action->setEnabled(false);
            }
        }
    }
}

void MainWindow::enableActionsForOpenTabs()
{
    QList<QAction *> fileActions = ui->menuFile->actions();
    foreach (QAction * action, fileActions) {
        if (action->objectName() == "actionSave" ||
            action->objectName() == "actionSaveAll" ||
            action->objectName() == "actionSaveAs" ||
            action->objectName() == "actionClose"
        ) {
            action->setEnabled(true);
        }
    }
    QList<QAction *> editActions = ui->menuEdit->actions();
    foreach (QAction * action, editActions) {
        if (action->objectName() == "actionFindReplace") {
            action->setEnabled(true);
        }
    }
    if (gitCommandsEnabled) {
        QList<QAction *> gitActions = ui->menuGit->actions();
        foreach (QAction * action, gitActions) {
            if (action->objectName() == "actionGitDiffCurrent" ||
                action->objectName() == "actionGitAddCurrent" ||
                action->objectName() == "actionGitResetCurrent" ||
                action->objectName() == "actionGitDiffCurrentCommit"
            ) {
                action->setEnabled(true);
            }
        }
    }
}

void MainWindow::disableActionsForEmptyProject()
{
    QList<QAction *> fileActions = ui->menuFile->actions();
    foreach (QAction * action, fileActions) {
        if (action->objectName() == "actionCloseProject" ||
            action->objectName() == "actionUpdateProject" ||
            action->objectName() == "actionRescanProject" ||
            action->objectName() == "actionEditProject"
        ) {
            action->setEnabled(false);
        }
    }
    ui->todoTab->setEnabled(false);
}

void MainWindow::enableActionsForOpenProject()
{
    QList<QAction *> fileActions = ui->menuFile->actions();
    foreach (QAction * action, fileActions) {
        if (action->objectName() == "actionCloseProject" ||
            action->objectName() == "actionUpdateProject" ||
            action->objectName() == "actionRescanProject" ||
            action->objectName() == "actionEditProject"
        ) {
            action->setEnabled(true);
        }
    }
    ui->todoTab->setEnabled(true);
}

void MainWindow::projectLoadOnStart()
{
    QSettings windowSettings;
    QString projectPath = windowSettings.value("project_path").toString();
    if (projectPath.size() > 0 && Helper::folderExists(projectPath) && project->exists(projectPath)) {
        projectOpenRequested(projectPath);
    }
}

void MainWindow::openFromArgs() {
    if (args.length() <= 1) return;
    QStringList files;
    for (int i=1; i<args.length(); i++) {
        QString arg = args[i];
        if (!Helper::fileExists(arg)) continue;
        files.append(arg);
    }
    openTabsRequested(files, false);
    editorTabs->initHighlighters();
}

void MainWindow::on_actionOpenFile_triggered()
{
    setStatusBarText("");
    editorTabs->open(filebrowser->getRootPath());
}

void MainWindow::on_actionNewFile_triggered()
{
    filebrowser->showCreateFileDialog();
}

void MainWindow::on_actionNewFolder_triggered()
{
    filebrowser->showCreateFolderDialog();
}

void MainWindow::on_actionNewProject_triggered()
{
    filebrowser->showCreateProjectDialog(parsePHPLintEnabled, parsePHPCSEnabled);
}

void MainWindow::on_actionEditProject_triggered()
{
    if (!project->isOpen()) return;
    filebrowser->showEditProjectDialog(project->getName(), project->getPath(), project->isPHPLintEnabled(), project->isPHPCSEnabled());
}

void MainWindow::on_actionOpenProject_triggered()
{
    filebrowser->openProject();
}

void MainWindow::on_actionUpdateProject_triggered()
{
    if (!project->isOpen()) return;
    setStatusBarText(tr("Scanning project..."));
    emit parseProject(project->getPath());
}

void MainWindow::on_actionRescanProject_triggered()
{
    if (!project->isOpen()) return;
    project->deleteDataFile();
    setStatusBarText(tr("Scanning project..."));
    emit parseProject(project->getPath());
}

void MainWindow::on_actionCloseProject_triggered()
{
    project->save(editorTabs->getOpenTabFiles(), editorTabs->getOpenTabLines(), editorTabs->getCurrentTabIndex(), ui->todoEdit->toPlainText());
    project->close();
    reloadWords();
    disableActionsForEmptyProject();
    filebrowser->rebuildFileBrowserTree(filebrowser->getHomeDir());
    // update window title
    setWindowTitleText("");
}

void MainWindow::on_actionSave_triggered()
{
    editorTabs->save();
}

void MainWindow::on_actionSaveAll_triggered()
{
    editorTabs->saveAll();
}

void MainWindow::on_actionSaveAs_triggered()
{
    editorTabs->saveAs();
}

void MainWindow::on_actionClose_triggered()
{
    editorTabs->close();
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionUndo_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->redo();
}

void MainWindow::on_actionBack_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->back();
}

void MainWindow::on_actionForward_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->forward();
}

void MainWindow::on_actionFindReplace_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->findToggle();
}

void MainWindow::on_actionColorPicker_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    QColor initColor = Qt::white;
    bool withHash = true;
    if (textEditor != nullptr) {
        QTextCursor curs = textEditor->textCursor();
        QString text = curs.selectedText();
        if (text.size() > 0) {
            if (text.at(0) != "#") {
                withHash = false;
                text = "#" + text;
            }
            QColor textColor = QColor(text);
            if (textColor.isValid()) initColor = textColor;
        }
    }
    QColor color = QColorDialog::getColor(initColor, this, tr("Pick a color"));
    if (!color.isValid()) return;
    if (textEditor != nullptr) {
        QTextCursor curs = textEditor->textCursor();
        QString text = color.name();
        if (!withHash && text.size() > 0 && text.at(0) == "#") text = text.mid(1);
        curs.insertText(text);
        textEditor->setTextCursor(curs);
        textEditor->setFocus();
    }
}

void MainWindow::on_actionSearchInFiles_triggered()
{
    QString text = "";
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor != nullptr) {
        QTextCursor curs = textEditor->textCursor();
        text = curs.selectedText();
    }
    editorSearchInFilesRequested(text);
}

void MainWindow::on_actionShowHideSidebar_triggered()
{
    hideQAPanel();
    if (ui->sidebarDockWidget->isVisible()) {
        ui->sidebarDockWidget->hide();
    } else {
        ui->sidebarDockWidget->show();
    }
}

void MainWindow::on_actionShowHideToolbar_triggered()
{
    hideQAPanel();
    if (ui->mainToolBar->isVisible()) {
        ui->mainToolBar->hide();
    } else {
        ui->mainToolBar->show();
    }
}

void MainWindow::on_actionShowHideOutput_triggered()
{
    hideQAPanel();
    if (ui->outputDockWidget->isVisible()) {
        ui->outputDockWidget->hide();
    } else {
        ui->outputDockWidget->show();
    }
}

void MainWindow::on_actionQuickAccess_triggered()
{
    if (!qa->isVisible()) showQAPanel();
    else hideQAPanel();
}

void MainWindow::on_actionStartServers_triggered()
{
    bool ok;
    QString pwd = QInputDialog::getText(this, tr("Enter root password"), tr("Password:"), QLineEdit::Password, "", &ok);
    if (!ok) return;
    runServersCommand(SERVERS_START_CMD, pwd, tr("Starting apache2 and mariadb servers..."));
}

void MainWindow::on_actionStopServers_triggered()
{
    bool ok;
    QString pwd = QInputDialog::getText(this, tr("Enter root password"), tr("Password:"), QLineEdit::Password, "", &ok);
    if (!ok) return;
    runServersCommand(SERVERS_STOP_CMD, pwd, tr("Stopping apache2 and mariadb servers..."));
}

void MainWindow::on_actionServersStatus_triggered()
{
    bool ok;
    QString pwd = QInputDialog::getText(this, tr("Enter root password"), tr("Password:"), QLineEdit::Password, "", &ok);
    if (!ok) return;
    runServersCommand(SERVERS_STATUS_CMD, pwd, tr("Fetching status of apache2 and mariadb servers..."));
}

void MainWindow::runServersCommand(QString command, QString pwd, QString description)
{
    if (!serverCommandsEnabled) return;
    hideQAPanel();
    if (!ui->outputDockWidget->isVisible()) ui->outputDockWidget->show();
    ui->outputTabWidget->setCurrentIndex(OUTPUT_TAB_GIT_INDEX);
    ui->outputEdit->clear();
    ui->outputEdit->setHtml(Servers::highlightServersCommand(description, settings));
    emit serversCommand(command, pwd);
}

void MainWindow::serversCommandFinished(QString output)
{
    output = output.trimmed() + "\n\n" + tr("Finished.");
    ui->outputEdit->append(Servers::highlightServersCommandOutput(output, settings));
    QTextCursor cursor = ui->outputEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ui->outputEdit->setTextCursor(cursor);
}

void MainWindow::on_actionCompileSass_triggered()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr || textEditor->isModified()) return;
    QString ext = textEditor->getFileExtension().toLower();
    if (ext != "scss" && ext != "sass") return;
    QString fileName = textEditor->getFileName();
    QFileInfo fInfo(fileName);
    QString file = fInfo.baseName() + ".css";
    QString path = fInfo.absolutePath();
    bool ok;
    file = QInputDialog::getText(this, tr("Enter filename"), tr("Filename:"), QLineEdit::Normal, file, &ok);
    if (!ok) return;
    compileSass(fileName, path + "/" + file);
}

void MainWindow::compileSass(QString src, QString dst)
{
    if (!Helper::fileExists(src) || dst.size() == 0) return;
    hideQAPanel();
    if (!ui->outputDockWidget->isVisible()) ui->outputDockWidget->show();
    ui->outputTabWidget->setCurrentIndex(OUTPUT_TAB_GIT_INDEX);
    ui->outputEdit->clear();
    ui->outputEdit->setText(src + " >> " + dst + "\n");
    emit sassCommand(src, dst);
}

void MainWindow::sassCommandFinished(QString output)
{
    output = output.trimmed();
    if (output.size() == 0) output = tr("Finished.");
    ui->outputEdit->append(output);
    QTextCursor cursor = ui->outputEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ui->outputEdit->setTextCursor(cursor);
}

QString MainWindow::getGitWorkingDir()
{
    QString path;
    if (project->isOpen()) {
        path = project->getPath();
    } else {
        path = filebrowser->getRootPath();
    }
    return path;
}

void MainWindow::on_actionGitStatus_triggered()
{
    git->showStatus(getGitWorkingDir());
}

void MainWindow::on_actionGitLog_triggered()
{
    git->showLog(getGitWorkingDir());
}

void MainWindow::on_actionGitDiffTree_triggered()
{
    git->showLastCommitDiffTree(getGitWorkingDir());
}

void MainWindow::on_actionGitDiffAll_triggered()
{
    git->showUncommittedDiffAll(getGitWorkingDir());
}

void MainWindow::on_actionGitDiffCurrent_triggered()
{
    QString fileName = editorTabs->getCurrentTabFilename();
    if (fileName.size() == 0 || !Helper::fileExists(fileName)) return;
    git->showUncommittedDiffCurrent(getGitWorkingDir(), fileName);
}

void MainWindow::on_actionGitDiffAllCommit_triggered()
{
    git->showLastCommitDiffAll(getGitWorkingDir());
}

void MainWindow::on_actionGitDiffCurrentCommit_triggered()
{
    QString fileName = editorTabs->getCurrentTabFilename();
    if (fileName.size() == 0 || !Helper::fileExists(fileName)) return;
    git->showLastCommitDiffCurrent(getGitWorkingDir(), fileName);
}

void MainWindow::on_actionGitDiscardChanges_triggered()
{
    git->resetHardUncommitted(getGitWorkingDir());
}

void MainWindow::on_actionGitDiscardCommit_triggered()
{
    git->resetHardToPreviousCommit(getGitWorkingDir());
}

void MainWindow::on_actionGitRevert_triggered()
{
    git->revertLastCommit(getGitWorkingDir());
}

void MainWindow::on_actionGitResetAll_triggered()
{
    git->resetAll(getGitWorkingDir());
}

void MainWindow::on_actionGitResetCurrent_triggered()
{
    QString fileName = editorTabs->getCurrentTabFilename();
    if (fileName.size() == 0 || !Helper::fileExists(fileName)) return;
    git->resetCurrent(getGitWorkingDir(), fileName);
}

void MainWindow::on_actionGitAddAll_triggered()
{
    git->addAll(getGitWorkingDir());
}

void MainWindow::on_actionGitAddCurrent_triggered()
{
    QString fileName = editorTabs->getCurrentTabFilename();
    if (fileName.size() == 0 || !Helper::fileExists(fileName)) return;
    git->addCurrent(getGitWorkingDir(), fileName);
}

void MainWindow::on_actionGitCommit_triggered()
{
    bool ok;
    QString msg = QInputDialog::getText(this, tr("Commit message"), tr("Message:"), QLineEdit::Normal, "", &ok);
    if (!ok || msg.isEmpty()) return;
    git->commit(getGitWorkingDir(), msg);
}

void MainWindow::on_actionGitPush_triggered()
{
    git->pushOriginMaster(getGitWorkingDir());
}

void MainWindow::on_actionGitPull_triggered()
{
    git->pullOriginMaster(getGitWorkingDir());
}

void MainWindow::runGitCommand(QString path, QString command, QStringList attrs)
{
    if (!gitCommandsEnabled) return;
    if (!git->isCommandSafe(command) &&
        QMessageBox::question(this, tr("Are you sure ?"), tr("Do you really want to \"%1\" ?").arg(QString("git "+command+" "+attrs.join(" ")).trimmed()), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
    hideQAPanel();
    if (!ui->outputDockWidget->isVisible()) ui->outputDockWidget->show();
    ui->outputTabWidget->setCurrentIndex(OUTPUT_TAB_GIT_INDEX);
    ui->outputEdit->clear();
    QString attrStr = "";
    for (int i=0; i<attrs.size(); i++) {
        QString attr = attrs.at(i);
        if (attrStr.size() > 0) attrStr += " ";
        if (attr.indexOf(" ") >= 0) attrStr += "'" + attr + "'";
        else attrStr += attr;
    }
    QString cmdStr = path+"> git "+command+" "+attrStr;
    ui->outputEdit->setHtml(git->highlightCommand(cmdStr));
    emit gitCommand(path, command, attrs);
}

void MainWindow::gitCommandFinished(QString output)
{
    if (output.size() == 0) output = tr("Finished.");
    ui->outputEdit->append(git->highlightOutput(output));
    QTextCursor cursor = ui->outputEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ui->outputEdit->setTextCursor(cursor);
    ui->outputEdit->setFocus();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog(settings, this);
    if (dialog.exec() != QDialog::Accepted) return;
    settings->change(dialog.getData());
    emit restartApp();
}

void MainWindow::on_actionHelpShortcuts_triggered()
{
    HelpDialog dialog(this);
    dialog.shortcutsContent();
    dialog.exec();
}

void MainWindow::on_actionHelpAbout_triggered()
{
    HelpDialog dialog(this);
    dialog.aboutContent();
    dialog.exec();
}

void MainWindow::on_actionHelpContact_triggered()
{
    QString url = "mailto:" + AUTHOR_EMAIL_USERNAME + "@" + AUTHOR_EMAIL_DOMAIN;
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::on_actionHelpDonate_triggered()
{
    QDesktopServices::openUrl(QUrl(AUTHOR_CARD_URL + "/" + AUTHOR_CARD_ID));
}

void MainWindow::on_actionHelpZiraCMS_triggered()
{
    QDesktopServices::openUrl(QUrl(AUTHOR_CMS_URL));
}

void MainWindow::editorSearchInFilesRequested(QString text)
{
    if (text.size() == 0) text = lastSearchText;
    else lastSearchText = text;
    SearchDialog dialog(this);
    dialog.setText(text);
    if (project->isOpen()) {
        dialog.setDirectory(project->getPath());
    } else {
        dialog.setDirectory(filebrowser->getRootPath());
    }
    if (lastSearchExtensions.size() > 0) {
        dialog.setExtensions(lastSearchExtensions);
    } else {
        dialog.setExtensions(".*");
    }
    dialog.focusText();
    if (dialog.exec() != QDialog::Accepted) return;
    QString searchDirectory = dialog.getDirectory();
    QString searchText = dialog.getText();
    QString searchExtensions = dialog.getExtensions();
    bool searchOptionCase = dialog.getCaseOption();
    bool searchOptionWord = dialog.getWordOption();
    bool searchOptionRegexp = dialog.getRegexpOption();
    lastSearchText = searchText;
    lastSearchExtensions = searchExtensions;
    if (searchDirectory.size() == 0 || searchText.size() == 0) return;
    if (!Helper::folderExists(searchDirectory)) return;
    hideQAPanel();
    if (!ui->outputDockWidget->isVisible()) ui->outputDockWidget->show();
    ui->searchListWidget->clear();
    ui->outputTabWidget->setCurrentIndex(OUTPUT_TAB_SEARCH_INDEX);
    ui->outputTabWidget->setTabText(OUTPUT_TAB_SEARCH_INDEX, tr("Searching..."));
    setStatusBarText("Searching...");
    emit searchInFiles(searchDirectory, searchText, searchExtensions, searchOptionCase, searchOptionWord, searchOptionRegexp);
}

void MainWindow::searchInFilesFound(QString file, QString lineText, int line, int symbol)
{
    if (lineText.size() > 300) lineText = lineText.mid(0, 300) + "...";
    QFileInfo fInfo = QFileInfo(file);
    QString text = lineText;
    if (file.size() > 0 && line > 0 && symbol >= 0) {
        text = Helper::intToStr(ui->searchListWidget->count()+1) + ". [" + fInfo.fileName() + ":" + Helper::intToStr(line) + "] " + lineText.trimmed();
    }
    QListWidgetItem * item = new QListWidgetItem();
    item->setText(text);
    item->setToolTip(file);
    item->setData(Qt::UserRole, QVariant(file));
    item->setData(Qt::UserRole+1, QVariant(line));
    item->setData(Qt::UserRole+2, QVariant(symbol));
    item->setForeground(searchResultsColor);
    item->setBackground(outputBgColor);
    ui->searchListWidget->addItem(item);
}

void MainWindow::searchInFilesFinished()
{
    ui->outputTabWidget->setTabText(OUTPUT_TAB_SEARCH_INDEX, tr("Search")+"("+Helper::intToStr(ui->searchListWidget->count())+")");
    if (ui->searchListWidget->count() == 0) {
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(tr("No results"));
        item->setData(Qt::UserRole, QVariant(""));
        item->setData(Qt::UserRole+1, QVariant(-1));
        item->setData(Qt::UserRole+2, QVariant(-1));
        ui->searchListWidget->addItem(item);
    }
    setStatusBarText("Search finished");
}

void MainWindow::searchListItemDoubleClicked(QListWidgetItem *item)
{
    QString file = item->data(Qt::UserRole).toString();
    int line = item->data(Qt::UserRole+1).toInt();
    int symbol = item->data(Qt::UserRole+2).toInt();
    if (file.size() == 0 || line <= 0 || symbol < 0) return;
    if (!Helper::fileExists(file)) return;
    editorTabs->openFile(file);
    editorShowLineSymbol(line, symbol);
}

void MainWindow::progressChanged(int v)
{
    if (v < 100 && !progressBar->isVisible()) progressBar->show();
    if (v == 100 && progressBar->isVisible()) progressBar->hide();
    if (v >= 0 && v <= 100) progressBar->setValue(v);
}

void MainWindow::sidebarProgressChanged(int v)
{
    if (v < 100 && !ui->sidebarProgressBarWrapperWidget->isVisible()) ui->sidebarProgressBarWrapperWidget->show();
    //if (v == 100 && ui->sidebarProgressBarWrapperWidget->isVisible()) ui->sidebarProgressBarWrapperWidget->hide();
    if (v >= 0 && v <= 100) ui->sidebarProgressBar->setValue(v);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent * event)
{
    foreach (const QUrl &url, event->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        editorTabs->openFile(fileName);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    hideQAPanel();
    QMainWindow::mousePressEvent(e);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    hideQAPanel();
    QMainWindow::resizeEvent(event);
}

void MainWindow::editorFocused()
{
    hideQAPanel();
}

void MainWindow::showQAPanel()
{
    QRect editorTabsRectM = editorTabs->getGeometryMappedTo(this);
    int px = editorTabsRectM.x();
    int py = editorTabsRectM.y();
    int w = editorTabsRectM.width() / 2;
    int h = editorTabsRectM.height();
    int offsetX = editorTabsRectM.width() - w - 1;
    int offsetY = editorTabsRectM.height() - h + 1;
    qa->display(px + offsetX, py + offsetY, w, h - 2);
    qa->raise();
}

void MainWindow::hideQAPanel()
{
    if (!qa->isVisible()) return;
    qa->hide();
}

void MainWindow::quickAccessRequested(QString file, int line)
{
    if (file.size() > 0 && line > 0 && Helper::fileExists(file)) {
        hideQAPanel();
        editorTabs->openFile(file);
        Editor * editor = editorTabs->getActiveEditor();
        if (editor != nullptr && editor->getFileName() == file) {
            editor->gotoLine(line);
        }
    }
}

void MainWindow::quickFindRequested(QString text)
{

    QString dir = filebrowser->getRootPath();
    if (project->isOpen()) dir = project->getPath();
    WordsMapList words;
    QStringList wordPrefixes;
    if (project->isOpen()) {
        words.push_back(project->phpClassDeclarations);
        wordPrefixes.append("class: ");
        words.push_back(project->phpClassMethodDeclarations);
        wordPrefixes.append("method: ");
        words.push_back(project->phpFunctionDeclarations);
        wordPrefixes.append("function: ");
    }
    emit quickFind(dir, text, words, wordPrefixes);
}

void MainWindow::editorFilenameChanged(QString name)
{
    setWindowTitleText(name);
    QFileInfo fInfo(name);
    QString dir = fInfo.dir().absolutePath();
    if (dir.size() > 0) filebrowser->refreshFileBrowserDirectory(dir);
}

void MainWindow::editorTabOpened(int)
{
    navigator->clear();
    enableActionsForOpenTabs();
    Editor * editor = editorTabs->getActiveEditor();
    if (editor != nullptr) {
        setWindowTitleText(editor->getFileName());
        clearMessagesTabText();
        editorActionsChanged();
    }
}

void MainWindow::editorTabSwitched(int)
{
    progressBar->hide();
    navigator->clear();
    clearMessagesTabText();
    setStatusBarText(""); // update status bar
    Editor * editor = editorTabs->getActiveEditor();
    if (editor != nullptr) {
        setWindowModified(editor->isModified());
        setWindowTitleText(editor->getFileName());
        parseTab();
        editorActionsChanged();
    } else {
        setWindowModified(false);
        setWindowTitleText(""); // update window title
    }
}

void MainWindow::editorTabClosed(int)
{
    Editor * editor = editorTabs->getActiveEditor();
    if (editor == nullptr) {
        disableActionsForEmptyTabs();
    }
}

void MainWindow::editorModifiedStateChanged(bool m)
{
    setWindowModified(m);
}

void MainWindow::editorSaved(int index)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr || textEditor->getTabIndex() != index) return;
    parseTab();
}

void MainWindow::editorReady(int index)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr || textEditor->getTabIndex() != index) return;
    parseTab();
}

QString MainWindow::getTmpDirPath()
{
    return QDir::tempPath();
}

void MainWindow::parseTab()
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    int tabIndex = textEditor->getTabIndex();
    QString path = textEditor->getFileName();
    std::string modeType = textEditor->getModeType();
    clearMessagesTabText();
    if (modeType == MODE_UNKNOWN) return;
    if (modeType == MODE_MIXED) {
        if ((!project->isOpen() && parsePHPLintEnabled) || (project->isOpen() && project->isPHPLintEnabled())) emit parseLint(tabIndex, path);
        if (textEditor->isReady() && parsePHPEnabled) emit parseMixed(tabIndex, textEditor->getContent());
    }
    if (modeType == MODE_JS && textEditor->isReady() && parseJSEnabled) emit parseJS(tabIndex, textEditor->getContent());
    if (modeType == MODE_CSS && textEditor->isReady() && parseCSSEnabled) emit parseCSS(tabIndex, textEditor->getContent());
    if ((!project->isOpen() && parsePHPCSEnabled) || (project->isOpen() && project->isPHPCSEnabled())) emit parsePHPCS(tabIndex, path);
}

void MainWindow::parseLintFinished(int tabIndex, QStringList errorTexts, QStringList errorLines, QString output)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    if (tabIndex != textEditor->getTabIndex()) return;
    textEditor->clearErrors();
    if (errorTexts.size() > 0 && errorTexts.size() == errorLines.size()) {
        for (int i=0; i<errorTexts.size(); i++) {
            QString lineStr = errorLines.at(i);
            QString errorStr = errorTexts.at(i);
            textEditor->setError(lineStr.toInt(), errorStr);
            addMessagesTabText(outputMsgErrorTpl.arg(lineStr).arg(errorStr));
        }
    }
    textEditor->updateMarksAndMapArea();
    if (errorTexts.size() > 0 && errorTexts.size() == errorLines.size()) {
        setStatusBarText(tr("PARSE ERROR"));
        scrollMessagesTabToTop();
    } else if (output.size() > 0 && (errorTexts.size() == 0 || errorTexts.size() != errorLines.size())) {
        setStatusBarText(tr("PARSE ERROR"));
        addMessagesTabText(outputMsgErrorTpl.arg("unknown").arg(output));
        scrollMessagesTabToTop();
    } else {
        setStatusBarText(tr("PARSE OK"));
    }
}

void MainWindow::parsePHPCSFinished(int tabIndex, QStringList errorTexts, QStringList errorLines)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    if (tabIndex != textEditor->getTabIndex()) return;
    textEditor->clearWarnings();
    if (errorTexts.size() > 0 && errorTexts.size() == errorLines.size()) {
        for (int i=0; i<errorTexts.size(); i++) {
            QString lineStr = errorLines.at(i);
            QString errorStr = errorTexts.at(i);
            textEditor->setWarning(lineStr.toInt(), errorStr);
            addMessagesTabText(outputMsgWarningTpl.arg(lineStr).arg(errorStr));
        }
    }
    textEditor->updateMarksAndMapArea();
    scrollMessagesTabToTop();
}

void MainWindow::parseMixedFinished(int tabIndex, ParsePHP::ParseResult result)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    if (tabIndex != textEditor->getTabIndex()) return;
    textEditor->setParseResult(result);
    navigator->build(result);
    qa->setParseResult(result, textEditor->getFileName());
}

void MainWindow::parseJSFinished(int tabIndex, ParseJS::ParseResult result)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    if (tabIndex != textEditor->getTabIndex()) return;
    textEditor->setParseResult(result);
    navigator->build(result);
    qa->setParseResult(result, textEditor->getFileName());
}

void MainWindow::parseCSSFinished(int tabIndex, ParseCSS::ParseResult result)
{
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    if (tabIndex != textEditor->getTabIndex()) return;
    textEditor->setParseResult(result);
    navigator->build(result);
    qa->setParseResult(result, textEditor->getFileName());
}

void MainWindow::parseProjectFinished()
{
    setStatusBarText(tr("Loading project..."));
    reloadWords();
    project->loadWords(completeWords, highlightWords, helpWords);
    setStatusBarText(tr(""));
    if (ui->sidebarProgressBarWrapperWidget->isVisible()) ui->sidebarProgressBarWrapperWidget->hide();
    editorTabs->initHighlighters();
}

void MainWindow::projectCreateRequested(QString name, QString path, bool lintEnabled, bool csEnabled)
{
    if (!parsePHPEnabled) return;
    if (!project->create(name, path, lintEnabled, csEnabled, gitCommandsEnabled)) {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("Could not create the project."));
        msgBox.exec();
        return;
    }
    projectOpenRequested(path);
}

void MainWindow::projectEditRequested(QString name, QString path, bool lintEnabled, bool csEnabled)
{
    if (!project->edit(name, path, lintEnabled, csEnabled, editorTabs->getOpenTabFiles(), editorTabs->getOpenTabLines(), editorTabs->getCurrentTabIndex(), ui->todoEdit->toPlainText())) {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("Could not edit the project."));
        msgBox.exec();
        return;
    }
    clearMessagesTabText();
    setStatusBarText("Project saved.");
    Editor * editor = editorTabs->getActiveEditor();
    if (editor != nullptr) {
        setWindowModified(editor->isModified());
        setWindowTitleText(editor->getFileName());
        parseTab();
    } else {
        setWindowModified(false);
        setWindowTitleText(""); // update window title
    }
}

void MainWindow::projectOpenRequested(QString path)
{
    if (!parsePHPEnabled) return;
    if (!project->exists(path)) {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("Project not found in this directory."));
        msgBox.exec();
        return;
    }
    project->save(editorTabs->getOpenTabFiles(), editorTabs->getOpenTabLines(), editorTabs->getCurrentTabIndex(), ui->todoEdit->toPlainText());
    closeAllTabsRequested();
    if (!project->open(path)) {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("Could not open the project."));
        msgBox.exec();
        return;
    }
    filebrowser->rebuildFileBrowserTree(path);
    ui->outputEdit->clear();
    enableActionsForOpenProject();
    setStatusBarText(tr("Scanning project..."));
    emit parseProject(project->getPath());
}

void MainWindow::openTabsRequested(QStringList files, bool initHighlight)
{
    for (int i=0; i<files.size(); i++) {
        QString file = files.at(i);
        editorTabs->openFile(file, initHighlight);
    }
}

void MainWindow::gotoTabLinesRequested(QList<int> lines)
{
    editorTabs->setTabLines(lines);
}

void MainWindow::switchToTabRequested(int index)
{
    editorTabs->setActiveTab(index);
}

void MainWindow::showTodoRequested(QString text)
{
    ui->todoEdit->setPlainText(text);
}

void MainWindow::closeAllTabsRequested()
{
    editorTabs->closeSaved();
}

void MainWindow::reloadWords()
{
    highlightWords->reset();
    completeWords->reset();
    helpWords->reset();

    highlightWords->load();
    completeWords->load();
    helpWords->load();
}

void MainWindow::editorShowDeclaration(QString name)
{
    if (name.size() > 0 && name.at(0) == "\\") name = name.mid(1);
    if (name.size() == 0) return;
    QString path = "";
    int line = 0;
    project->findDeclaration(name, path, line);
    if (path.size() > 0 && line > 0 && Helper::fileExists(path)) {
        editorTabs->openFile(path);
        Editor * editor = editorTabs->getActiveEditor();
        if (editor != nullptr && editor->getFileName() == path) {
            editor->gotoLine(line);
        }
    }
}

void MainWindow::editorShowHelp(QString name)
{
    if (name.size() > 0 && name.at(0) == "\\") name = name.mid(1);
    if (name.size() == 0) return;
    QString php_manual_path = QString::fromStdString(settings->get("php_manual_path"));
    if (php_manual_path.size() == 0 || !Helper::folderExists(php_manual_path)) return;
    QString file = helpWords->findHelpFile(name);
    if (file.size() > 0 && Helper::fileExists(php_manual_path + "/" + file)) {
        setHelpTabSource(php_manual_path + "/" + file);
    } else {
        clearHelpTabSource();
    }
}

void MainWindow::editorParsePHPRequested(int index, QString text)
{
    if (!parsePHPEnabled) return;
    emit parseMixed(index, text);
}

void MainWindow::editorParseJSRequested(int index, QString text)
{
    if (!parseJSEnabled) return;
    emit parseJS(index, text);
}

void MainWindow::editorParseCSSRequested(int index, QString text)
{
    if (!parseCSSEnabled) return;
    emit parseCSS(index, text);
}

void MainWindow::clearMessagesTabText()
{
    ui->messagesBrowser->setText("");
    QString msgTabText = ui->outputTabWidget->tabText(OUTPUT_TAB_MESSAGES_INDEX);
    if (msgTabText.size()>1) {
        msgTabText.replace(QRegularExpression("[(].*[)]"), "");
    }
    ui->outputTabWidget->setTabText(OUTPUT_TAB_MESSAGES_INDEX, msgTabText);
    outputMsgCount = 0;
}

void MainWindow::addMessagesTabText(QString text)
{
    if (text.size() == 0) return;
    outputMsgCount++;
    ui->messagesBrowser->append(text);
    QString msgTabText = ui->outputTabWidget->tabText(OUTPUT_TAB_MESSAGES_INDEX);
    if (msgTabText.size()>1) {
        msgTabText.replace(QRegularExpression("[(].*[)]"), "");
    }
    msgTabText += "("+Helper::intToStr(outputMsgCount)+")";
    ui->outputTabWidget->setTabText(OUTPUT_TAB_MESSAGES_INDEX, msgTabText);
}

void MainWindow::clearHelpTabSource()
{
    ui->helpBrowser->clear();
}

void MainWindow::setHelpTabSource(QString path)
{
    if (path.size() == 0 || !Helper::fileExists(path)) return;
    hideQAPanel();
    if (!ui->outputDockWidget->isVisible()) ui->outputDockWidget->show();
    ui->outputTabWidget->setCurrentIndex(OUTPUT_TAB_HELP_INDEX);
    QString source = Helper::loadTextFile(path, PHP_MANUAL_ENCODING, PHP_MANUAL_ENCODING, true);
    source.replace(phpManualHeaderExpr, "");
    source.replace(phpManualBreadcrumbsExpr, "");
    ui->helpBrowser->setHtml(source);
}

void MainWindow::helpBrowserAnchorClicked(QUrl url)
{
    QString file = url.toString();
    if (file.indexOf("#") >= 0) file = file.mid(0, file.indexOf("#"));
    if (file.size() == 0) return;
    QString php_manual_path = QString::fromStdString(settings->get("php_manual_path"));
    if (php_manual_path.size() == 0 || !Helper::folderExists(php_manual_path)) return;
    if (!Helper::fileExists(php_manual_path + "/" + file)) return;
    setHelpTabSource(php_manual_path + "/" + file);
}

void MainWindow::messagesBrowserAnchorClicked(QUrl url)
{
    QString lineStr = url.toString();
    int line = lineStr.toInt();
    if (line <= 0) return;
    editorShowLine(line);
}

void MainWindow::scrollMessagesTabToTop()
{
    QTextCursor textCursor = ui->messagesBrowser->textCursor();
    textCursor.movePosition(QTextCursor::Start);
    ui->messagesBrowser->setTextCursor(textCursor);
}

void MainWindow::editorShowLine(int line)
{
    if (line <= 0) return;
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->gotoLine(line);
}

void MainWindow::editorShowLineSymbol(int line, int symbol)
{
    if (line <= 0 || symbol < 0) return;
    Editor * textEditor = editorTabs->getActiveEditor();
    if (textEditor == nullptr) return;
    textEditor->gotoLineSymbol(line, symbol);
}

void MainWindow::setWindowTitleText(QString text)
{
    QString title = APPLICATION_NAME;
    QString projectTitle = project->getName();
    if (projectTitle.size() > 0) title = projectTitle;
    if (text.size() > 0) title += " - " + text;
    setWindowTitle(title + " [*]");
}

void MainWindow::setStatusBarText(QString text)
{
    Editor * editor = editorTabs->getActiveEditor();
    if (editor == nullptr) {
        statusBar()->showMessage("");
        return;
    }
    int blocksCount = editor->document()->blockCount();
    QTextCursor curs = editor->textCursor();
    int blockNumber = curs.block().blockNumber();
    int cursPosInBlock = curs.positionInBlock();
    std::string tabType = editor->getTabType();
    int tabWidth = editor->getTabWidth();
    QString tabMode = QString::fromStdString(tabType).toUpper()+" : "+Helper::intToStr(tabWidth);
    QString newLineMode = QString::fromStdString(editor->getNewLineMode()).toUpper();
    QString encoding = QString::fromStdString(editor->getEncoding()).toUpper();
    QString overwriteMode = editor->isOverwrite() ? "OVERWRITE" : "INSERT";

    QString separator = "    |    ";
    if (text.size()>0) text = separator + text;
    statusBar()->showMessage(
                tr("Line") + ": " + Helper::intToStr(blockNumber+1)+" / "+Helper::intToStr(blocksCount) +
                separator + tr("Column") + ": " + Helper::intToStr(cursPosInBlock+1) +
                separator + tabMode +
                separator + newLineMode +
                separator + encoding +
                separator + overwriteMode +
                text
            );
}

void MainWindow::toolbarOrientationChanged(Qt::Orientation orientation)
{
    if (orientation == Qt::Vertical) {
        ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    }
}

void MainWindow::outputDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::RightDockWidgetArea) {
        ui->outputTabWidget->setTabPosition(QTabWidget::East);
    } else if (area == Qt::LeftDockWidgetArea) {
        ui->outputTabWidget->setTabPosition(QTabWidget::West);
    } else {
        ui->outputTabWidget->setTabPosition(QTabWidget::North);
    }
}

void MainWindow::sidebarDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::RightDockWidgetArea) {
        ui->sidebarTabWidget->setTabPosition(QTabWidget::East);
    } else {
        ui->sidebarTabWidget->setTabPosition(QTabWidget::West);
    }
}

void MainWindow::workerMessage(QString text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
}

void MainWindow::restartApp()
{
    if (QMessageBox::question(this, tr("Restart required"), tr("Some changes will take effect after restart. Restart now ?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok
        && close()
    ) {
        qApp->exit(MainWindow::EXIT_CODE_RESTART);
    }
}

void MainWindow::applyThemeColors()
{
    QString style = "";
    if (theme == THEME_DARK) {
        QFile f(":/styles/dark/style");
        f.open(QIODevice::ReadOnly);
        QTextStream in(&f);
        style += in.readAll() + "\n";
        f.close();
    } else if (theme == THEME_LIGHT) {
        QFile f(":/styles/light/style");
        f.open(QIODevice::ReadOnly);
        QTextStream in(&f);
        style += in.readAll() + "\n";
        f.close();
    }

    if (colorSheme == COLOR_SCHEME_DARK) {
        QFile f(":/styles/dark/scheme");
        f.open(QIODevice::ReadOnly);
        QTextStream in(&f);
        style += in.readAll() + "\n";
        f.close();
    } else {
        QFile f(":/styles/light/scheme");
        f.open(QIODevice::ReadOnly);
        QTextStream in(&f);
        style += in.readAll() + "\n";
        f.close();
    }

    if (style.size() > 0) setStyleSheet(style);
}
