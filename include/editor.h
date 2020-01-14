/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include <QTextEdit>
#include <QRegularExpression>
#include <QLabel>
#include <QHash>
#include "spellcheckerinterface.h"
#include "settings.h"
#include "highlight.h"
#include "completepopup.h"
#include "highlightwords.h"
#include "completewords.h"
#include "helpwords.h"
#include "spellwords.h"
#include "tooltip.h"
#include "parsephp.h"
#include "parsejs.h"
#include "parsecss.h"
#include "git.h"

extern const int BIG_FILE_SIZE;

class Editor : public QTextEdit
{
    Q_OBJECT
public:
    Editor(SpellCheckerInterface * spellChecker, Settings * settings, HighlightWords * highlightWords, CompleteWords * completeWords, HelpWords * helpWords, SpellWords * spellWords, QWidget * parent = nullptr);
    ~Editor() override;
    void init();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineMarkAreaPaintEvent(QPaintEvent *event);
    void lineMapAreaPaintEvent(QPaintEvent *event);
    void searchPaintEvent(QPaintEvent *event);
    void breadcrumbsPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    int lineMarkAreaWidth();
    int lineMapAreaWidth();
    int searchWidgetHeight();
    int breadcrumbsHeight();
    void setFileName(QString name);
    QString getFileName();
    QString getFileExtension();
    void initMode(QString ext);
    void initHighlighter();
    void reset();
    void detectTabsMode();
    void convertNewLines(QString & txt);
    std::string getTabType();
    int getTabWidth();
    std::string getNewLineMode();
    std::string getEncoding();
    std::string getFallbackEncoding();
    bool isOverwrite();
    void hidePopups();
    void searchText(QString search, bool CaSe, bool Word, bool RegE, bool backwards = false, bool fromStart = false);
    void replaceText(QString search, QString replace, bool CaSe, bool Word, bool RegE);
    void replaceAllText(QString search, QString replace, bool CaSe, bool Word, bool RegE);
    void showSearch();
    void closeSearch();
    void searchFlagsChanged(QString searchStr, bool CaSe, bool Word, bool RegE);
    void clearWarnings();
    void setWarning(int line, QString text = "");
    void clearErrors();
    void setError(int line, QString text = "");
    void scrollLineMap(int y);
    void showLineNumber(int y);
    void showLineMap(int y);
    void showLineMark(int y);
    void addLineMark(int y);
    void setTabIndex(int index);
    int getTabIndex();
    bool isModified();
    void setModified(bool m);
    void updateSizes();
    QString getContent();
    void updateMarksAndMapArea();
    std::string getModeType();
    bool isReady();
    void gotoLine(int line, bool focus = true);
    void gotoLineSymbol(int line, int symbol);
    int getCursorLine();
    void setParseResult(ParsePHP::ParseResult result);
    void setParseResult(ParseJS::ParseResult result);
    void setParseResult(ParseCSS::ParseResult result);
    void setGitAnnotations(QHash<int, Git::Annotation> annotations);
    void setGitDiffLines(QHash<int, Git::DiffLine> mLines);
    bool isUndoable();
    bool isRedoable();
    bool isBackable();
    bool isForwadable();
    void highlightUnusedVars(bool update = true);
    void resetExtraSelections();
    void setParseError(bool error);
    bool getParseError();
    void highlightError(int pos, int length);
    void highlightErrorLine(int line);
    void setIsBigFile(bool isBig);
protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void clearTextHoverFormat();
    void clearErrorsFormat();
    void insertFromMimeData(const QMimeData *source) override;
    void updateWidgetsGeometry();
    void setTabsSettings();
    void updateViewportMargins();
    void updateLineWidgetsArea();
    void updateLineAnnotationView();
    void showLineAnnotation();
    QString cleanUpText(QString blockText);
    void cleanForSave();
    void showTooltip(int x, int y, QString text, bool richText = true, int fixedWidth = 0);
    void showTooltip(QTextCursor * curs, QString text, bool richText = true, int fixedWidth = 0);
    void hideTooltip();
    void setBreadcrumbsText(QString text);
    int findFirstVisibleBlockIndex();
    int findLastVisibleBlockIndex();
    int getFirstVisibleBlockIndex();
    int getLastVisibleBlockIndex();
    void highlightExtras(QChar prevChar='\0', QChar nextChar='\0', QChar cursorTextPrevChar='\0', QString cursorText="", int cursorTextPos=-1, std::string mode="");
    void highlightCloseCharPair(QChar openChar, QChar closeChar, QList<QTextEdit::ExtraSelection> * extraSelections);
    void highlightOpenCharPair(QChar openChar, QChar closeChar, QList<QTextEdit::ExtraSelection> * extraSelections);
    void highlightCloseTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections);
    void highlightOpenTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections);
    void highlightPHPCloseSpecialTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections);
    void highlightPHPOpenSpecialTagPair(QString tagName, int pos, QList<QTextEdit::ExtraSelection> * extraSelections);
    void highlightCurrentLine(QList<QTextEdit::ExtraSelection> * extraSelections);
    void showCompletePopup();
    void hideCompletePopup();
    void detectCompleteText(QString text, QChar cursorTextPrevChar, int cursorTextPos, std::string mode, int state);
    void detectCompleteTextHTML(QString text, QChar cursorTextPrevChar, int state);
    void detectCompleteTextCSS(QString text, QChar cursorTextPrevChar);
    void detectCompleteTextJS(QString text, int cursorTextPos);
    void detectCompleteTextPHP(QString text, int cursorTextPos);
    void detectCompleteTextPHPGlobalContext(QString text, int cursorTextPos, QChar prevChar, QChar prevPrevChar, QString prevWord, QTextCursor curs);
    void detectCompleteTextPHPObjectContext(QString text, int cursorTextPos, QChar prevChar, QChar prevPrevChar, QString prevWord, QTextCursor curs);
    void detectCompleteTextPHPNotFoundContext(QString text, QChar prevChar, QChar prevPrevChar);
    void detectCompleteTextRequest(QString text, int cursorTextPos, QChar prevChar, QChar prevPrevChar, std::string mode);
    void detectParsOpenAtCursor(QTextCursor & curs);
    void detectParsCloseAtCursor(QTextCursor & curs);
    QString detectCompleteTypeAtCursorPHP(QTextCursor & curs, QString nsName, QString clsName, QString funcName);
    void followTooltip();
    QChar findPrevCharNonSpaceAtCursos(QTextCursor & curs);
    QChar findNextCharNonSpaceAtCursos(QTextCursor & curs);
    QString findPrevWordNonSpaceAtCursor(QTextCursor & curs, std::string mode);
    QString findNextWordNonSpaceAtCursor(QTextCursor & curs, std::string mode);
    QString completeClassNamePHPAtCursor(QTextCursor & curs, QString prevWord, QString nsName);
    void scrollToMiddle(QTextCursor cursor, int line);
    void initSpellChecker();
    void suggestWords(QStringList words, int cursorTextPos);
    bool isKnownWord(QString word);
public slots:
    void save(QString name = "");
    void back();
    void forward();
    void findToggle();
protected slots:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void blockCountChanged(int);
    void horizontalScrollbarValueChanged(int);
    void verticalScrollbarValueChanged(int);
    void verticalScrollbarValueChangedDelayed();
    void textChanged();
    void textChangedDelayed();
    void cursorPositionChanged();
    void cursorPositionChangedDelayed();
    void backtab();
    void comment();
    void switchOverwrite();
    void completePopupSelected(QString text, QString data);
    void tooltip(int offset = 0);
    void parseResultChanged();
    void parseResultPHPChanged(bool async = true);
    void parseResultJSChanged(bool async = true);
    void parseResultCSSChanged(bool async = true);
    void showDeclarationRequested();
    void showHelpRequested();
    void showSearchRequested();
    void gotoLineRequest();
    void onUndoAvailable(bool available);
    void onRedoAvailable(bool available);
    void searchInFilesRequested();
    void highlightProgressChanged(int percent);
    void spellProgressChanged(int percent);
    void contentsChange(int position, int charsRemoved, int charsAdded);
    void duplicateLine();
    void deleteLine();
    void reloadRequested();
    void spellCheck(bool suggest = true, bool forceRehighlight = true);
    void spellCheckPasted();
private:
    SpellCheckerInterface * spellChecker;
    CompleteWords * CW;
    HighlightWords * HW;
    HelpWords * HPW;
    SpellWords * SW;
    int tabIndex;
    std::string tabWidthStr;
    std::string tabTypeStr;
    std::string detectTabTypeStr;
    int tabWidth;
    std::string tabType;
    bool detectTabType;
    std::string newLineMode;
    std::string encoding;
    std::string encodingFallback;
    bool overwrite;
    bool highlighterInitialized;
    bool isUndoAvailable;
    bool isRedoAvailable;

    QWidget * lineNumber;
    QWidget * lineMark;
    QWidget * lineMap;
    CompletePopup * completePopup;
    QWidget * search;
    Highlight * highlight;
    QWidget * breadcrumbs;
    QWidget * lineAnnotation;

    QFont editorFont;
    QFont editorPopupFont;
    QFont editorTooltipFont;
    QFont editorBreadcrumbsFont;

    QColor lineNumberBgColor;
    QColor lineNumberColor;
    QColor lineNumberModifiedBgColor;
    QColor lineNumberModifiedColor;
    QColor lineNumberDeletedBorderColor;
    QColor lineMarkBgColor;
    QColor lineMapBgColor;
    QColor lineMapScrollBgColor;
    QColor lineMapScrollAreaBgColor;
    QColor searchBgColor;
    QColor breadcrumbsBgColor;
    QColor breadcrumbsWarningBgColor;
    QColor breadcrumbsErrorBgColor;
    QColor breadcrumbsColor;
    QColor widgetBorderColor;
    QColor selectedLineBgColor;
    QColor selectedWordBgColor;
    QColor selectedCharBgColor;
    QColor selectedCharColor;
    QColor selectedTagBgColor;
    QColor selectedExpressionBgColor;
    QColor searchWordBgColor;
    QColor textColor;
    QColor bgColor;
    QColor searchInputBgColor;
    QColor searchInputErrorBgColor;
    QColor lineMarkColor;
    QColor lineErrorColor;
    QColor lineWarningColor;
    QColor lineMarkRectColor;
    QColor lineErrorRectColor;
    QColor lineWarningRectColor;
    QColor progressColor;

    QRegularExpression tagExpr;
    QRegularExpression tagOpenExpr;
    QRegularExpression phpExpr;
    QRegularExpression strExpr;
    QRegularExpression functionExpr;
    QRegularExpression tagExprIf;
    QRegularExpression tagExprElse;
    QRegularExpression tagExprEndif;
    QRegularExpression tagExprSwitch;
    QRegularExpression tagExprCase;
    QRegularExpression tagExprEndswitch;
    QRegularExpression tagExprWhile;
    QRegularExpression tagExprEndwhile;
    QRegularExpression tagExprFor;
    QRegularExpression tagExprEndfor;
    QRegularExpression tagExprForeach;
    QRegularExpression tagExprEndforeach;
    QRegularExpression stripTagsExpr;
    QRegularExpression functionNameExpr;
    QRegularExpression functionParamsExpr;
    QRegularExpression functionWordExpr;
    QRegularExpression classNameExpr;
    QRegularExpression colorExpr;
    QRegularExpression spellWordExpr;

    ParsePHP parserPHP;
    ParsePHP::ParseResult parseResultPHP;
    ParseJS parserJS;
    ParseJS::ParseResult parseResultJS;
    ParseCSS parserCSS;
    ParseCSS::ParseResult parseResultCSS;
    QHash<int, Git::Annotation> gitAnnotations;
    QHash<int, Git::DiffLine> gitDiffLines;

    QString fileName;
    QString extension;
    bool is_ready;
    std::string modeOnKeyPress;
    int lastKeyPressed;
    int lastKeyPressedBlockNumber;
    Tooltip tooltipLabel;
    QString tooltipSavedText;
    int tooltipSavedPageOffset;
    QString tooltipSavedOrigName;
    QStringList tooltipSavedList;
    QString tooltipBoldColorStr;
    QString tooltipBoldTagStart;
    QString tooltipBoldTagEnd;
    int tooltipSavedBlockNumber;

    bool focused;
    bool cursorPositionChangeLocked;
    bool scrollBarValueChangeLocked;
    bool textChangeLocked;
    bool modified;
    int lastModifiedMsec;
    bool warningDisplayed;
    bool parseLocked;
    bool showBreadcrumbs;
    bool cleanBeforeSave;

    bool searchCaSe;
    bool searchWord;
    bool searchRegE;
    QString searchString;

    std::unordered_map<int, std::string> markPoints;
    std::unordered_map<int, std::string>::iterator markPointsIterator;
    std::unordered_map<int, int> modifiedLines;
    std::unordered_map<int, int>::iterator modifiedLinesIterator;

    QVector<int> backPositions;
    QVector<int> forwardPositions;
    int lastCursorPositionBlockNumber;

    bool isParseError;
    bool parsePHPEnabled;
    bool parseJSEnabled;
    bool parseCSSEnabled;
    QColor unusedVariableColor;
    QList<QTextEdit::ExtraSelection> wordsExtraSelections;
    QList<QTextEdit::ExtraSelection> errorsExtraSelections;
    bool experimentalMode;
    int gitAnnotationLastLineNumber;
    bool annotationsEnabled;
    int parseResultChangedDelay;
    bool spellCheckerEnabled;
    bool spellLocked;
    QVector<int> spellBlocksQueue;
    QVector<int> spellPastedBlocksQueue;
    int spellCheckInitBlockNumber;
    bool isBlocksHeightEquals;
    bool isBigFile;
    int highlightProgressPercent;
    int spellProgressPercent;
signals:
    void ready(int index);
    void statusBarText(int index, QString text);
    void modifiedStateChanged(int index, bool m);
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
    void focusIn(int index);
    void breadcrumbsClick(int index);
    void warning(int index, QString slug, QString text);
    void showPopupText(int index, QString text);
    void showPopupError(int index, QString text);
};

#endif // EDITOR_H
