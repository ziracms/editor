/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "editor.h"
#include "linenumber.h"
#include "linemark.h"
#include "linemap.h"
#include "breadcrumbs.h"
#include "completepopup.h"
#include "annotation.h"
#include "search.h"
#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QTextStream>
#include <QMimeData>
#include <QShortcut>
#include <QMessageBox>
#include <QToolTip>
#include <QApplication>
#include <QTextLayout>
#include <QDesktopWidget>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QFileInfo>
#include <QDateTime>
#include <QMenu>
#include <QInputDialog>
#include <QAction>
#include <QScreen>
#include "math.h"
#include "helper.h"

const std::string CRLF = "crlf";
const std::string CR = "cr";
const std::string LF = "lf";

const int INTERVAL_SCROLL_HIGHLIGHT_MILLISECONDS = 500;
const int INTERVAL_TEXT_CHANGED_MILLISECONDS = 200;
const int INTERVAL_CURSOR_POS_CHANGED_MILLISECONDS = 200;
const int INTERVAL_SPELL_CHECK_MILLISECONDS = 500;

const int SPELLCHECKER_INIT_BLOCKS_COUNT = 10;

const int TOOLTIP_OFFSET = 20;
const int TOOLTIP_SCREEN_MARGIN = 10;

const int LINE_NUMBER_WIDGET_PADDING = 20;
const int LINE_MARK_WIDGET_WIDTH = 20;
const int LINE_MARK_WIDGET_RECT_WIDTH = 10;
const int LINE_MARK_WIDGET_LINE_WIDTH = 2;
const int LINE_MAP_WIDGET_WIDTH = 20;
const int SEARCH_WIDGET_HEIGHT = 100;
const int BREADCRUMBS_WIDGET_HEIGHT = 21;

const int LINE_MAP_LINE_NUMBER_OFFSET = 10;
const int LINE_MAP_PROGRESS_WIDTH = 3;
const int LINE_MAP_PROGRESS_HEIGHT = 50;

const QString BREADCRUMBS_DELIMITER = " \u21e2 ";

const QString TOOLTIP_DELIMITER = "[:OR:]";
const QString TOOLTIP_PAGER_TPL = " | <b>[<u>%1</u>/%2]</b>";
const QString TOOLTIP_COLOR_TPL = "<span style=\"background:%1;\">&nbsp;&nbsp;&nbsp;</span>";

const int SEARCH_LIMIT = 10000;
const int BIG_FILE_SIZE = 512000;

const int LONG_LINE_CHARS_COUNT = 72;
const int FIRST_BLOCK_BIN_SEARCH_SCROLL_VALUE = 300;

const QString SNIPPET_PREFIX = "Snippet: @";

Editor::Editor(SpellCheckerInterface * spellChecker, Settings * settings, HighlightWords * highlightWords, CompleteWords * completeWords, HelpWords * helpWords, SpellWords * spellWords, QWidget * parent) : QTextEdit(parent), spellChecker(spellChecker), tooltipLabel(settings)
{
    setMinimumSize(0, 0);
    setMaximumSize(16777215, 16777215);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptRichText(false);
    setAcceptDrops(false);

    //document()->setDocumentMargin(0);

    wrapLines = false;
    std::string wrapLinesStr = settings->get("editor_wrap_long_lines");
    if (wrapLinesStr == "yes") wrapLines = true;
    if (wrapLines) {
        setLineWrapMode(LineWrapMode::FixedColumnWidth);
        setLineWrapColumnOrWidth(LONG_LINE_CHARS_COUNT);
        setWordWrapMode(QTextOption::WrapMode::WordWrap);
    } else {
        setLineWrapMode(LineWrapMode::NoWrap);
        setWordWrapMode(QTextOption::WrapMode::NoWrap);
    }

    QString theme = QString::fromStdString(settings->get("theme"));

    //QFont generalFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    QFont generalFont = QApplication::font();

    // editor font
    std::string fontFamily = settings->get("editor_font_family");
    std::string fontSize = settings->get("editor_font_size");
    std::string popupFontSize = settings->get("editor_popup_font_size");
    std::string tooltipFontSize = settings->get("editor_tooltip_font_size");
    std::string breadcrumbsFontSize = settings->get("editor_breadcrumbs_font_size");

    if (fontFamily=="") {
        QFont sysFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        editorFont.setFamily(sysFont.family());
        editorFont.setStyleHint(QFont::Monospace);
        editorPopupFont.setFamily(sysFont.family());
        editorPopupFont.setStyleHint(QFont::Monospace);
        editorTooltipFont.setFamily(sysFont.family());
        editorTooltipFont.setStyleHint(QFont::Monospace);
        editorBreadcrumbsFont.setFamily(sysFont.family());
        editorBreadcrumbsFont.setStyleHint(QFont::Monospace);
    } else {
        editorFont.setStyleHint(QFont::Monospace);
        editorFont.setFamily(QString::fromStdString(fontFamily));
        editorPopupFont.setStyleHint(QFont::Monospace);
        editorPopupFont.setFamily(QString::fromStdString(fontFamily));
        editorTooltipFont.setStyleHint(QFont::Monospace);
        editorTooltipFont.setFamily(QString::fromStdString(fontFamily));
        editorBreadcrumbsFont.setStyleHint(QFont::Monospace);
        editorBreadcrumbsFont.setFamily(QString::fromStdString(fontFamily));
    }
    editorFont.setPointSize(std::stoi(fontSize));
    editorPopupFont.setPointSize(std::stoi(popupFontSize));
    editorTooltipFont.setPointSize(std::stoi(tooltipFontSize));
    editorBreadcrumbsFont.setPointSize(std::stoi(breadcrumbsFontSize));

    // bold text issue workaround
    editorFont.setStyleName("");
    editorPopupFont.setStyleName("");
    editorTooltipFont.setStyleName("");
    editorBreadcrumbsFont.setStyleName("");

    setFont(editorFont);

    // updating font family if using fallback font
    QFontInfo fontInfo(editorFont);
    if (fontInfo.family() != editorFont.family()) {
        std::unordered_map<std::string, std::string> sData;
        sData["editor_font_family"] = fontInfo.family().toStdString();
        settings->change(sData);
        // showing message in initMode after slots connection
    }

    // colors
    std::string lineNumberBgColorStr = settings->get("editor_line_number_bg_color");
    std::string lineNumberColorStr = settings->get("editor_line_number_color");
    std::string lineNumberModifiedBgColorStr = settings->get("editor_line_number_modified_bg_color");
    std::string lineNumberModifiedColorStr = settings->get("editor_line_number_modified_color");
    std::string lineNumberDeletedBorderColorStr = settings->get("editor_line_number_deleted_border_color");
    std::string lineMarkBgColorStr = settings->get("editor_line_mark_bg_color");
    std::string lineMapBgColorStr = settings->get("editor_line_map_bg_color");
    std::string lineMapScrollBgColorStr = settings->get("editor_line_map_scroll_bg_color");
    std::string lineMapScrollAreaBgColorStr = settings->get("editor_line_map_scroll_area_bg_color");
    std::string searchBgColorStr = settings->get("editor_search_bg_color");
    std::string breadcrumbsBgColorStr = settings->get("editor_breadcrumbs_bg_color");
    std::string breadcrumbsWarningBgColorStr = settings->get("editor_breadcrumbs_warning_bg_color");
    std::string breadcrumbsErrorBgColorStr = settings->get("editor_breadcrumbs_error_bg_color");
    std::string breadcrumbsColorStr = settings->get("editor_breadcrumbs_color");
    std::string widgetBorderColorStr = settings->get("editor_widget_border_color");
    std::string selectedLineBgColorStr = settings->get("editor_selected_line_bg_color");
    std::string selectedWordBgColorStr = settings->get("editor_selected_word_bg_color");
    std::string selectedCharBgColorStr = settings->get("editor_selected_char_bg_color");
    std::string selectedCharColorStr = settings->get("editor_selected_char_color");
    std::string selectedTagBgColorStr = settings->get("editor_selected_tag_bg_color");
    std::string selectedExpressionBgColorStr = settings->get("editor_selected_expression_bg_color");
    std::string searchWordBgColorStr = settings->get("editor_search_word_bg_color");
    std::string searchWordColorStr = settings->get("editor_search_word_color");
    std::string searchInputBgColorStr = settings->get("editor_search_input_bg_color");
    std::string searchInputErrorBgColorStr = settings->get("editor_search_input_error_bg_color");
    std::string lineMarkColorStr = settings->get("editor_line_mark_color");
    std::string lineErrorColorStr = settings->get("editor_line_error_color");
    std::string lineWarningColorStr = settings->get("editor_line_warning_color");
    std::string lineMarkRectColorStr = settings->get("editor_line_mark_rect_color");
    std::string lineErrorRectColorStr = settings->get("editor_line_error_rect_color");
    std::string lineWarningRectColorStr = settings->get("editor_line_warning_rect_color");
    std::string textColorStr = settings->get("editor_text_color");
    std::string bgColorStr = settings->get("editor_bg_color");
    std::string progressColorStr = settings->get("progress_color");

    lineNumberBgColor = QColor(lineNumberBgColorStr.c_str());
    lineNumberColor = QColor(lineNumberColorStr.c_str());
    lineNumberModifiedBgColor = QColor(lineNumberModifiedBgColorStr.c_str());
    lineNumberModifiedColor = QColor(lineNumberModifiedColorStr.c_str());
    lineNumberDeletedBorderColor = QColor(lineNumberDeletedBorderColorStr.c_str());
    lineMarkBgColor = QColor(lineMarkBgColorStr.c_str());
    lineMapBgColor = QColor(lineMapBgColorStr.c_str());
    lineMapScrollBgColor = QColor(lineMapScrollBgColorStr.c_str());
    lineMapScrollAreaBgColor = QColor(lineMapScrollAreaBgColorStr.c_str());
    searchBgColor = QColor(searchBgColorStr.c_str());
    breadcrumbsBgColor = QColor(breadcrumbsBgColorStr.c_str());
    breadcrumbsWarningBgColor = QColor(breadcrumbsWarningBgColorStr.c_str());
    breadcrumbsErrorBgColor = QColor(breadcrumbsErrorBgColorStr.c_str());
    breadcrumbsColor = QColor(breadcrumbsColorStr.c_str());
    widgetBorderColor = QColor(widgetBorderColorStr.c_str());
    selectedLineBgColor = QColor(selectedLineBgColorStr.c_str());
    selectedWordBgColor = QColor(selectedWordBgColorStr.c_str());
    selectedCharBgColor = QColor(selectedCharBgColorStr.c_str());
    selectedCharColor = QColor(selectedCharColorStr.c_str());
    selectedTagBgColor = QColor(selectedTagBgColorStr.c_str());
    selectedExpressionBgColor = QColor(selectedExpressionBgColorStr.c_str());
    searchWordBgColor = QColor(searchWordBgColorStr.c_str());
    searchWordColor = QColor(searchWordColorStr.c_str());
    searchInputBgColor = QColor(searchInputBgColorStr.c_str());
    searchInputErrorBgColor = QColor(searchInputErrorBgColorStr.c_str());
    lineMarkColor = QColor(lineMarkColorStr.c_str());
    lineErrorColor = QColor(lineErrorColorStr.c_str());
    lineWarningColor = QColor(lineWarningColorStr.c_str());
    lineMarkRectColor = QColor(lineMarkRectColorStr.c_str());
    lineErrorRectColor = QColor(lineErrorRectColorStr.c_str());
    lineWarningRectColor = QColor(lineWarningRectColorStr.c_str());
    textColor = QColor(textColorStr.c_str());
    bgColor = QColor(bgColorStr.c_str());
    progressColor = QColor(progressColorStr.c_str());

    QPalette p;
    p.setColor(QPalette::Base, bgColor);
    p.setColor(QPalette::Text, textColor);
    setPalette(p);

    // tab width
    tabWidthStr = settings->get("editor_tab_width");
    tabTypeStr = settings->get("editor_tab_type");
    detectTabTypeStr = settings->get("editor_tab_type_detect");
    setTabsSettings();

    // new lines
    std::string newLineModeStr = settings->get("editor_new_line_mode");
    if (newLineModeStr != CRLF && newLineModeStr != CR && newLineModeStr != LF) {
        newLineMode = LF;
    } else {
        newLineMode = newLineModeStr;
    }

    // encoding
    encoding = settings->get("editor_encoding");
    encodingFallback = settings->get("editor_fallback_encoding");

    // regexps
    tagExpr = QRegularExpression("<(?:[/])?[a-zA-Z][^<>]*>");
    tagOpenExpr = QRegularExpression("<([a-zA-Z][a-zA-Z0-9]*)[^<>]*>");
    phpExpr = QRegularExpression("<[?].+?[?]>");
    strExpr = QRegularExpression("([\"']).*?(\\1)");
    functionExpr = QRegularExpression("\\bfunction\\b[\\s]+(?:[&][\\s]*)?([a-zA-Z_][a-zA-Z0-9_]*)[\\s]*[(]([^{]*)[)](?:[\\s]*[:][\\s]*((?:[\\?][\\s]*)?[a-zA-Z_][a-zA-Z0-9_]*))?");
    tagExprIf = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?if[\\s]*[(].*?[)][\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprElse = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?else[\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprEndif = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?endif[\\s]*[;]*[\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprSwitch = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?switch[\\s]*[(].*?[)][\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprCase = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?case[\\s]*[a-zA-Z0-9_\"']+[\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprEndswitch = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?endswitch[\\s]*[;]*[\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprWhile = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?while[\\s]*[(].*?[)][\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprEndwhile = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?endwhile[\\s]*[;]*[\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprFor = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?for[\\s]*[(].*?[)][\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprEndfor = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?endfor[\\s]*[;]*[\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprForeach = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?foreach[\\s]*[(].*?[)][\\s]*[:][\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    tagExprEndforeach = QRegularExpression("<\\?(?:php)?[\\s]+(?:[/][*].*?[*][/][\\s]*)?endforeach[\\s]*[;]*[\\s]*(?:[/][*].*?[*][/][\\s]*)?(?:[/][/].*?)?\\?>");
    stripTagsExpr = QRegularExpression("<[a-zA-Z/][^<>]*?>");
    functionNameExpr = QRegularExpression("\\b([a-zA-Z0-9_]+)\\b[\\s]*[(]");
    functionParamsExpr = QRegularExpression("^([^(]+)[(](.*)[)](.*)$");
    functionWordExpr = QRegularExpression("(?:^|[^a-zA-Z0-9_\\$]+)function(?:[^a-zA-Z0-9_]+|$)");
    classNameExpr = QRegularExpression("([a-zA-Z0-9_\\\\]+)[\\s]*[(]");
    colorExpr = QRegularExpression("^[#](?:[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9])(?:[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9])?(?:[a-fA-F0-9][a-fA-F0-9])?$");
    spellWordExpr = QRegularExpression("([\\p{L}0-9_'\\$\\-]+)");

    // some features is enabled only in experimental mode
    experimentalMode = false;
    std::string experimentalModeStr = settings->get("experimental_mode_enabled");
    if (experimentalModeStr == "yes") experimentalMode = true;

    // highlighter
    highlight = new Highlight(settings, highlightWords, document());
    std::string unusedVariableColorStr = settings->get("highlight_unused_variable_color");
    unusedVariableColor = QColor(unusedVariableColorStr.c_str());
    connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(contentsChange(int,int,int)));
    connect(highlight, SIGNAL(progressChanged(int)), this, SLOT(highlightProgressChanged(int)));

    // update area slots
    connect(this->document(), SIGNAL(blockCountChanged(int)), this, SLOT(blockCountChanged(int)));
    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(verticalScrollbarValueChanged(int)));
    connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(horizontalScrollbarValueChanged(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

    // shortcuts
    QString shortcutBackTabStr = QString::fromStdString(settings->get("shortcut_backtab"));
    QShortcut * shortcutShiftTab = new QShortcut(QKeySequence(shortcutBackTabStr), this);
    shortcutShiftTab->setContext(Qt::WidgetShortcut);
    connect(shortcutShiftTab, SIGNAL(activated()), this, SLOT(backtab()));

    QString shortcutSaveStr = QString::fromStdString(settings->get("shortcut_save"));
    QShortcut * shortcutSave = new QShortcut(QKeySequence(shortcutSaveStr), this);
    shortcutSave->setContext(Qt::WidgetShortcut);
    connect(shortcutSave, SIGNAL(activated()), this, SLOT(save()));

    QString shortcutCommentStr = QString::fromStdString(settings->get("shortcut_comment"));
    QShortcut * shortcutComment = new QShortcut(QKeySequence(shortcutCommentStr), this);
    shortcutComment->setContext(Qt::WidgetShortcut);
    connect(shortcutComment, SIGNAL(activated()), this, SLOT(comment()));

    QString shortcutOverwriteModeStr = QString::fromStdString(settings->get("shortcut_overwrite_mode"));
    QShortcut * shortcutOverwriteMode = new QShortcut(QKeySequence(shortcutOverwriteModeStr), this);
    shortcutOverwriteMode->setContext(Qt::WidgetShortcut);
    connect(shortcutOverwriteMode, SIGNAL(activated()), this, SLOT(switchOverwrite()));

    QString shortcutTooltipStr = QString::fromStdString(settings->get("shortcut_tooltip"));
    QShortcut * shortcutTooltip = new QShortcut(QKeySequence(shortcutTooltipStr), this);
    shortcutTooltip->setContext(Qt::WidgetShortcut);
    connect(shortcutTooltip, SIGNAL(activated()), this, SLOT(tooltip()));

    QString shortcutSearchStr = QString::fromStdString(settings->get("shortcut_search"));
    QShortcut * shortcutSearch = new QShortcut(QKeySequence(shortcutSearchStr), this);
    shortcutSearch->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutSearch, SIGNAL(activated()), this, SLOT(findToggle()));

    QString shortcutHelpStr = QString::fromStdString(settings->get("shortcut_help"));
    QShortcut * shortcutHelp = new QShortcut(QKeySequence(shortcutHelpStr), this);
    shortcutHelp->setContext(Qt::WidgetShortcut);
    connect(shortcutHelp, SIGNAL(activated()), this, SLOT(showHelpRequested()));

    QString shortcutDuplicateStr = QString::fromStdString(settings->get("shortcut_duplicate_line"));
    QShortcut * shortcutDuplicate = new QShortcut(QKeySequence(shortcutDuplicateStr), this);
    shortcutDuplicate->setContext(Qt::WidgetShortcut);
    connect(shortcutDuplicate, SIGNAL(activated()), this, SLOT(duplicateLine()));

    QString shortcutDeleteStr = QString::fromStdString(settings->get("shortcut_delete_line"));
    QShortcut * shortcutDelete = new QShortcut(QKeySequence(shortcutDeleteStr), this);
    shortcutDelete->setContext(Qt::WidgetShortcut);
    connect(shortcutDelete, SIGNAL(activated()), this, SLOT(deleteLine()));

    QString shortcutContextMenuStr = QString::fromStdString(settings->get("shortcut_context_menu"));
    QShortcut * shortcutContextMenu = new QShortcut(QKeySequence(shortcutContextMenuStr), this);
    shortcutContextMenu->setContext(Qt::WidgetShortcut);
    connect(shortcutContextMenu, SIGNAL(activated()), this, SLOT(contextMenu()));

    // order matters
    // annotation
    lineAnnotation = new Annotation(this, settings);
    // line number area
    lineNumber = new LineNumber(this);
    // line mark area
    lineMark = new LineMark(this);
    // line map area
    lineMap = new LineMap(this);

    // breadcrumbs
    breadcrumbs = new Breadcrumbs(this);
    breadcrumbs->setFont(editorBreadcrumbsFont);
    showBreadcrumbs = false;
    std::string showBreadcrumbsStr = settings->get("editor_breadcrumbs_enabled");
    if (showBreadcrumbsStr == "yes") showBreadcrumbs = true;

    qaBtn = new QToolButton(breadcrumbs);
    qaBtn->setIcon(QIcon(":/icons/separator-double.png"));
    //qaBtn->setIconSize(QSize(breadcrumbs->height(), breadcrumbs->height()));
    qaBtn->setToolTip(tr("Quick Access"));
    qaBtn->setProperty("BreadcrumbsButton", "QuickAccess");
    if (theme == THEME_SYSTEM || theme.indexOf(STYLE_PLUGIN_DISPLAY_NAME_SUFFIX) > 0) {
        qaBtn->setStyleSheet("border:none");
        qaBtn->setCursor(Qt::PointingHandCursor);
    }
    connect(qaBtn, SIGNAL(pressed()), this, SLOT(qaBtnClicked()));

    // complete popup
    completePopup = new CompletePopup(this);
    completePopup->setFont(editorPopupFont);
    connect(completePopup, SIGNAL(itemDataClicked(QString, QString)), this, SLOT(completePopupSelected(QString, QString)));

    // search widget
    search = new Search(this);
    search->setFont(generalFont);
    QPalette searchPallete = search->palette();
    searchPallete.setColor(QPalette::Foreground, textColor);
    searchPallete.setColor(QPalette::Base, searchBgColor.lighter(110));
    searchPallete.setColor(QPalette::ButtonText, textColor);
    searchPallete.setColor(QPalette::Button, searchBgColor.lighter(110));
    search->setPalette(searchPallete);
    searchCaSe = false;
    searchWord = false;
    searchRegE = false;
    searchString = "";

    extension = "";
    modeOnKeyPress = "";
    lastKeyPressed = -1;
    lastKeyPressedBlockNumber = -1;
    tooltipSavedText = "";
    tooltipSavedList.clear();
    tooltipSavedPageOffset = -1;
    tooltipSavedOrigName = "";
    tooltipSavedBlockNumber = -1;

    focused = false;
    cursorPositionChangeLocked = false;
    scrollBarValueChangeLocked = false;
    overwrite = false;
    tabIndex = -1;
    parseLocked = false;
    isUndoAvailable = false;
    isRedoAvailable = false;
    lastCursorPositionBlockNumber = -1;
    isBigFile = false;

    connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(onUndoAvailable(bool)));
    connect(this, SIGNAL(redoAvailable(bool)), this, SLOT(onRedoAvailable(bool)));

    // tooltip label
    tooltipLabel.setFont(editorTooltipFont);

    tooltipBoldColorStr = QString::fromStdString(settings->get("editor_tooltip_bold_color"));
    tooltipBoldTagStart = "<b style=\"color:"+tooltipBoldColorStr+"\">";
    tooltipBoldTagEnd = "</b>";

    CW = completeWords;
    HW = highlightWords;
    HPW = helpWords;
    SW = spellWords;

    parsePHPEnabled = false;
    std::string parsePHPEnabledStr = settings->get("parser_enable_parse_php");
    if (parsePHPEnabledStr == "yes") parsePHPEnabled = true;

    parseJSEnabled = false;
    std::string parseJSEnabledStr = settings->get("parser_enable_parse_js");
    if (parseJSEnabledStr == "yes") parseJSEnabled = true;

    parseCSSEnabled = false;
    std::string parseCSSEnabledStr = settings->get("parser_enable_parse_css");
    if (parseCSSEnabledStr == "yes") parseCSSEnabled = true;

    cleanBeforeSave = false;
    std::string cleanBeforeSaveStr = settings->get("editor_clean_before_save");
    if (cleanBeforeSaveStr == "yes") cleanBeforeSave = true;

    annotationsEnabled = false;
    std::string annotationsEnabledStr = settings->get("editor_show_annotations");
    if (annotationsEnabledStr == "yes") annotationsEnabled = true;

    int parseResultChangedDelayMS = std::stoi(settings->get("editor_parse_interval"));
    if (parseResultChangedDelayMS >= 1000) parseResultChangedDelay = parseResultChangedDelayMS;
    else parseResultChangedDelay = 5000;

    spellCheckerEnabled = false;
    std::string spellCheckerEnabledStr = settings->get("spellchecker_enabled");
    if (spellCheckerEnabledStr == "yes") spellCheckerEnabled = true;

    drawLongLineMarker = false;
    std::string drawLongLineMarkerStr = settings->get("editor_long_line_marker_enabled");
    if (drawLongLineMarkerStr == "yes") drawLongLineMarker = true;

    // snippets
    QString snippets = QString::fromStdString(settings->get("snippets")).trimmed();
    QString customSnippetsFile = QString::fromStdString(settings->get("custom_snippets_file"));
    if (Helper::fileExists(customSnippetsFile)) {
        QString customSnippets = Helper::loadFile(customSnippetsFile, getEncoding(), getFallbackEncoding()).trimmed();
        if (customSnippets.size() > 0) snippets += "\n"+customSnippets;
    }
    if (snippets.size() > 0) {
        QRegularExpression snippetsExpr("[[][{]([a-zA-Z]+)[:][@]([a-zA-Z0-9]{2,})[}][]][\\s]*[=][\\s]*[[][[]([^]]+)[]][]]");
        QRegularExpressionMatch snippetMatch;
        QStringList snippetsList = snippets.split("\n");
        for (QString snippet : snippetsList) {
            if (snippet.indexOf("//")==0) continue;
            snippetMatch = snippetsExpr.match(snippet);
            if (snippetMatch.capturedStart() >= 0) {
                std::string mode = snippetMatch.captured(1).toLower().toStdString();
                QString snippetCode = snippetMatch.captured(2).trimmed();
                QString snippetText = snippetMatch.captured(3).trimmed();
                if (snippetCode.size() == 0 || snippetText.size() == 0) continue;
                if (mode == MODE_PHP) {
                    phpSnippets[snippetCode] = snippetText;
                } else if (mode == MODE_JS) {
                    jsSnippets[snippetCode] = snippetText;
                } else if (mode == MODE_CSS) {
                    cssSnippets[snippetCode] = snippetText;
                } else if (mode == MODE_HTML) {
                    htmlSnippets[snippetCode] = snippetText;
                }
            }
        }
    }

    // cursor is not set to default sometimes
    horizontalScrollBar()->setCursor(Qt::ArrowCursor);
    verticalScrollBar()->setCursor(Qt::ArrowCursor);

    // for Android
    inputEventKey = -1;
    setInputMethodHints(Qt::ImhNoPredictiveText | Qt::ImhMultiLine);
}

Editor::~Editor()
{
    delete highlight;
}

void Editor::init()
{
    setMouseTracking(true);
    updateViewportMargins();
}

void Editor::setTabIndex(int index)
{
    tabIndex = index;
}

int Editor::getTabIndex()
{
    return tabIndex;
}

void Editor::setTabsSettings()
{
    tabWidth = std::stoi(tabWidthStr);
    QFontMetrics fm(editorFont);
    /* QTextEdit::setTabStopWidth() is deprecated */
    /*
    setTabStopWidth(tabWidth * fm.width(' '));
    */
    /* QFontMetrics::width is deprecated */
    /*
    setTabStopDistance(tabWidth * fm.width(' '));
    */
    setTabStopDistance(tabWidth * fm.horizontalAdvance(" "));
    tabType = tabTypeStr;
    if (detectTabTypeStr == "yes") detectTabType = true;
    else detectTabType = false;
}

void Editor::reset()
{
    setReadOnly(true);
    highlight->resetMode();
    fileName = "";
    extension = "";
    highlighterInitialized = false;
    is_ready = false;
    cursorPositionChangeLocked = false;
    scrollBarValueChangeLocked = false;
    textChangeLocked = false;
    modeOnKeyPress = "";
    lastKeyPressed = -1;
    lastKeyPressedBlockNumber = -1;
    tooltipSavedText = "";
    tooltipSavedList.clear();
    tooltipSavedPageOffset = -1;
    tooltipSavedOrigName = "";
    tooltipSavedBlockNumber = -1;
    setTabsSettings();
    hidePopups();
    static_cast<LineMap *>(lineMap)->clear();
    static_cast<LineMark *>(lineMark)->clear();
    markPoints.clear();
    modifiedLines.clear();
    modified = false;
    lastModifiedMsec = 0;
    warningDisplayed = false;
    parseLocked = false;
    isUndoAvailable = false;
    isRedoAvailable = false;
    backPositions.clear();
    forwardPositions.clear();
    lastCursorPositionBlockNumber = -1;
    isParseError = false;
    gitAnnotationLastLineNumber = -1;
    gitAnnotations.clear();
    gitDiffLines.clear();
    spellLocked = false;
    spellBlocksQueue.clear();
    spellPastedBlocksQueue.clear();
    errorsExtraSelections.clear();
    spellCheckInitBlockNumber = 0;
    highlightProgressPercent = 0;
    spellProgressPercent = 0;
    firstVisibleBlockIndex = -1;
    lastVisibleBlockIndex = -1;
}

void Editor::highlightProgressChanged(int percent)
{
    highlightProgressPercent = percent;
    if (highlightProgressPercent > 100) highlightProgressPercent = 100;
    if (highlightProgressPercent < 0) highlightProgressPercent = 0;
    lineMap->repaint();
}

void Editor::spellProgressChanged(int percent)
{
    spellProgressPercent = percent;
    if (spellProgressPercent > 100) spellProgressPercent = 100;
    if (spellProgressPercent < 0) spellProgressPercent = 0;
    lineMap->update();
}

void Editor::hidePopups()
{
    hideCompletePopup();
    hideTooltip();
}

void Editor::setFileName(QString name)
{
    fileName = name;
    QFileInfo fInfo(fileName);
    QDateTime dtModified = fInfo.lastModified();
    lastModifiedMsec = dtModified.time().msec();
}

QString Editor::getFileName()
{
    return fileName;
}

bool Editor::isModified()
{
    return modified;
}

void Editor::setModified(bool m)
{
    modified = m;
    emit modifiedStateChanged(tabIndex, modified);
}

void Editor::setParseError(bool error)
{
    isParseError = error;
}

bool Editor::getParseError()
{
    return isParseError;
}

QString Editor::getFileExtension()
{
    return extension;
}

void Editor::setIsBigFile(bool isBig)
{
    isBigFile = isBig;
}

void Editor::initMode(QString ext)
{
    extension = ext;
    modified = false;
    document()->setModified(modified);
    emit modifiedStateChanged(tabIndex, modified);
    updateWidgetsGeometry();

    // check font
    QFontInfo fontInfo(editorFont);
    if (fontInfo.family() != editorFont.family()) {
        //Helper::showMessage(QObject::tr("Font \"%1\" was not loaded. Using \"%2\" font family.").arg(editorFont.family()).arg(fontInfo.family()));
        emit showPopupText(tabIndex, QObject::tr("Font \"%1\" was not loaded. Using \"%2\" font family.").arg(editorFont.family()).arg(fontInfo.family()));
    }
}

void Editor::initHighlighter()
{
    if (highlighterInitialized) return;
    highlighterInitialized = true;

    highlight->setIsBigFile(isBigFile);
    highlight->initMode(extension, getLastVisibleBlockIndex());

    bool isFocused = hasFocus();
    int line = getCursorLine();
    highlight->setFirstRunMode(true);
    highlight->rehighlight();
    highlight->setFirstRunMode(false);
    if (line > 1) gotoLine(line, false);
    if (isFocused) setFocus();

    setReadOnly(false);
    emit statusBarText(tabIndex, "");
    is_ready = true;
    cursorPositionChangedDelayed();
    emit ready(tabIndex);
    if (highlight->getModeType() == MODE_MIXED) highlightUnusedVars(false);
    initSpellChecker();
}

void Editor::initSpellChecker()
{
    if (!spellCheckerEnabled || spellChecker == nullptr || isBigFile) return;
    int totalBlocks = document()->blockCount();
    while(spellCheckInitBlockNumber < totalBlocks) {
        for (int i=0; i<SPELLCHECKER_INIT_BLOCKS_COUNT; i++) {
            spellBlocksQueue.append(i+spellCheckInitBlockNumber);
        }
        spellCheckInitBlockNumber += SPELLCHECKER_INIT_BLOCKS_COUNT;
        spellCheck(false, false);
        int percent = (spellCheckInitBlockNumber * 100) / totalBlocks;
        if (percent - spellProgressPercent > 10) spellProgressChanged(percent);
        QCoreApplication::processEvents();
        if (tabIndex < 0) break;
    }
    spellProgressChanged(100);
}

void Editor::spellCheckPasted()
{
    if (!spellCheckerEnabled || spellChecker == nullptr || isBigFile) return;
    if (tabIndex < 0) return;
    if (spellPastedBlocksQueue.size() == 0) return;
    spellBlocksQueue.append(spellPastedBlocksQueue.last());
    spellCheck(false, false);
    spellPastedBlocksQueue.removeLast();
    if (spellPastedBlocksQueue.size() > 0) {
        QTimer::singleShot(INTERVAL_SPELL_CHECK_MILLISECONDS, this, SLOT(spellCheckPasted()));
    }
}

std::string Editor::getModeType()
{
    return highlight->getModeType();
}

bool Editor::isReady()
{
    return is_ready;
}

void Editor::paintEvent(QPaintEvent *e)
{
    if (highlight->isDirty()) return;

    if (drawLongLineMarker) {
        QFontMetrics fm(font());
        /* QFontMetrics::width is deprecated */
        /*
        int longMarkerX = fm.width(QString("w").repeated(LONG_LINE_CHARS_COUNT));
        */
        int longMarkerX = fm.horizontalAdvance(QString("w").repeated(LONG_LINE_CHARS_COUNT));
        int scrollX = horizontalScrollBar()->value();

        QPainter painter(viewport());
        QPen pen(widgetBorderColor);
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        painter.drawLine(longMarkerX-scrollX, 0, longMarkerX-scrollX, viewport()->geometry().height());
    }

    QTextEdit::paintEvent(e);
}

void Editor::setParseResult(ParsePHP::ParseResult result)
{
    parseResultPHP = result;
    parseLocked = false;
}

void Editor::setParseResult(ParseJS::ParseResult result)
{
    parseResultJS = result;
    parseLocked = false;
}

void Editor::setParseResult(ParseCSS::ParseResult result)
{
    parseResultCSS = result;
    parseLocked = false;
}

void Editor::setGitAnnotations(QHash<int, Git::Annotation> annotations)
{
    if (warningDisplayed) return;
    gitAnnotations = annotations;
    gitAnnotationLastLineNumber = -1;
    showLineAnnotation();
}

void Editor::setGitDiffLines(QHash<int, Git::DiffLine> mLines)
{
    if (warningDisplayed) return;
    gitDiffLines = mLines;
    updateLineWidgetsArea();
}

int Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    QFontMetrics fm(editorFont);
    /* QFontMetrics::width is deprecated */
    /*
    int w = fm.width("0");
    */
    int w = fm.horizontalAdvance("0");
    int space = LINE_NUMBER_WIDGET_PADDING + w * digits;
    return space;
}

int Editor::lineMarkAreaWidth()
{
    return LINE_MARK_WIDGET_WIDTH;
}

int Editor::lineMapAreaWidth()
{
    return LINE_MAP_WIDGET_WIDTH;
}

int Editor::searchWidgetHeight()
{
    return SEARCH_WIDGET_HEIGHT;
}

int Editor::breadcrumbsHeight()
{
    if (!showBreadcrumbs) return 0;
    if (editorBreadcrumbsFont.pointSize() > 10) {
        return editorBreadcrumbsFont.pointSize() + 11;
    }
    return BREADCRUMBS_WIDGET_HEIGHT;
}

void Editor::updateViewportMargins()
{
    int lineW = lineNumberAreaWidth();
    int markW = lineMarkAreaWidth();
    int mapW = lineMapAreaWidth();
    int searchH = searchWidgetHeight();
    if (!search->isVisible()) searchH = 0;
    int breadcrumbsH = breadcrumbsHeight();
    setViewportMargins(lineW + markW, breadcrumbsH, mapW, searchH);
}

void Editor::updateLineWidgetsArea()
{
      lineNumber->update();
      lineMark->update();
}

void Editor::updateLineAnnotationView()
{
    if (!annotationsEnabled) return;
    if (static_cast<Annotation *>(lineAnnotation)->getText().size() == 0) return;

    int blockNumber = getFirstVisibleBlockIndex();
    if (blockNumber < 0) return;
    if (blockNumber>0) blockNumber--;

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    int top = contentsMargins().top();

    if (blockNumber == 0) {
        top += static_cast<int>(document()->documentMargin()) - verticalScrollBar()->sliderPosition();
    } else {
        QTextBlock prev_block = document()->findBlockByNumber(blockNumber-1);
        int prev_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).y());
        int prev_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).height());
        top += prev_y + prev_h - verticalScrollBar()->sliderPosition();
    }

    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    while (block.isValid()) {
        if (block.isVisible() && block.blockNumber() == textCursor().block().blockNumber()) {
            int y = top;
            if (breadcrumbs->isVisible()) y += breadcrumbs->geometry().height();
            int x = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).width());
            int h = bottom - top;
            if (wrapLines && block.layout() != nullptr) {
                x = static_cast<int>(block.layout()->lineAt(block.layout()->lineCount()-1).naturalTextWidth());
                if (block.layout()->lineCount() > 1) {
                    y += h;
                    h = static_cast<int>(block.layout()->lineAt(block.layout()->lineCount()-1).height());
                    y -= h;
                }
            }
            x += lineNumber->geometry().width() + lineMark->geometry().width();
            x += ANNOTATION_LEFT_MARGIN;
            QFontMetrics fm(font());
            /* QFontMetrics::width is deprecated */
            /*
            int tw = fm.width(static_cast<Annotation *>(lineAnnotation)->getText());
            */
            int tw = fm.horizontalAdvance(static_cast<Annotation *>(lineAnnotation)->getText());
            tw += h; // icon
            int bw = geometry().width() - lineMap->geometry().width();
            bw -= ANNOTATION_RIGHT_MARGIN;
            if (x + tw < bw) x += bw - x - tw;
            if (horizontalScrollBar()->isVisible()) x -= horizontalScrollBar()->value();
            lineAnnotation->move(x, y);
            static_cast<Annotation *>(lineAnnotation)->setSize(tw, h);
            if (!lineAnnotation->isVisible()) static_cast<Annotation *>(lineAnnotation)->fadeIn();
            return;
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        blockNumber++;
    }
    lineAnnotation->hide();
}

void Editor::showLineAnnotation()
{
    if (!annotationsEnabled || gitAnnotations.size() == 0) return;
    QTextBlock block = textCursor().block();
    int line = block.blockNumber() + 1;
    if (gitAnnotationLastLineNumber == line) {
        static_cast<Annotation *>(lineAnnotation)->setText("");
        if (lineAnnotation->isVisible()) lineAnnotation->hide();
        return;
    }
    QString annotationText = "";
    int error = static_cast<LineMark *>(lineMark)->getError(line, annotationText);
    if (error > 0 && annotationText.size() > 0) {
        annotationText = annotationText.replace("\t", QString(" ").repeated(tabWidth));
    } else if (gitAnnotations.contains(line)) {
        Git::Annotation annotation = gitAnnotations.value(line);
        annotationText = tr("git") + ": " + annotation.comment + " / " + annotation.committer + " [" + annotation.committerDate + "]";
    }
    if (annotationText.size() > 0) {
        static_cast<Annotation *>(lineAnnotation)->setText(annotationText);
        updateLineAnnotationView();
        gitAnnotationLastLineNumber = line;
    }
}

std::string Editor::getTabType()
{
    return tabType;
}

int Editor::getTabWidth()
{
    return tabWidth;
}

std::string Editor::getNewLineMode()
{
    return newLineMode;
}

std::string Editor::getEncoding()
{
    return encoding;
}

std::string Editor::getFallbackEncoding()
{
    return encodingFallback;
}
bool Editor::isOverwrite()
{
    return overwrite;
}

void Editor::detectTabsMode()
{
    if (!detectTabType) return;
    QTextCursor curs = textCursor();
    curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    int indentPos = -1;
    do {
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int total = blockText.size();
        curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        int pos = curs.positionInBlock();
        if (pos > 0) {
            QChar lastChar = blockText[pos-1];
            if (lastChar == "{" || indentPos>=0) {
                curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                if (indentPos>0 && !curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, indentPos)) {
                    indentPos = -1;
                    continue;
                }
                pos = curs.positionInBlock();
                QString prefix = "";
                bool isSpace = false;
                while(pos < total) {
                    if (!curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) break;
                    pos = curs.positionInBlock();
                    QChar prevChar = blockText[pos-1];
                    if (prevChar == "\t" && prefix.size()==0) {
                        isSpace = false;
                        prefix = prevChar;
                        break;
                    } else if (prevChar == " ") {
                        prefix += prevChar;
                        isSpace = true;
                    } else {
                        break;
                    }
                }
                if (indentPos<0) {
                    indentPos = prefix.size();
                } else if (prefix.size()>0) {
                    if (!isSpace) {
                        tabType = "tabs";
                    } else {
                        tabType = "spaces";
                        tabWidth = prefix.size();
                    }
                    break;
                } else {
                    indentPos = -1;
                }
            }
        } else {
            indentPos = -1;
        }
    } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
}

void Editor::convertNewLines(QString & txt)
{
    if (txt.indexOf("\r\n") >= 0) {
        txt.replace("\r\n", "\n");
    } else if (txt.indexOf("\n") < 0 && txt.indexOf("\r") >= 0) {
        txt.replace("\r", "\n");
    }
}

QString Editor::cleanUpText(QString blockText)
{
    int bStart = -1, bLength = -1, bOffset = 0;
    do {
        QRegularExpressionMatch match = phpExpr.match(blockText, bOffset);
        bStart = match.capturedStart();
        if (bStart>=0) {
            bLength = match.capturedLength();
            QString r(" ");
            blockText = blockText.replace(bStart, bLength, r.repeated(bLength));
            bOffset = bStart + bLength;
        }
    } while (bStart>=0);
    bStart = -1; bLength = -1; bOffset = 0;
    do {
        QRegularExpressionMatch match = strExpr.match(blockText, bOffset);
        bStart = match.capturedStart();
        if (bStart>=0) {
            bLength = match.capturedLength();
            QString r(" ");
            blockText = blockText.replace(bStart, bLength, r.repeated(bLength));
            bOffset = bStart + bLength;
        }
    } while (bStart>=0);
    return blockText;
}

void Editor::showTooltip(int x, int y, QString text, bool richText, int fixedWidth)
{
    if (!focused) return;
    /* QDesktopWidget::screenGeometry() is deprecated */
    /*
    QRect rec = QApplication::desktop()->screenGeometry();
    int  screenWidth = rec.width();
    */
    QScreen * screen = QGuiApplication::primaryScreen();
    int  screenWidth = screen->geometry().width();
    QFontMetrics fm = tooltipLabel.fontMetrics();
    QMargins mm = tooltipLabel.contentsMargins();
    QString strippedText(text.replace("\t", QString(" ").repeated(tabWidth)));
    if (richText) strippedText.replace("&lt;", "<").replace("&gt;", ">").replace("<br />","\n").replace(stripTagsExpr,"").replace("&nbsp;"," ");
    int textLineWidth = 0;
    QStringList strippedList;
    strippedList = strippedText.split("\n");
    for (int i=0; i<strippedList.size(); i++) {
        /* QFontMetrics::width is deprecated */
        /*
        int textLineW = fm.width(strippedList.at(i));
        */
        int textLineW = fm.horizontalAdvance(strippedList.at(i));
        if (textLineW > textLineWidth) textLineWidth = textLineW;
    }
    int tooltipWidth = textLineWidth + mm.left() + mm.right() + 1;
    if (fixedWidth > 0) tooltipWidth = fixedWidth;
    int tooltipLeft = x;
    QPoint pL(tooltipLeft, 0);
    QPoint pLG = QWidget::mapToGlobal(pL);
    if (pLG.x() + tooltipWidth + TOOLTIP_SCREEN_MARGIN > screenWidth) {
        if (tooltipWidth + 2 * TOOLTIP_SCREEN_MARGIN < screenWidth) {
            pLG.setX(screenWidth - tooltipWidth - TOOLTIP_SCREEN_MARGIN);
        } else {
            pLG.setX(TOOLTIP_SCREEN_MARGIN);
            tooltipWidth = screenWidth - 2 * TOOLTIP_SCREEN_MARGIN;
        }
    }
    if (pLG.x() + tooltipWidth + 10 > screenWidth) {
        tooltipWidth = screenWidth - pLG.x() - 10;
    }
    tooltipLabel.setFixedWidth(tooltipWidth);
    int extraLinesCo = 0;
    for (int i=0; i<strippedList.size(); i++) {
        /* QFontMetrics::width is deprecated */
        /*
        int textLineW = fm.width(strippedList.at(i)) + mm.left() + mm.right() + 1;
        */
        int textLineW = fm.horizontalAdvance(strippedList.at(i)) + mm.left() + mm.right() + 1;
        if (textLineW > tooltipWidth) extraLinesCo += static_cast<int>(std::ceil(static_cast<double>(textLineW) / tooltipWidth))-1;
    }
    tooltipLabel.setFixedHeight(fm.height()*(strippedList.size()+extraLinesCo) + mm.top() + mm.bottom());
    if (richText) tooltipLabel.setTextFormat(Qt::RichText);
    else tooltipLabel.setTextFormat(Qt::PlainText);
    tooltipLabel.setText(text);
    tooltipLabel.show();
    int tooltipHeight = tooltipLabel.geometry().height();
    int tooltipTop = y - tooltipHeight;
    QPoint pR (0, tooltipTop);
    QPoint pRG = QWidget::mapToGlobal(pR);
    if (pRG.y() < 0) pRG.setY(0);
    tooltipLabel.move(QPoint(pLG.x(), pRG.y()));
}

void Editor::showTooltip(QTextCursor * curs, QString text, bool richText, int fixedWidth)
{
    int viewLeft = viewport()->geometry().left();
    int viewTop = viewport()->geometry().top();
    int cursLeft = cursorRect(* curs).left();
    int cursTop = cursorRect(* curs).top();
    showTooltip(viewLeft + cursLeft - TOOLTIP_OFFSET, viewTop + cursTop - TOOLTIP_OFFSET, text, richText, fixedWidth);
}

void Editor::hideTooltip()
{
    if (tooltipLabel.isVisible()) tooltipLabel.hide();
}

bool Editor::event(QEvent *e)
{
    // track mouse
    Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
    if (e->type() == QEvent::ToolTip){
        QHelpEvent * helpEvent = static_cast<QHelpEvent *>(e);
        QPoint helpPoint = helpEvent->pos();
        helpPoint.setX(helpPoint.x() - viewport()->geometry().left());
        helpPoint.setY(helpPoint.y() - viewport()->geometry().top());
        QTextCursor curs = cursorForPosition(helpPoint);
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int total = blockText.size();
        int pos = curs.positionInBlock();
        if (!highlight->isStateOpen(&block, pos)) {
            QChar prevChar = '\0', nextChar = '\0';
            if (pos > 0 && curs.selectedText().size()==0) prevChar = blockText[pos - 1];
            if (pos < total && curs.selectedText().size()==0) nextChar = blockText[pos];
            // text under cursor
            QString cursorText = "", prevText = "", nextText = "";
            bool hasAlpha = false;
            std::string mode = highlight->findModeAtCursor(&block, pos);
            QChar cursorTextPrevChar = '\0';
            int cursorTextPos = pos;
            if (curs.selectedText().size()==0) {
                for (int i=pos; i>0; i--) {
                    QString c = blockText.mid(i-1, 1);
                    if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
                    cursorTextPrevChar = c[0];
                    if (mode == MODE_PHP) {
                        if (isalnum(c[0].toLatin1()) || c=="$" || c=="_" || c=="\\") prevText = c + prevText;
                        else break;
                    } else if (mode == MODE_JS) {
                        if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") prevText = c + prevText;
                        else break;
                    } else if (mode == MODE_CSS) {
                        if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") prevText = c + prevText;
                        else break;
                    } else {
                        if (isalnum(c[0].toLatin1()) || c=="_") prevText = c + prevText;
                        else break;
                    }
                    cursorTextPos = i-1;
                }
                for (int i=pos; i<total; i++) {
                    QString c = blockText.mid(i, 1);
                    if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
                    if (mode == MODE_PHP) {
                        if (isalnum(c[0].toLatin1()) || c=="$" || c=="_" || c=="\\") nextText += c;
                        else break;
                    } else if (mode == MODE_JS) {
                        if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") nextText += c;
                        else break;
                    } else if (mode == MODE_CSS) {
                        if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") nextText += c;
                        else break;
                    } else {
                        if (isalnum(c[0].toLatin1()) || c=="_") nextText += c;
                        else break;
                    }
                }
                if (prevText.size()>0 && nextText.size()>0 && hasAlpha) {
                    cursorText = prevText + nextText;
                }
            }
            if (cursorText.size() > 0) {
                clearTextHoverFormat();
                if (modifiers & Qt::ControlModifier) {
                    // show underline
                    curs.movePosition(QTextCursor::StartOfBlock);
                    curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                    curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, cursorText.size());
                    QTextEdit::ExtraSelection selectedWordSelection;
                    selectedWordSelection.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                    selectedWordSelection.cursor = curs;
                    wordsExtraSelections.append(selectedWordSelection);
                    highlightExtras();
                    // function or class method description
                    QString name = "", descName = "", descText = "";
                    if (cursorText.size() > 0 && mode == MODE_PHP) {
                        name = cursorText;
                        curs.movePosition(QTextCursor::StartOfBlock);
                        curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                        QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
                        QChar prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
                        QString nsName = highlight->findNsPHPAtCursor(&block, cursorTextPos);
                        QString clsName = highlight->findClsPHPAtCursor(&block, cursorTextPos);
                        QString funcName = highlight->findFuncPHPAtCursor(&block, cursorTextPos);
                        if ((prevChar == ">" && prevPrevChar == "-") || (prevChar == ":" && prevPrevChar == ":")) {
                            curs.movePosition(QTextCursor::StartOfBlock);
                            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                            QString prevType = detectCompleteTypeAtCursorPHP(curs, nsName, clsName, funcName);
                            if (prevType.size() > 0 && prevType.at(0) == "\\") prevType = prevType.mid(1);
                            if (prevType.size() > 0) {
                                name = prevType + "::" + cursorText;
                                HPW->phpClassMethodDescsIterator = HPW->phpClassMethodDescs.find(name.toStdString());
                                if (HPW->phpClassMethodDescsIterator != HPW->phpClassMethodDescs.end()) {
                                    descName = QString::fromStdString(HPW->phpClassMethodDescsIterator->first);
                                    descText = QString::fromStdString(HPW->phpClassMethodDescsIterator->second);
                                    descText.replace("<", "&lt;").replace(">", "&gt;");
                                }
                            }
                        } else {
                            HPW->phpFunctionDescsIterator = HPW->phpFunctionDescs.find(name.toLower().toStdString());
                            if (HPW->phpFunctionDescsIterator != HPW->phpFunctionDescs.end()) {
                                descName = QString::fromStdString(HPW->phpFunctionDescsIterator->first);
                                descText = QString::fromStdString(HPW->phpFunctionDescsIterator->second);
                                descText.replace("<", "&lt;").replace(">", "&gt;");
                            }
                            if (descName.size() == 0 || descText.size() == 0) {
                                name = completeClassNamePHPAtCursor(curs, name, nsName);
                            }
                        }
                        if (name.size() > 0) {
                            CW->tooltipsIteratorPHP = CW->tooltipsPHP.find(name.toStdString());
                            if (CW->tooltipsIteratorPHP != CW->tooltipsPHP.end()) {
                                QString fName = QString::fromStdString(CW->tooltipsIteratorPHP->first);
                                QString params = QString::fromStdString(CW->tooltipsIteratorPHP->second);
                                if (params.indexOf(TOOLTIP_DELIMITER) >= 0) {
                                    params.replace(TOOLTIP_DELIMITER, "<br />" + fName);
                                }
                                if (fName.indexOf("::")>0) fName = getFixedCompleteClassMethodName(fName, params);
                                descName = fName + " " + params;
                            }
                        }
                    }
                    if (descName.size() > 0) {
                        QString tooltipText = descName;
                        if (descText.size() > 0) tooltipText += "<br /><br />" + descText.replace("\n", "<br />");
                        curs.movePosition(QTextCursor::StartOfBlock);
                        curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                        showTooltip(& curs, tooltipText);
                    }
                } else {
                    // function definition
                    curs.movePosition(QTextCursor::StartOfBlock);
                    curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                    QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
                    QChar prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
                    bool showAllowed = false;
                    if ((prevChar != ">" || prevPrevChar != "-") && (prevChar != ":" || prevPrevChar != ":")) showAllowed = true;
                    CW->tooltipsIteratorPHP = CW->tooltipsPHP.find(cursorText.toStdString());
                    if (showAllowed && CW->tooltipsIteratorPHP != CW->tooltipsPHP.end()) {
                        QString fName = QString::fromStdString(CW->tooltipsIteratorPHP->first);
                        QString params = QString::fromStdString(CW->tooltipsIteratorPHP->second);
                        if (params.indexOf(TOOLTIP_DELIMITER) >= 0) {
                            params.replace(TOOLTIP_DELIMITER, "<br />" + fName);
                        }
                        QString tooltipText = fName + " " + params;
                        showTooltip(& curs, tooltipText);
                    } else {
                        QRegularExpressionMatch m = colorExpr.match(cursorText);
                        if (m.capturedStart()==0 && QColor::isValidColor(cursorText)) {
                            showTooltip(& curs, TOOLTIP_COLOR_TPL.arg(cursorText)+" "+cursorText);
                        }
                    }
                }
            } else {
                clearTextHoverFormat();
                if (prevText.size() > 0 && nextText.size() > 0) {
                    QString nonAlphaText = prevText+nextText;
                    QRegularExpressionMatch m = colorExpr.match(nonAlphaText);
                    if (m.capturedStart()==0 && QColor::isValidColor(nonAlphaText)) {
                        showTooltip(& curs, TOOLTIP_COLOR_TPL.arg(nonAlphaText)+" "+nonAlphaText);
                    }
                }
            }
            return true;
        }
    }
    return QTextEdit::event(e);
}

void Editor::clearTextHoverFormat()
{
    if (wordsExtraSelections.size() > 0) {
        wordsExtraSelections.clear();
    }
}

void Editor::clearErrorsFormat()
{
    if (errorsExtraSelections.size() > 0) {
        errorsExtraSelections.clear();
        highlightExtras();
    }
}

void Editor::highlightError(int pos, int length)
{
    if (pos < 0) return;
    // show wave underline
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    if (pos > cursor.position()) return;
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, length);
    QTextEdit::ExtraSelection errorSelection;
    errorSelection.format.setUnderlineColor(lineErrorColor);
    errorSelection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorSelection.cursor = cursor;
    errorsExtraSelections.append(errorSelection);
    highlightExtras();
}

void Editor::highlightErrorLine(int line)
{
    if (line < 1) return;
    // show wave underline
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    if (line > 1) cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line-1);
    QString blockText = cursor.block().text();
    do {
        int pos = cursor.positionInBlock();
        if (pos >= blockText.size()) break;
        QChar c = blockText[pos];
        if (!c.isSpace()) break;
    } while(cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor));
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QTextEdit::ExtraSelection errorSelection;
    errorSelection.format.setUnderlineColor(lineErrorColor);
    errorSelection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorSelection.cursor = cursor;
    errorsExtraSelections.append(errorSelection);
    highlightExtras();
}

void Editor::suggestWords(QStringList words, int cursorTextPos)
{
    completePopup->clearItems();
    for (QString word : words) {
        completePopup->addItem(word, word);
        if (completePopup->count() >= completePopup->limit()) break;
    }
    if (completePopup->count()>0) {
        completePopup->setTextStartPos(cursorTextPos);
        showCompletePopup();
    }
}

void Editor::updateSizes()
{
    updateViewportMargins();
    updateWidgetsGeometry();
}

void Editor::resizeEvent(QResizeEvent * e)
{
    QTextEdit::resizeEvent(e);
    firstVisibleBlockIndex = -1;
    lastVisibleBlockIndex = -1;
    hidePopups();
    updateWidgetsGeometry();
    updateLineAnnotationView();
    if (static_cast<Search *>(search)->isVisible()) {
        static_cast<Search *>(search)->updateScrollBar();
    }
}

void Editor::updateWidgetsGeometry()
{
    QRect cr = contentsRect();
    int lineW = lineNumberAreaWidth();
    int markW = lineMarkAreaWidth();
    int mapW = lineMapAreaWidth();
    int searchH = searchWidgetHeight();
    if (!search->isVisible()) searchH = 0;
    int breadcrumbsH = breadcrumbsHeight();

    int vScrollW = verticalScrollBar()->geometry().width();
    int hScrollH = horizontalScrollBar()->geometry().height();
    if (verticalScrollBar()->maximum() <= verticalScrollBar()->minimum()) vScrollW = 0;
    if (horizontalScrollBar()->maximum() <= horizontalScrollBar()->minimum() || searchH == 0) hScrollH = 0;

    search->setGeometry(QRect(cr.left(), cr.bottom()-searchH-hScrollH+1, cr.width()-mapW-vScrollW, searchH+hScrollH));
    QMargins searchMargins = search->contentsMargins();
    searchMargins.setBottom(hScrollH);
    search->setContentsMargins(searchMargins);
    lineNumber->setGeometry(QRect(cr.left(), cr.top()+breadcrumbsH, lineW, cr.height()-breadcrumbsH-searchH-hScrollH));
    lineMark->setGeometry(QRect(cr.left()+lineW, cr.top()+breadcrumbsH, markW, cr.height()-breadcrumbsH-searchH-hScrollH));
    lineMap->setGeometry(QRect(cr.right()-mapW-vScrollW+1, cr.top(), mapW+vScrollW, cr.height()));
    breadcrumbs->setGeometry(QRect(cr.left(), cr.top(), cr.width()-mapW-vScrollW, breadcrumbsH));
    qaBtn->setGeometry(0, 0, lineW, breadcrumbsH-1);
}

void Editor::backtab()
{
    if (!focused) return;
    QTextCursor curs = textCursor();
    if (curs.selectedText().size()==0) {
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int pos = curs.positionInBlock();
        int count = 0;
        if (pos > 0) {
            QChar prevChar = blockText[pos-1];
            if (prevChar == "\t" && curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor)) {
                count++;
            }
        }
        if (count==0) {
            while (pos > 0) {
                QChar prevChar = blockText[pos-1];
                if (prevChar == " " && curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor)) {
                    count++;
                    pos = curs.positionInBlock();
                    if (count >= tabWidth) break;
                } else {
                    break;
                }
            }
        }
        if (count>0) {
            curs.deleteChar();
        }
    }
    if (curs.selectedText().size()>0) {
        bool changed = false;
        int start = curs.selectionStart();
        int end = curs.selectionEnd();
        curs.setPosition(end);
        int endBlockNumber = curs.block().blockNumber();
        curs.setPosition(start);
        do {
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            QTextBlock block = curs.block();
            QString blockText = block.text();
            int total = blockText.size();
            int pos = curs.positionInBlock();
            int count = 0;
            if (pos < total) {
                QChar nextChar = blockText[pos];
                if (nextChar == "\t" && curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor)) {
                    count++;
                }
            }
            if (count == 0) {
                while (pos < total) {
                    QChar nextChar = blockText[pos];
                    if (nextChar == " " && curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor)) {
                        count++;
                        pos = curs.positionInBlock();
                        if (count >= tabWidth) break;
                    } else {
                        break;
                    }
                }
            }
            if (count>0) {
                if (!changed) {
                    curs.beginEditBlock();
                    changed = true;
                }
                curs.deleteChar();
            }
            if (curs.block().blockNumber()>=endBlockNumber) break;
        } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
        if (changed) {
            curs.endEditBlock();
        }
    }
}

void Editor::duplicateLine()
{
    QTextCursor curs = textCursor();
    if (curs.hasSelection()) return;
    QString text = curs.block().text();
    curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    curs.insertText("\n"+text);

    // spell check
    if (textCursor().blockNumber() != curs.blockNumber() && spellCheckerEnabled && spellChecker != nullptr && !isBigFile) {
        spellPastedBlocksQueue.append(curs.block().blockNumber());
        QTimer::singleShot(INTERVAL_SPELL_CHECK_MILLISECONDS, this, SLOT(spellCheckPasted()));
    }
}

void Editor::deleteLine()
{
    QTextCursor curs = textCursor();
    if (curs.hasSelection()) return;
    curs.beginEditBlock();
    if (curs.block().text().size() > 0) {
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
    if (curs.block().blockNumber() < document()->blockCount()-1) {
        curs.deleteChar();
    } else if (curs.block().blockNumber() > 0) {
        curs.deletePreviousChar();
    }
    curs.endEditBlock();
}

void Editor::comment()
{
    if (!focused) return;
    QTextCursor curs = textCursor();
    int selectedSize = curs.selectedText().size();
    if (selectedSize==0) {
        curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        QTextBlock block = curs.block();
        QString blockText = block.text();
        if (blockText.trimmed().size() == 0) return;
        int pos = curs.positionInBlock();
        int total = blockText.size();
        std::string mode = highlight->findModeAtCursor(&block, pos);
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        block = curs.block();
        blockText = block.text();
        pos = curs.positionInBlock();
        total = blockText.size();
        if (highlight->findModeAtCursor(&block, pos) != mode) return;
        if (mode == MODE_PHP || mode == MODE_JS) {
            if (blockText.trimmed().indexOf("//") == 0) {
                if (blockText.indexOf("/") > 0) curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, blockText.indexOf("/"));
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
                curs.deleteChar();
            } else {
                curs.insertText("//");
            }
        } else if (mode == MODE_CSS) {
            if (pos+1 < total && blockText[pos] == "/" && blockText[pos+1] == "*" && pos+1 < total-2 && blockText[total-2] == "*" && blockText[total-1] == "/") {
                curs.beginEditBlock();
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
                curs.deleteChar();
                curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 2);
                curs.deleteChar();
                curs.endEditBlock();
            } else {
                curs.beginEditBlock();
                curs.insertText("/*");
                curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                curs.insertText("*/");
                curs.endEditBlock();
            }
        } else if (mode == MODE_HTML) {
            if (pos+3 < total && blockText[pos] == "<" && blockText[pos+1] == "!" && blockText[pos+2] == "-" && blockText[pos+3] == "-" &&
                pos+3 < total-3 && blockText[total-3] == "-" && blockText[total-2] == "-" && blockText[total-1] == ">"
            ) {
                curs.beginEditBlock();
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
                curs.deleteChar();
                curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 3);
                curs.deleteChar();
                curs.endEditBlock();
            } else {
                curs.beginEditBlock();
                curs.insertText("<!--");
                curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                curs.insertText("-->");
                curs.endEditBlock();
            }
        }
    }
    if (selectedSize>0) {
        int start = curs.selectionStart();
        int end = curs.selectionEnd();
        curs.setPosition(end);
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int pos = curs.positionInBlock();
        int total = blockText.size();
        std::string mode = highlight->findModeAtCursor(&block, pos);
        int endBlockNumber = block.blockNumber();
        curs.setPosition(start);
        block = curs.block();
        blockText = block.text();
        pos = curs.positionInBlock();
        total = blockText.size();
        if (highlight->findModeAtCursor(&block, pos) != mode) return;
        int moveEnd = end;
        curs.beginEditBlock();
        if (mode == MODE_PHP || mode == MODE_JS) {
            do {
                curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                block = curs.block();
                blockText = block.text();
                if (blockText.trimmed().size() == 0) continue;
                pos = curs.positionInBlock();
                total = blockText.size();
                if (highlight->findModeAtCursor(&block, pos) != mode) break;
                if (blockText.trimmed().indexOf("//") == 0) {
                    if (blockText.indexOf("/") > 0) curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, blockText.indexOf("/"));
                    curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
                    curs.deleteChar();
                    moveEnd -= 2;
                } else {
                    curs.insertText("//");
                    moveEnd += 2;
                }
                if (curs.block().blockNumber()>=endBlockNumber) break;
            } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
        } else if (mode == MODE_CSS) {
            bool hasStart = false, hasEnd = false;
            curs.setPosition(start);
            block = curs.block();
            blockText = block.text();
            pos = curs.positionInBlock();
            total = blockText.size();
            if (pos+1 < total && blockText[pos] == "/" && blockText[pos+1] == "*") {
                hasStart = true;
            }
            curs.setPosition(end);
            block = curs.block();
            blockText = block.text();
            pos = curs.positionInBlock();
            total = blockText.size();
            if (selectedSize >= 4 && pos>1 && blockText[pos-2] == "*" && blockText[pos-1] == "/") {
                hasEnd = true;
            }
            if (hasStart && hasEnd) {
                curs.setPosition(start);
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
                curs.deleteChar();
                curs.setPosition(end - 2);
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 2);
                curs.deleteChar();
                moveEnd -= 4;
            } else {
                curs.setPosition(start);
                curs.insertText("/*");
                curs.setPosition(end + 2);
                curs.insertText("*/");
                moveEnd += 4;
            }
        } else if (mode == MODE_HTML) {
            bool hasStart = false, hasEnd = false;
            curs.setPosition(start);
            block = curs.block();
            blockText = block.text();
            pos = curs.positionInBlock();
            total = blockText.size();
            if (pos+3 < total && blockText[pos] == "<" && blockText[pos+1] == "!" && blockText[pos+2] == "-" && blockText[pos+3] == "-") {
                hasStart = true;
            }
            curs.setPosition(end);
            block = curs.block();
            blockText = block.text();
            pos = curs.positionInBlock();
            total = blockText.size();
            if (selectedSize >= 7 && pos>2 && blockText[pos-3] == "-" && blockText[pos-2] == "-" && blockText[pos-1] == ">") {
                hasEnd = true;
            }
            if (hasStart && hasEnd) {
                curs.setPosition(start);
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
                curs.deleteChar();
                curs.setPosition(end - 4);
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 3);
                curs.deleteChar();
                moveEnd -= 7;
            } else {
                curs.setPosition(start);
                curs.insertText("<!--");
                curs.setPosition(end + 4);
                curs.insertText("-->");
                moveEnd += 7;
            }
        }
        curs.endEditBlock();
        if (moveEnd != end) {
            curs.setPosition(start);
            curs.setPosition(moveEnd, QTextCursor::KeepAnchor);
            setTextCursor(curs);
        }
    }
}

void Editor::focusInEvent(QFocusEvent *e)
{
    focused = true;
    if (fileName.size() > 0 && !Helper::fileExists(fileName) && !warningDisplayed) {
        setFileIsDeleted();
    } else if (fileName.size() > 0 && Helper::fileExists(fileName) && lastModifiedMsec > 0 && !warningDisplayed) {
        QFileInfo fInfo(fileName);
        QDateTime dtModified = fInfo.lastModified();
        if (dtModified.time().msec() != lastModifiedMsec) {
            setFileIsOutdated();
        }
    }
    if (static_cast<Search *>(search)->isVisible()) {
        static_cast<Search *>(search)->updateScrollBar();
    }
    QTextEdit::focusInEvent(e);
    emit focusIn(tabIndex);
}

void Editor::focusOutEvent(QFocusEvent *e)
{
    focused = false;
    hidePopups();
    QTextEdit::focusOutEvent(e);
}

void Editor::setFileIsDeleted()
{
    if (warningDisplayed) return;
    warningDisplayed = true;
    if (modified) emit modifiedStateChanged(tabIndex, false);
    modified = true;
    emit warning(tabIndex, "deleted", QObject::tr("File \"%1\" not found.").arg(fileName));
    emit modifiedStateChanged(tabIndex, modified);
}

void Editor::setFileIsOutdated()
{
    if (warningDisplayed) return;
    warningDisplayed = true;
    if (modified) emit modifiedStateChanged(tabIndex, false);
    modified = true;
    emit warning(tabIndex, "outdated", QObject::tr("File \"%1\" was modified externally.").arg(fileName));
    emit modifiedStateChanged(tabIndex, modified);
}

bool Editor::onKeyPress(QKeyEvent *e)
{
    bool shift = false, ctrl = false;
    if (e->modifiers() & Qt::ShiftModifier) shift = true;
    if (e->modifiers() & Qt::ControlModifier) ctrl = true;
    int code = e->key();
    lastKeyPressed = code;
    lastKeyPressedBlockNumber = textCursor().block().blockNumber();
    bool ignoreKey = false; // workaround for wrong key code, android emulator bug ?
    #if defined(Q_OS_ANDROID)
    if (code == Qt::Key_BracketLeft && e->text() != "[") ignoreKey = true;
    if (code == Qt::Key_ParenLeft && e->text() != "(") ignoreKey = true;
    if (code == Qt::Key_BraceLeft && e->text() != "{") ignoreKey = true;
    if (code == Qt::Key_BracketRight && e->text() != "]") ignoreKey = true;
    if (code == Qt::Key_ParenRight && e->text() != ")") ignoreKey = true;
    if (code == Qt::Key_BraceRight && e->text() != "}") ignoreKey = true;
    if (code == Qt::Key_Apostrophe && e->text() != "'") ignoreKey = true;
    if (code == Qt::Key_QuoteDbl && e->text() != "\"") ignoreKey = true;
    #endif
    // insert quotes & brackets pair
    if ((code == Qt::Key_QuoteDbl || code == Qt::Key_Apostrophe || code == Qt::Key_BraceLeft || code == Qt::Key_BracketLeft || code == Qt::Key_ParenLeft) && !ctrl && !ignoreKey) {
        QTextCursor curs = textCursor();
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int total = blockText.size();
        int pos = curs.positionInBlock();
        QChar nextChar = '\0';
        if (pos < total) nextChar = blockText[pos];
        bool doInsert = true;
        if ((code == Qt::Key_QuoteDbl && blockText.count("\"")%2!=0) ||
            (code == Qt::Key_Apostrophe && blockText.count("'")%2!=0) ||
            (code == Qt::Key_BraceLeft && blockText.count("{")!=blockText.count("}")) ||
            (code == Qt::Key_BracketLeft && blockText.count("[")!=blockText.count("]")) ||
            (code == Qt::Key_ParenLeft && blockText.count("(")!=blockText.count(")"))
        ) {
            doInsert = false;
        }
        if (nextChar == '\0') doInsert = true;
        else if (isalpha(nextChar.toLatin1()) || nextChar == "$") doInsert = false;
        if (doInsert && highlight->isStateOpen(&block ,pos)) doInsert = false;
        if (doInsert) {
            QString insert = "";
            if (code == Qt::Key_QuoteDbl) insert = "\"\"";
            if (code == Qt::Key_Apostrophe) insert = "''";
            if (code == Qt::Key_BraceLeft) insert = "{}";
            if (code == Qt::Key_BracketLeft) insert = "[]";
            if (code == Qt::Key_ParenLeft) insert = "()";
            if (insert.size()>0) {
                curs.insertText(insert);
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
                setTextCursor(curs);
                return false;
            }
        }
    }
    // skip & move cursor to next char
    if ((code == Qt::Key_QuoteDbl || code == Qt::Key_Apostrophe || code == Qt::Key_BraceRight || code == Qt::Key_BracketRight || code == Qt::Key_ParenRight) && !ctrl && !ignoreKey) {
        QTextCursor curs = textCursor();
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int total = blockText.size();
        int pos = curs.positionInBlock();
        QChar nextChar = '\0';
        if (pos < total) nextChar = blockText[pos];
        if ((code == Qt::Key_QuoteDbl && nextChar == "\"" && blockText.count("\"")%2==0) ||
            (code == Qt::Key_Apostrophe && nextChar == "'" && blockText.count("'")%2==0) ||
            (code == Qt::Key_BraceRight && nextChar == "}" && blockText.count("{")==blockText.count("}")) ||
            (code == Qt::Key_BracketRight && nextChar == "]" && blockText.count("[")==blockText.count("]")) ||
            (code == Qt::Key_ParenRight && nextChar == ")" && blockText.count("(")==blockText.count(")"))
        ) {
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
            setTextCursor(curs);
            return false;
        }
    }
    // indent
    if (code == Qt::Key_Tab && !shift && !ctrl) {
        QTextCursor curs = textCursor();
        if (curs.selectedText().size()==0 && tabType == "spaces") {
            QString insert = " ";
            curs.insertText(insert.repeated(tabWidth));
            return false;
        }
        if (curs.selectedText().size()>0) {
            QString indent = (tabType == "spaces") ? QString(" ").repeated(tabWidth) : "\t";
            curs.beginEditBlock();
            int start = curs.selectionStart();
            int end = curs.selectionEnd();
            curs.setPosition(end);
            int endBlockNumber = curs.block().blockNumber();
            curs.setPosition(start);
            do {
                curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                curs.insertText(indent);
                if (curs.block().blockNumber()>=endBlockNumber) break;
            } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
            curs.endEditBlock();
            return false;
        }
    }
    // remove spaces, quotes, brackets
    if (code == Qt::Key_Backspace && !shift && !ctrl) {
        QTextCursor curs = textCursor();
        if (curs.selectedText().size()==0) {
            QTextBlock block = curs.block();
            QString blockText = block.text();
            int pos = curs.positionInBlock();
            int count = 0;
            // quotes & brackets
            QChar pc = '\0', nc = '\0';
            if (pos > 0) pc = blockText[pos - 1];
            if (pos < blockText.size()) nc = blockText[pos];
            if ((pc == Qt::Key_QuoteDbl && nc == Qt::Key_QuoteDbl) ||
                (pc == Qt::Key_Apostrophe && nc == Qt::Key_Apostrophe) ||
                (pc == Qt::Key_BraceLeft && nc == Qt::Key_BraceRight) ||
                (pc == Qt::Key_BracketLeft && nc == Qt::Key_BracketRight) ||
                (pc == Qt::Key_ParenLeft && nc == Qt::Key_ParenRight)
            ) {
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 2);
                count += 2;
            }
            // spaces
            if (count == 0) {
                while (pos > 0) {
                    QChar prevChar = blockText[pos-1];
                    if (prevChar == " " && curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor)) {
                        count++;
                        pos = curs.positionInBlock();
                        if (count >= tabWidth) break;
                    } else {
                        break;
                    }
                }
            }
            if (count>0) {
                curs.deleteChar();
                return false;
            }
        }
    }
    // complete, indent and comment on enter
    if (code == Qt::Key_Return && !shift && !ctrl) {
        if (search->isVisible() && static_cast<Search *>(search)->isFocused()) {
            return false;
        }
        // complete
        if (completePopup->isVisible() && completePopup->count()>0) {
            completePopup->chooseCurrentItem();
            return false;
        }
        QTextCursor curs = textCursor();
        QTextBlock block = curs.block();
        QString blockText = block.text();
        QString blockTextTrimmed = blockText.trimmed();
        int total = blockText.size();
        int pos = curs.positionInBlock();
        int state = highlight->findStateAtCursor(&block, pos);
        bool isCommentOpen = false, isCommentLine = false, isCommentClose = false, isPHPDoc = false;
        if (pos > 1 && blockText[pos-2] == "/" && blockText[pos-1] == "*") {
            isCommentOpen = true;
        } else if (pos > 2 && blockText[pos-3] == "/" && blockText[pos-2] == "*" && blockText[pos-1] == "*") {
            isCommentOpen = true;
            isPHPDoc = true;
        } else if (pos > 2 && blockText[pos-3] == " " && blockText[pos-2] == "*" && blockText[pos-1] == " ") {
            isCommentLine = true;
        } else if (blockText.indexOf(QRegularExpression("^[\\s]*[ ][*][ ]")) >= 0) {
            isCommentLine = true;
        } else if (pos > 2 && blockText[pos-3] == " " && blockText[pos-2] == "*" && blockText[pos-1] == "/") {
            isCommentClose = true;
        }
        QString prefix = "";
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        pos = curs.positionInBlock();
        while(pos < total) {
            if (!curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) break;
            pos = curs.positionInBlock();
            QChar prevChar = blockText[pos-1];
            if ((prevChar == " " || prevChar == "\t")) {
                prefix += prevChar;
            } else {
                break;
            }
        }
        if ((isCommentClose || isCommentLine) && prefix.size()>0) prefix = prefix.mid(0, prefix.size()-1);
        if (isCommentOpen && state != STATE_COMMENT_ML_PHP && state != STATE_COMMENT_ML_JS && state != STATE_COMMENT_ML_CSS) isCommentOpen = false;
        if (isCommentLine && state != STATE_COMMENT_ML_PHP && state != STATE_COMMENT_ML_JS && state != STATE_COMMENT_ML_CSS) isCommentLine = false;
        // comment
        if (isCommentOpen || isCommentLine) {
            QTextCursor cursor = textCursor();
            QTextBlock block = cursor.block();
            QVector<QString> phpDocParams;
            QVector<QString> phpDocParamTypes;
            QString phpDocFuncName = "", phpDocReturnVal = "";
            std::string mode = highlight->findModeAtCursor(& block, cursor.positionInBlock());
            if (mode == MODE_PHP || mode == MODE_JS || mode == MODE_CSS) {
                if (isPHPDoc && mode == MODE_PHP) {
                    QTextCursor cursorP = textCursor();
                    while (cursorP.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) {
                        QString blockPtext = cursorP.block().text().trimmed();
                        if (blockPtext.size()==0) continue;
                        QRegularExpressionMatch matchP = functionExpr.match(blockPtext);
                        if (matchP.capturedStart()>=0) {
                            phpDocFuncName = matchP.captured(1);
                            phpDocReturnVal = matchP.captured(3).replace(QRegularExpression("[\\?][\\s]+"), "?");
                            QString params = matchP.captured(2);
                            params = params.trimmed();
                            if (params.size()==0) break;
                            QRegularExpressionMatch funcMatch = functionWordExpr.match(params);
                            if (funcMatch.capturedStart() >= 0) break;
                            QStringList paramsList = params.split(",");
                            for (int i=0; i<paramsList.size(); i++) {
                                QString param = paramsList.at(i);
                                QStringList parList = param.split("=");
                                if (parList.size()>2) continue;
                                QString par = parList.at(0);
                                par = par.trimmed();
                                QStringList parTypeList = par.replace(QRegularExpression("[&][\\s]+"),"&").split(QRegularExpression("[\\s]+"));
                                if (parTypeList.size()==1) {
                                    phpDocParams.append(parTypeList.at(0));
                                    phpDocParamTypes.append("mixed");
                                } else if (parTypeList.size()==2) {
                                    phpDocParams.append(parTypeList.at(1));
                                    phpDocParamTypes.append(parTypeList.at(0));
                                }
                            }
                        }
                        break;
                    }
                }
                HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
                if (blockData != nullptr && (blockData->state == STATE_COMMENT_ML_PHP || blockData->state == STATE_COMMENT_ML_JS || blockData->state == STATE_COMMENT_ML_CSS)) {
                    blockData->state = STATE_NONE;
                    cursor.block().setUserData(blockData);
                }
                cursor.beginEditBlock();
                cursor.insertText("\n" + prefix + " * ");
                if (isPHPDoc && mode == MODE_PHP) {
                    for (int i=0; i<phpDocParams.size(); i++) {
                        QString paramType = phpDocParamTypes.at(i);
                        cursor.insertText("\n" + prefix + " * @param "+paramType+" "+phpDocParams.at(i));
                    }
                    if (phpDocReturnVal.size()>0) {
                        cursor.insertText("\n" + prefix + " * ");
                        cursor.insertText("\n" + prefix + " * @return "+phpDocReturnVal);
                    }
                }
                if (isCommentOpen) cursor.insertText("\n" + prefix + " */");
                cursor.endEditBlock();
                if (cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) {
                    highlight->highlightChanges(cursor);
                    cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
                }
                if (isCommentOpen) {
                    if (isPHPDoc) {
                        if (phpDocParams.size()>0) {
                            cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, phpDocParams.size());
                        }
                        if (phpDocReturnVal.size()>0) {
                            cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 2);
                        }
                    }
                    cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                    setTextCursor(cursor);
                } else if (isCommentLine) {
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                    setTextCursor(cursor); // fix scroll
                }
                return false;
            }
        }
        // remove indent in emply line (disabled)
        if (prefix.size()>0 && blockTextTrimmed.size()==0) {
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, blockText.size());
            curs.deleteChar();
        }
        // indent
        QString indent = "";
        QTextCursor cursor = textCursor();
        block = cursor.block();
        blockText = block.text();
        total = blockText.size();
        pos = cursor.positionInBlock();
        QChar prevChar = '\0', nextChar = '\0', lastChar = '\0';
        if (pos > 0) prevChar = blockText[pos-1];
        if (pos < total) {
            nextChar = blockText[pos];
            if (total > pos+1) lastChar = blockText[total-1];
        }
        if (prevChar == "{") {
            indent = (tabType == "spaces") ? QString(" ").repeated(tabWidth) : "\t";
        }
        // fix empty indent
        if (nextChar == '\0' && prefix.size()==0 && indent.size()==0 && !isCommentClose && !isCommentLine && curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) {
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            block = curs.block();
            blockText = block.text();
            blockTextTrimmed = blockText.trimmed();
            total = blockText.size();
            pos = curs.positionInBlock();
            while(pos < total) {
                if (!curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) break;
                pos = curs.positionInBlock();
                QChar prevChar = blockText[pos-1];
                if ((prevChar == " " || prevChar == "\t")) {
                    prefix += prevChar;
                } else {
                    break;
                }
            }
            curs.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
        }
        if ((prefix.size()>0 || indent.size()>0)) {
            QTextCursor cursor = textCursor();
            cursor.beginEditBlock();
            cursor.insertText("\n"+prefix+indent);
            if (prevChar == "{" && (nextChar == "}" || lastChar == "}")) {
                if (nextChar != "}" && lastChar == "}") {
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
                }
                cursor.insertText("\n"+prefix);
                cursor.endEditBlock();
                cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                setTextCursor(cursor);
            } else {
                cursor.endEditBlock();
                setTextCursor(cursor); // fix scroll
            }
            return false;
        }
    }
    // saving mode for keyrelease event
    if (code == Qt::Key_Greater && !ctrl) {
        QTextCursor curs = textCursor();
        QTextBlock block = curs.block();
        int pos = curs.positionInBlock();
        modeOnKeyPress = highlight->findModeAtCursor(&block, pos);
    }
    // complete popup navigation
    if ((code == Qt::Key_Down || code == Qt::Key_Up) && completePopup->isVisible() && completePopup->count()>0) {
        if (code == Qt::Key_Down) completePopup->selectNextItem();
        if (code == Qt::Key_Up) completePopup->selectPreviousItem();
        return false;
    }
    // hide popup
    if (code == Qt::Key_Escape && completePopup->isVisible()) {
        hideCompletePopup();
        return false;
    }
    // switch tooltip page
    if ((code == Qt::Key_Down || code == Qt::Key_Up) && tooltipLabel.isVisible() && tooltipSavedList.size()>1 && tooltipSavedPageOffset >= 0 && tooltipSavedPageOffset < tooltipSavedList.size()) {
        if (code == Qt::Key_Down) tooltipSavedPageOffset++;
        if (code == Qt::Key_Up) tooltipSavedPageOffset--;
        if (tooltipSavedPageOffset < 0) tooltipSavedPageOffset = tooltipSavedList.size()-1;
        if (tooltipSavedPageOffset >= tooltipSavedList.size()) tooltipSavedPageOffset = 0;
        tooltipSavedText = tooltipSavedList.at(tooltipSavedPageOffset);
        QString fName = "", params = "";
        int kSep = tooltipSavedText.indexOf("(");
        if (kSep > 0) {
            fName = tooltipSavedText.mid(0, kSep).trimmed();
            params = tooltipSavedText.mid(kSep).trimmed();
        }
        QString toolTipText = tooltipSavedOrigName + " " + params;
        QTextCursor cursor = textCursor();
        showTooltip(& cursor, toolTipText);
        followTooltip();
        return false;
    }
    return true;
}

void Editor::keyPressEvent(QKeyEvent *e)
{
    if (!onKeyPress(e)) return;
    QTextEdit::keyPressEvent(e);
}

bool Editor::onKeyRelease(QKeyEvent *e)
{
    bool ctrl = false;
    if (e->modifiers() & Qt::ControlModifier) ctrl = true;
    int code = e->key();
    if (code == Qt::Key_Greater && !ctrl && modeOnKeyPress == MODE_HTML) {
        hideCompletePopup();
    }
    // fix indent on right brace insert
    if (code == Qt::Key_BraceRight && !ctrl && textCursor().block().text().trimmed() == "}") {
        bool foundPrefix = false;
        QString prefix = "";
        QChar openChar = '{';
        QChar closeChar = '}';
        QTextCursor cursor = textCursor();
        int pos = cursor.positionInBlock()-1;
        HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
        if (blockData != nullptr && blockData->specialChars.size()>0 && blockData->specialCharsPos.size()>0 && blockData->specialChars.size()==blockData->specialCharsPos.size()) {
            bool sFound = false;
            int count = 0;
            int positionInBlock = -1;
            int iterations = 0;
            do {
                if (blockData->specialChars.size()>0 && blockData->specialCharsPos.size()>0) {
                    for (int i=blockData->specialChars.size()-1; i>=0; i--) {
                        iterations++;
                        if (iterations > SEARCH_LIMIT) break;
                        QChar c = blockData->specialChars.at(i);
                        if (!sFound && c == closeChar && blockData->specialCharsPos.at(i) == pos) {
                            sFound = true;
                        } else if (sFound && c == closeChar) {
                            count++;
                        } else if (sFound && c == openChar && count > 0) {
                            count--;
                        } else if (sFound && c == openChar && count == 0) {
                            positionInBlock = blockData->specialCharsPos.at(i);
                            int total = cursor.block().text().size();
                            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                            do {
                                int pos = cursor.positionInBlock();
                                if (pos >= total) break;
                                QChar chr = cursor.block().text().at(pos);
                                if (chr.isSpace()) prefix += chr;
                                else break;
                            } while(cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor));
                            foundPrefix = true;
                            break;
                        }
                    }
                }
                if (!sFound) break;
                if (!cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor)) break;
                blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
                if (blockData == nullptr || blockData->specialChars.size()!=blockData->specialCharsPos.size()) break;
            } while(positionInBlock < 0);
        }
        if (foundPrefix && textCursor().block().text() != prefix + "}" && ((tabType == "spaces" && prefix.indexOf("\t") < 0) || (tabType == "tabs" && prefix.indexOf(" ") < 0))) {
            QTextCursor curs = textCursor();
            curs.beginEditBlock();
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            curs.deleteChar();
            curs.insertText(prefix + "}");
            curs.endEditBlock();
        }
    }
    return true;
}

void Editor::keyReleaseEvent(QKeyEvent *e)
{
    if (!onKeyRelease(e)) return;
    QTextEdit::keyReleaseEvent(e);
}

void Editor::inputMethodEvent(QInputMethodEvent *e)
{
    QString c = e->commitString();
    if (c.size() == 1) {
        inputEventKey = QKeySequence::fromString(c)[0];
        Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
        QKeyEvent * event = new QKeyEvent(QEvent::KeyPress, inputEventKey, modifiers, c);
        if (!onKeyPress(event)) e->setCommitString("");
        delete event;
    } else {
        lastKeyPressed = -1;
        lastKeyPressedBlockNumber = textCursor().blockNumber();
    }
    QTextEdit::inputMethodEvent(e);
}

void Editor::contextMenu()
{
    QContextMenuEvent * cEvent = new QContextMenuEvent(QContextMenuEvent::Keyboard, mapFromGlobal(QCursor::pos()));
    contextMenuEvent(cEvent);
    delete cEvent;
}

void Editor::insertFromMimeData(const QMimeData *source)
{
    // clean, indent & insert clipboard text
    if (source->hasText()) {
        QTextCursor curs = textCursor();
        QTextBlock block = curs.block();
        QString blockText = block.text();
        int total = blockText.size();
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        int pos = curs.positionInBlock();
        QString prefix = "";
        while(pos < total) {
            if (!curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) break;
            pos = curs.positionInBlock();
            QChar prevChar = blockText[pos-1];
            if ((prevChar == " " || prevChar == "\t")) {
                prefix += prevChar;
            } else {
                break;
            }
        }
        QString text = source->text();
        QString textF = "";
        text = text.replace("\r\n","\n");
        QStringList textList = text.split(QRegularExpression("[\r\n]"));
        int count = 0;
        if (textList.size()>0) {
            QString space = (tabType == "spaces") ? QString(" ").repeated(tabWidth) : "\t";
            if (textList.size()>1) {
                QString line = textList.at(0);
                textF += line.trimmed() + "\n";
                for (int i=0; i<textList.size(); i++) {
                    line = textList.at(i);
                    line = line.trimmed();
                    if (line.size()==0) {
                        if (i>0) textF += prefix + space.repeated(count) + "\n";
                        continue;
                    }
                    QChar first = line[0];
                    QChar last = line[line.size()-1];
                    if (first == "}") {
                        count--;
                        if (count < 0) count = 0;
                    }
                    if (i>0) {
                        textF += prefix + space.repeated(count) + line + "\n";
                    }
                    if (last == "{" && first != "/") {
                        count++;
                    }
                }
                textF = textF.trimmed();
            } else {
                textF = text;
            }
            QTextCursor cursor = textCursor();
            int startBlockNumber = cursor.block().blockNumber();
            if (tabType == "spaces") textF = textF.replace("\t", space);
            cursor.insertText(textF);
            do {
                // modified state
                modifiedLinesIterator = modifiedLines.find(cursor.block().blockNumber() + 1);
                if (modifiedLinesIterator == modifiedLines.end()) {
                    modifiedLines[cursor.block().blockNumber() + 1] = cursor.block().blockNumber() + 1;
                    HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
                    if (blockData != nullptr) {
                        blockData->isModified = true;
                        cursor.block().setUserData(blockData);
                    }
                }
                // spell check
                if (spellCheckerEnabled && spellChecker != nullptr && !isBigFile) {
                    spellPastedBlocksQueue.append(cursor.block().blockNumber());
                }
                if (cursor.block().blockNumber() == startBlockNumber) break;
            } while(cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor));
            lineNumber->update();
            if (spellCheckerEnabled && spellChecker != nullptr && !isBigFile) {
                QTimer::singleShot(INTERVAL_SPELL_CHECK_MILLISECONDS, this, SLOT(spellCheckPasted()));
            }
            return;
        }
    }
    QTextEdit::insertFromMimeData(source);
}

void Editor::blockCountChanged(int /*blockCount*/)
{
    updateViewportMargins();
    updateLineWidgetsArea();
    lineAnnotation->hide();
    // updating mark points, modified lines
    markPoints.clear();
    modifiedLines.clear();
    QTextCursor curs = textCursor();
    curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    do {
        HighlightData * blockData = dynamic_cast<HighlightData *>(curs.block().userData());
        if (blockData != nullptr && blockData->hasMarkPoint) {
            markPoints[curs.block().blockNumber()+1] = curs.block().text().toStdString();
        }
        if (blockData != nullptr && blockData->isModified) {
            modifiedLines[curs.block().blockNumber()+1] = curs.block().blockNumber()+1;
        }
    } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));

    emit statusBarText(tabIndex, ""); // update status bar
}

void Editor::horizontalScrollbarValueChanged(int /* sliderPos */)
{
    updateLineAnnotationView();
    if (static_cast<Search *>(search)->isVisible()) {
        static_cast<Search *>(search)->updateScrollBar();
    }
}

void Editor::verticalScrollbarValueChanged(int /* sliderPos */)
{
    firstVisibleBlockIndex = -1;
    lastVisibleBlockIndex = -1;

    hidePopups();
    updateLineWidgetsArea();
    updateLineAnnotationView();
    lineMap->update();

    if (is_ready && !scrollBarValueChangeLocked) {
        scrollBarValueChangeLocked = true;
        QTimer::singleShot(INTERVAL_SCROLL_HIGHLIGHT_MILLISECONDS, this, SLOT(verticalScrollbarValueChangedDelayed()));
    }
}

void Editor::verticalScrollbarValueChangedDelayed()
{
    highlight->updateBlocks(getLastVisibleBlockIndex());
    scrollBarValueChangeLocked = false;
}

void Editor::mousePressEvent(QMouseEvent *e)
{
    hideCompletePopup();
    QTextEdit::mousePressEvent(e);
}

void Editor::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        showDeclarationRequested();
    } else {
        hideTooltip();
    }
    QTextEdit::mouseReleaseEvent(e);
}

void Editor::contentsChange(int position, int charsRemoved, int charsAdded)
{
    if (!is_ready || isReadOnly()) return;
    QTextBlock block = document()->findBlock(position);
    if (!block.isValid()) return;

    QTextBlock lastBlock = document()->findBlock(position + charsAdded + (charsRemoved > 0 ? 1 : 0));
    if (!lastBlock.isValid()) {
        QTextCursor curs = QTextCursor(block);
        highlight->highlightChanges(curs);
        return;
    }

    int endPosition = lastBlock.position() + lastBlock.length();
    bool forceHighlightOfNextBlock = false;
    while (block.isValid() && (block.position() < endPosition)) {
        const int stateBeforeHighlight = block.userState();
        highlight->rehighlightBlock(block);
        forceHighlightOfNextBlock = (block.userState() != stateBeforeHighlight);
        block = block.next();
    }
    if (forceHighlightOfNextBlock && block.isValid()) {
        QTextCursor curs = QTextCursor(block);
        highlight->highlightChanges(curs);
    }
}

void Editor::textChanged()
{
    if (!is_ready || isReadOnly()) return;
    Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
    bool ctrl = modifiers & Qt::ControlModifier;
    if (!textChangeLocked && lastKeyPressed != Qt::Key_Backspace && lastKeyPressed != Qt::Key_Delete && lastKeyPressed != Qt::Key_Return && lastKeyPressed != Qt::Key_Tab && lastKeyPressed != Qt::Key_Space && !ctrl) {
        textChangeLocked = true;
        QTimer::singleShot(INTERVAL_TEXT_CHANGED_MILLISECONDS, this, SLOT(textChangedDelayed()));
    } else if (!textChangeLocked) {
        hideCompletePopup();
    }
    bool _modified = modified;
    if (document()->isModified()) {
        modified = true;
    } else {
        modified = false;
    }
    if (_modified != modified) {
        emit modifiedStateChanged(tabIndex, modified);
    }

    // parse
    if (!parseLocked && isReady()) {
        parseLocked = true;
        QTimer::singleShot(parseResultChangedDelay, this, SLOT(parseResultChanged()));
    }

    // spell check
    if (spellCheckerEnabled && spellChecker != nullptr) {
        if (spellBlocksQueue.size() == 0 || (spellBlocksQueue.last() != textCursor().block().blockNumber())) {
            spellBlocksQueue.append(textCursor().block().blockNumber());
        }
        if (!spellLocked) {
            spellLocked = true;
            QTimer::singleShot(INTERVAL_SPELL_CHECK_MILLISECONDS, this, SLOT(spellCheck()));
        }
    }

    // set line modified status
    QTextCursor curs = textCursor();
    if ((lastKeyPressedBlockNumber == curs.block().blockNumber() && lastKeyPressed != Qt::Key_Return) || (curs.positionInBlock() == 0 && curs.block().text().size() > 0) || (curs.positionInBlock() > 0 && curs.positionInBlock() != curs.block().text().size())) {
        modifiedLinesIterator = modifiedLines.find(curs.block().blockNumber() + 1);
        if (modifiedLinesIterator == modifiedLines.end()) {
            modifiedLines[curs.block().blockNumber() + 1] = curs.block().blockNumber() + 1;
            HighlightData * blockData = dynamic_cast<HighlightData *>(curs.block().userData());
            if (blockData != nullptr) {
                blockData->isModified = true;
                curs.block().setUserData(blockData);
            }
            lineNumber->update();
        }
        if (lastKeyPressedBlockNumber < curs.block().blockNumber()) {
            modifiedLinesIterator = modifiedLines.find(lastKeyPressedBlockNumber + 1);
            if (modifiedLinesIterator == modifiedLines.end()) {
                modifiedLines[lastKeyPressedBlockNumber + 1] = lastKeyPressedBlockNumber + 1;
                if (lastKeyPressedBlockNumber == curs.block().blockNumber()-1) {
                    curs.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
                } else {
                    curs.movePosition(QTextCursor::Start);
                    if (lastKeyPressedBlockNumber > 0) curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lastKeyPressedBlockNumber);
                }
                HighlightData * blockData = dynamic_cast<HighlightData *>(curs.block().userData());
                if (blockData != nullptr) {
                    blockData->isModified = true;
                    curs.block().setUserData(blockData);
                }
                lineNumber->update();
            }
        }
    }
    // workaround for Android
    if (inputEventKey >= 0) {
        Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
        QKeyEvent * event = new QKeyEvent(QEvent::KeyRelease, inputEventKey, modifiers);
        onKeyRelease(event);
        delete event;
        inputEventKey = -1;
    }
}

void Editor::textChangedDelayed()
{
    textChangeLocked = false;
    // complete popup
    QTextCursor curs = textCursor();
    if (curs.selectedText().size()!=0) return;
    QTextBlock block = curs.block();
    QString blockText = block.text();
    int total = blockText.size();
    int pos = curs.positionInBlock();
//    if (highlight->isStateOpen(&block, pos)) {
//        hideCompletePopup();
//        return;
//    }
    std::string mode = highlight->findModeAtCursor(&block, pos);
    int state = highlight->findStateAtCursor(&block, pos);
    if (mode != MODE_HTML && state != STATE_NONE) return;
    //if (mode == MODE_HTML && state != STATE_TAG) return; // snippets won't work
    if (mode == MODE_UNKNOWN) return;
    QChar prevChar = '\0', nextChar = '\0';
    if (pos > 0 && curs.selectedText().size()==0) prevChar = blockText[pos - 1];
    if (pos < total && curs.selectedText().size()==0) nextChar = blockText[pos];
    // text till cursor
    QString cursorText = "", prevText = "";
    QChar cursorTextPrevChar = '\0';
    int cursorTextPos = pos;
    for (int i=pos; i>0; i--) {
        QString c = blockText.mid(i-1, 1);
        cursorTextPrevChar = c[0];
        if (mode == MODE_PHP || mode == MODE_JS) {
            if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") prevText = c + prevText;
            else break;
        } else if (mode == MODE_CSS) {
            if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") prevText = c + prevText;
            else break;
        } else {
            if (isalnum(c[0].toLatin1()) || c=="_") prevText = c + prevText;
            else break;
        }
        cursorTextPos = i-1;
    }
    if (prevText.size()>0) {
        QChar c = prevText[0];
        if (isalpha(c.toLatin1())) cursorText = prevText;
        else if ((mode == MODE_PHP || mode == MODE_JS) && (c=="$" || c=="_")) cursorText = prevText;
        else if (mode == MODE_CSS && (c=="#" || c=="." || c=="-" || c=="_")) cursorText = prevText;
        else if (c=="_") cursorText = prevText;
    }
    hideCompletePopup();
    if (cursorText.size() > 0 && (nextChar == '\0' || !isalnum(nextChar.toLatin1()))) {
        detectCompleteText(cursorText, cursorTextPrevChar, cursorTextPos, mode, state);
    }
}

bool Editor::isKnownWord(QString word)
{
    bool known = false;
    SW->wordsIterator = SW->words.find(word.toLower().toStdString());
    if (SW->wordsIterator != SW->words.end()) {
        known = true;
    }
    if (!known) {
        SW->wordsCSIterator = SW->wordsCS.find(word.toStdString());
        if (SW->wordsCSIterator != SW->wordsCS.end()) {
            known = true;
        }
    }
    if (!known) {
        HW->phpwordsIterator = HW->phpwords.find(word.toLower().toStdString());
        if (HW->phpwordsIterator != HW->phpwords.end()) {
            known = true;
        }
    }
    if (!known) {
        HW->phpwordsCSIterator = HW->phpwordsCS.find(word.toStdString());
        if (HW->phpwordsCSIterator != HW->phpwordsCS.end()) {
            known = true;
        }
    }
    if (!known) {
        HW->jswordsCSIterator = HW->jswordsCS.find(word.toStdString());
        if (HW->jswordsCSIterator != HW->jswordsCS.end()) {
            known = true;
        }
    }
    if (!known) {
        HW->csswordsIterator = HW->csswords.find(word.toLower().toStdString());
        if (HW->csswordsIterator != HW->csswords.end()) {
            known = true;
        }
    }
    if (!known) {
        HW->htmlwordsIterator = HW->htmlwords.find(word.toLower().toStdString());
        if (HW->htmlwordsIterator != HW->htmlwords.end()) {
            known = true;
        }
    }
    return known;
}

void Editor::spellCheck(bool suggest, bool forceRehighlight)
{
    spellLocked = false;
    if (!spellCheckerEnabled || spellChecker == nullptr) return;
    QTextCursor cursor = textCursor();
    int pos = cursor.positionInBlock();
    QString blockText = cursor.block().text();
    // text till cursor
    QString cursorText = "";
    int cursorTextPos = pos;
    if (suggest) {
        for (int i=pos; i>0; i--) {
            QChar c = blockText[i-1];
            if (c.isLetter() || c=="_") cursorText = c + cursorText;
            else break;
            cursorTextPos = i-1;
        }
    }
    QRegularExpressionMatch m;
    for (int i=0; i<spellBlocksQueue.size(); i++) {
        bool misspelled = false;
        int blockNumber = spellBlocksQueue.at(i);
        if (blockNumber == cursor.block().blockNumber() + 1) {
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        } else {
            cursor.movePosition(QTextCursor::Start);
            if (blockNumber > 0) cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);
        }
        QTextBlock block = cursor.block();
        if (block.blockNumber() != spellBlocksQueue.at(i)) continue;
        HighlightData * blockData = dynamic_cast<HighlightData *>(block.userData());
        if (blockData == nullptr) continue;
        blockData->spellStarts.clear();
        blockData->spellLengths.clear();
        QString blockText = cursor.block().text();
        if (blockText.trimmed().size() == 0) continue;
        int offset = 0;
        do {
            m = spellWordExpr.match(blockText, offset);
            if (m.capturedStart() >= 0) {
                offset = m.capturedStart() + m.capturedLength();
                int start = m.capturedStart(1);
                int length = m.capturedLength(1);
                QString word = blockText.mid(start, length);
                bool hasLetter = false;
                for (int i=0; i<word.size(); i++) {
                    QChar c = word[i];
                    if (c.isLetter()) hasLetter = true;
                }
                if (!hasLetter) continue;
                if (word.size() > 0 && word[0] == "$") continue;
                if (word.size()>1 && word[0]=='\'' && word[word.size()-1]=='\'') {
                    word = word.mid(1,word.size()-2);
                    start += 1;
                    length -= 2;
                }
                if (word.size() > 1 && word[word.size()-1] == "$") {
                    word = word.mid(0, word.size()-1);
                    length -= 1;
                }
                if (word.size() < 2) continue;
                std::string mode = highlight->findModeAtCursor(&block, start);
                int state = highlight->findStateAtCursor(&block, start);
                //if ((mode == MODE_PHP || mode == MODE_JS || mode == MODE_CSS) && state == STATE_NONE) continue;
                if (mode == MODE_PHP && state != STATE_COMMENT_ML_PHP) continue;
                if (mode == MODE_JS && state != STATE_COMMENT_ML_JS) continue;
                if (mode == MODE_CSS && state != STATE_COMMENT_ML_CSS) continue;
                if (mode == MODE_HTML && state != STATE_NONE) continue;
                bool doSuggest = suggest;
                if (word.size() < 4) doSuggest = false;
                if (mode != MODE_HTML && mode != MODE_UNKNOWN) doSuggest = false;
                if (lastKeyPressed == Qt::Key_Backspace || lastKeyPressed == Qt::Key_Delete) doSuggest = false;
                if (!isKnownWord(word) && !spellChecker->check(word)) {
                    misspelled = true;
                    blockData->spellStarts.append(start);
                    blockData->spellLengths.append(length);
                    if (doSuggest && word == cursorText && start == cursorTextPos) {
                        hideCompletePopup();
                        QStringList suggestions = spellChecker->suggest(word);
                        suggestWords(suggestions, cursorTextPos);
                    }
                }
            }
        } while(m.capturedStart() >= 0);
        block.setUserData(blockData);
        if (forceRehighlight || misspelled) {
            blockSignals(true);
            highlight->rehighlightBlock(block);
            blockSignals(false);
        }
    }
    spellBlocksQueue.clear();
}

QChar Editor::findPrevCharNonSpaceAtCursos(QTextCursor & curs)
{
    QChar prevChar = '\0';
    int cursPos = curs.position();
    while(curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor)) {
        QString blockText = curs.block().text();
        int pos = curs.positionInBlock();
        if (pos >= blockText.size()) continue;
        prevChar = blockText[pos];
        if (!prevChar.isSpace()) {
            break;
        }
    }
    if (prevChar!="." && prevChar!="-" && prevChar!=">" && prevChar!=":" && prevChar!="\\") {
        curs.setPosition(cursPos);
    }
    return prevChar;
}

QChar Editor::findNextCharNonSpaceAtCursos(QTextCursor & curs)
{
    QChar nextChar = '\0';
    int cursPos = curs.position();
    while(curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) {
        QString blockText = curs.block().text();
        int pos = curs.positionInBlock();
        if (pos >= blockText.size()) continue;
        nextChar = blockText[pos];
        if (!nextChar.isSpace()) {
            break;
        }
    }
    if (nextChar!="." && nextChar!="-" && nextChar!=">" && nextChar!=":" && nextChar!="\\") {
        curs.setPosition(cursPos);
    }
    return nextChar;
}

QString Editor::findPrevWordNonSpaceAtCursor(QTextCursor & curs, std::string mode)
{
    QString prevText = "";
    while(curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor)) {
        QString blockText = curs.block().text();
        int pos = curs.positionInBlock();
        if (pos >= blockText.size()) continue;
        QChar c = blockText[pos];
        if (prevText.size() == 0 && c.isSpace()) continue;
        if (mode == MODE_PHP || mode == MODE_JS) {
            if (isalnum(c.toLatin1()) || c=="$" || c=="_") {
                prevText = c + prevText;
            } else {
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
                break;
            }
        } else if (mode == MODE_CSS) {
            if (isalnum(c.toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") {
                prevText = c + prevText;
            } else {
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
                break;
            }
        } else {
            if (isalnum(c.toLatin1()) || c=="_") {
                prevText = c + prevText;
            } else {
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
                break;
            }
        }
        if (prevText.size() > 0 && pos == 0) break;
    }
    return prevText;
}

QString Editor::findNextWordNonSpaceAtCursor(QTextCursor & curs, std::string mode)
{
    QString nextText = "";
    while(curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) {
        QString blockText = curs.block().text();
        int pos = curs.positionInBlock();
        if (pos >= blockText.size()) continue;
        QChar c = blockText[pos];
        if (nextText.size() == 0 && c.isSpace()) continue;
        if (mode == MODE_PHP || mode == MODE_JS) {
            if (isalnum(c.toLatin1()) || c=="$" || c=="_") {
                nextText += c;
            } else {
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
                break;
            }
        } else if (mode == MODE_CSS) {
            if (isalnum(c.toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") {
                nextText += c;
            } else {
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
                break;
            }
        } else {
            if (isalnum(c.toLatin1()) || c=="_") {
                nextText += c;
            } else {
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
                break;
            }
        }
        if (nextText.size() > 0 && pos == 0) break;
    }
    return nextText;
}

QString Editor::completeClassNamePHPAtCursor(QTextCursor & curs, QString prevWord, QString nsName)
{
    if (!parsePHPEnabled || prevWord.size() == 0) return prevWord;
    QString _clsName = prevWord;
    QChar _prevChar = findPrevCharNonSpaceAtCursos(curs);
    if (_prevChar == "\\") {
        do {
            QString _prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
            if (_prevWord.size() == 0 || _prevWord[0] == "$" || _prevWord == "new") {
                _clsName = "\\" + _clsName;
                break;
            }
            _clsName = _prevWord + "\\" + _clsName;
            _prevChar = findPrevCharNonSpaceAtCursos(curs);
        } while(_prevChar == "\\");
    }
    QString clsAlias = "";
    if (_clsName.indexOf("\\") < 0) clsAlias = _clsName;
    if (_clsName[0] != "\\") clsAlias = _clsName.mid(0, _clsName.indexOf("\\"));
    if (clsAlias == "namespace") {
        QString ns = "";
        if (nsName.size() > 0) ns = "\\" + nsName;
        _clsName = ns + _clsName.mid(clsAlias.size());
        clsAlias = "";
    }
    if (clsAlias.size() > 0) {
        // search imports
        ParsePHP::ParseResultNamespace ns;
        ParsePHP::ParseResultImport imp;
        for (int i=0; i<parseResultPHP.namespaces.size(); i++) {
            ParsePHP::ParseResultNamespace _ns = parseResultPHP.namespaces.at(i);
            if (_ns.name == nsName) {
                ns = _ns;
                break;
            }
        }
        for (int c=0; c<ns.importsIndexes.size(); c++) {
            if (parseResultPHP.imports.size() <= ns.importsIndexes.at(c)) break;
            ParsePHP::ParseResultImport _imp = parseResultPHP.imports.at(ns.importsIndexes.at(c));
            if (_imp.type == IMPORT_TYPE_CLASS && _imp.name == clsAlias) {
                imp = _imp;
                break;
            }
        }
        if (ns.name==nsName && imp.name==clsAlias) {
            if (_clsName.indexOf("\\") < 0) _clsName = imp.path;
            if (_clsName[0] != "\\") _clsName = imp.path+_clsName.mid(_clsName.indexOf("\\"));
        }
    }
    if (_clsName[0] != "\\") {
        QString ns = "\\";
        if (nsName.size() > 0) ns += nsName + "\\";
        _clsName = ns + _clsName;
    }
    if (_clsName[0] == "\\") _clsName = _clsName.mid(1);
    return _clsName;
}

void Editor::showCompletePopup()
{
    int blockHeight = static_cast<int>(document()->documentLayout()->blockBoundingRect(textCursor().block()).height());
    int viewLeft = viewport()->geometry().left();
    int viewTop = viewport()->geometry().top();
    int viewWidth = viewport()->geometry().width();
    int viewHeight = viewport()->geometry().height();
    int cursLeft = cursorRect().left();
    int cursTop = cursorRect().top();

    QTextCursor curs = textCursor();
    int cursorTextPos = completePopup->getTextStartPos();
    if (cursorTextPos >= 0 && cursorTextPos < curs.positionInBlock()) {
        curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, curs.positionInBlock() - cursorTextPos);
        cursLeft = cursorRect(curs).left();
    }
    if (curs.block().layout() != nullptr && curs.block().layout()->lineCount() > 1) {
        QTextLine line = curs.block().layout()->lineForTextPosition(curs.positionInBlock());
        if (line.isValid()) {
            blockHeight = static_cast<int>(line.height());
        }
    }

    completePopup->showPopup(cursLeft, cursTop, viewLeft, viewTop, viewWidth, viewHeight, blockHeight);
}

void Editor::hideCompletePopup()
{
    completePopup->hidePopup();
    completePopup->clearItems();
}

void Editor::detectCompleteTextHTML(QString text, QChar cursorTextPrevChar, int state)
{
    // snippets
    if (cursorTextPrevChar == "@" && htmlSnippets.contains(text)) {
        completePopup->addItem(SNIPPET_PREFIX+text, htmlSnippets[text]);
    }
    if (state != STATE_TAG) return;
    if ((cursorTextPrevChar == "<" || cursorTextPrevChar == "/")  && completePopup->count() < completePopup->limit()) {
        // html tags
        for (auto & it : CW->htmlAllTagsComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
    // events
    if (state == STATE_TAG && completePopup->count() < completePopup->limit()) {
        for (auto & it : CW->jsEventsComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
}

void Editor::detectCompleteTextCSS(QString text, QChar cursorTextPrevChar)
{
    QTextCursor curs = textCursor();
    int pos = curs.positionInBlock();
    QString blockText = curs.block().text();
    QString blockTextTillCursos = blockText.mid(0, pos);
    int braceOpens = blockTextTillCursos.count("{");
    int braceCloses = blockTextTillCursos.count("}");
    int braces = braceOpens - braceCloses;
    bool cssMediaScope = false;
    if (curs.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor)) {
        HighlightData * blockData = dynamic_cast<HighlightData *>(curs.block().userData());
        if (blockData != nullptr) {
            braces += blockData->bracesCSS;
            cssMediaScope = blockData->cssMediaScope;
        }
    }
    int propOffset = blockTextTillCursos.lastIndexOf(";");
    if (propOffset < 0) propOffset = 0;
    int colIndex = blockTextTillCursos.indexOf(":", propOffset);
    // snippets
    if (cursorTextPrevChar == "@" && cssSnippets.contains(text)) {
        completePopup->addItem(SNIPPET_PREFIX+text, cssSnippets[text]);
    }
    if (((braces > 0 && !cssMediaScope) || (braces > 1 && cssMediaScope)) && colIndex < 0 && completePopup->count() < completePopup->limit()) {
        // css props
        for (auto & it : CW->cssPropertiesComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
    if (colIndex >= 0 && completePopup->count() < completePopup->limit()) {
        // css vals
        for (auto & it : CW->cssValuesComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
    if (completePopup->count() < completePopup->limit()) {
        // css id & class selectors
        for (int i=parseResultCSS.names.size()-1; i>=0; i--){
            ParseCSS::ParseResultName _name = parseResultCSS.names.at(i);
            QString k = _name.name;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(k, k);
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
    if (completePopup->count() < completePopup->limit()) {
        // html tags
        for (auto & it : CW->htmlAllTagsComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
    if (cursorTextPrevChar == ":" && completePopup->count() < completePopup->limit()) {
        // css pseudo
        for (auto & it : CW->cssPseudoComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    }
}

void Editor::detectCompleteTextJS(QString text, int cursorTextPos, QChar cursorTextPrevChar)
{
    QTextCursor curs = textCursor();
    curs.movePosition(QTextCursor::StartOfBlock);
    if (cursorTextPos > 0) curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
    QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
    QString prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_JS);
    // global context
    if (prevChar != "." && prevWord != "function") {
        // snippets
        if (cursorTextPrevChar == "@" && jsSnippets.contains(text)) {
            completePopup->addItem(SNIPPET_PREFIX+text, jsSnippets[text]);
        }
        // js specials
        if (completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->jsSpecialsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // js objects
        if (completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->jsObjectsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // js functions
        if (completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->jsFunctionsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // js interfaces
        if (completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->jsInterfacesComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // parsed classes
        if (prevWord == "var" && completePopup->count() < completePopup->limit()) {
            for (int i=0; i<parseResultJS.classes.size(); i++){
                ParseJS::ParseResultClass cls = parseResultJS.classes.at(i);
                QString k = cls.name;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(k, k);
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // parsed functions
        if (completePopup->count() < completePopup->limit()) {
            for (int i=0; i<parseResultJS.functions.size(); i++){
                ParseJS::ParseResultFunction func = parseResultJS.functions.at(i);
                if (func.clsName.size() > 0) continue;
                QString k = func.name;
                QString p = "( " + func.args + " )";
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(k, p);
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // parsed vars
        std::unordered_map<std::string, std::string> vars;
        std::unordered_map<std::string, std::string>::iterator varsIterator;
        if (completePopup->count() < completePopup->limit()) {
            for (int i=parseResultJS.variables.size()-1; i>=0; i--){
                ParseJS::ParseResultVariable _variable = parseResultJS.variables.at(i);
                //if (_variable.clsName.size() > 0) continue;
                QString k = _variable.name;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    varsIterator = vars.find(k.toStdString());
                    if (varsIterator == vars.end()) {
                        vars[k.toStdString()] = k.toStdString();
                        completePopup->addItem(k, k);
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
        }
        // highlighted vars
        if (completePopup->count() < completePopup->limit()) {
            HighlightData * blockData = dynamic_cast<HighlightData *>(curs.block().userData());
            if (blockData != nullptr && blockData->varsChainJS.size()>0 && (blockData->varsChainJS.indexOf(text, 0, Qt::CaseInsensitive)==0 || blockData->varsChainJS.indexOf(","+text, 0, Qt::CaseInsensitive)>0)) {
                QStringList varsList = blockData->varsChainJS.split(",");
                for (QString k : varsList) {
                    if (k == text) continue; // need this
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                        varsIterator = vars.find(k.toStdString());
                        if (varsIterator == vars.end()) {
                            vars[k.toStdString()] = k.toStdString();
                            completePopup->addItem(k, k);
                            if (completePopup->count() >= completePopup->limit()) break;
                        }
                    }
                }
            }
        }
    } else if (prevChar == ".") {
        // object context
        QString k = "prototype";
        if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
            completePopup->addItem(k, k);
        }
        // methods
        if (completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->jsMethodsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // events
        if (completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->jsEventsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // parsed functions
        if (completePopup->count() < completePopup->limit()) {
            for (int i=0; i<parseResultJS.functions.size(); i++){
                ParseJS::ParseResultFunction func = parseResultJS.functions.at(i);
                if (func.clsName.size() == 0) continue;
                QString k = func.name;
                QString p = "( " + func.args + " )";
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(k, p);
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        // parsed props
        std::unordered_map<std::string, std::string> vars;
        std::unordered_map<std::string, std::string>::iterator varsIterator;
        if (completePopup->count() < completePopup->limit()) {
            for (int i=0; i<parseResultJS.variables.size(); i++){
                ParseJS::ParseResultVariable _variable = parseResultJS.variables.at(i);
                if (_variable.clsName.size() == 0) continue;
                QString k = _variable.name;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    varsIterator = vars.find(k.toStdString());
                    if (varsIterator == vars.end()) {
                        vars[k.toStdString()] = k.toStdString();
                        completePopup->addItem(k, k);
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
        }
    }
}

void Editor::detectCompleteTextPHPGlobalContext(QString text, int cursorTextPos, QChar prevChar, QChar prevPrevChar, QString prevWord, QTextCursor curs, QChar cursorTextPrevChar)
{
    bool completeDetectedPHP = false;
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    QString nsName = highlight->findNsPHPAtCursor(& block, cursorTextPos);
    QString clsName = highlight->findClsPHPAtCursor(& block, cursorTextPos);
    QString funcName = highlight->findFuncPHPAtCursor(& block, cursorTextPos);
    if (text[0] != "$" && prevWord != "function"  && prevWord != "class" && prevWord != "interface" && prevWord != "trait" && prevWord != "namespace" && prevWord != "use") {
        if (prevWord != "new" && (prevChar != "?" || text != "php") && (prevChar != ":" || prevPrevChar != ":")) {
            // snippets
            if (cursorTextPrevChar == "@" && phpSnippets.contains(text)) {
                completePopup->addItem(SNIPPET_PREFIX+text, phpSnippets[text]);
            }
            // php specials
            if (prevChar != "\\" && completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpSpecialsComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            // php functions
            if (prevChar != "\\" && completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpFunctionsComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second), TOOLTIP_DELIMITER);
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            // php consts
            if (prevChar != "\\" && completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpConstsComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            // detect class name
            QString _clsName = "";
            if (prevChar == "\\") {
                _clsName = completeClassNamePHPAtCursor(curs, prevWord, nsName);
            }
            // php classes (without params)
            if (completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpClassesComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    QString _text = nsName.size() > 0 ? nsName + "\\" + text : text;
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                        QString name = QString::fromStdString(it.first);
                        if (_clsName.size() > 0 && name.indexOf(_clsName+"\\")==0) {
                            name = name.mid(_clsName.size()+1);
                            completePopup->addItem(name, name);
                        } else if (_clsName.size() == 0) {
                            completePopup->addItem(name, "\\"+name);
                        }
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            if (completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpClassesComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    QString _text = nsName.size() > 0 ? nsName + "\\" + text : text;
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)>=0 && k.indexOf(_text, 0, Qt::CaseInsensitive)!=0) {
                        QString name = QString::fromStdString(it.first);
                        if (_clsName.size() > 0 && name.indexOf(_clsName+"\\")==0) {
                            name = name.mid(_clsName.size()+1);
                            completePopup->addItem(name, name);
                        } else if (_clsName.size() == 0) {
                            completePopup->addItem(name, "\\"+name);
                        }
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            completeDetectedPHP = true;
        } else if (prevWord == "new") {
            // php classes (with params)
            for (auto & it : CW->phpClassesComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                QString _text = nsName.size() > 0 ? nsName + "\\" + text : text;
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second), TOOLTIP_DELIMITER);
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
            if (completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpClassesComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    QString _text = nsName.size() > 0 ? nsName + "\\" + text : text;
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)>=0 && k.indexOf(_text, 0, Qt::CaseInsensitive)!=0) {
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second), TOOLTIP_DELIMITER);
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            completeDetectedPHP = true;
        } else if (prevChar == ":" && prevPrevChar == ":" && prevWord.size() > 0 && prevWord[0] != "$") {
            // detect class name
            QString _clsName;
            if ((prevWord.toLower() == "self" || prevWord.toLower() == "static") && clsName.size() > 0) {
                _clsName = nsName.size() > 0 ? nsName + "\\" + clsName : clsName;
            } else if (prevWord.toLower() == "parent" && clsName.size() > 0) {
                QString ns = "";
                if (nsName.size() > 0) ns = nsName + "\\";
                for (int i=0; i<parseResultPHP.classes.size(); i++) {
                    ParsePHP::ParseResultClass _cls = parseResultPHP.classes.at(i);
                    if (_cls.name == "\\"+ns+clsName) {
                        QString parentClass = _cls.parent;
                        if (parentClass.size() > 0 && parentClass.at(0) == "\\") parentClass = parentClass.mid(1);
                        if (parentClass.size() > 0) {
                            _clsName = parentClass;
                        }
                        break;
                    }
                }
            } else {
                _clsName= completeClassNamePHPAtCursor(curs, prevWord, nsName);
            }
            // php class consts
            for (auto & it : CW->phpClassConstsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(_clsName+"::"+text, 0, Qt::CaseInsensitive)==0) {
                    //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    QString classConstComplete = getFixedCompleteClassConstName(QString::fromStdString(it.first));
                    completePopup->addItem(classConstComplete, QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
            if (completePopup->count() < completePopup->limit()) {
                // php class methods
                for (auto & it : CW->phpClassMethodsComplete) {
                    QString k = QString::fromStdString(it.first);
                    //if (k == text) continue;
                    if (k.indexOf(_clsName+"::"+text, 0, Qt::CaseInsensitive)==0) {
                        //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            completeDetectedPHP = true;
        } else if (prevChar == "?" && text == "php") {
            // do not detect php tag
            completeDetectedPHP = true;
        }
    } else if (text[0] == "_" && prevWord == "function") {
        // php magic methods
        for (auto & it : CW->phpMagicComplete) {
            QString k = QString::fromStdString(it.first);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
    } else if (text[0] == "$") {
        std::unordered_map<std::string, std::string> vars;
        std::unordered_map<std::string, std::string>::iterator varsIterator;
        bool isSelf = false, isClass = false;
        if (prevChar == ":" && prevPrevChar == ":") {
            isClass = true;
        }
        if (isClass && (prevWord.toLower() == "self" || prevWord.toLower() == "static")) {
            isSelf = true;
        }
        // parsed vars
        if (clsName != "anonymous class" && funcName != "anonymous function" && !isClass) {
            QString ns = "\\";
            if (nsName.size() > 0) ns += nsName + "\\";
            QString _clsName = clsName.size() > 0 ? ns + clsName : "";
            QString _funcName = funcName;
            if (_clsName.size() == 0 && _funcName.size() > 0) _funcName = ns + _funcName;
            for (int i=parseResultPHP.variables.size()-1; i>=0; i--) {
                ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                QString k = _variable.name;
                if (_variable.clsName != _clsName || _variable.funcName != _funcName) continue;
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    varsIterator = vars.find(k.toStdString());
                    if (varsIterator == vars.end()) {
                        vars[k.toStdString()] = k.toStdString();
                        completePopup->addItem(k, k);
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            completeDetectedPHP = true;
        }
        // highlighted vars
        if (!isClass && completePopup->count() < completePopup->limit()) {
            HighlightData * blockData = dynamic_cast<HighlightData *>(curs.block().userData());
            if (blockData != nullptr && blockData->varsChainPHP.size()>0 && (blockData->varsChainPHP.indexOf(text, 0, Qt::CaseInsensitive)==0 || blockData->varsChainPHP.indexOf(","+text, 0, Qt::CaseInsensitive)>0)) {
                QStringList varsList = blockData->varsChainPHP.split(",");
                for (QString k : varsList) {
                    if (k == text) continue; // need this
                    if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                        varsIterator = vars.find(k.toStdString());
                        if (varsIterator == vars.end()) {
                            vars[k.toStdString()] = k.toStdString();
                            completePopup->addItem(k, k);
                            if (completePopup->count() >= completePopup->limit()) break;
                        }
                    }
                }
            }
        }
        // php self::$var
        if (clsName.size() > 0 && clsName != "anonymous class" && funcName != "anonymous function" && isSelf && completePopup->count() < completePopup->limit()) {
            QString ns = "\\";
            if (nsName.size() > 0) ns += nsName + "\\";
            QStringList vars = highlight->getKnownVars(ns + clsName, "");
            for (int i=vars.size()-1; i>=0; i--) {
                QString k = vars.at(i);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(k, k);
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
            completeDetectedPHP = true;
        }
        // php globals
        if (!isClass && completePopup->count() < completePopup->limit()) {
            for (auto & it : CW->phpGlobalsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
            completeDetectedPHP = true;
        }
        // class props
        if (isClass && !isSelf && prevWord.size() > 0 && prevWord[0] != "$" && completePopup->count() < completePopup->limit()) {
            // detect class name
            QString _clsName;
            if (prevWord.toLower() == "parent" && clsName.size() > 0) {
                QString ns = "";
                if (nsName.size() > 0) ns = nsName + "\\";
                for (int i=0; i<parseResultPHP.classes.size(); i++) {
                    ParsePHP::ParseResultClass _cls = parseResultPHP.classes.at(i);
                    if (_cls.name == "\\"+ns+clsName) {
                        QString parentClass = _cls.parent;
                        if (parentClass.size() > 0 && parentClass.at(0) == "\\") parentClass = parentClass.mid(1);
                        if (parentClass.size() > 0) {
                            _clsName = parentClass;
                        }
                        break;
                    }
                }
            } else {
                _clsName= completeClassNamePHPAtCursor(curs, prevWord, nsName);
            }
            // php class vars
            for (auto & it : CW->phpClassPropsComplete) {
                QString k = QString::fromStdString(it.first);
                //if (k == text) continue;
                if (k.indexOf(_clsName+"::"+text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
    } else {
        // do not detect if previous word is known keyword
        completeDetectedPHP = true;
    }
    // last attempt
    if (!completeDetectedPHP) {
        // detect type
    }
}

void Editor::detectCompleteTextPHPObjectContext(QString text, int cursorTextPos, QChar prevChar, QChar prevPrevChar, QString prevWord, QTextCursor curs)
{
    if (prevChar != ">" || prevPrevChar != "-" || text[0] == "$") return;
    bool completeDetectedPHP = false;
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    QString nsName = highlight->findNsPHPAtCursor(& block, cursorTextPos);
    QString clsName = highlight->findClsPHPAtCursor(& block, cursorTextPos);
    QString funcName = highlight->findFuncPHPAtCursor(& block, cursorTextPos);
    bool isThis = false;
    if (prevWord == "$this") {
        isThis = true;
    }
    // php $this->
    if (clsName.size() > 0 && clsName != "anonymous class" && funcName != "anonymous function" && isThis) {
        QString ns = "\\";
        if (nsName.size() > 0) ns += nsName + "\\";
        // $this->var
        QStringList vars = highlight->getKnownVars(ns + clsName, "");
        for (int i=vars.size()-1; i>=0; i--) {
            QString k = vars.at(i);
            if (k.size() < 2) continue;
            k = k.mid(1);
            //if (k == text) continue;
            if (k.indexOf(text, 0, Qt::CaseInsensitive)==0) {
                completePopup->addItem(k, k);
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
        if (completePopup->count() < completePopup->limit()) {
            // $this->method()
            QString _text = ns + clsName + "::" + text;
            for (auto & it : CW->phpClassMethodsComplete) {
                QString k = "\\"+QString::fromStdString(it.first);
                //if (k == _text) continue;
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        if (completePopup->count() < completePopup->limit()) {
            // $this->prop
            QString _text = ns + clsName + "::$" + text;
            for (auto & it : CW->phpClassPropsComplete) {
                QString k = "\\"+QString::fromStdString(it.first);
                //if (k == _text) continue;
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        completeDetectedPHP = true;
    } else if (prevWord.size() > 0 && prevWord[0] == "$") {
        QChar _prevChar = findPrevCharNonSpaceAtCursos(curs);
        QChar _prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
        QString _prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
        if (_prevChar != ":" || _prevPrevChar != ":" || (_prevWord.toLower() != "self" && _prevWord.toLower() != "static")) {
            // $object->
            if (clsName != "anonymous class" && funcName != "anonymous function") {
                QString ns = "\\";
                QString _clsName = clsName;
                QString _funcName = funcName;
                if (nsName.size() > 0) ns += nsName + "\\";
                if (_clsName.size() > 0) _clsName = ns + _clsName;
                else if (_funcName.size() > 0) _funcName = ns + _funcName;
                ParsePHP::ParseResultVariable variable;
                for (int i=0; i<parseResultPHP.variables.size(); i++) {
                    ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                    if (_variable.name == prevWord && _variable.clsName == _clsName && _variable.funcName == _funcName) {
                        variable = _variable;
                        break;
                    }
                }
                if (variable.name == prevWord && variable.type.size() > 0) {
                    // class methods
                    QString _text = variable.type + "::" + text;
                    for (auto & it : CW->phpClassMethodsComplete) {
                        QString k = "\\"+QString::fromStdString(it.first);
                        //if (k == _text) continue;
                        if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                            //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                            QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                            completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                            if (completePopup->count() >= completePopup->limit()) break;
                        }
                    }
                    if (completePopup->count() < completePopup->limit()) {
                        // class props
                        QString _text = variable.type + "::$" + text;
                        for (auto & it : CW->phpClassPropsComplete) {
                            QString k = "\\"+QString::fromStdString(it.first);
                            //if (k == _text) continue;
                            if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                                completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                                if (completePopup->count() >= completePopup->limit()) break;
                            }
                        }
                    }
                    completeDetectedPHP = true;
                }
            }
        } else if (_prevChar == ":" && _prevPrevChar == ":" && (_prevWord.toLower() == "self" || _prevWord.toLower() == "static") && clsName.size() > 0 && clsName != "anonymous class" && funcName != "anonymous function") {
            // self::$object->
            QString ns = "\\";
            if (nsName.size() > 0) ns += nsName + "\\";
            QString _clsName = ns + clsName;
            ParsePHP::ParseResultVariable variable;
            for (int i=0; i<parseResultPHP.variables.size(); i++) {
                ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                if (_variable.name == prevWord && _variable.clsName == _clsName && _variable.funcName.size() == 0) {
                    variable = _variable;
                    break;
                }
            }
            if (variable.name == prevWord && variable.type.size() > 0) {
                // class methods
                QString _text = variable.type + "::" + text;
                for (auto & it : CW->phpClassMethodsComplete) {
                    QString k = "\\"+QString::fromStdString(it.first);
                    //if (k == _text) continue;
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                        //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
                if (completePopup->count() < completePopup->limit()) {
                    // class props
                    QString _text = variable.type + "::$" + text;
                    for (auto & it : CW->phpClassPropsComplete) {
                        QString k = "\\"+QString::fromStdString(it.first);
                        //if (k == _text) continue;
                        if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                            completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                            if (completePopup->count() >= completePopup->limit()) break;
                        }
                    }
                }
                completeDetectedPHP = true;
            }
        }
    } else if (prevWord.size() > 0 && prevWord[0] != "$") {
        QChar _prevChar = findPrevCharNonSpaceAtCursos(curs);
        QChar _prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
        QString _prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
        if (_prevChar == ">" && _prevPrevChar == "-" && _prevWord == "$this" && clsName.size() > 0 && clsName != "anonymous class" && funcName != "anonymous function") {
            // $this->object->
            QString ns = "\\";
            if (nsName.size() > 0) ns += nsName + "\\";
            QString _clsName = ns + clsName;
            ParsePHP::ParseResultVariable variable;
            for (int i=0; i<parseResultPHP.variables.size(); i++) {
                ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                if (_variable.name == "$"+prevWord && _variable.clsName == _clsName && _variable.funcName.size() == 0) {
                    variable = _variable;
                    break;
                }
            }
            if (variable.name == "$"+prevWord && variable.type.size() > 0) {
                // class methods
                QString _text = variable.type + "::" + text;
                for (auto & it : CW->phpClassMethodsComplete) {
                    QString k = "\\"+QString::fromStdString(it.first);
                    //if (k == _text) continue;
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                        //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
                if (completePopup->count() < completePopup->limit()) {
                    // class props
                    QString _text = variable.type + "::$" + text;
                    for (auto & it : CW->phpClassPropsComplete) {
                        QString k = "\\"+QString::fromStdString(it.first);
                        //if (k == _text) continue;
                        if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                            completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                            if (completePopup->count() >= completePopup->limit()) break;
                        }
                    }
                }
                completeDetectedPHP = true;
            }
        }
    }
    // last attempt
    if (!completeDetectedPHP) {
        QTextCursor curs = textCursor();
        curs.movePosition(QTextCursor::StartOfBlock);
        if (cursorTextPos > 0) curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
        // detect type
        QString type = detectCompleteTypeAtCursorPHP(curs, nsName, clsName, funcName);
        if (type.size() > 0) {
            if (type[0] != "\\") type = "\\" + type;
            // class methods
            QString _text = type + "::" + text;
            for (auto & it : CW->phpClassMethodsComplete) {
                QString k = "\\"+QString::fromStdString(it.first);
                //if (k == _text) continue;
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
            if (completePopup->count() < completePopup->limit()) {
                // class props
                QString _text = type + "::$" + text;
                for (auto & it : CW->phpClassPropsComplete) {
                    QString k = "\\"+QString::fromStdString(it.first);
                    //if (k == _text) continue;
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
        }
    }
}

QString Editor::getFixedCompleteClassMethodName(QString clsMethodComplete, QString params)
{
    if (clsMethodComplete.indexOf(":") <= 0) return clsMethodComplete;
    QString cls = clsMethodComplete.mid(0, clsMethodComplete.indexOf(":"));
    QString func = clsMethodComplete.mid(cls.size()+2);
    CW->phpClassParentsIterator = CW->phpClassParents.find(cls.toStdString());
    if (CW->phpClassParentsIterator != CW->phpClassParents.end()) {
        QStringList parentsList = QString::fromStdString(CW->phpClassParentsIterator->second).split(",");
        for (QString _cls: parentsList) {
            QString _clsMethod = _cls + "::" + func;
            std::map<std::string, std::string>::iterator it = CW->phpClassMethodsComplete.find(_clsMethod.toStdString());
            if (it != CW->phpClassMethodsComplete.end() && QString::fromStdString(it->second) == params) {
                cls = _cls;
            } else {
                break;
            }
        }
    }
    return cls + "::" + func;
}

QString Editor::getFixedCompleteClassConstName(QString clsConstComplete)
{
    if (clsConstComplete.indexOf(":") <= 0) return clsConstComplete;
    QString cls = clsConstComplete.mid(0, clsConstComplete.indexOf(":"));
    QString cons = clsConstComplete.mid(cls.size()+2);
    CW->phpClassParentsIterator = CW->phpClassParents.find(cls.toStdString());
    if (CW->phpClassParentsIterator != CW->phpClassParents.end()) {
        QStringList parentsList = QString::fromStdString(CW->phpClassParentsIterator->second).split(",");
        for (QString _cls: parentsList) {
            QString _clsConst = _cls + "::" + cons;
            std::map<std::string, std::string>::iterator it = CW->phpClassConstsComplete.find(_clsConst.toStdString());
            if (it != CW->phpClassConstsComplete.end()) {
                cls = _cls;
            } else {
                break;
            }
        }
    }
    return cls + "::" + cons;
}

void Editor::detectParsOpenAtCursor(QTextCursor & curs)
{
    int pars = 0;
    while(curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor)) {
        QString blockText = curs.block().text();
        int pos = curs.positionInBlock();
        if (pos >= blockText.size()) continue;
        QChar prevChar = blockText[pos];
        if (prevChar == ")") {
            pars++;
        } else if (prevChar == "(") {
            pars--;
        }
        if (pars == 0) {
            break;
        }
    }
}

void Editor::detectParsCloseAtCursor(QTextCursor & curs)
{
    int pars = 0;
    while(curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) {
        QString blockText = curs.block().text();
        int pos = curs.positionInBlock();
        if (pos >= blockText.size()) continue;
        QChar nextChar = blockText[pos];
        if (nextChar == "(") {
            pars++;
        } else if (nextChar == ")") {
            pars--;
        }
        if (pars == 0) {
            break;
        }
    }
}

QString Editor::detectCompleteTypeAtCursorPHP(QTextCursor & curs, QString nsName, QString clsName, QString funcName)
{
    int cursorTextBlockNumber = curs.block().blockNumber();
    int cursorTextPos = curs.positionInBlock();
    QString prevType = "";
    if (!parsePHPEnabled) return prevType;
    // search begin of statement
    QChar prevChar = '\0';
    QChar prevPrevChar = '\0';
    QString prevWord = "", keyW = "";
    int keyWPos = -1;
    do {
        prevChar = findPrevCharNonSpaceAtCursos(curs);
        prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
        prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
        if (prevWord.toLower() == "return" || prevWord.toLower() == "else" || prevWord.toLower() == "echo") break;
        /*
        HW->phpwordsIterator = HW->phpwords.find(prevWord.toLower().toStdString());
        if (HW->phpwordsIterator != HW->phpwords.end()) break;
        */
        if (prevChar == ")") {
            detectParsOpenAtCursor(curs);
            prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
            if (prevWord.toLower() == "if" || prevWord.toLower() == "echo") break;
            /*
            HW->phpwordsIterator = HW->phpwords.find(prevWord.toLower().toStdString());
            if (HW->phpwordsIterator != HW->phpwords.end()) break;
            */
            if (prevWord.size() > 0) {
                keyW = prevWord;
                keyWPos = curs.position();
            }
            prevChar = findPrevCharNonSpaceAtCursos(curs);
            prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
            prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
        }
        if (prevWord.size() > 0) {
            keyW = prevWord;
            keyWPos = curs.position();
        }
        if (curs.positionInBlock() > 0) {
            QString blockText = curs.block().text();
            if (blockText[curs.positionInBlock()-1] == "\\") {
                prevType = completeClassNamePHPAtCursor(curs, prevWord, nsName);
                keyWPos += keyW.size();
                keyW = "";
                break;
            }
        }
    } while ((prevChar == ">" && prevPrevChar == "-") || (prevChar == ":" && prevPrevChar == ":"));
    if (keyW.size() > 0) keyWPos += keyW.size();
    if (keyW.size() > 0 && keyW[0] == "$") {
        if (keyW == "$this") {
            if (clsName != "anonymous class") {
                QString ns = "";
                if (nsName.size() > 0) ns = nsName + "\\";
                prevType = ns + clsName;
            }
        } else if (funcName != "anonymous function") {
            QString ns = "\\";
            if (nsName.size() > 0) ns += nsName + "\\";
            QString _clsName = clsName, _funcName = funcName;
            if (clsName.size() > 0) _clsName = ns + clsName;
            else if (funcName.size() > 0) _funcName = ns + funcName;
            for (int i=0; i<parseResultPHP.variables.size(); i++) {
                ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                if (_variable.name == keyW && _variable.clsName == _clsName && _variable.funcName == _funcName) {
                    // detect variable type
                    if (_variable.type.size() == 0 && _variable.line > 0 && _variable.line-1 < cursorTextBlockNumber) {
                        QTextCursor _curs = textCursor();
                        _curs.movePosition(QTextCursor::Start);
                        if (_variable.line > 1) _curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, _variable.line-1);
                        QString _blockText = _curs.block().text();
                        QRegularExpression r = QRegularExpression(QRegularExpression::escape(_variable.name)+"[\\s]*[=](.+?[)])[\\s]*[;]");
                        QRegularExpressionMatch m = r.match(_blockText);
                        QString _varType = "";
                        if (m.capturedStart(1) > 0) {
                            int _cursorTextPos = m.capturedStart(1)+m.capturedLength(1);
                            _curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, _cursorTextPos);
                            _varType = detectCompleteTypeAtCursorPHP(_curs, nsName, clsName, funcName);
                        } else {
                            QRegularExpression rr = QRegularExpression(QRegularExpression::escape(_variable.name)+"[\\s]*[=](.+?[)])");
                            QRegularExpressionMatch mm = rr.match(_blockText);
                            if (mm.capturedStart(1) > 0) {
                                int cp = -1;
                                while (_curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) {
                                    _blockText = _curs.block().text();
                                    cp = _blockText.indexOf(";");
                                    if (cp >= 0) break;
                                }
                                if (cp >= 0) {
                                    _curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cp);
                                    _varType = detectCompleteTypeAtCursorPHP(_curs, nsName, clsName, funcName);
                                }
                            }
                        }
                        if (_varType.size() == 0) _varType = "mixed";
                        _variable.type = "\\"+_varType;
                        parseResultPHP.variables.replace(i, _variable);
                    }
                    prevType = _variable.type;
                    break;
                }
            }
            // detect global variable
            if (prevType.size() == 0 && funcName.size() > 0) {
                QString varName = "";
                QString ns = "\\";
                if (nsName.size() > 0) ns += nsName + "\\";
                QString _clsName = clsName, _funcName = funcName;
                if (clsName.size() > 0) _clsName = ns + clsName;
                else if (funcName.size() > 0) _funcName = ns + funcName;
                for (int i=0; i<parseResultPHP.functions.size(); i++) {
                    ParsePHP::ParseResultFunction _func = parseResultPHP.functions.at(i);
                    if (_func.name == _funcName && _func.clsName == _clsName && _func.line > 0 && _func.line-1 != cursorTextBlockNumber) {
                        QTextCursor _curs = textCursor();
                        _curs.movePosition(QTextCursor::Start);
                        if (_func.line > 1) _curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, _func.line-1);
                        _curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
                        QString _blockText = _curs.block().text();
                        while(_blockText.trimmed().size()==0 || _blockText.trimmed()=="{") {
                            _curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
                            _blockText = _curs.block().text();
                        }
                        QRegularExpression r = QRegularExpression("global[\\s]+(.+?)[;]");
                        QRegularExpressionMatch m = r.match(_blockText);
                        if (m.capturedStart(1) > 0) {
                            QStringList varList = m.captured(1).split(",");
                            for (int i=0; i<varList.size(); i++) {
                                QString _varName = varList.at(i);
                                _varName = _varName.trimmed();
                                if (_varName == keyW) {
                                    varName = _varName;
                                }
                            }
                        }
                    }
                }
                if (varName.size() > 0) {
                    for (int i=0; i<parseResultPHP.variables.size(); i++) {
                        ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                        if (_variable.name == varName && _variable.clsName.size() == 0 && _variable.funcName.size() == 0) {
                            prevType = _variable.type;
                            break;
                        }
                    }
                }
            }
        }
    } else if (keyW.size() > 0 && keyW[0] != "$" && keyW.toLower() != "self" && keyW.toLower() != "static" && keyW.toLower() != "parent") {
        if (keyW.indexOf("\\") < 0) {
            CW->phpFunctionTypesIterator = CW->phpFunctionTypes.find(keyW.toStdString());
            if (CW->phpFunctionTypesIterator != CW->phpFunctionTypes.end()) {
                prevType = QString::fromStdString(CW->phpFunctionTypesIterator->second);
            } else {
                prevType = completeClassNamePHPAtCursor(curs, keyW, nsName);
            }
        } else {
            prevType = completeClassNamePHPAtCursor(curs, keyW, nsName);
        }
    } else if (keyW.size() > 0 && (keyW.toLower() == "self" || keyW.toLower() == "static")) {
        if (clsName != "anonymous class") {
            QString ns = "";
            if (nsName.size() > 0) ns = nsName + "\\";
            prevType = ns + clsName;
        }
    } else if (keyW.size() > 0 && keyW.toLower() == "parent") {
        if (clsName != "anonymous class") {
            QString ns = "";
            if (nsName.size() > 0) ns = nsName + "\\";
            for (int i=0; i<parseResultPHP.classes.size(); i++) {
                ParsePHP::ParseResultClass _cls = parseResultPHP.classes.at(i);
                if (_cls.name == "\\"+ns+clsName) {
                    QString parentClass = _cls.parent;
                    if (parentClass.size() > 0 && parentClass.at(0) == "\\") parentClass = parentClass.mid(1);
                    if (parentClass.size() > 0) {
                        prevType = parentClass;
                    }
                    break;
                }
            }
        }
    }
    if (prevType.size() > 0 && prevType[0] == "\\") prevType = prevType.mid(1);
    if (prevType.size() > 0) {
        // search end of statement
        if (keyWPos >= 0) curs.setPosition(keyWPos);
        QChar nextChar = findNextCharNonSpaceAtCursos(curs);
        QChar nextNextChar = findNextCharNonSpaceAtCursos(curs);
        QString nextWord = "";
        do {
            nextWord = findNextWordNonSpaceAtCursor(curs, MODE_PHP);
            nextChar = findNextCharNonSpaceAtCursos(curs);
            nextNextChar = findNextCharNonSpaceAtCursos(curs);
            if (nextWord.size() == 0) break;
            if ((curs.block().blockNumber() == cursorTextBlockNumber && curs.positionInBlock() >= cursorTextPos) || curs.block().blockNumber() > cursorTextBlockNumber) {
                break;
            }
            if (nextChar == "(") {
                // search function return type
                QString _funcName = prevType+"::"+nextWord;
                if (_funcName[0] == "\\") _funcName = _funcName.mid(1);
                CW->phpClassMethodTypesIterator = CW->phpClassMethodTypes.find(_funcName.toStdString());
                if (CW->phpClassMethodTypesIterator != CW->phpClassMethodTypes.end()) {
                    prevType = QString::fromStdString(CW->phpClassMethodTypesIterator->second);
                } else {
                    CW->phpClassParentsIterator = CW->phpClassParents.find(prevType.toStdString());
                    if (CW->phpClassParentsIterator != CW->phpClassParents.end()) {
                        QString parent = QString::fromStdString(CW->phpClassParentsIterator->second);
                        if (parent.size() > 0) {
                            QStringList parentList = parent.split(",");
                            for (int i=0; i<parentList.size(); i++) {
                                QString parentClass = parentList.at(i);
                                _funcName = parentClass + "::" + nextWord;
                                CW->phpClassMethodTypesIterator = CW->phpClassMethodTypes.find(_funcName.toStdString());
                                if (CW->phpClassMethodTypesIterator != CW->phpClassMethodTypes.end()) {
                                    prevType = QString::fromStdString(CW->phpClassMethodTypesIterator->second);
                                    break;
                                }
                            }
                        }
                    }
                }
                detectParsCloseAtCursor(curs);
                nextChar = findNextCharNonSpaceAtCursos(curs);
                nextNextChar = findNextCharNonSpaceAtCursos(curs);
            } else {
                // search variable type
                if (nextWord[0] != "$") nextWord = "$" + nextWord;
                QString _clsName = "\\" + prevType;
                for (int i=0; i<parseResultPHP.variables.size(); i++) {
                    ParsePHP::ParseResultVariable _variable = parseResultPHP.variables.at(i);
                    if (_variable.name == nextWord && _variable.clsName == _clsName && _variable.funcName.size() == 0) {
                        prevType = _variable.type;
                        break;
                    }
                }
            }
            if (prevType.size() == 0) break;
            if ((curs.block().blockNumber() == cursorTextBlockNumber && curs.positionInBlock() >= cursorTextPos) || curs.block().blockNumber() > cursorTextBlockNumber) {
                break;
            }
        } while ((nextChar == "-" && nextNextChar == ">") || (nextChar == ":" && nextNextChar == ":"));
    }
    return prevType;
}

void Editor::detectCompleteTextPHPNotFoundContext(QString text, QChar prevChar, QChar prevPrevChar)
{
    if (prevChar == ">" && prevPrevChar == "-") {
        // class methods
        QTextCursor curs = textCursor();
        if (text.size() > 0) curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, text.size());
        QTextBlock block = curs.block();
        int pos = curs.positionInBlock();
        QString nsName = highlight->findNsPHPAtCursor(& block, pos);
        QString clsName = highlight->findClsPHPAtCursor(& block, pos);
        QString funcName = highlight->findFuncPHPAtCursor(& block, pos);
        QString prevType = detectCompleteTypeAtCursorPHP(curs, nsName, clsName, funcName);
        std::unordered_map<std::string, std::string> addedClassMethods;
        std::unordered_map<std::string, std::string>::iterator addedClassMethodsIterator;
        if (prevType.size() > 0) {
            QString _text = prevType + "::" + text;
            for (auto & it : CW->phpClassMethodsComplete) {
                QString k = QString::fromStdString(it.first);
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    addedClassMethodsIterator = addedClassMethods.find(classMethodComplete.toStdString());
                    if (addedClassMethodsIterator != addedClassMethods.end()) continue;
                    addedClassMethods[classMethodComplete.toStdString()] = classMethodComplete.toStdString();
                    completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
            if (completePopup->count() < completePopup->limit()) {
                // class props
                QString _text = prevType + "::$" + text;
                for (auto & it : CW->phpClassPropsComplete) {
                    QString k = QString::fromStdString(it.first);
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
        }
        if (text.size() > 0) {
            QString _text = "::" + text;
            if (completePopup->count() < completePopup->limit()) {
                for (auto & it : CW->phpClassMethodsComplete) {
                    QString k = QString::fromStdString(it.first);
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)>0) {
                        if (prevType.size() > 0 && k.indexOf(prevType+"::", 0, Qt::CaseInsensitive)==0) continue;
                        //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        QString _clsName = k.mid(0, k.indexOf(":"));
                        QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        addedClassMethodsIterator = addedClassMethods.find(classMethodComplete.toStdString());
                        if (addedClassMethodsIterator != addedClassMethods.end()) continue;
                        addedClassMethods[classMethodComplete.toStdString()] = classMethodComplete.toStdString();
                        completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
            if (completePopup->count() < completePopup->limit()) {
                // class props
                QString _text = "::$" + text;
                for (auto & it : CW->phpClassPropsComplete) {
                    QString k = QString::fromStdString(it.first);
                    if (k.indexOf(_text, 0, Qt::CaseInsensitive)>0) {
                        if (prevType.size() > 0 && k.indexOf(prevType+"::", 0, Qt::CaseInsensitive)==0) continue;
                        completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                        if (completePopup->count() >= completePopup->limit()) break;
                    }
                }
            }
        }
    } else if (prevChar == ":" && prevPrevChar == ":" && text.size() > 0) {
        // class consts
        QString _text = text + "::";
        for (auto & it : CW->phpClassConstsComplete) {
            QString k = QString::fromStdString(it.first);
            if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                QString classConstComplete = getFixedCompleteClassConstName(QString::fromStdString(it.first));
                completePopup->addItem(classConstComplete, QString::fromStdString(it.second));
                if (completePopup->count() >= completePopup->limit()) break;
            }
        }
        if (completePopup->count() < completePopup->limit()) {
            // class methods
            QString _text = text + "::";
            for (auto & it : CW->phpClassMethodsComplete) {
                QString k = QString::fromStdString(it.first);
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    //completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    QString classMethodComplete = getFixedCompleteClassMethodName(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    completePopup->addItem(classMethodComplete, QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
        if (completePopup->count() < completePopup->limit()) {
            // class props
            QString _text = text+"::$";
            for (auto & it : CW->phpClassPropsComplete) {
                QString k = QString::fromStdString(it.first);
                if (k.indexOf(_text, 0, Qt::CaseInsensitive)==0) {
                    completePopup->addItem(QString::fromStdString(it.first), QString::fromStdString(it.second));
                    if (completePopup->count() >= completePopup->limit()) break;
                }
            }
        }
    }
}

void Editor::detectCompleteTextPHP(QString text, int cursorTextPos, QChar cursorTextPrevChar)
{
    QTextCursor curs = textCursor();
    curs.movePosition(QTextCursor::StartOfBlock);
    if (cursorTextPos > 0) curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
    QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
    QChar prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
    QString prevWord = findPrevWordNonSpaceAtCursor(curs, MODE_PHP);
    if (prevChar != ">" || prevPrevChar != "-") {
        detectCompleteTextPHPGlobalContext(text, cursorTextPos, prevChar, prevPrevChar, prevWord, curs, cursorTextPrevChar);
    } else if (prevChar == ">" && prevPrevChar == "-" && text[0] != "$") {
        detectCompleteTextPHPObjectContext(text, cursorTextPos, prevChar, prevPrevChar, prevWord, curs);
    }
}

void Editor::detectCompleteText(QString text, QChar cursorTextPrevChar, int cursorTextPos, std::string mode, int state)
{
    int min = 2;
    if (mode == MODE_HTML) min = 1;
    if (text.size() < min) return;
    completePopup->clearItems();

    if (mode == MODE_HTML) {
        detectCompleteTextHTML(text, cursorTextPrevChar, state);
    } else if (mode == MODE_CSS) {
        detectCompleteTextCSS(text, cursorTextPrevChar);
    } else if (mode == MODE_JS) {
        detectCompleteTextJS(text, cursorTextPos, cursorTextPrevChar);
    } else if (mode == MODE_PHP) {
        detectCompleteTextPHP(text, cursorTextPos, cursorTextPrevChar);
    }

    if (completePopup->count()>0) {
        completePopup->setTextStartPos(cursorTextPos);
        showCompletePopup();
    }
}

void Editor::detectCompleteTextRequest(QString text, int cursorTextPos, QChar prevChar, QChar prevPrevChar, std::string mode)
{
    completePopup->clearItems();

    if (mode == MODE_PHP) {
        detectCompleteTextPHPNotFoundContext(text, prevChar, prevPrevChar);
    }

    if (completePopup->count()>0) {
        completePopup->setTextStartPos(cursorTextPos);
        showCompletePopup();
    }
}

void Editor::completePopupSelected(QString text, QString data)
{
    if (text.size() == 0) return;
    QString origText = text;
    int cursorTextPos = completePopup->getTextStartPos();
    QTextCursor curs = textCursor();
    QTextBlock block = curs.block();
    int pos = curs.positionInBlock();
    QString blockText = block.text();
    int total = blockText.size();
    int startPos = curs.position();
    std::string mode = highlight->findModeAtCursor(& block, pos);
    int state = highlight->findStateAtCursor(& block, pos);
    QChar nextChar = '\0';
    if (pos < total) nextChar = blockText[pos];
    int moveCursorBack = 0;
    bool isSnippet = false;
    if (cursorTextPos >= 0 && cursorTextPos <= pos) {
        if (text.indexOf(SNIPPET_PREFIX) == 0) {
            // indent
            QString prefix = "";
            QTextCursor cursor = textCursor();
            QString blockText = cursor.block().text();
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            int pos = cursor.positionInBlock();
            int total = blockText.size();
            while(pos < total) {
                if (!cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) break;
                pos = cursor.positionInBlock();
                QChar prevChar = blockText[pos-1];
                if ((prevChar == " " || prevChar == "\t")) {
                    prefix += prevChar;
                } else {
                    break;
                }
            }
            QString indent = (tabType == "spaces") ? QString(" ").repeated(tabWidth) : "\t";
            text = data.replace("\\n","\n"+prefix).replace("\\t", indent);
            int cp = text.indexOf("{$cursor}");
            if (cp >= 0) {
                text.replace("{$cursor}","");
                moveCursorBack = text.size() - cp;
            }
            isSnippet = true;
        } else if (mode == MODE_PHP) {
            QTextCursor cursor = textCursor();
            if (cursorTextPos < pos) {
                cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, pos - cursorTextPos);
            }
            QChar prevChar = findPrevCharNonSpaceAtCursos(cursor);
            QChar prevPrevChar = findPrevCharNonSpaceAtCursos(cursor);
            QString prevWord = findPrevWordNonSpaceAtCursor(cursor, mode);
            if ((prevWord == "new" && prevChar != "\\") || (data == "\\"+text && prevChar != "\\")) {
                // new class
                QString nsName = highlight->findNsPHPAtCursor(& block, cursorTextPos);
                //QString clsName = highlight->findClsPHPAtCursor(& block, pos);
                //QString funcName = highlight->findFuncPHPAtCursor(& block, pos);
                if (nsName.size() > 0) {
                    QString clsName = text;
                    int p = text.lastIndexOf("\\");
                    if (p > 0 && p < text.size()-1) {
                        clsName = text.mid(p+1);
                    }
                    int line = -1;
                    // update parse result
                    parseResultPHPChanged(false);
                    // find imports
                    ParsePHP::ParseResultNamespace ns;
                    ParsePHP::ParseResultImport imp;
                    for (int i=0; i<parseResultPHP.namespaces.size(); i++) {
                        ParsePHP::ParseResultNamespace _ns = parseResultPHP.namespaces.at(i);
                        if (_ns.name == nsName) {
                            ns = _ns;
                            line = _ns.line;
                            break;
                        }
                    }
                    for (int c=0; c<ns.importsIndexes.size(); c++) {
                        if (parseResultPHP.imports.size() <= ns.importsIndexes.at(c)) break;
                        ParsePHP::ParseResultImport _imp = parseResultPHP.imports.at(ns.importsIndexes.at(c));
                        line = _imp.line;
                        if (_imp.type == IMPORT_TYPE_CLASS && _imp.name == clsName) {
                            imp = _imp;
                            break;
                        }
                    }
                    // import class
                    if (ns.name == nsName && line > 0 && imp.name != clsName && text != nsName + "\\" + clsName) {
                        QString insertImportText = "use "+text+";\n";
                        QTextCursor cursor = textCursor();
                        cursor.movePosition(QTextCursor::Start);
                        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
                        if (line == ns.line) {
                            QString blockText = cursor.block().text().trimmed();
                            if (blockText.size() == 0) {
                                cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
                                insertImportText += "\n";
                            }
                        }
                        blockSignals(true);
                        cursor.beginEditBlock();
                        cursor.insertText(insertImportText);
                        cursor.endEditBlock();
                        blockSignals(false);
                        text = clsName;
                        emit showPopupText(tabIndex, insertImportText);
                    } else if (ns.name == nsName && line > 0 && imp.name == clsName && imp.path != "\\"+text) {
                        text = "\\" + text;
                    } else if (ns.name == nsName && line > 0) {
                        text = clsName;
                    }
                }
            } else if (prevChar == ">" && prevPrevChar == "-") {
                // object method or prop
                if (text.indexOf("::") > 0) {
                    text = text.mid(text.indexOf("::")+2);
                    if (text[0] == "$") text = text.mid(1);
                }
            } else if (prevChar == ":" && prevPrevChar == ":") {
                if (text.indexOf("::") > 0) {
                    text = text.mid(text.indexOf("::")+2);
                }
            }
        } else if (mode == MODE_HTML) {
            QString tag = text;
            CW->htmlTagsIterator = CW->htmlTags.find(tag.toLower().toStdString());
            if (CW->htmlTagsIterator != CW->htmlTags.end()) {
                QChar prevChar = '\0';
                if (cursorTextPos - 1 >= 0) prevChar = blockText[cursorTextPos - 1];
                if (prevChar == '<') {
                    text += "></"+tag+">";
                    moveCursorBack = tag.size() + 3;
                } else if (prevChar == '/') {
                    text += ">";
                }
            } else if (state == STATE_TAG && text.toLower().indexOf("on") == 0) {
                text += "=\"\"";
                moveCursorBack = 1;
            }
        }
        if (cursorTextPos < pos) {
            if (isSnippet && cursorTextPos > 0) cursorTextPos--;
            curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, pos - cursorTextPos);
        }
        bool blockS = true;
        if (mode != MODE_HTML && state != STATE_NONE) blockS = false;
        if (mode == MODE_HTML && state != STATE_TAG) blockS = false;
        if (mode == MODE_UNKNOWN) blockS = false;
        if (blockS) blockSignals(true);
        curs.beginEditBlock();
        curs.insertText(text);
        // show tooltip
        QString tooltipText = "", tooltipOrigText = "";
        QStringList tooltipList;
        if (data.size() > 0 && data[0] == "(") {
            data.replace("<", "&lt;").replace(">", "&gt;");
            tooltipList = data.split(TOOLTIP_DELIMITER);
            if (tooltipList.size() > 1) {
                QString param = tooltipList.at(0);
                data = param.trimmed() + TOOLTIP_PAGER_TPL.arg(1).arg(tooltipList.size());
            }
        }
        if (data.size() > 0 && data[0] == "(" && !isSnippet) {
            if (nextChar == '\0' || (nextChar == ')' && blockText.count("(") == blockText.count(")"))) {
                curs.insertText("()");
                curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
            } else if (nextChar != "(") {
                curs.insertText("(");
            } else {
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
            }
            if (data != "()") {
                tooltipText = text + " " + data;
                tooltipOrigText = origText + " " + data;
            }
        } else if (moveCursorBack > 0) {
            curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, moveCursorBack);
        }
        curs.endEditBlock();
        if (blockS) blockSignals(false);
        setTextCursor(curs);
        if (tooltipText.size() > 0) {
            showTooltip(& curs, tooltipOrigText);
            tooltipSavedText = tooltipText;
            tooltipSavedList.clear();
            for (int i=0; i<tooltipList.size(); i++) {
                QString param = tooltipList.at(i);
                tooltipSavedList.append(text + " " + param.replace("<", "&lt;").replace(">", "&gt;").trimmed() + TOOLTIP_PAGER_TPL.arg(i+1).arg(tooltipList.size()));
            }
            tooltipSavedPageOffset = 0;
            tooltipSavedOrigName = origText;
            tooltipSavedBlockNumber = curs.block().blockNumber();
        }
        if (isSnippet) {
            int co = text.count("\n");
            QTextCursor cursor = textCursor();
            cursor.setPosition(startPos);
            int b = 0;
            do {
                b++;
                QTextBlock block = cursor.block();
                highlight->rehighlightBlock(block);
                if (b > co) break;
            } while(cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
        }
    }
    hideCompletePopup();
}

void Editor::parseResultChanged()
{
    std::string modeType = highlight->getModeType();
    if (modeType == MODE_MIXED) {
        parseResultPHPChanged();
    } else if (modeType == MODE_JS) {
        parseResultJSChanged();
    } else if (modeType == MODE_CSS) {
        parseResultCSSChanged();
    }
}

void Editor::parseResultPHPChanged(bool async)
{
    if (!parsePHPEnabled) return;
    QTextCursor curs = textCursor();
    QTextBlock block = curs.block();
    int pos = curs.positionInBlock();
    std::string mode = highlight->findModeAtCursor(& block, pos);
    if (mode != MODE_PHP) return;
    QString content = getContent();
    if (!async) parseResultPHP = parserPHP.parse(content);
    else emit parsePHP(getTabIndex(), content);
}

void Editor::parseResultJSChanged(bool async)
{
    if (!parseJSEnabled) return;
    QString content = getContent();
    if (!async) parseResultJS = parserJS.parse(content);
    else emit parseJS(getTabIndex(), content);
}

void Editor::parseResultCSSChanged(bool async)
{
    if (!parseCSSEnabled) return;
    QString content = getContent();
    if (!async) parseResultCSS = parserCSS.parse(content);
    else emit parseCSS(getTabIndex(), content);
}

void Editor::tooltip(int offset)
{
    if (!focused) return;
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int cursPos = cursor.positionInBlock();
    QString blockText = cursor.block().text();
    if (highlight->isStateOpen(&block, cursPos)) return;
    std::string mode = highlight->findModeAtCursor(&block, cursPos);
    if (mode != MODE_PHP && mode != MODE_JS) return;
    // show complete popup
    QTextCursor curs = textCursor();
    QString prevText = "";
    QChar prevChar = '\0', prevPrevChar = '\0';
    int cursorTextPos = cursPos;
    if (cursPos > 1 && blockText[cursPos-1] == ":" && blockText[cursPos-2] == ":") {
        prevChar = ':'; prevPrevChar = ':';
        cursPos -= 2;
        while (cursPos > 0) {
            QChar _prevChar = blockText[cursPos-1];
            if (!_prevChar.isSpace()) break;
            cursPos--;
        }
    }
    if (cursPos > 0 && cursor.selectedText().size()==0) {
        // text till cursor
        QChar cursorTextPrevChar = '\0';
        for (int i=cursPos; i>0; i--) {
            QString c = blockText.mid(i-1, 1);
            cursorTextPrevChar = c[0];
            if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") prevText = c + prevText;
            else break;
            cursorTextPos = i-1;
        }
        if ((prevText.size() > 0 && prevText[0] != "$") || prevText.size() == 0) {
            curs.movePosition(QTextCursor::StartOfBlock);
            if (cursorTextPos > 0) curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
            if (prevChar == '\0' && prevPrevChar == '\0') {
                prevChar = findPrevCharNonSpaceAtCursos(curs);
                prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
            }
        }
    }
    if ((prevChar == ">" && prevPrevChar == "-") || (prevChar == ":" && prevPrevChar == ":")) {
        if (prevChar == ":" && prevPrevChar == ":") {
            QString nsName = highlight->findNsPHPAtCursor(& block, cursPos);
            QString clsName = highlight->findClsPHPAtCursor(& block, cursPos);
            if ((prevText.toLower() == "self" || prevText.toLower() == "static") && clsName.size() > 0) {
                prevText = nsName.size() > 0 ? nsName + "\\" + clsName : clsName;
            } else if (prevText.toLower() == "parent" && clsName.size() > 0) {
                QString ns = "";
                if (nsName.size() > 0) ns = nsName + "\\";
                for (int i=0; i<parseResultPHP.classes.size(); i++) {
                    ParsePHP::ParseResultClass _cls = parseResultPHP.classes.at(i);
                    if (_cls.name == "\\"+ns+clsName) {
                        QString parentClass = _cls.parent;
                        if (parentClass.size() > 0 && parentClass.at(0) == "\\") parentClass = parentClass.mid(1);
                        if (parentClass.size() > 0) {
                            prevText = parentClass;
                        }
                        break;
                    }
                }
            } else {
                prevText = completeClassNamePHPAtCursor(curs, prevText, nsName);
            }
            cursorTextPos = cursor.positionInBlock();
        }
        detectCompleteTextRequest(prevText, cursorTextPos, prevChar, prevPrevChar, mode);
        return;
    }
    // show tooltip
    int tnOffset = 0, thCaptured = -1, tooltipStart = -1;
    QString tooltipName = "";
    do {
        do {
            QRegularExpressionMatch tnMatch = classNameExpr.match(blockText, tnOffset);
            thCaptured = tnMatch.capturedStart();
            int tnLength = tnMatch.capturedLength();
            if (thCaptured >= 0) {
                if (cursPos >= 0 && thCaptured >= cursPos) break;
                if (tnMatch.captured(1) != "array") {
                    tooltipStart = thCaptured;
                    tooltipName = tnMatch.captured(1);
                }
                tnOffset = thCaptured + tnLength;
            }
        } while (thCaptured >= 0);
        cursPos = -1;
        tnOffset = 0;
    } while(tooltipStart < 0 && cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor));
    if (tooltipName.size()>0) {
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, tooltipStart);
        int cursorP = cursor.position();
        QChar prevChar = findPrevCharNonSpaceAtCursos(cursor);
        QChar prevPrevChar = findPrevCharNonSpaceAtCursos(cursor);
        if (tooltipName[0] == "\\") tooltipName = tooltipName.mid(1);
        CW->tooltipsIteratorPHP = CW->tooltipsPHP.find(tooltipName.toStdString());
        if (CW->tooltipsIteratorPHP != CW->tooltipsPHP.end() && (prevChar != ">" || prevPrevChar != "-") && (prevChar != ":" || prevPrevChar != ":") && prevChar != "$") {
            QString fName = QString::fromStdString(CW->tooltipsIteratorPHP->first);
            QString params = QString::fromStdString(CW->tooltipsIteratorPHP->second);
            params.replace("<", "&lt;").replace(">", "&gt;");
            if (tooltipSavedOrigName != fName || tooltipSavedPageOffset < 0 || offset > 0) {
                tooltipSavedPageOffset = offset;
            } else {
                offset = tooltipSavedPageOffset;
            }
            QStringList paramsList;
            if (params.size() > 0 && params[0] == "(") {
                paramsList = params.split(TOOLTIP_DELIMITER);
                if (offset < 0 || offset >= paramsList.size()) offset = 0;
                if (paramsList.size() > 1) {
                    QString param = paramsList.at(offset);
                    params = param.trimmed() + TOOLTIP_PAGER_TPL.arg(offset+1).arg(paramsList.size());
                }
            }
            tooltipSavedText = fName + " " + params;
            tooltipSavedList.clear();
            for (int i=0; i<paramsList.size(); i++) {
                QString param = paramsList.at(i);
                tooltipSavedList.append(fName + " " + param.replace("<", "&lt;").replace(">", "&gt;").trimmed() + TOOLTIP_PAGER_TPL.arg(i+1).arg(paramsList.size()));
            }
            tooltipSavedOrigName = fName;
            tooltipSavedBlockNumber = cursor.block().blockNumber();
            showTooltip(& cursor, tooltipSavedText);
            followTooltip();
        } else if (tooltipSavedText.size() > 0 && tooltipSavedBlockNumber == cursor.block().blockNumber()) {
            QString fName = "", params = "";
            int kSep = tooltipSavedText.indexOf("(");
            if (kSep > 0) {
                fName = tooltipSavedText.mid(0, kSep).trimmed();
                params = tooltipSavedText.mid(kSep).trimmed();
            }
            if (fName == tooltipName) {
                QString tooltipText = tooltipSavedOrigName + " " + params;
                showTooltip(& cursor, tooltipText);
                followTooltip();
            }
        } else if ((prevChar == ">" && prevPrevChar == "-") || (prevChar == ":" && prevPrevChar == ":")) {
            cursor.setPosition(cursorP);
            QString nsName = highlight->findNsPHPAtCursor(& block, cursor.positionInBlock());
            QString clsName = highlight->findClsPHPAtCursor(& block, cursor.positionInBlock());
            QString funcName = highlight->findFuncPHPAtCursor(& block, cursor.positionInBlock());
            QString prevType = detectCompleteTypeAtCursorPHP(cursor, nsName, clsName, funcName);
            if (prevType.size() > 0 && prevType.at(0) == "\\") prevType = prevType.mid(1);
            if (prevType.size() > 0) {
                CW->tooltipsIteratorPHP = CW->tooltipsPHP.find(prevType.toStdString()+"::"+tooltipName.toStdString());
                if (CW->tooltipsIteratorPHP != CW->tooltipsPHP.end()) {
                    QString fName = QString::fromStdString(CW->tooltipsIteratorPHP->first);
                    QString params = QString::fromStdString(CW->tooltipsIteratorPHP->second);
                    fName = getFixedCompleteClassMethodName(fName, params);
                    params.replace("<", "&lt;").replace(">", "&gt;");
                    QStringList paramsList;
                    if (params.size() > 0 && params[0] == "(") {
                        paramsList = params.split(TOOLTIP_DELIMITER);
                        if (offset < 0 || offset >= paramsList.size()) offset = 0;
                        if (paramsList.size() > 1) {
                            QString param = paramsList.at(offset);
                            params = param.trimmed() + TOOLTIP_PAGER_TPL.arg(offset+1).arg(paramsList.size());
                        }
                    }
                    showTooltip(& cursor, fName + " " + params);
                }
            }
        }
    }
}

void Editor::followTooltip()
{
    if (tooltipLabel.isVisible() && tooltipSavedText.size() > 0) {
        QRegularExpressionMatch tMatch = functionParamsExpr.match(tooltipSavedText);
        if (tMatch.capturedStart() >= 0 && tMatch.captured(2).trimmed().size() > 0) {
            bool isVoid = tMatch.captured(2).trimmed() == "void";
            QString tooltipName = tMatch.captured(1);
            QStringList tooltipTextList = tMatch.captured(2).split(",");
            QString tooltipEnd = tMatch.captured(3);
            QTextCursor cursor = textCursor();
            int blockNumberStart = cursor.block().blockNumber();
            int cursPosStart = cursor.positionInBlock();
            int cursPos = cursor.positionInBlock();
            QRegularExpression tnExp("\\b"+QRegularExpression::escape(tooltipName.trimmed())+"\\b[\\s]*[(]", QRegularExpression::CaseInsensitiveOption);
            int tnOffset = 0, thCaptured = -1, tooltipStart = -1;
            do {
                do {
                    QRegularExpressionMatch tnMatch = tnExp.match(cursor.block().text(), tnOffset);
                    thCaptured = tnMatch.capturedStart();
                    int tnLength = tnMatch.capturedLength();
                    if (thCaptured >= 0) {
                        if (cursPos >= 0 && thCaptured + tnLength > cursPos) break;
                        tooltipStart = thCaptured;
                        tnOffset = thCaptured + tnLength;
                    }
                } while (thCaptured >= 0);
                cursPos = -1;
                tnOffset = 0;
            } while(tooltipStart < 0 && cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor));
            int tooltipOffset = 0;
            if (tooltipStart >= 0) {
                bool strSQOpen = false, strDQOpen = false;
                int pCo = 0;
                int blockNumber = cursor.block().blockNumber();
                QString blockText = cursor.block().text();
                cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, tooltipStart);
                while(cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor)) {
                    if (blockNumber != cursor.block().blockNumber()) {
                        blockNumber = cursor.block().blockNumber();
                        blockText = cursor.block().text();
                    }
                    QChar c = '\0';
                    int pos = cursor.positionInBlock();
                    if (blockNumber == blockNumberStart && pos > cursPosStart) break;
                    if (pos > 0) c = blockText[pos-1];
                    if (c == "'" && !strDQOpen) strSQOpen = !strSQOpen;
                    if (c == "\"" && !strSQOpen) strDQOpen = !strDQOpen;
                    if (c == ")" && !strSQOpen && !strDQOpen) pCo--;
                    if (c == "(" && !strSQOpen && !strDQOpen) pCo++;
                    if (c == "," && pCo == 1 && !strSQOpen && !strDQOpen) tooltipOffset++;
                    if ((c == ")" && pCo <= 0 && !strSQOpen && !strDQOpen) || c == ";" || c == "{" || c == "}") {
                        tooltipOffset = -1;
                        break;
                    }
                    if (blockNumber == blockNumberStart && pos >= blockText.size()) break;
                }
                if (tooltipOffset >= 0 && !isVoid) {
                    QString newText = tooltipSavedOrigName + " (";
                    for (int i=0; i<tooltipTextList.size(); i++) {
                        if (i > 0) newText += ",";
                        QString part = tooltipTextList.at(i);
                        if (i == tooltipOffset) {
                            part = tooltipBoldTagStart + part + tooltipBoldTagEnd;
                        }
                        newText += part;
                    }
                    newText += ")" + tooltipEnd;
                    tooltipLabel.setText(newText);
                } else if (tooltipOffset < 0) {
                    hideTooltip();
                }
            } else {
                hideTooltip();
            }
        }
    }
}

void Editor::cursorPositionChanged()
{
    if (!is_ready) return;
    // highlight current line
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        highlightCurrentLine(& extraSelections);
    }
    setExtraSelections(extraSelections);
    lineAnnotation->hide();
    if (!cursorPositionChangeLocked) {
        cursorPositionChangeLocked = true;
        QTimer::singleShot(INTERVAL_CURSOR_POS_CHANGED_MILLISECONDS, this, SLOT(cursorPositionChangedDelayed()));
    }
    // save cursor position
    QTextCursor curs = textCursor();
    int cursorPositionBlockNumber = curs.blockNumber();
    if (cursorPositionBlockNumber != lastCursorPositionBlockNumber) {
        lastCursorPositionBlockNumber = cursorPositionBlockNumber;
        backPositions.append(curs.position());
        if (backPositions.size() > 10) backPositions.removeFirst();
        forwardPositions.clear();
        emit backForwardChanged(getTabIndex());
    }
}

void Editor::cursorPositionChangedDelayed()
{
    cursorPositionChangeLocked = false;
    clearTextHoverFormat();
    // highlight word
    QTextCursor curs = textCursor();
    QTextBlock block = curs.block();
    QString blockText = block.text();
    int total = blockText.size();
    int pos = curs.positionInBlock();
    QChar prevChar = '\0', nextChar = '\0';
    if (pos > 0 && curs.selectedText().size()==0) prevChar = blockText[pos - 1];
    if (pos < total && curs.selectedText().size()==0) nextChar = blockText[pos];
    // text under cursor
    QString cursorText = "", prevText = "", nextText = "";
    bool hasAlpha = false;
    std::string mode = highlight->findModeAtCursor(&block, pos);
    QChar cursorTextPrevChar = '\0';
    int cursorTextPos = pos;
    if (curs.selectedText().size()==0) {
        for (int i=pos; i>0; i--) {
            QString c = blockText.mid(i-1, 1);
            if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
            cursorTextPrevChar = c[0];
            if (mode == MODE_PHP || mode == MODE_JS) {
                if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") prevText = c + prevText;
                else break;
            } else if (mode == MODE_CSS) {
                if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") prevText = c + prevText;
                else break;
            } else {
                if (isalnum(c[0].toLatin1()) || c=="_") prevText = c + prevText;
                else break;
            }
            cursorTextPos = i-1;
        }
        for (int i=pos; i<total; i++) {
            QString c = blockText.mid(i, 1);
            if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
            if (mode == MODE_PHP || mode == MODE_JS) {
                if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") nextText += c;
                else break;
            } else if (mode == MODE_CSS) {
                if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") nextText += c;
                else break;
            } else {
                if (isalnum(c[0].toLatin1()) || c=="_") nextText += c;
                else break;
            }
        }
        if (mode == MODE_PHP && prevText.size()==0 && nextText.size()>0 && hasAlpha &&
            (nextText == "if" || nextText == "else" || nextText == "endif" || nextText == "switch" || nextText == "case" || nextText == "endswitch" || nextText == "while" || nextText == "endwhile" || nextText == "for" || nextText == "endfor" || nextText == "foreach" || nextText == "endforeach")
        ) {
            cursorText = prevText + nextText;
        } else if (mode == MODE_PHP && prevText.size()>0 && nextText.size()==0 && hasAlpha &&
            (prevText == "endif" || prevText == "endswitch" || prevText == "endwhile" || prevText == "endfor" || prevText == "endforeach")
        ) {
            cursorText = prevText + nextText;
        } else if ((cursorTextPrevChar == "<" || cursorTextPrevChar == "/") && (prevText.size()>0 || nextText.size()>0) && hasAlpha) {
            cursorText = prevText + nextText;
        } else if (prevText.size()>0 && nextText.size()>0 && hasAlpha) {
            cursorText = prevText + nextText;
        }
    } else {
        cursorText = curs.selectedText();
    }
    highlightExtras(prevChar, nextChar, cursorTextPrevChar, cursorText, cursorTextPos, mode);
    if (showBreadcrumbs) {
        QString scopeName = "";
        if (mode == MODE_PHP) {
            followTooltip();
            //QString nsName = highlight->findNsPHPAtCursor(& block, pos);
            QString clsName = highlight->findClsPHPAtCursor(& block, pos);
            QString funcName = highlight->findFuncPHPAtCursor(& block, pos);
            if (funcName == "anonymous function") clsName = "";
            else if (funcName.size() > 0 && clsName == "anonymous class") funcName = "";
            //if (nsName.size() > 0) scopeName += nsName + BREADCRUMBS_DELIMITER;
            if (clsName.size() > 0) scopeName += clsName + BREADCRUMBS_DELIMITER;
            if (funcName.size() > 0) scopeName += funcName;
        } else if (mode == MODE_JS) {
            followTooltip();
            scopeName += highlight->findFuncJSAtCursor(& block, pos);
        } else if (mode == MODE_CSS) {
            QString mediaName = highlight->findMediaCSSAtCursor(& block, pos);
            if (mediaName.size() > 0) {
                scopeName += "@media ( " + mediaName.trimmed() + " )";
            }
        } else if (mode == MODE_HTML) {
            QString tagChain = highlight->findTagChainHTMLAtCursor(& block, pos);
            if (tagChain.size() > 0) {
                scopeName += tagChain.replace(",", BREADCRUMBS_DELIMITER);
            }
        } else {
            scopeName = "";
        }
        setBreadcrumbsText(scopeName.trimmed());
    }
    showLineAnnotation();
    if (isReady()) emit statusBarText(tabIndex, ""); // update status bar
}

int Editor::findFirstVisibleBlockIndex()
{
    if (verticalScrollBar()->sliderPosition() <= FIRST_BLOCK_BIN_SEARCH_SCROLL_VALUE) {
        return searchFirstVisibleBlockIndexLinear();
    } else {
        return searchFirstVisibleBlockIndexBinary();
    }
}

int Editor::searchFirstVisibleBlockIndexLinear()
{
    int top = contentsMargins().top();
    QTextCursor curs = QTextCursor(document());
    curs.movePosition(QTextCursor::Start);
    for(int i = 0; i < document()->blockCount(); i++)
    {
        QTextBlock block = curs.block();
        int block_y = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).y());
        if (verticalScrollBar()->sliderPosition()<=block_y) {
            return i;
        }
        curs.movePosition(QTextCursor::NextBlock);
    }
    return 0;
}

int Editor::searchFirstVisibleBlockIndexBinary()
{
    int top = contentsMargins().top();
    int total = document()->blockCount();
    if (total < 2) return 0;
    int offsetLeft = 0, offsetRight = total;
    int blockNumber = 0;
    do {
        blockNumber = offsetLeft + (offsetRight - offsetLeft) / 2;
        QTextBlock block = document()->findBlockByNumber(blockNumber);
        int block_y = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).y());
        if (verticalScrollBar()->sliderPosition()>block_y) {
            offsetLeft = blockNumber;
            blockNumber++; // block is partly visible ?
            if (offsetLeft + 1 >= offsetRight) break;
        } else if (verticalScrollBar()->sliderPosition()<block_y) {
            offsetRight = blockNumber;
        } else {
            break;
        }
    } while (offsetLeft < offsetRight);
    return blockNumber;
}

int Editor::findLastVisibleBlockIndex()
{
    int top = contentsMargins().top();
    int bottom = contentsMargins().bottom();
    int first = getFirstVisibleBlockIndex();
    QTextCursor curs = QTextCursor(document());
    curs.movePosition(QTextCursor::Start);
    if (first > 0) curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, first);
    for(int i = first; i < document()->blockCount(); i++)
    {
        QTextBlock block = curs.block();
        int block_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).y());
        int block_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        if (viewport()->geometry().height()+verticalScrollBar()->sliderPosition()-top-bottom<block_y+block_h) {
            if (i > 0) i--; // block is partly visible ?
            return i;
        } else if (viewport()->geometry().height()+verticalScrollBar()->sliderPosition()-top-bottom==block_y+block_h) {
            return i;
        }
        curs.movePosition(QTextCursor::NextBlock);
    }
    return document()->blockCount()-1;
}

int Editor::getFirstVisibleBlockIndex()
{
    if (firstVisibleBlockIndex < 0) {
        firstVisibleBlockIndex = findFirstVisibleBlockIndex();
    }
    return firstVisibleBlockIndex;
}

int Editor::getLastVisibleBlockIndex()
{
    if (lastVisibleBlockIndex < 0) {
        lastVisibleBlockIndex = findLastVisibleBlockIndex();
    }
    return lastVisibleBlockIndex;
}

void Editor::setBreadcrumbsText(QString text) {
    static_cast<Breadcrumbs *>(breadcrumbs)->setText(text);
    breadcrumbs->update();
}

void Editor::breadcrumbsPaintEvent(QPaintEvent *event)
{
    int errorsCount = static_cast<LineMark *>(lineMark)->getErrorsCount();
    int warningsCount = static_cast<LineMark *>(lineMark)->getWarningsCount();

    QPainter painter(breadcrumbs);
    if (errorsCount > 0) {
        painter.fillRect(event->rect(), breadcrumbsErrorBgColor);
    } else if (warningsCount > 0) {
        painter.fillRect(event->rect(), breadcrumbsWarningBgColor);
    } else {
        painter.fillRect(event->rect(), breadcrumbsBgColor);
    }

    painter.setPen(widgetBorderColor);
    painter.drawLine(0, breadcrumbs->height()-1, breadcrumbs->width(), breadcrumbs->height()-1);

    QFontMetrics fm(editorBreadcrumbsFont);
    int lineW = lineNumberAreaWidth();
    int markW = lineMarkAreaWidth();
    int offset = (breadcrumbs->height()-fm.height())/2;
    if (offset < 0) offset = 0;

    bool pretty = true;
    painter.setPen(breadcrumbsColor);
    QString text = static_cast<Breadcrumbs *>(breadcrumbs)->getText();
    if (pretty) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QPixmap delimiter(":/icons/separator.png");
        int listOffset = 0;
        /*
        int listMargin = 10;
        int lineSize = 2;
        */
        QStringList textList = text.split(BREADCRUMBS_DELIMITER.trimmed());
        for (int i=0; i<textList.size(); i++) {
            QString _text = textList.at(i);
            _text = _text.trimmed();
            if (_text.size() == 0) continue;
            /* QFontMetrics::width is deprecated */
            /*
            int width = fm.width(_text);
            */
            int width = fm.horizontalAdvance(_text);
            painter.drawText(lineW+markW+listOffset, offset, width, fm.height(), Qt::AlignLeft, _text);
            /*
            listOffset += width+listMargin-listMargin/4;
            QPointF p1(lineW+markW+listOffset, breadcrumbs->height()/4);
            QPointF p2(lineW+markW+listOffset+listMargin/2, breadcrumbs->height()/2);
            QPointF p3(lineW+markW+listOffset, breadcrumbs->height()-breadcrumbs->height()/4-1);
            painter.drawLine(p1, p2);
            painter.drawLine(p2, p3);
            listOffset += listMargin+listMargin/4+lineSize;
            */
            listOffset += width;
            painter.drawPixmap(lineW+markW+listOffset, 0, breadcrumbs->height(), breadcrumbs->height(), delimiter);
            listOffset += breadcrumbs->height();
        }
    } else {
        painter.drawText(lineW+markW, offset, breadcrumbs->width(), fm.height(), Qt::AlignLeft, text);
    }
}

void Editor::searchPaintEvent(QPaintEvent *event)
{
    QPainter painter(search);
    painter.fillRect(event->rect(), searchBgColor);

    painter.setPen(widgetBorderColor);
    painter.drawLine(0, 0, search->width(), 0);
}

void Editor::lineMarkAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineMark);
    painter.fillRect(event->rect(), lineMarkBgColor);
    int markW = lineMarkAreaWidth();
    QFontMetrics fm(editorFont);

    int blockNumber = getFirstVisibleBlockIndex();
    if (blockNumber < 0) return;
    if (blockNumber>0) blockNumber--;

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    //int top = contentsMargins().top();
    int top = 0; // widget has offset

    if (blockNumber == 0) {
        top += static_cast<int>(document()->documentMargin()) - verticalScrollBar()->sliderPosition();
    } else {
        QTextBlock prev_block = document()->findBlockByNumber(blockNumber-1);
        int prev_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).y());
        int prev_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).height());
        top += prev_y + prev_h - verticalScrollBar()->sliderPosition();
    }

    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible()) {
            int line = blockNumber + 1;
            if (gitDiffLines.size() > 0 && gitDiffLines.contains(line)) {
                Git::DiffLine mLine = gitDiffLines.value(line);
                if (mLine.isModified) {
                    painter.fillRect(0, top, markW, bottom - top, lineNumberModifiedBgColor);
                }
                if (mLine.isDeleted) {
                    painter.fillRect(0, top, markW, 1, lineNumberDeletedBorderColor);
                }
            }
            QString errorText = "", warningText = "", markText = "";
            int error = static_cast<LineMark *>(lineMark)->getError(line, errorText);
            int warning = static_cast<LineMark *>(lineMark)->getWarning(line, warningText);
            int mark = static_cast<LineMark *>(lineMark)->getMark(line, markText);
            if (error > 0 || warning > 0 || mark > 0) {
                if (error > 0) {
                    painter.fillRect(markW-LINE_MARK_WIDGET_RECT_WIDTH, top, LINE_MARK_WIDGET_RECT_WIDTH, fm.height(), lineErrorRectColor);
                } else if (warning > 0) {
                    painter.fillRect(markW-LINE_MARK_WIDGET_RECT_WIDTH, top, LINE_MARK_WIDGET_RECT_WIDTH, fm.height(), lineWarningRectColor);
                }
                if (mark > 0) {
                    painter.fillRect(markW-LINE_MARK_WIDGET_LINE_WIDTH, top, LINE_MARK_WIDGET_LINE_WIDTH, bottom-top, lineMarkRectColor);
                }
            }
            int markRectWidth = LINE_MARK_WIDGET_RECT_WIDTH;
            if (error >= 0 || warning >= 0) markRectWidth /= 2;
            markPointsIterator = markPoints.find(line);
            if (markPointsIterator != markPoints.end()) {
                painter.fillRect(markW-markRectWidth, top, markRectWidth, fm.height(), lineMarkRectColor);
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        blockNumber++;
    }
}

void Editor::lineMapAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineMap);
    painter.fillRect(event->rect(), lineMapBgColor);
    QVector<int> marks = static_cast<LineMap *>(lineMap)->getMarks();
    QVector<int> warnings = static_cast<LineMap *>(lineMap)->getWarnings();
    QVector<int> errors = static_cast<LineMap *>(lineMap)->getErrors();
    int mapW = lineMapAreaWidth();
    int height = lineMap->geometry().height();
    if (verticalScrollBar()->isVisible() && height > verticalScrollBar()->geometry().height()) {
        height = verticalScrollBar()->geometry().height();
    }
    if (verticalScrollBar()->isVisible()){
        int sHeight = verticalScrollBar()->value() * height / verticalScrollBar()->maximum();
        painter.fillRect(0, 0, mapW, height, lineMapScrollAreaBgColor);
        painter.fillRect(0, 0, mapW, sHeight, lineMapScrollBgColor);
    }
    int lines = qMax(1, document()->blockCount());
    int offset = (height / lines) / 2;
    painter.setPen(lineMarkColor);
    for (int i=0; i<marks.size(); i++){
        int top = marks.at(i) * height / lines;
        if (top > height) continue;
        top -= offset;
        if (top < 1) top = 1;
        if (top > height-1) top = height-1;
        painter.drawLine(QPoint(0, top), QPoint(mapW, top));
    }
    for (auto & iterator : markPoints) {
        int top = iterator.first * height / lines;
        if (top > height) continue;
        top -= offset;
        if (top < 1) top = 1;
        if (top > height-1) top = height-1;
        painter.drawLine(QPoint(0, top), QPoint(mapW, top));
    }
    for (auto & iterator: modifiedLines) {
        int top = iterator.first * height / lines;
        if (top > height) continue;
        top -= offset;
        if (top < 1) top = 1;
        if (top > height-1) top = height-1;
        painter.drawLine(QPoint(mapW / 2, top), QPoint(mapW, top));
    }
    painter.setPen(lineWarningColor);
    for (int i=0; i<warnings.size(); i++){
        int top = warnings.at(i) * height / lines;
        if (top > height) continue;
        top -= offset;
        if (top < 1) top = 1;
        if (top > height-1) top = height-1;
        painter.drawLine(QPoint(0, top), QPoint(mapW, top));
    }
    painter.setPen(lineErrorColor);
    for (int i=0; i<errors.size(); i++){
        int top = errors.at(i) * height / lines;
        if (top > height) continue;
        top -= offset;
        if (top < 1) top = 1;
        if (top > height-1) top = height-1;
        painter.drawLine(QPoint(0, top), QPoint(mapW, top));
    }
    // draw highlight progress
    if (highlightProgressPercent > 0 && highlightProgressPercent < 100){
        int pHeight = highlightProgressPercent * height / 100;
        painter.fillRect(0, 0, LINE_MAP_PROGRESS_WIDTH, pHeight, progressColor);
    }
    // draw spell progress
    if (spellProgressPercent > 0 && spellProgressPercent < 100){
        int p = spellProgressPercent * height / 100;
        int pHeight = LINE_MAP_PROGRESS_HEIGHT;
        int y = p - (pHeight / 2);
        if (y < 0) y = 0;
        if (y + pHeight > height) pHeight = height - y;
        painter.fillRect(0, y, LINE_MAP_PROGRESS_WIDTH, pHeight, progressColor);
    }
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumber);
    painter.fillRect(event->rect(), lineNumberBgColor);

    int blockNumber = getFirstVisibleBlockIndex();
    if (blockNumber < 0) return;
    if (blockNumber>0) blockNumber--;

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    QFontMetrics fm(editorFont);
    //int top = contentsMargins().top();
    int top = 0; // widget has offset

    if (blockNumber == 0) {
        top += static_cast<int>(document()->documentMargin()) - verticalScrollBar()->sliderPosition();
    } else {
        QTextBlock prev_block = document()->findBlockByNumber(blockNumber-1);
        int prev_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).y());
        int prev_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).height());
        top += prev_y + prev_h - verticalScrollBar()->sliderPosition();
    }

    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible()) {
            QString number = QString::number(blockNumber + 1);
            if (gitDiffLines.size() > 0 && gitDiffLines.contains(blockNumber + 1)) {
                Git::DiffLine mLine = gitDiffLines.value(blockNumber + 1);
                if (mLine.isModified) {
                    painter.fillRect(0, top, lineNumber->width(), bottom - top, lineNumberModifiedBgColor);
                    painter.setPen(lineNumberModifiedColor);
                } else {
                    painter.setPen(lineNumberColor);
                }
                if (mLine.isDeleted) {
                    painter.fillRect(0, top, lineNumber->width(), 1, lineNumberDeletedBorderColor);
                }
            } else {
                modifiedLinesIterator = modifiedLines.find(blockNumber + 1);
                if (modifiedLinesIterator != modifiedLines.end()) {
                    painter.fillRect(0, top, LINE_NUMBER_WIDGET_PADDING / 2, bottom - top, lineNumberModifiedBgColor);
                }
                painter.setPen(lineNumberColor);
            }
            painter.drawText(0, top, lineNumber->width(), fm.height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        blockNumber++;
    }
}

void Editor::highlightCloseCharPair(QChar openChar, QChar closeChar, QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QTextCursor cursor = textCursor();
    int pos = cursor.positionInBlock()-1;
    HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
    if (blockData != nullptr && blockData->specialChars.size()>0 && blockData->specialCharsPos.size()>0 && blockData->specialChars.size()==blockData->specialCharsPos.size()) {
        bool sFound = false;
        int count = 0;
        int positionInBlock = -1;
        int iterations = 0;
        do {
            if (blockData->specialChars.size()>0 && blockData->specialCharsPos.size()>0) {
                for (int i=blockData->specialChars.size()-1; i>=0; i--) {
                    iterations++;
                    if (iterations > SEARCH_LIMIT) break;
                    QChar c = blockData->specialChars.at(i);
                    if (!sFound && c == closeChar && blockData->specialCharsPos.at(i) == pos) {
                        sFound = true;
                    } else if (sFound && c == closeChar) {
                        count++;
                    } else if (sFound && c == openChar && count > 0) {
                        count--;
                    } else if (sFound && c == openChar && count == 0) {
                        positionInBlock = blockData->specialCharsPos.at(i);
                        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                        int absPos = cursor.position();
                        cursor.setPosition(absPos+positionInBlock, QTextCursor::MoveAnchor);
                        //cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, positionInBlock);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                        QTextEdit::ExtraSelection charSelection;
                        charSelection.format.setBackground(selectedCharBgColor);
                        charSelection.format.setForeground(selectedCharColor);
                        charSelection.cursor = cursor;
                        extraSelections->append(charSelection);

                        QTextCursor cursorPair = textCursor();
                        cursorPair.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
                        QTextEdit::ExtraSelection charSelectionPair;
                        charSelectionPair.format.setBackground(selectedCharBgColor);
                        charSelectionPair.format.setForeground(selectedCharColor);
                        charSelectionPair.cursor = cursorPair;
                        extraSelections->append(charSelectionPair);

                        // line mark
                        if (cursor.block().blockNumber() < cursorPair.block().blockNumber()) {
                            for (int i=cursor.block().blockNumber(); i<=cursorPair.block().blockNumber(); i++) {
                                static_cast<LineMark *>(lineMark)->addMark(i+1);
                            }
                        }
                        // tooltip
                        int firstBlockIndex = getFirstVisibleBlockIndex();
                        if (cursor.block().blockNumber() != cursorPair.block().blockNumber() && cursor.block().blockNumber() < firstBlockIndex) {
                            QString tooltipText = cursor.block().text();
                            while(tooltipText.trimmed() == "{" && cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor)) {
                                QString prevTooltipText = cursor.block().text();
                                if (prevTooltipText.trimmed().size() > 0) {
                                    tooltipText = prevTooltipText + " " + tooltipText.trimmed();
                                    positionInBlock = prevTooltipText.size() + 1;
                                }
                            }
                            int xOffset = 0;
                            if (positionInBlock >= 0) {
                                QRegularExpression indentExpr = QRegularExpression("^[\\s]+");
                                QRegularExpressionMatch indentMatch = indentExpr.match(tooltipText);
                                if (indentMatch.capturedStart()==0) {
                                    QString indent = indentMatch.captured();
                                    int spaces = indent.count(" ");
                                    int tabs = indent.count("\t");
                                    int cursOffset = spaces + tabs * tabWidth;
                                    QFontMetrics fm(editorFont);
                                    /* QFontMetrics::width is deprecated */
                                    /*
                                    xOffset = fm.width(" ") * cursOffset - horizontalScrollBar()->sliderPosition();
                                    */
                                    xOffset = fm.horizontalAdvance(" ") * cursOffset - horizontalScrollBar()->sliderPosition();
                                    if (xOffset < 0) xOffset = 0;
                                }
                            }
                            tooltipSavedText = "";
                            tooltipSavedList.clear();
                            tooltipSavedPageOffset = -1;
                            tooltipSavedOrigName = "";
                            tooltipSavedBlockNumber = -1;
                            showTooltip(xOffset+lineNumberAreaWidth()+lineMarkAreaWidth(), 0, tooltipText.trimmed(), false);
                        }
                        break;
                    }
                }
            }
            if (!sFound) break;
            if (!cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor)) break;
            blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
            if (blockData == nullptr || blockData->specialChars.size()!=blockData->specialCharsPos.size()) break;
        } while(positionInBlock < 0);
    }
}

void Editor::highlightOpenCharPair(QChar openChar, QChar closeChar, QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QTextCursor cursor = textCursor();
    int pos = cursor.positionInBlock();
    HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
    if (blockData != nullptr && blockData->specialChars.size()>0 && blockData->specialCharsPos.size()>0 && blockData->specialChars.size()==blockData->specialCharsPos.size()) {
        bool sFound = false;
        int count = 0;
        int positionInBlock = -1;
        int iterations = 0;
        do {
            if (blockData->specialChars.size()>0 && blockData->specialCharsPos.size()>0) {
                for (int i=0; i<blockData->specialChars.size(); i++) {
                    iterations++;
                    if (iterations > SEARCH_LIMIT) break;
                    QChar c = blockData->specialChars.at(i);
                    if (!sFound && c == openChar && blockData->specialCharsPos.at(i) == pos) {
                        sFound = true;
                    } else if (sFound && c == openChar) {
                        count++;
                    } else if (sFound && c == closeChar && count > 0) {
                        count--;
                    } else if (sFound && c == closeChar && count == 0) {
                        positionInBlock = blockData->specialCharsPos.at(i);

                        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                        int absPos = cursor.position();
                        cursor.setPosition(absPos+positionInBlock, QTextCursor::MoveAnchor);
                        //cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, positionInBlock);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                        QTextEdit::ExtraSelection charSelection;
                        charSelection.format.setBackground(selectedCharBgColor);
                        charSelection.format.setForeground(selectedCharColor);
                        charSelection.cursor = cursor;
                        extraSelections->append(charSelection);

                        QTextCursor cursorPair = textCursor();
                        cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                        QTextEdit::ExtraSelection charSelectionPair;
                        charSelectionPair.format.setBackground(selectedCharBgColor);
                        charSelectionPair.format.setForeground(selectedCharColor);
                        charSelectionPair.cursor = cursorPair;
                        extraSelections->append(charSelectionPair);

                        // line mark
                        if (cursor.block().blockNumber() > cursorPair.block().blockNumber()) {
                            for (int i=cursorPair.block().blockNumber(); i<=cursor.block().blockNumber(); i++) {
                                static_cast<LineMark *>(lineMark)->addMark(i+1);
                            }
                        }
                        break;
                    }
                }
            }
            if (!sFound) break;
            if (!cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) break;
            blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
            if (blockData == nullptr || blockData->specialChars.size()!=blockData->specialCharsPos.size()) break;
        } while(positionInBlock < 0);
    }
}

void Editor::highlightCloseTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QTextCursor cursor = textCursor();
    HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
    if (blockData != nullptr && blockData->specialWords.size()>0 && blockData->specialWordsPos.size()>0 && blockData->specialWords.size()==blockData->specialWordsPos.size()) {
        bool sFound = false;
        int count = 0;
        int positionInBlock = -1;
        int iterations = 0;
        do {
            if (blockData->specialWords.size()>0 && blockData->specialWordsPos.size()>0) {
                for (int i=blockData->specialWords.size()-1; i>=0; i--) {
                    iterations++;
                    if (iterations > SEARCH_LIMIT) break;
                    QString w = blockData->specialWords.at(i);
                    if (!sFound && w == "/"+tagName && blockData->specialWordsPos.at(i) == pos) {
                        sFound = true;
                    } else if (sFound && w == "/"+tagName) {
                        count++;
                    } else if (sFound && w == tagName && count > 0) {
                        count--;
                    } else if (sFound && w == tagName && count == 0) {
                        positionInBlock = blockData->specialWordsPos.at(i);
                        bool _found = false;
                        int _start = -1, _length = 0, _offset = 0;
                        QString blockText = cleanUpText(cursor.block().text());
                        do {
                            QRegularExpressionMatch _match = tagExpr.match(blockText, _offset);
                            _start = _match.capturedStart();
                            if (_start>=0) {
                                _length = _match.capturedLength();
                                if (_start < positionInBlock && _start+_length > positionInBlock) {
                                    _found = true;
                                    break;
                                }
                                _offset = _start + _length;
                            }
                        } while (_start>=0);
                        if (!_found) {
                            _start = positionInBlock;
                            _length = tagName.size();
                        }
                        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, _start);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, _length);
                        QTextEdit::ExtraSelection charSelection;
                        charSelection.format.setBackground(selectedTagBgColor);
                        charSelection.cursor = cursor;
                        extraSelections->append(charSelection);

                        QTextCursor cursorPair = textCursor();
                        _found = false;
                        _start = -1; _length = 0; _offset = 0;
                        QString blockTextPair = cleanUpText(cursorPair.block().text());
                        do {
                            QRegularExpressionMatch _match = tagExpr.match(blockTextPair, _offset);
                            _start = _match.capturedStart();
                            if (_start>=0) {
                                _length = _match.capturedLength();
                                if (_start < pos && _start+_length > pos) {
                                    _found = true;
                                    break;
                                }
                                _offset = _start + _length;
                            }
                        } while (_start>=0);
                        if (!_found) {
                            _start = pos;
                            _length = tagName.size();
                        }
                        cursorPair.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                        cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, _start);
                        cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, _length);
                        QTextEdit::ExtraSelection charSelectionPair;
                        charSelectionPair.format.setBackground(selectedTagBgColor);
                        charSelectionPair.cursor = cursorPair;
                        extraSelections->append(charSelectionPair);

                        // line mark
                        if (cursor.block().blockNumber() < cursorPair.block().blockNumber()) {
                            for (int i=cursor.block().blockNumber(); i<=cursorPair.block().blockNumber(); i++) {
                                static_cast<LineMark *>(lineMark)->addMark(i+1);
                            }
                        }
                        break;
                    }
                }
            }
            if (!sFound) break;
            if (!cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor)) break;
            blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
            if (blockData == nullptr || blockData->specialWords.size()!=blockData->specialWordsPos.size()) break;
        } while(positionInBlock < 0);
    }
}

void Editor::highlightOpenTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QTextCursor cursor = textCursor();
    HighlightData * blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
    if (blockData != nullptr && blockData->specialWords.size()>0 && blockData->specialWordsPos.size()>0 && blockData->specialWords.size()==blockData->specialWordsPos.size()) {
        bool sFound = false;
        int count = 0;
        int positionInBlock = -1;
        int iterations = 0;
        do {
            if (blockData->specialWords.size()>0 && blockData->specialWordsPos.size()>0) {
                for (int i=0; i<blockData->specialWords.size(); i++) {
                    iterations++;
                    if (iterations > SEARCH_LIMIT) break;
                    QString w = blockData->specialWords.at(i);
                    if (!sFound && w == tagName && blockData->specialWordsPos.at(i) == pos) {
                        sFound = true;
                    } else if (sFound && w == tagName) {
                        count++;
                    } else if (sFound && w == "/"+tagName && count > 0) {
                        count--;
                    } else if (sFound && w == "/"+tagName && count == 0) {
                        positionInBlock = blockData->specialWordsPos.at(i);
                        bool _found = false;
                        int _start = -1, _length = 0, _offset = 0;
                        QString blockText = cleanUpText(cursor.block().text());
                        do {
                            QRegularExpressionMatch _match = tagExpr.match(blockText, _offset);
                            _start = _match.capturedStart();
                            if (_start>=0) {
                                _length = _match.capturedLength();
                                if (_start < positionInBlock && _start+_length > positionInBlock) {
                                    _found = true;
                                    break;
                                }
                                _offset = _start + _length;
                            }
                        } while (_start>=0);
                        if (!_found) {
                            _start = positionInBlock;
                            _length = tagName.size();
                        }
                        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, _start);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, _length);
                        QTextEdit::ExtraSelection charSelection;
                        charSelection.format.setBackground(selectedTagBgColor);
                        charSelection.cursor = cursor;
                        extraSelections->append(charSelection);

                        QTextCursor cursorPair = textCursor();
                        _found = false;
                        _start = -1; _length = 0; _offset = 0;
                        QString blockTextPair = cleanUpText(cursorPair.block().text());
                        do {
                            QRegularExpressionMatch _match = tagExpr.match(blockTextPair, _offset);
                            _start = _match.capturedStart();
                            if (_start>=0) {
                                _length = _match.capturedLength();
                                if (_start < pos && _start+_length > pos) {
                                    _found = true;
                                    break;
                                }
                                _offset = _start + _length;
                            }
                        } while (_start>=0);
                        if (!_found) {
                            _start = pos;
                            _length = tagName.size();
                        }
                        cursorPair.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                        cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, _start);
                        cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, _length);
                        QTextEdit::ExtraSelection charSelectionPair;
                        charSelectionPair.format.setBackground(selectedTagBgColor);
                        charSelectionPair.cursor = cursorPair;
                        extraSelections->append(charSelectionPair);

                        // line mark
                        if (cursor.block().blockNumber() > cursorPair.block().blockNumber()) {
                            for (int i=cursorPair.block().blockNumber(); i<=cursor.block().blockNumber(); i++) {
                                static_cast<LineMark *>(lineMark)->addMark(i+1);
                            }
                        }
                        break;
                    }
                }
            }
            if (!sFound) break;
            if (!cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) break;
            blockData = dynamic_cast<HighlightData *>(cursor.block().userData());
            if (blockData == nullptr || blockData->specialWords.size()!=blockData->specialWordsPos.size()) break;
        } while(positionInBlock < 0);
    }
}

void Editor::highlightPHPCloseSpecialTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QRegularExpression openTagExpr, closeTagExpr, foundTagExpr;
    if (tagName == "endif" || tagName == "else") {
        openTagExpr = tagExprIf;
        closeTagExpr = tagExprEndif;
        if (tagName == "else") foundTagExpr = tagExprElse;
        else foundTagExpr = closeTagExpr;
    } else if (tagName == "endswitch" || tagName == "case") {
        openTagExpr = tagExprSwitch;
        closeTagExpr = tagExprEndswitch;
        if (tagName == "case") foundTagExpr = tagExprCase;
        else foundTagExpr = closeTagExpr;
    } else if (tagName == "endwhile") {
        openTagExpr = tagExprWhile;
        closeTagExpr = tagExprEndwhile;
        foundTagExpr = closeTagExpr;
    } else if (tagName == "endfor") {
        openTagExpr = tagExprFor;
        closeTagExpr = tagExprEndfor;
        foundTagExpr = closeTagExpr;
    } else if (tagName == "endforeach") {
        openTagExpr = tagExprForeach;
        closeTagExpr = tagExprEndforeach;
        foundTagExpr = closeTagExpr;
    } else {
        return;
    }
    QTextCursor cursor = textCursor();
    QString blockText = cursor.block().text();
    bool sFound = false;
    int maxOffset = blockText.size();
    int count = 0;
    int positionInBlock = -1;
    int lengthInBlock = 0;
    int closePositionInBlock = -1;
    int closeLengthInBlock = 0;
    int iterations = 0;
    do {
        QVector<int> openPos, openLength, closePos, closeLength, sortedPos, sortedLength, sortedTag;
        int offset = 0, start = -1;
        do {
            iterations++;
            if (iterations > SEARCH_LIMIT) break;
            QRegularExpressionMatch match;
            if (!sFound) {
                match = foundTagExpr.match(blockText, offset);
            } else {
                match = closeTagExpr.match(blockText, offset);
            }
            start = match.capturedStart();
            if (start>=0) {
                int length = match.capturedLength();
                offset = start + length;
                if (!sFound && start<=pos && start+length>=pos) {
                    sFound = true;
                    closePositionInBlock = start;
                    closeLengthInBlock = length;
                    maxOffset = offset;
                    break;
                }
                closePos.append(start);
                closeLength.append(length);
            }
        } while(start>=0);
        if (!sFound) break;
        offset = 0; start = -1;
        do {
            iterations++;
            if (iterations > SEARCH_LIMIT) break;
            QRegularExpressionMatch match = openTagExpr.match(blockText, offset);
            start = match.capturedStart();
            if (start>=0) {
                int length = match.capturedLength();
                if (start>=maxOffset) break;
                offset = start + length;
                openPos.append(start);
                openLength.append(length);
            }
        } while(start>=0);
        int openI = 0, closeI = 0;
        if (openPos.size()==0) openI = -1;
        if (closePos.size()==0) closeI = -1;
        while(openI >= 0 || closeI >= 0) {
            int open = -1, close = -1;
            if (openI >= 0) open = openPos.at(openI);
            if (closeI >= 0) close = closePos.at(closeI);
            if (open >= 0 && (open < close || close < 0)) {
                sortedPos.append(open);
                sortedLength.append(openLength.at(openI));
                sortedTag.append(1);
                openI++;
                if (openI>=openPos.size()) openI = -1;
            } else if (close >= 0 && (open > close || open < 0)) {
                sortedPos.append(close);
                sortedLength.append(closeLength.at(closeI));
                sortedTag.append(-1);
                closeI++;
                if (closeI>=closePos.size()) closeI = -1;
            }
        }
        if (sortedPos.size() != openPos.size()+closePos.size()) break;
        for (int i=sortedPos.size()-1; i>=0; i--) {
            int tag = sortedTag.at(i);
            if (tag < 0) {
                count++;
            } else if (tag > 0 && count > 0) {
                count--;
            } else if (tag > 0 && count == 0) {
                positionInBlock = sortedPos.at(i);
                lengthInBlock = sortedLength.at(i);

                cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, positionInBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, lengthInBlock);
                QTextEdit::ExtraSelection charSelection;
                charSelection.format.setBackground(selectedExpressionBgColor);
                charSelection.cursor = cursor;
                extraSelections->append(charSelection);

                QTextCursor cursorPair = textCursor();
                cursorPair.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, closePositionInBlock);
                cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, closeLengthInBlock);
                QTextEdit::ExtraSelection charSelectionPair;
                charSelectionPair.format.setBackground(selectedExpressionBgColor);
                charSelectionPair.cursor = cursorPair;
                extraSelections->append(charSelectionPair);

                // line mark
                if (cursor.block().blockNumber() < cursorPair.block().blockNumber()) {
                    for (int i=cursor.block().blockNumber(); i<=cursorPair.block().blockNumber(); i++) {
                        static_cast<LineMark *>(lineMark)->addMark(i+1);
                    }
                }
                break;
            }
        }
        if (!cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor)) break;
        blockText = cursor.block().text();
        maxOffset = blockText.size();
    } while(positionInBlock < 0);
}

void Editor::highlightPHPOpenSpecialTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QRegularExpression openTagExpr, closeTagExpr;
    if (tagName == "if") {
        openTagExpr = tagExprIf;
        closeTagExpr = tagExprEndif;
    } else if (tagName == "switch") {
        openTagExpr = tagExprSwitch;
        closeTagExpr = tagExprEndswitch;
    } else if (tagName == "while") {
        openTagExpr = tagExprWhile;
        closeTagExpr = tagExprEndwhile;
    } else if (tagName == "for") {
        openTagExpr = tagExprFor;
        closeTagExpr = tagExprEndfor;
    } else if (tagName == "foreach") {
        openTagExpr = tagExprForeach;
        closeTagExpr = tagExprEndforeach;
    } else {
        return;
    }
    QTextCursor cursor = textCursor();
    QString blockText = cursor.block().text();
    bool sFound = false;
    int minOffset = 0;
    int count = 0;
    int positionInBlock = -1;
    int lengthInBlock = 0;
    int openPositionInBlock = -1;
    int openLengthInBlock = 0;
    int iterations = 0;
    do {
        QVector<int> openPos, openLength, closePos, closeLength, sortedPos, sortedLength, sortedTag;
        int offset = 0, start = -1;
        do {
            iterations++;
            if (iterations > SEARCH_LIMIT) break;
            QRegularExpressionMatch match = openTagExpr.match(blockText, offset);
            start = match.capturedStart();
            if (start>=0) {
                int length = match.capturedLength();
                offset = start + length;
                if (!sFound && start<=pos && start+length>=pos) {
                    sFound = true;
                    openPositionInBlock = start;
                    openLengthInBlock = length;
                    minOffset = offset;
                } else if (sFound) {
                    openPos.append(start);
                    openLength.append(length);
                }
            }
        } while(start>=0);
        if (!sFound) break;
        offset = 0; start = -1;
        do {
            iterations++;
            if (iterations > SEARCH_LIMIT) break;
            QRegularExpressionMatch match = closeTagExpr.match(blockText, offset);
            start = match.capturedStart();
            if (start>=0) {
                int length = match.capturedLength();
                offset = start + length;
                if (start<minOffset) continue;
                closePos.append(start);
                closeLength.append(length);
            }
        } while(start>=0);
        int openI = 0, closeI = 0;
        if (openPos.size()==0) openI = -1;
        if (closePos.size()==0) closeI = -1;
        while(openI >= 0 || closeI >= 0) {
            int open = -1, close = -1;
            if (openI >= 0) open = openPos.at(openI);
            if (closeI >= 0) close = closePos.at(closeI);
            if (open >= 0 && (open < close || close < 0)) {
                sortedPos.append(open);
                sortedLength.append(openLength.at(openI));
                sortedTag.append(1);
                openI++;
                if (openI>=openPos.size()) openI = -1;
            } else if (close >= 0 && (open > close || open < 0)) {
                sortedPos.append(close);
                sortedLength.append(closeLength.at(closeI));
                sortedTag.append(-1);
                closeI++;
                if (closeI>=closePos.size()) closeI = -1;
            }
        }
        if (sortedPos.size() != openPos.size()+closePos.size()) break;
        for (int i=0; i<sortedPos.size(); i++) {
            int tag = sortedTag.at(i);
            if (tag > 0) {
                count++;
            } else if (tag < 0 && count > 0) {
                count--;
            } else if (tag < 0 && count == 0) {
                positionInBlock = sortedPos.at(i);
                lengthInBlock = sortedLength.at(i);

                cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, positionInBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, lengthInBlock);
                QTextEdit::ExtraSelection charSelection;
                charSelection.format.setBackground(selectedExpressionBgColor);
                charSelection.cursor = cursor;
                extraSelections->append(charSelection);

                QTextCursor cursorPair = textCursor();
                cursorPair.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, openPositionInBlock);
                cursorPair.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, openLengthInBlock);
                QTextEdit::ExtraSelection charSelectionPair;
                charSelectionPair.format.setBackground(selectedExpressionBgColor);
                charSelectionPair.cursor = cursorPair;
                extraSelections->append(charSelectionPair);

                // line mark
                if (cursor.block().blockNumber() > cursorPair.block().blockNumber()) {
                    for (int i=cursorPair.block().blockNumber(); i<=cursor.block().blockNumber(); i++) {
                        static_cast<LineMark *>(lineMark)->addMark(i+1);
                    }
                }
                break;
            }
        }
        if (!cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor)) break;
        blockText = cursor.block().text();
        minOffset = 0;
    } while(positionInBlock < 0);
}

void Editor::resetExtraSelections()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    highlightCurrentLine(& extraSelections);
    setExtraSelections(extraSelections);
}

void Editor::highlightCurrentLine(QList<QTextEdit::ExtraSelection> * extraSelections)
{
    QTextEdit::ExtraSelection selectedLineSelection;
    selectedLineSelection.format.setBackground(selectedLineBgColor);
    selectedLineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selectedLineSelection.cursor = textCursor();
    selectedLineSelection.cursor.clearSelection();
    if (selectedLineSelection.cursor.block().layout() != nullptr && selectedLineSelection.cursor.block().layout()->lineCount() > 1) {
        selectedLineSelection.cursor.movePosition(QTextCursor::StartOfBlock);
        selectedLineSelection.cursor.movePosition(QTextCursor::NextRow, QTextCursor::KeepAnchor);
    }
    extraSelections->append(selectedLineSelection);
}

void Editor::highlightExtras(QChar prevChar, QChar nextChar, QChar cursorTextPrevChar, QString cursorText, int cursorTextPos, std::string mode)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        static_cast<LineMark *>(lineMark)->clearMarks();
        // current line
        highlightCurrentLine(& extraSelections);

        // word under cursor
        if (cursorText.size()>0) {
            if (mode == MODE_PHP && (cursorText == "if" || cursorText == "switch" || cursorText == "while" || cursorText == "for" || cursorText == "foreach")) {
                highlightPHPOpenSpecialTagPair(cursorText, cursorTextPos, & extraSelections);
            } else if (mode == MODE_PHP && (cursorText == "endif" || cursorText == "else" || cursorText == "endswitch" || cursorText == "case" || cursorText == "endwhile" || cursorText == "endfor" || cursorText == "endforeach")) {
                highlightPHPCloseSpecialTagPair(cursorText, cursorTextPos, & extraSelections);
            } else if (mode == MODE_HTML && cursorTextPrevChar == "<") {
                highlightOpenTagPair(cursorText, cursorTextPos, & extraSelections);
            } else if (mode == MODE_HTML && cursorTextPrevChar == "/") {
                highlightCloseTagPair(cursorText, cursorTextPos, & extraSelections);
            } else if (!search->isVisible()) {
                QTextCursor selectedWordCursor(document());
                QTextDocument::FindFlags findFlags = QTextDocument::FindCaseSensitively;
                if (!textCursor().hasSelection()) findFlags |= QTextDocument::FindWholeWords;
                selectedWordCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
                int iterator = 0;
                while(!selectedWordCursor.isNull() && !selectedWordCursor.atEnd()) {
                    iterator++;
                    if (iterator > SEARCH_LIMIT) break;
                    selectedWordCursor = document()->find(cursorText, selectedWordCursor, findFlags);
                    if (!selectedWordCursor.isNull()) {
                        QTextEdit::ExtraSelection selectedWordSelection;
                        selectedWordSelection.format.setBackground(selectedWordBgColor);
                        selectedWordSelection.cursor = selectedWordCursor;
                        extraSelections.append(selectedWordSelection);
                    }
                }
            }
        }
        // search words
        if (search->isVisible() && searchString.size() > 0) {
            static_cast<LineMap *>(lineMap)->clearMarks();
            QTextCursor searchWordCursor(document());
            QTextDocument::FindFlags findFlags = nullptr;
            if (searchCaSe) findFlags |= QTextDocument::FindCaseSensitively;
            if (searchWord) findFlags |= QTextDocument::FindWholeWords;
            searchWordCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            int searchWordBlockNumber = -1;
            int co = 0;
            while(!searchWordCursor.isNull() && !searchWordCursor.atEnd()) {
                if (!searchRegE) searchWordCursor = document()->find(searchString, searchWordCursor, findFlags);
                else searchWordCursor = document()->find(QRegularExpression(searchString), searchWordCursor, findFlags);
                if (!searchWordCursor.isNull()) {
                    QTextEdit::ExtraSelection selectedWordSelection;
                    selectedWordSelection.format.setBackground(searchWordBgColor);
                    selectedWordSelection.format.setForeground(searchWordColor);
                    selectedWordSelection.cursor = searchWordCursor;
                    extraSelections.append(selectedWordSelection);
                    if (searchWordBlockNumber != searchWordCursor.block().blockNumber()) {
                        searchWordBlockNumber = searchWordCursor.block().blockNumber();
                        static_cast<LineMap *>(lineMap)->addMark(searchWordBlockNumber+1);
                    }
                }
                co++;
                if (co >= SEARCH_LIMIT) break; // too much results
            }
        }
        // repaint line map
        lineMap->update();

        // braces
        if (prevChar=="}") {
            highlightCloseCharPair('{', '}', & extraSelections);
        }
        if (nextChar=="{") {
            highlightOpenCharPair('{', '}', & extraSelections);
        }

        // parentheses
        if (prevChar==")") {
            highlightCloseCharPair('(', ')', & extraSelections);
        }
        if (nextChar=="(") {
            highlightOpenCharPair('(', ')', & extraSelections);
        }

        // square brackets
        if (prevChar=="]") {
            highlightCloseCharPair('[', ']', & extraSelections);
        }
        if (nextChar=="[") {
            highlightOpenCharPair('[', ']', & extraSelections);
        }

        // repaint line mark
        lineMark->update();

        // hovered words
        for (int i=0; i<wordsExtraSelections.size(); i++) {
            extraSelections.append(wordsExtraSelections.at(i));
        }

        // errors
        for (int i=0; i<errorsExtraSelections.size(); i++) {
            extraSelections.append(errorsExtraSelections.at(i));
        }
    }
    setExtraSelections(extraSelections);
}

void Editor::clearWarnings()
{
    static_cast<LineMap *>(lineMap)->clearWarnings();
    static_cast<LineMark *>(lineMark)->clearWarnings();
}

void Editor::setWarning(int line, QString text)
{
    static_cast<LineMap *>(lineMap)->addWarning(line);
    static_cast<LineMark *>(lineMark)->addWarning(line, text);
}

void Editor::clearErrors()
{
    static_cast<LineMap *>(lineMap)->clearErrors();
    static_cast<LineMark *>(lineMark)->clearErrors();
    breadcrumbs->setToolTip("");
    clearErrorsFormat();
}

void Editor::setError(int line, QString text)
{
    static_cast<LineMap *>(lineMap)->addError(line);
    static_cast<LineMark *>(lineMark)->addError(line, text);
    breadcrumbs->setToolTip(text+" ["+tr("Line")+": "+Helper::intToStr(line)+"]");
}

void Editor::updateMarksAndMapArea()
{
    lineMark->update();
    lineMap->update();
    breadcrumbs->update();
}

void Editor::findToggle()
{
    QTextCursor curs = textCursor();
    if (!search->isVisible() || curs.hasSelection()) {
        if (curs.hasSelection()) {
            searchString = curs.selectedText();
            static_cast<Search *>(search)->setFindEditText(searchString);
        }
        showSearch();
        highlightExtras();
    } else {
        closeSearch();
    }
}

void Editor::showSearch()
{
    search->show();
    static_cast<Search *>(search)->setFindEditFocus();
    static_cast<Search *>(search)->updateScrollBar();
    updateViewportMargins();
}

void Editor::closeSearch()
{
    search->hide();
    static_cast<Search *>(search)->updateScrollBar();
    updateViewportMargins();
    setFocus();
    static_cast<LineMap *>(lineMap)->clearMarks();
    highlightExtras();
}

void Editor::searchFlagsChanged(QString searchStr, bool CaSe, bool Word, bool RegE)
{
    searchString = searchStr;
    searchCaSe = CaSe;
    searchWord = Word;
    searchRegE = RegE;
    highlightExtras();
}

void Editor::searchText(QString searchTxt, bool CaSe, bool Word, bool RegE, bool backwards, bool fromStart)
{
    searchString = searchTxt;
    searchCaSe = CaSe;
    searchWord = Word;
    searchRegE = RegE;
    if (searchTxt.size() == 0) return;
    QTextCursor curs = textCursor();
    if (curs.hasSelection()) {
        int startSel = curs.selectionStart();
        int endSel = curs.selectionEnd();
        curs.clearSelection();
        if (!backwards) curs.setPosition(endSel);
        else curs.setPosition(startSel);
    }
    if (fromStart) {
        if (!backwards) curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        else curs.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    }
    if (backwards && curs.positionInBlock() == 0) curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor); // bug ?
    QTextDocument::FindFlags flags = nullptr;
    if (CaSe) flags |= QTextDocument::FindCaseSensitively;
    if (Word) flags |= QTextDocument::FindWholeWords;
    if (backwards) flags |= QTextDocument::FindBackward;
    QTextCursor resCurs;
    if (!RegE) {
        resCurs = document()->find(searchTxt, curs, flags);
    } else {
        resCurs = document()->find(QRegularExpression(searchTxt), curs, flags);
    }
    if (!resCurs.isNull()) {
        setTextCursor(resCurs);
        static_cast<Search *>(search)->setFindEditBg(searchInputBgColor);
        static_cast<Search *>(search)->setFindEditProp("results", "found");
        if (fromStart && !backwards) {
            emit showPopupText(tabIndex, tr("Searching from the beginning..."));
        } else if (fromStart && backwards) {
            emit showPopupText(tabIndex, tr("Searching from the end..."));
        }
    } else {
        if (!fromStart) return searchText(searchTxt, CaSe, Word, RegE, backwards, true);
        else {
            static_cast<Search *>(search)->setFindEditBg(searchInputErrorBgColor);
            static_cast<Search *>(search)->setFindEditProp("results", "notfound");
        }
    }
    highlightExtras();
}

void Editor::replaceText(QString searchTxt, QString replaceTxt, bool CaSe, bool Word, bool RegE)
{
    QTextCursor curs = textCursor();
    if (curs.hasSelection()) {
        curs.insertText(replaceTxt);
        setTextCursor(curs);
    } else {
        searchText(searchTxt, CaSe, Word, RegE);
    }
}

void Editor::replaceAllText(QString searchTxt, QString replaceTxt, bool CaSe, bool Word, bool RegE)
{
    QTextCursor curs = textCursor();
    if (curs.hasSelection()) searchTxt = curs.selectedText();
    searchString = searchTxt;
    searchCaSe = CaSe;
    searchWord = Word;
    searchRegE = RegE;
    if (searchTxt.size() == 0) return;
    curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    curs.beginEditBlock();
    QTextDocument::FindFlags flags = nullptr;
    if (CaSe) flags |= QTextDocument::FindCaseSensitively;
    if (Word) flags |= QTextDocument::FindWholeWords;
    QTextCursor resCurs;
    int co = 0;
    do {
        if (!RegE) {
            resCurs = document()->find(searchTxt, curs, flags);
        } else {
            resCurs = document()->find(QRegularExpression(searchTxt), curs, flags);
        }
        if (!resCurs.isNull()) {
            curs.setPosition(resCurs.selectionStart(), QTextCursor::MoveAnchor);
            curs.setPosition(resCurs.selectionEnd(), QTextCursor::KeepAnchor);
            curs.insertText(replaceTxt);
            co++;
        }
    } while(!resCurs.isNull());
    curs.endEditBlock();
    if (co > 0) {
        setTextCursor(curs);
        static_cast<Search *>(search)->setFindEditBg(searchInputBgColor);
        static_cast<Search *>(search)->setFindEditProp("results", "found");
    } else {
        static_cast<Search *>(search)->setFindEditBg(searchInputErrorBgColor);
        static_cast<Search *>(search)->setFindEditProp("results", "notfound");
    }
    highlightExtras();
}

void Editor::scrollLineMap(int y)
{
    if (y < 0 || !verticalScrollBar()->isVisible()) return;
    int height = lineMap->geometry().height();
    if (verticalScrollBar()->geometry().height() < height) {
        height = verticalScrollBar()->geometry().height();
    }
    if (height <= 0) return;
    if (y > height) y = height;
    int p = (y * verticalScrollBar()->maximum()) / height;
    hideTooltip();
    verticalScrollBar()->setValue(p);
}

int Editor::getCursorLine()
{
    QTextCursor cursor = textCursor();
    return cursor.blockNumber() + 1;
}

void Editor::gotoLine(int line, bool focus) {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    if (line > 1) cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line-1);
    QString blockText = cursor.block().text();
    QFontMetrics fm(font());
    /* QFontMetrics::width is deprecated */
    /*
    if (fm.width(blockText) > viewport()->width()) {
    */
    if (fm.horizontalAdvance(blockText) > viewport()->width()) {
        do {
            int pos = cursor.positionInBlock();
            if (pos >= blockText.size()) break;
            QChar c = blockText[pos];
            if (!c.isSpace()) break;
        } while(cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor));
    } else {
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }
    setTextCursor(cursor);
    if (focus) setFocus();
    scrollToMiddle(cursor, line);
}

void Editor::gotoLineSymbol(int line, int symbol) {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    if (symbol >= 0 && symbol <= cursor.position()) {
        cursor.setPosition(symbol);
        setTextCursor(cursor);
        setFocus();
        scrollToMiddle(cursor, line);
    }
}

void Editor::showLineNumber(int y)
{
    if (gitAnnotations.size() == 0) return;
    if (y < 0) return;
    Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
    if (!(modifiers & Qt::ControlModifier)) return;
    int blockNumber = getFirstVisibleBlockIndex();
    if (blockNumber < 0) return;
    if (blockNumber>0) blockNumber--;

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    //int top = contentsMargins().top();
    int top = 0; // widget has offset

    if (blockNumber == 0) {
        top += static_cast<int>(document()->documentMargin()) - verticalScrollBar()->sliderPosition();
    } else {
        QTextBlock prev_block = document()->findBlockByNumber(blockNumber-1);
        int prev_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).y());
        int prev_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).height());
        top += prev_y + prev_h - verticalScrollBar()->sliderPosition();
    }

    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    QString tooltipText = "";
    while (block.isValid() && top < y) {
        if (block.isVisible() && top < y && bottom > y) {
            int line = blockNumber + 1;
            if (gitAnnotations.contains(line)) {
                Git::Annotation annotation = gitAnnotations.value(line);
                tooltipText = tr("Line") + " " + Helper::intToStr(annotation.line) + ": " + annotation.comment + "\n" + annotation.committer + " [" + annotation.committerDate + "]";
            }
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        blockNumber++;
    }

    if (tooltipText.size() > 0) {
        /*
        QFontMetrics fm(editorTooltipFont);
        int linesCo = tooltipText.split("\n").size();
        if (y < fm.height()*linesCo) y += fm.height()*linesCo;
        showTooltip(lineMark->geometry().x()+lineMark->geometry().width()+TOOLTIP_OFFSET, y+TOOLTIP_OFFSET, tooltipText, false);
        */
        QTextCursor curs = textCursor();
        curs.movePosition(QTextCursor::Start);
        if (blockNumber > 0) curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);
        showTooltip(&curs, tooltipText, false);
    } else {
        hideTooltip();
    }
}

void Editor::showLineMap(int y)
{
    if (y < 0 || !verticalScrollBar()->isVisible()) return;
    Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
    if (!(modifiers & Qt::ControlModifier)) return;
    int height = lineMap->geometry().height();
    if (verticalScrollBar()->geometry().height() < height) {
        height = verticalScrollBar()->geometry().height();
    }
    if (height <= 0) return;
    if (y > height) y = height;
    int n = (y * document()->blockCount()) / height;
    QTextCursor curs = textCursor();
    curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    QString tooltipText = "";
    int maxChars = 50, blocksCount = document()->blockCount();
    QString blocksCountStr = Helper::intToStr(blocksCount);
    maxChars += blocksCountStr.size()+2;
    QFontMetrics fm(editorTooltipFont);
    /* QFontMetrics::width is deprecated */
    /*
    int tooltipWidth = fm.width(QString(" ").repeated(maxChars+3)) + tooltipLabel.contentsMargins().left() + tooltipLabel.contentsMargins().right() + 1;
    */
    int tooltipWidth = fm.horizontalAdvance(QString(" ").repeated(maxChars+3)) + tooltipLabel.contentsMargins().left() + tooltipLabel.contentsMargins().right() + 1;
    int linesCo = 0;
    do {
        QTextBlock block = curs.block();
        int blockNumber = block.blockNumber();
        if (blockNumber >= n-LINE_MAP_LINE_NUMBER_OFFSET && blockNumber <= n+LINE_MAP_LINE_NUMBER_OFFSET) {
            if (tooltipText.size() > 0) tooltipText += "\n";
            QString lineNumberStr = Helper::intToStr(blockNumber+1);
            if (lineNumberStr.size()<blocksCountStr.size()) lineNumberStr = QString(" ").repeated(blocksCountStr.size() - lineNumberStr.size())+lineNumberStr;
            markPointsIterator = markPoints.find(blockNumber+1);
            if (markPointsIterator != markPoints.end()) lineNumberStr = "> " + lineNumberStr;
            else lineNumberStr = "  "+lineNumberStr;
            QString blockText = lineNumberStr+"  "+block.text().replace("\t", QString(" ").repeated(tabWidth));
            if (blockText.size()>maxChars) blockText=blockText.mid(0, maxChars)+"...";
            tooltipText += blockText;
            linesCo++;
        }
        if (blockNumber >= n+LINE_MAP_LINE_NUMBER_OFFSET) break;
    } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
    if (tooltipText.size() > 0) {
        //if (y < fm.height()*linesCo) y += fm.height()*linesCo;
        //int tY = y+TOOLTIP_OFFSET;
        int mapHeight = lineMap->geometry().height();
        if (verticalScrollBar()->isVisible() && verticalScrollBar()->geometry().height() < mapHeight) mapHeight = verticalScrollBar()->geometry().height();
        int tY = lineMap->geometry().top() + mapHeight - TOOLTIP_OFFSET;
        showTooltip(lineMap->geometry().x()-tooltipWidth-TOOLTIP_OFFSET, tY, tooltipText, false, tooltipWidth);
    } else {
        hideTooltip();
    }
}

void Editor::showLineMark(int y)
{
    if (y < 0) return;
    int blockNumber = getFirstVisibleBlockIndex();
    if (blockNumber < 0) return;
    if (blockNumber>0) blockNumber--;

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    //int top = contentsMargins().top();
    int top = 0; // widget has offset

    if (blockNumber == 0) {
        top += static_cast<int>(document()->documentMargin()) - verticalScrollBar()->sliderPosition();
    } else {
        QTextBlock prev_block = document()->findBlockByNumber(blockNumber-1);
        int prev_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).y());
        int prev_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).height());
        top += prev_y + prev_h - verticalScrollBar()->sliderPosition();
    }

    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    QString tooltipText = "";
    while (block.isValid() && top < y) {
        if (block.isVisible() && top < y && bottom > y) {
            int line = blockNumber + 1;
            QString errorText = "", warningText = "", markText = "";
            int error = static_cast<LineMark *>(lineMark)->getError(line, errorText);
            int warning = static_cast<LineMark *>(lineMark)->getWarning(line, warningText);
            int mark = static_cast<LineMark *>(lineMark)->getMark(line, markText);
            if (error > 0 || warning > 0 || mark > 0) {
                if (error > 0 && errorText.size() > 0) {
                    tooltipText = errorText.replace("\t", QString(" ").repeated(tabWidth));
                } else if (warning > 0 && warningText.size() > 0) {
                    tooltipText = warningText.replace("\t", QString(" ").repeated(tabWidth));
                } else if (mark > 0 && markText.size() > 0) {
                    tooltipText = markText.replace("\t", QString(" ").repeated(tabWidth));
                }
            }
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        blockNumber++;
    }

    if (tooltipText.size() > 0) {
        /*
        QFontMetrics fm(editorTooltipFont);
        int linesCo = tooltipText.split("\n").size();
        if (y < fm.height()*linesCo) y += fm.height()*linesCo;
        showTooltip(lineMark->geometry().x()+lineMark->geometry().width()+TOOLTIP_OFFSET, y+TOOLTIP_OFFSET, tooltipText, false);
        */
        QTextCursor curs = textCursor();
        curs.movePosition(QTextCursor::Start);
        if (blockNumber > 0) curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);
        showTooltip(&curs, tooltipText, false);
    } else {
        hideTooltip();
    }
}

void Editor::addLineMark(int y)
{
    if (y < 0) return;
    int blockNumber = getFirstVisibleBlockIndex();
    if (blockNumber < 0) return;
    if (blockNumber>0) blockNumber--;

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    //int top = contentsMargins().top();
    int top = 0; // widget has offset

    if (blockNumber == 0) {
        top += static_cast<int>(document()->documentMargin()) - verticalScrollBar()->sliderPosition();
    } else {
        QTextBlock prev_block = document()->findBlockByNumber(blockNumber-1);
        int prev_y = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).y());
        int prev_h = static_cast<int>(document()->documentLayout()->blockBoundingRect(prev_block).height());
        top += prev_y + prev_h - verticalScrollBar()->sliderPosition();
    }

    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    bool changed = false;
    while (block.isValid() && top < y) {
        if (block.isVisible() && top < y && bottom > y) {
            int line = blockNumber + 1;
            QString text = block.text();
            markPointsIterator = markPoints.find(line);
            if (markPointsIterator == markPoints.end()) {
                markPoints[line] = text.toStdString();
                HighlightData * blockData = dynamic_cast<HighlightData *>(block.userData());
                if (blockData != nullptr) {
                    blockData->hasMarkPoint = true;
                    block.setUserData(blockData);
                }
            } else {
                markPoints.erase(markPointsIterator);
                HighlightData * blockData = dynamic_cast<HighlightData *>(block.userData());
                if (blockData != nullptr) {
                    blockData->hasMarkPoint = false;
                    block.setUserData(blockData);
                }
            }
            changed = true;
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
        blockNumber++;
    }

    if (changed) updateMarksAndMapArea();
}

QString Editor::getContent()
{
    QString text = toPlainText();
    if (newLineMode != LF) {
        QString nl = "\n";
        if (newLineMode == CR) nl = "\r";
        if (newLineMode == CRLF) nl = "\r\n";
        text = text.replace("\n", nl);
    }
    return text;
}

void Editor::highlightUnusedVars(bool update)
{
    if (!experimentalMode) return;
    if (isBigFile || highlight->getFoundModes().contains(QString::fromStdString(MODE_HTML))) return;
    setReadOnly(true);
    QVector<QTextBlock> unusedVarsBlocks;
    highlight->unusedVars.clear();
    // update highlighter
    if (update) {
        highlight->setHighlightVarsMode(true);
        highlight->rehighlight();
        highlight->setHighlightVarsMode(false);
    }
    QStringList knownFunctions = highlight->getKnownFunctions();
    // function vars
    for (int i=0; i<knownFunctions.size(); i++) {
        QString funcKey = knownFunctions.at(i);
        QString clsName = "", funcName = "";
        if (funcKey.indexOf("::") >= 0) {
            clsName = funcKey.mid(0, funcKey.indexOf("::"));
            funcName = funcKey.mid(funcKey.indexOf("::")+2);
        } else {
            funcName = funcKey;
        }
        if (funcName.size() == 0) continue;
        QStringList vars = highlight->getKnownVars(clsName, funcName);
        QStringList used = highlight->getUsedVars(clsName, funcName);
        QString usedChain = used.join(",") + ",";
        for (int i=0; i<vars.size(); i++) {
            QString varName =vars.at(i);
            if (usedChain.indexOf(varName + ",") < 0) {
                int pos = highlight->getKnownVarPosition(clsName, funcName, varName);
                int blockNumber = highlight->getKnownVarBlockNumber(clsName, funcName, varName);
                if (pos >= 0 && blockNumber >= 0) {
                    QTextCursor curs = textCursor();
                    curs.movePosition(QTextCursor::Start);
                    if (blockNumber > 0) curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);
                    QString k = clsName + "::" + funcName + "::" + varName;
                    highlight->unusedVars[k.toStdString()] = pos;
                    unusedVarsBlocks.append(curs.block());
                }
            }
        }
    }
    // global vars
    QStringList vars = highlight->getKnownVars("", "");
    QStringList used = highlight->getUsedVars("", "");
    QString usedChain = used.join(",") + ",";
    for (int i=0; i<vars.size(); i++) {
        QString varName =vars.at(i);
        if (usedChain.indexOf(varName + ",") < 0) {
            int pos = highlight->getKnownVarPosition("", "", varName);
            int blockNumber = highlight->getKnownVarBlockNumber("", "", varName);
            if (pos >= 0 && blockNumber >= 0) {
                QTextCursor curs = textCursor();
                curs.movePosition(QTextCursor::Start);
                if (blockNumber > 0) curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);
                QString k = "::::" + varName;
                highlight->unusedVars[k.toStdString()] = pos;
                unusedVarsBlocks.append(curs.block());
            }
        }
    }
    // highlight
    blockSignals(true);
    std::unordered_map<int, int> highlightedBlocks;
    std::unordered_map<int, int>::iterator highlightedBlocksIterator;
    for (int i=0; i<unusedVarsBlocks.size(); i++) {
        QTextBlock block = unusedVarsBlocks.at(i);
        highlightedBlocksIterator = highlightedBlocks.find(block.blockNumber());
        if (highlightedBlocksIterator != highlightedBlocks.end()) continue;
        highlight->rehighlightBlock(block);
        highlightedBlocks[block.blockNumber()] = 1;
    }
    blockSignals(false);
    setReadOnly(false);
}

void Editor::cleanForSave()
{
    if (!cleanBeforeSave) return;
    QTextCursor curs = textCursor();
    curs.beginEditBlock();
    curs.movePosition(QTextCursor::Start);
    do {
        QString blockText = curs.block().text();
        if (blockText.size() == 0) continue;
        curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        do {
            int pos = curs.positionInBlock();
            if (pos <= 0) break;
            QChar c = blockText.at(pos-1);
            if (!c.isSpace()) break;
        } while(curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor));
        if (curs.hasSelection()) {
            curs.deleteChar();
            curs.clearSelection();
        }
    } while(curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor));
    curs.endEditBlock();
}

void Editor::save(QString name)
{
    if (name.size() == 0 && (fileName.size()==0 || !modified)) return;
    bool nameChanged = false;
    if (name.size() > 0) nameChanged = true;
    QFileInfo fInfo(fileName);
    if (name.size() == 0 && !fInfo.exists()) {
        if (!Helper::showQuestion(tr("Save"), tr("File not found. Create new one ?"))) return;
        nameChanged = true;
    }
    QDateTime dtModified = fInfo.lastModified();
    if (name.size() == 0 && fInfo.exists() && dtModified.time().msec() != lastModifiedMsec) {
        if (!Helper::showQuestion(tr("Save"), tr("File was modified externally. Save it anyway ?"))) return;
    }
    cleanForSave();
    QString text = getContent();
    if (name.size() == 0) name = fileName;
    if (!Helper::saveTextFile(name, text, encoding)) {
        Helper::showMessage(QObject::tr("Could not save file. Check file permissions."));
        return;
    }
    setFileName(name);
    modified = false;
    document()->setModified(modified);
    warningDisplayed = false;
    emit modifiedStateChanged(tabIndex, modified);
    emit statusBarText(tabIndex, tr("Saved"));
    if (nameChanged) emit filenameChanged(tabIndex, fileName);
    emit saved(tabIndex);
    if (highlight->getModeType() == MODE_MIXED) highlightUnusedVars(true);
}

void Editor::switchOverwrite()
{
    if (!overwrite) {
        overwrite = true;
        QFontMetrics fm(editorFont);
        /* QFontMetrics::width is deprecated */
        /*
        setCursorWidth(fm.width(' '));
        */
        setCursorWidth(fm.horizontalAdvance(" "));
    } else {
        overwrite = false;
        setCursorWidth(1);
    }
    setOverwriteMode(overwrite);
    emit statusBarText(tabIndex, ""); // update status bar
}

void Editor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu * menu = createStandardContextMenu();
    menu->setFont(QApplication::font());
    menu->addAction(tr("Reload"), this, SLOT(reloadRequested()));
    menu->addSeparator();
    menu->addAction(tr("Find \\ Replace"), this, SLOT(showSearchRequested()));
    menu->addAction(tr("Open declaration"), this, SLOT(showDeclarationRequested()));
    menu->addAction(tr("Search in files"), this, SLOT(searchInFilesRequested()));
    menu->addSeparator();
    menu->addAction(tr("Save"), this, SLOT(save()));
    menu->addSeparator();
    QAction * backAction = menu->addAction(tr("Back"), this, SLOT(back()));
    QAction * forwardAction = menu->addAction(tr("Forward"), this, SLOT(forward()));
    menu->addAction(tr("Go to ..."), this, SLOT(gotoLineRequest()));
    menu->addSeparator();
    menu->addAction(tr("Help"), this, SLOT(showHelpRequested()));

    if (!isBackable()) backAction->setEnabled(false);
    if (!isForwadable()) forwardAction->setEnabled(false);
    menu->exec(event->globalPos());
    delete menu;
}

void Editor::gotoLineRequest()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Go to"),
                                         tr("Line:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        int line = text.toInt();
        if (line > 0) gotoLine(line);
    }
}

void Editor::showDeclarationRequested()
{
    QTextCursor curs = textCursor();
    QTextBlock block = curs.block();
    QString blockText = block.text();
    int total = blockText.size();
    int pos = curs.positionInBlock();
    std::string mode = highlight->findModeAtCursor(&block, pos);
    if (!highlight->isStateOpen(&block, pos)) {
        QChar prevChar = '\0', nextChar = '\0';
        if (pos > 0 && curs.selectedText().size()==0) prevChar = blockText[pos - 1];
        if (pos < total && curs.selectedText().size()==0) nextChar = blockText[pos];
        // text under cursor
        QString cursorText = "", prevText = "", nextText = "";
        bool hasAlpha = false;

        QChar cursorTextPrevChar = '\0';
        int cursorTextPos = pos;
        if (curs.selectedText().size()==0) {
            for (int i=pos; i>0; i--) {
                QString c = blockText.mid(i-1, 1);
                if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
                cursorTextPrevChar = c[0];
                if (mode == MODE_PHP) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_" || c=="\\") prevText = c + prevText;
                    else break;
                } else if (mode == MODE_JS) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") prevText = c + prevText;
                    else break;
                } else if (mode == MODE_CSS) {
                    if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") prevText = c + prevText;
                    else break;
                } else {
                    if (isalnum(c[0].toLatin1()) || c=="_") prevText = c + prevText;
                    else break;
                }
                cursorTextPos = i-1;
            }
            for (int i=pos; i<total; i++) {
                QString c = blockText.mid(i, 1);
                if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
                if (mode == MODE_PHP) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_" || c=="\\") nextText += c;
                    else break;
                } else if (mode == MODE_JS) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") nextText += c;
                    else break;
                } else if (mode == MODE_CSS) {
                    if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") nextText += c;
                    else break;
                } else {
                    if (isalnum(c[0].toLatin1()) || c=="_") nextText += c;
                    else break;
                }
            }
            if (prevText.size()>0 && nextText.size()>0 && hasAlpha) {
                cursorText = prevText + nextText;
            }
        } else {
            cursorText = curs.selectedText();
            int cp = curs.selectionStart();
            curs.clearSelection();
            curs.setPosition(cp);
            if (cursorText.indexOf("::") >= 0) {
                int p = cursorText.lastIndexOf("::");
                cursorText = cursorText.mid(p+2);
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, p+2);
            }
            cursorTextPos = curs.positionInBlock();
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
        }
        int goLine = 0;
        QString name = "";
        if (cursorText.size() > 0 && mode == MODE_PHP) {
            name = cursorText;
            curs.movePosition(QTextCursor::StartOfBlock);
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
            QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
            QChar prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
            QString nsName = highlight->findNsPHPAtCursor(&block, cursorTextPos);
            QString clsName = highlight->findClsPHPAtCursor(&block, cursorTextPos);
            QString funcName = highlight->findFuncPHPAtCursor(&block, cursorTextPos);
            if ((prevChar == ">" && prevPrevChar == "-") || (prevChar == ":" && prevPrevChar == ":")) {
                curs.movePosition(QTextCursor::StartOfBlock);
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                QString prevType = detectCompleteTypeAtCursorPHP(curs, nsName, clsName, funcName);
                if (prevType.size() > 0 && prevType.at(0) == "\\") prevType = prevType.mid(1);
                if (prevType.size() > 0) {
                    name = prevType + "::" + cursorText;
                }
            }
            if (name.size() > 0) {
                if (name.indexOf("::") < 0) {
                    QString ns = "\\";
                    if (nsName.size() > 0) ns += nsName + "\\";
                    QString _clsName = clsName, _funcName = funcName;
                    if (clsName.size() > 0) _clsName = ns + clsName;
                    else if (funcName.size() > 0) _funcName = ns + funcName;
                    for (int i=0; i<parseResultPHP.functions.size(); i++) {
                        ParsePHP::ParseResultFunction _func = parseResultPHP.functions.at(i);
                        if (_func.name == ns+name && _func.clsName == _clsName && _func.line > 0 && _func.line-1 != block.blockNumber()) {
                            goLine = _func.line;
                        }
                    }
                    if (goLine == 0) {
                        for (int i=0; i<parseResultPHP.classes.size(); i++) {
                            ParsePHP::ParseResultClass _cls = parseResultPHP.classes.at(i);
                            if (_cls.name == ns+name && _cls.line > 0 && _cls.line-1 != block.blockNumber()) {
                                goLine = _cls.line;
                            }
                        }
                    }
                    if (goLine == 0) {
                        name = completeClassNamePHPAtCursor(curs, name, nsName);
                    }
                } else {
                    QStringList nameList = name.split("::");
                    for (int i=0; i<parseResultPHP.classes.size(); i++) {
                        ParsePHP::ParseResultClass _cls = parseResultPHP.classes.at(i);
                        if (_cls.name == "\\"+nameList.at(0) && _cls.line > 0 && _cls.line-1 != block.blockNumber()) {
                            for (int y=0; y<_cls.functionIndexes.size(); y++) {
                                if (_cls.functionIndexes.at(y) >= parseResultPHP.functions.size()) break;
                                ParsePHP::ParseResultFunction _func = parseResultPHP.functions.at(_cls.functionIndexes.at(y));
                                if (_func.name == nameList.at(1) && _func.line > 0 && _func.line-1 != block.blockNumber()) {
                                    goLine = _func.line;
                                }
                            }
                        }
                    }
                }
            }
        } else if (cursorText.size() > 0 && mode == MODE_JS) {
            name = cursorText;
            curs.movePosition(QTextCursor::StartOfBlock);
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
            QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
            for (int i=0; i<parseResultJS.functions.size(); i++) {
                ParseJS::ParseResultFunction _func = parseResultJS.functions.at(i);
                if (prevChar == "." && _func.clsName.size() == 0) continue;
                if (_func.name == name && _func.line > 0 && _func.line-1 != block.blockNumber()) {
                    goLine = _func.line;
                }
            }
        }
        if (goLine > 0) {
            gotoLine(goLine);
        } else if (name.size() > 0) {
            emit showDeclaration(getTabIndex(), name);
        }
    }
}

void Editor::showHelpRequested()
{
    hideCompletePopup();

    QTextCursor curs = textCursor();
    QTextBlock block = curs.block();
    QString blockText = block.text();
    int total = blockText.size();
    int pos = curs.positionInBlock();
    std::string mode = highlight->findModeAtCursor(&block, pos);
    if (!highlight->isStateOpen(&block, pos)) {
        QChar prevChar = '\0', nextChar = '\0';
        if (pos > 0 && curs.selectedText().size()==0) prevChar = blockText[pos - 1];
        if (pos < total && curs.selectedText().size()==0) nextChar = blockText[pos];
        // text under cursor
        QString cursorText = "", prevText = "", nextText = "";
        bool hasAlpha = false;

        QChar cursorTextPrevChar = '\0';
        int cursorTextPos = pos;
        if (curs.selectedText().size()==0) {
            for (int i=pos; i>0; i--) {
                QString c = blockText.mid(i-1, 1);
                if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
                cursorTextPrevChar = c[0];
                if (mode == MODE_PHP) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_" || c=="\\") prevText = c + prevText;
                    else break;
                } else if (mode == MODE_JS) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") prevText = c + prevText;
                    else break;
                } else if (mode == MODE_CSS) {
                    if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") prevText = c + prevText;
                    else break;
                } else {
                    if (isalnum(c[0].toLatin1()) || c=="_") prevText = c + prevText;
                    else break;
                }
                cursorTextPos = i-1;
            }
            for (int i=pos; i<total; i++) {
                QString c = blockText.mid(i, 1);
                if (!hasAlpha && isalpha(c[0].toLatin1())) hasAlpha = true;
                if (mode == MODE_PHP) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_" || c=="\\") nextText += c;
                    else break;
                } else if (mode == MODE_JS) {
                    if (isalnum(c[0].toLatin1()) || c=="$" || c=="_") nextText += c;
                    else break;
                } else if (mode == MODE_CSS) {
                    if (isalnum(c[0].toLatin1()) || c=="#" || c=="." || c=="-" || c=="_") nextText += c;
                    else break;
                } else {
                    if (isalnum(c[0].toLatin1()) || c=="_") nextText += c;
                    else break;
                }
            }
            if (prevText.size()>0 && nextText.size()>0 && hasAlpha) {
                cursorText = prevText + nextText;
            }
        } else {
            cursorText = curs.selectedText();
            int cp = curs.selectionStart();
            curs.clearSelection();
            curs.setPosition(cp);
            if (cursorText.indexOf("::") >= 0) {
                int p = cursorText.lastIndexOf("::");
                cursorText = cursorText.mid(p+2);
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, p+2);
            }
            cursorTextPos = curs.positionInBlock();
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
        }
        QString name = "";
        if (cursorText.size() > 0 && mode == MODE_PHP) {
            name = cursorText;
            curs.movePosition(QTextCursor::StartOfBlock);
            curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
            QChar prevChar = findPrevCharNonSpaceAtCursos(curs);
            QChar prevPrevChar = findPrevCharNonSpaceAtCursos(curs);
            QString nsName = highlight->findNsPHPAtCursor(&block, cursorTextPos);
            QString clsName = highlight->findClsPHPAtCursor(&block, cursorTextPos);
            QString funcName = highlight->findFuncPHPAtCursor(&block, cursorTextPos);
            if ((prevChar == ">" && prevPrevChar == "-") || (prevChar == ":" && prevPrevChar == ":")) {
                curs.movePosition(QTextCursor::StartOfBlock);
                curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, cursorTextPos);
                QString prevType = detectCompleteTypeAtCursorPHP(curs, nsName, clsName, funcName);
                if (prevType.size() > 0 && prevType.at(0) == "\\") prevType = prevType.mid(1);
                if (prevType.size() > 0) {
                    name = prevType + "::" + cursorText;
                }
            } else {
                name = completeClassNamePHPAtCursor(curs, name, nsName);
            }
        }
        if (name.size() > 0) {
            emit showHelp(getTabIndex(), name);
        }
    }
}

void Editor::showSearchRequested()
{
    findToggle();
}

void Editor::onUndoAvailable(bool available)
{
    isUndoAvailable = available;
    emit undoRedoChanged(getTabIndex());
}

void Editor::onRedoAvailable(bool available)
{
    isRedoAvailable = available;
    emit undoRedoChanged(getTabIndex());
}

bool Editor::isUndoable()
{
    return isUndoAvailable;
}

bool Editor::isRedoable()
{
    return isRedoAvailable;
}

bool Editor::isBackable()
{
    return backPositions.size() > 1;
}

bool Editor::isForwadable()
{
    return forwardPositions.size() > 0;
}

void Editor::back()
{
    if (!isBackable()) return;
    int position = backPositions.last();
    backPositions.removeLast();
    forwardPositions.append(position);
    position = backPositions.last();

    blockSignals(true);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    if (position >= 0 && position <= cursor.position()) {
        cursor.setPosition(position);
        setTextCursor(cursor);
        setFocus();
    }
    blockSignals(false);
    int line = cursor.blockNumber() + 1;
    scrollToMiddle(cursor, line);

    emit backForwardChanged(getTabIndex());
}

void Editor::forward()
{
    if (!isForwadable()) return;
    int position = forwardPositions.last();
    forwardPositions.removeLast();
    backPositions.append(position);

    blockSignals(true);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    if (position >= 0 && position <= cursor.position()) {
        cursor.setPosition(position);
        setTextCursor(cursor);
        setFocus();
    }
    blockSignals(false);
    int line = cursor.blockNumber() + 1;
    scrollToMiddle(cursor, line);

    emit backForwardChanged(getTabIndex());
}

void Editor::reloadRequested()
{
    if (fileName.size() == 0 || !Helper::fileExists(fileName)) return;
    if (!Helper::showQuestion(tr("Reload"), tr("Reload %1 ?").arg(fileName))) return;
    QString txt = Helper::loadFile(fileName, getEncoding(), getFallbackEncoding());
    QString _fileName = fileName;
    QString _extension = extension;
    reset();
    highlight->resetMode();
    setFileName(_fileName);
    convertNewLines(txt);
    setPlainText(txt);
    //resetExtraSelections();
    initMode(_extension);
    setFocus();
    detectTabsMode();

    emit modifiedStateChanged(tabIndex, modified);
    emit reloaded(tabIndex);
    initHighlighter();
}

void Editor::scrollToMiddle(QTextCursor cursor, int line)
{
    if (line <= 0) return;
    if (verticalScrollBar()->isVisible()) {
        int minV = verticalScrollBar()->minimum();
        int maxV = verticalScrollBar()->maximum();
        int v = verticalScrollBar()->value();
        int firstVisibleBlockIndex = getFirstVisibleBlockIndex();
        int lastVisibleBlockIndex = getLastVisibleBlockIndex();
        int blockHeight = static_cast<int>(document()->documentLayout()->blockBoundingRect(cursor.block()).height());
        int visibleBlocksCount = lastVisibleBlockIndex - firstVisibleBlockIndex;
        int middleLine = firstVisibleBlockIndex + visibleBlocksCount / 2 + 1;
        if (middleLine > line) {
            int offset = (middleLine - line) * blockHeight;
            if (offset < 0) offset = 0;
            int newV = v - offset;
            if (newV < minV) newV = minV;
            verticalScrollBar()->setValue(newV);
        } else if (middleLine < line) {
            int offset = (line - middleLine) * blockHeight;
            if (offset < 0) offset = 0;
            int newV = v + offset;
            if (newV > maxV) newV = maxV;
            verticalScrollBar()->setValue(newV);
        }
    }
}

void Editor::searchInFilesRequested()
{
    QTextCursor curs = textCursor();
    QString text = curs.selectedText();
    emit searchInFiles(text);
}

void Editor::qaBtnClicked()
{
    emit breadcrumbsClick(tabIndex);
}
