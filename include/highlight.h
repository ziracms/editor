/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "settings.h"
#include "highlightdata.h"
#include "highlightwords.h"

extern const std::string MODE_PHP;
extern const std::string MODE_JS;
extern const std::string MODE_CSS;
extern const std::string MODE_HTML;
extern const std::string MODE_MIXED;
extern const std::string MODE_UNKNOWN;

extern const int STATE_NONE;
extern const int STATE_COMMENT_ML_CSS;
extern const int STATE_COMMENT_ML_JS;
extern const int STATE_COMMENT_SL_JS;
extern const int STATE_COMMENT_ML_PHP;
extern const int STATE_COMMENT_SL_PHP;
extern const int STATE_COMMENT_ML_HTML;
extern const int STATE_STRING_SQ_HTML;
extern const int STATE_STRING_DQ_HTML;
extern const int STATE_STRING_SQ_JS;
extern const int STATE_STRING_DQ_JS;
extern const int STATE_STRING_SQ_PHP;
extern const int STATE_STRING_DQ_PHP;
extern const int STATE_STRING_HEREDOC;
extern const int STATE_STRING_NOWDOC;
extern const int STATE_STRING_SQ_CSS;
extern const int STATE_STRING_DQ_CSS;
extern const int STATE_TAG;
extern const int STATE_REGEXP_JS;

class Highlight : public QObject
{
    Q_OBJECT
public:
    Highlight(Settings * settings, HighlightWords * HW, QTextDocument * parent);
    ~Highlight();
    void rehighlight();
    void rehighlightBlock(QTextBlock & block);
    void highlightChanges(QTextCursor curs);
    void updateBlocks(int lastBlockNumber);
    void resetMode();
    void initMode(QString ext, int lastBlockNumber);
    std::string getModeType();
    std::string getMode();
    std::string findModeAtCursor(QTextBlock * block, int pos);
    int findStateAtCursor(QTextBlock * block, int pos);
    bool isStateOpen(QTextBlock * block, int pos);
    QString findNsPHPAtCursor(QTextBlock * block, int pos);
    QString findClsPHPAtCursor(QTextBlock * block, int pos);
    QString findFuncPHPAtCursor(QTextBlock * block, int pos);
    QString findFuncJSAtCursor(QTextBlock * block, int pos);
    QString findMediaCSSAtCursor(QTextBlock * block, int pos);
    QString findTagChainHTMLAtCursor(QTextBlock * block, int pos);
    QStringList getKnownVars(QString clsName, QString funcName);
    QStringList getUsedVars(QString clsName, QString funcName);
    QStringList getKnownFunctions(QString clsName = "");
    int getKnownVarPosition(QString clsName, QString funcName, QString varName);
    int getKnownVarBlockNumber(QString clsName, QString funcName, QString varName);
    void setHighlightVarsMode(bool varsMode);
    void setFirstRunMode(bool runMode);
    bool isDirty();
    std::unordered_map<std::string, int> unusedVars;
    std::unordered_map<std::string, int>::iterator unusedVarsIterator;
protected:
    void highlightBlock(QTextBlock & block, bool markDirty = true);
    void setFormat(int start, int count, const QTextCharFormat &format);
    QTextCharFormat format(int pos) const;
    void applyFormatChanges(bool markDirty = true);
    bool parseBlock(const QString & text);
    void reset();
    void addSpecialChar(QChar c, int pos);
    void addSpecialWord(QString w, int pos);
    void restoreState();
    void highlightString(int start, int length, const QTextCharFormat format);
    void highlightChar(int start, const QTextCharFormat format);
    void changeBlockState();
    bool detectMode(const QChar & c, int pos, bool isWSpace, bool isLast);
    bool detectModeOpen(const QChar & c, int pos, bool isWSpace, bool isLast);
    bool detectModeClose(const QChar & c, int pos, bool isWSpace);
    bool detectTag(const QChar c, int pos);
    bool detectCommentHTML(const QChar c);
    bool detectStringSQHTML(const QChar c, int pos);
    bool detectStringDQHTML(const QChar c, int pos);
    bool detectStringSQCSS(const QChar c, int pos);
    bool detectStringDQCSS(const QChar c, int pos);
    bool detectStringSQJS(const QChar c, int pos);
    bool detectStringDQJS(const QChar c, int pos);
    bool detectStringSQPHP(const QChar c, int pos);
    bool detectStringDQPHP(const QChar c, int pos);
    bool detectRegexpJS(const QChar c, int pos, bool isWSPace, bool isAlnum);
    bool detectMLCommentCSS(const QChar c, int pos);
    bool detectMLCommentJS(const QChar c, int pos);
    bool detectMLCommentPHP(const QChar c, int pos);
    bool detectSLCommentJS(const QChar c, int pos);
    bool detectSLCommentPHP(const QChar c, int pos);
    bool detectStringBPHP(const QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast);
    int detectKeywordJS(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast);
    int detectKeywordPHP(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast, bool forceDetect);
    int detectKeywordCSS(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast);
    int detectKeywordHTML(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast);
    bool detectExpressionPHP(const QChar c, int pos);
    bool detectExpressionJS(const QChar c, int pos);
    bool parseMode(const QChar & c, int pos, bool isWSpace, bool isLast, std::string & pMode, int & pState);
    void parseHTML(const QChar & c, int pos, bool isAlpha, bool isAlnum, bool isLast);
    void parseCSS(const QChar & c, int pos, bool isAlpha, bool isAlnum, bool isWSpace, bool isLast, int & keywordCSSStartPrev, int & keywordCSSLengthPrev, bool & cssValuePart);
    void parseJS(const QChar & c, int pos, bool isAlpha, bool isAlnum, bool isWSpace, bool isLast, int & keywordJSStartPrev, int & keywordJSLengthPrev);
    void parsePHP(const QChar c, int pos, bool isAlpha, bool isAlnum, bool isWSpace, bool isLast, int & keywordPHPStartPrev, int & keywordPHPLengthPrev);
    void updateState(const QChar & c, int pos, int & pState);
    void openBlockDataLists();
    void closeBlockDataLists(int textSize);
    void highlightUnderline();
private:
    QTextDocument * doc;
    QVector<QTextCharFormat> formatChanges;
    HighlightWords * HW;
    bool highlightSpaces;
    bool highlightTabs;
    bool dirty;

    std::unordered_map<std::string, std::string> modeTypes;
    std::unordered_map<std::string, std::string>::iterator modeTypesIterator;
    std::unordered_map<std::string, std::string> variables;
    std::unordered_map<std::string, std::string>::iterator variablesIterator;
    std::unordered_map<std::string, std::string> usedVariables;
    std::unordered_map<std::string, std::string>::iterator usedVariablesIterator;
    std::unordered_map<std::string, std::string> clsProps;
    std::unordered_map<std::string, std::string>::iterator clsPropsIterator;

    std::unordered_map<std::string, std::string> knownVars;
    std::unordered_map<std::string, std::string>::iterator knownVarsIterator;
    std::unordered_map<std::string, int> knownVarsBlocks;
    std::unordered_map<std::string, int>::iterator knownVarsBlocksIterator;
    std::unordered_map<std::string, int> knownVarsPositions;
    std::unordered_map<std::string, int>::iterator knownVarsPositionsIterator;
    std::unordered_map<std::string, std::string> usedVars;
    std::unordered_map<std::string, std::string>::iterator usedVarsIterator;
    std::unordered_map<std::string, std::string> knownFunctions;
    std::unordered_map<std::string, std::string>::iterator knownFunctionsIterator;

    std::unordered_map<std::string, std::string> jsNames;
    std::unordered_map<std::string, std::string>::iterator jsNamesIterator;
    std::unordered_map<std::string, std::string> cssNames;
    std::unordered_map<std::string, std::string>::iterator cssNamesIterator;

    QVector<QChar> specialChars;
    QVector<int> specialCharsPos;
    QVector<QString> specialWords;
    QVector<int> specialWordsPos;

    bool enabled;
    QTextBlock cBlock;
    HighlightData * blockData;
    std::string modeType;
    std::string mode;
    std::string prevMode;
    QVector<int> modeStarts;
    QVector<int> modeEnds;
    QVector<std::string> modeTags;
    QVector<int> stateStarts;
    QVector<int> stateEnds;
    QVector<int> stateIds;
    QString stringBlock;
    QString stringBstring;
    int state;
    int prevState;
    int prevPrevState;
    unsigned long int block_state;
    QString modeString;
    QString modeStringC;
    std::string modeExpect;
    std::string modeExpectC;
    bool modeSkip;
    bool modeSkipC;
    int modeSpos;
    int modeCpos;
    int modeCposed;
    QString prevModeString;
    QString prevModeStringC;
    std::string prevModeExpect;
    std::string prevModeExpectC;
    bool prevModeSkip;
    bool prevModeSkipC;
    int prevModeSpos;
    int prevModeCpos;
    int tagOpened;
    int commentHTMLOpened;
    int stringSQOpenedHTML;
    int stringDQOpenedHTML;
    int stringSQOpenedCSS;
    int stringDQOpenedCSS;
    int stringSQOpenedJS;
    int stringDQOpenedJS;
    int stringSQOpenedPHP;
    int stringDQOpenedPHP;
    int stringBOpened;
    int stringBStart;
    int stringBExpect;
    int commentSLOpenedJS;
    int commentMLOpenedJS;
    int commentSLOpenedPHP;
    int commentMLOpenedPHP;
    int commentMLOpenedCSS;
    QString stringEscStringCSS;
    QString stringEscStringJS;
    QString prevStringEscStringCSS;
    QString prevStringEscStringJS;
    QString stringEscStringPHP;
    QString commentHTMLString;
    QString commentJSStringML;
    QString commentPHPStringML;
    QString commentJSStringSL;
    QString commentPHPStringSL;
    QString commentCSSStringML;
    int regexpOpenedJS;
    QString regexpEscStringJS;
    QString prevRegexpEscStringJS;
    QString regexpPrevCharJS;
    QString keywordStringJS;
    QString keywordStringCSS;
    QString keywordStringPHP;
    QString keywordStringHTML;
    int keywordJSOpened;
    int keywordCSSOpened;
    int keywordPHPOpened;
    int keywordHTMLOpened;
    int exprOpenedPHP;
    int exprOpenedJS;
    QString exprEscStringPHP;
    QString exprEscStringJS;
    QString prevExprEscStringJS;
    QString keywordJSprevChar;
    QString keywordCSSprevChar;
    QString keywordCSSprevPrevChar;
    QString keywordPHPprevChar;
    QString keywordPHPprevPrevChar;
    QString keywordPHPprevString;
    QString keywordPHPprevStringPrevChar;
    QString keywordJSprevString;
    QString keywordJSprevStringPrevChar;
    QString stringEscVariablePHP;
    QString keywordHTMLprevChar;
    QString keywordHTMLprevPrevChar;
    int keywordPHPScopedOpened;
    bool keywordPHPScoped;
    int keywordJSScopedOpened;
    bool keywordJSScoped;
    QString stringEscVariableJS;
    QString prevStringEscVariableJS;
    int bracesCSS;
    int bracesJS;
    int bracesPHP;
    int parensCSS;
    int parensJS;
    int parensPHP;
    bool cssMediaScope;
    int underlineStart;
    int underlineEnd;
    QString nsNamePHP;
    QList<int> nsScopeChainPHP;
    QString nsChainPHP;
    QVector<int> nsStartsPHP;
    QVector<int> nsEndsPHP;
    QVector<QString> nsNamesPHP;
    QString clsNamePHP;
    QList<int> clsScopeChainPHP;
    QString clsChainPHP;
    QVector<int> clsStartsPHP;
    QVector<int> clsEndsPHP;
    QVector<QString> clsNamesPHP;
    QString funcNamePHP;
    QList<int> funcScopeChainPHP;
    QString funcChainPHP;
    QVector<int> funcStartsPHP;
    QVector<int> funcEndsPHP;
    QVector<QString> funcNamesPHP;
    QString expectedNsNamePHP;
    QString expectedClsNamePHP;
    QString expectedFuncNamePHP;
    int expectedFuncParsPHP;
    QStringList expectedFuncArgsPHP;
    QVector<int> expectedFuncArgsPHPPositions;
    QVector<int> expectedFuncArgsPHPBlocks;
    int nsScopePHP;
    int clsScopePHP;
    int funcScopePHP;
    QStringList varsChainsPHP;
    QStringList usedVarsChainsPHP;
    QString varsGlobChainPHP;
    QString usedVarsGlobChainPHP;
    QString varsClsChainPHP;
    QString varsChainPHP;
    QString usedVarsChainPHP;
    bool clsOpenPHP;
    QVector<bool> clsOpensPHP;
    QString varsClsOpenChainPHP;
    QStringList clsPropsChainPHP;
    QString funcNameJS;
    QList<int> funcScopeChainJS;
    QString funcChainJS;
    QVector<int> funcStartsJS;
    QVector<int> funcEndsJS;
    QVector<QString> funcNamesJS;
    QString expectedFuncNameJS;
    QString expectedFuncVarJS;
    int expectedFuncParsJS;
    int funcScopeJS;
    QString varsChainJS;
    QStringList expectedFuncArgsJS;
    QString mediaNameCSS;
    QVector<int> mediaStartsCSS;
    QVector<int> mediaEndsCSS;
    QVector<QString> mediaNamesCSS;
    QString expectedMediaNameCSS;
    QString cssNamesChain;
    int expectedMediaParsCSS;
    int mediaScopeCSS;
    bool isColorKeyword;
    QString tagChainHTML;
    QVector<int> tagChainStartsHTML;
    QVector<int> tagChainEndsHTML;
    QVector<QString> tagChainsHTML;
    bool highlightVarsMode;
    bool firstRunMode;
    bool rehighlightBlockMode;
    int lastVisibleBlockNumber;
signals:
    void progressChanged(int percent);
};

#endif // HIGHLIGHT_H
