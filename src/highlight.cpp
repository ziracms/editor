/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "highlight.h"
#include <QTextDocument>
#include <QTextStream>
#include <QTextCursor>
#include <QTextBlock>
#include "helper.h"

const std::string MODE_PHP = "php";
const std::string MODE_JS = "js";
const std::string MODE_CSS = "css";
const std::string MODE_HTML = "html";
const std::string MODE_MIXED = "mixed";
const std::string MODE_UNKNOWN = "unknown";

const int STATE_NONE = 0;
const int STATE_COMMENT_ML_CSS = 1;
const int STATE_COMMENT_ML_JS = 2;
const int STATE_COMMENT_SL_JS = 3;
const int STATE_COMMENT_ML_PHP = 4;
const int STATE_COMMENT_SL_PHP = 5;
const int STATE_COMMENT_ML_HTML = 6;
const int STATE_STRING_SQ_HTML = 7;
const int STATE_STRING_DQ_HTML = 8;
const int STATE_STRING_SQ_JS = 9;
const int STATE_STRING_DQ_JS = 10;
const int STATE_STRING_SQ_PHP = 11;
const int STATE_STRING_DQ_PHP = 12;
const int STATE_STRING_HEREDOC = 13;
const int STATE_STRING_NOWDOC = 14;
const int STATE_STRING_SQ_CSS = 15;
const int STATE_STRING_DQ_CSS = 16;
const int STATE_TAG = 17;
const int STATE_REGEXP_JS = 18;
const int STATE_EXPRESSION_JS = 19;
const int STATE_EXPRESSION_PHP = 20;

const int EXTRA_HIGHLIGHT_BLOCKS_COUNT = 100; // should be >= 1

Highlight::Highlight(Settings * settings, HighlightWords * hWords, QTextDocument * parent) :
    doc(parent)
{
    std::string highlightTabsStr = settings->get("highlight_tabs");
    std::string highlightSpacesStr = settings->get("highlight_spaces");
    if (highlightTabsStr == "yes") highlightTabs = true;
    else highlightTabs = false;
    if (highlightSpacesStr == "yes") highlightSpaces = true;
    else highlightSpaces = false;

    // highlight modes by extension
    QString phpExtentionsStr = QString::fromStdString(settings->get("highlight_php_extensions"));
    QStringList phpExtensionsList = phpExtentionsStr.split(",");
    for (int i=0; i<phpExtensionsList.size(); i++) {
        QString ext = phpExtensionsList.at(i);
        modeTypes[ext.trimmed().toStdString()] = MODE_MIXED;
    }
    QString jsExtentionsStr = QString::fromStdString(settings->get("highlight_js_extensions"));
    QStringList jsExtensionsList = jsExtentionsStr.split(",");
    for (int i=0; i<jsExtensionsList.size(); i++) {
        QString ext = jsExtensionsList.at(i);
        modeTypes[ext.trimmed().toStdString()] = MODE_JS;
    }
    QString cssExtentionsStr = QString::fromStdString(settings->get("highlight_css_extensions"));
    QStringList cssExtensionsList = cssExtentionsStr.split(",");
    for (int i=0; i<cssExtensionsList.size(); i++) {
        QString ext = cssExtensionsList.at(i);
        modeTypes[ext.trimmed().toStdString()] = MODE_CSS;
    }
    QString htmlExtentionsStr = QString::fromStdString(settings->get("highlight_html_extensions"));
    QStringList htmlExtensionsList = htmlExtentionsStr.split(",");
    for (int i=0; i<htmlExtensionsList.size(); i++) {
        QString ext = htmlExtensionsList.at(i);
        modeTypes[ext.trimmed().toStdString()] = MODE_HTML;
    }

    std::string spellColorStr = settings->get("editor_line_warning_color");
    spellColor = QColor(QString::fromStdString(spellColorStr));

    enabled = false;
    modeType = MODE_UNKNOWN;
    block_state = 0;
    highlightVarsMode = false;
    firstRunMode = false;
    rehighlightBlockMode = false;
    lastVisibleBlockNumber = -1;
    dirty = false;
    isBigFile = false;

    HW = hWords;
}

void Highlight::setFormat(int start, int count, const QTextCharFormat &format)
{
    if (start < 0 || start >= formatChanges.count()) return;
    const int end = qMin(start + count, formatChanges.count());
    for (int i = start; i < end; i++) {
        formatChanges[i] = format;
    }
}

QTextCharFormat Highlight::format(int pos) const
{
    if (pos < 0 || pos >= formatChanges.count()) return QTextCharFormat();
    return formatChanges.at(pos);
}

void Highlight::applyFormatChanges(bool markDirty)
{
    QTextLayout *layout = cBlock.layout();
    QVector<QTextLayout::FormatRange> ranges;
    int i = 0;
    while (i < formatChanges.count()) {
        QTextLayout::FormatRange r;
        while (i < formatChanges.count() && formatChanges.at(i) == r.format) ++i;
        if (i == formatChanges.count()) break;
        r.start = i;
        r.format = formatChanges.at(i);
        while (i < formatChanges.count() && formatChanges.at(i) == r.format) ++i;
        Q_ASSERT(i <= formatChanges.count());
        r.length = i - r.start;
        ranges << r;
    }
    layout->setFormats(ranges);
    if (markDirty) doc->markContentsDirty(cBlock.position(), cBlock.length() > 0 ? 1 : 0);
}

std::string Highlight::getModeType()
{
    return modeType;
}

std::string Highlight::getMode()
{
    return mode;
}

void Highlight::reset()
{
    state = STATE_NONE;
    prevState = STATE_NONE;
    prevPrevState = STATE_NONE;
    mode = MODE_HTML;
    prevMode = mode;
    modeStarts.clear();
    modeEnds.clear();
    modeTags.clear();
    stateStarts.clear();
    stateEnds.clear();
    stateIds.clear();
    stringBlock = "";
    stringBstring = "";
    modeString = "";
    modeStringC = "";
    modeExpect = "";
    modeExpectC = "";
    modeSkip = false;
    modeSkipC = false;
    modeSpos = -1;
    modeCpos = -1;
    modeCposed = -1;
    prevModeString = "";
    prevModeStringC = "";
    prevModeExpect = "";
    prevModeExpectC = "";
    prevModeSkip = false;
    prevModeSkipC = false;
    prevModeSpos = -1;
    prevModeCpos = -1;
    tagOpened = -1;
    commentHTMLString = "";
    commentHTMLOpened = -1;
    stringSQOpenedHTML = -1;
    stringDQOpenedHTML = -1;
    stringSQOpenedCSS = -1;
    stringDQOpenedCSS = -1;
    stringSQOpenedJS = -1;
    stringDQOpenedJS = -1;
    stringSQOpenedPHP = -1;
    stringDQOpenedPHP = -1;
    stringBOpened = -1;
    stringBStart = -1;
    commentSLOpenedJS = -1;
    commentMLOpenedJS = -1;
    commentSLOpenedPHP = -1;
    commentMLOpenedPHP = -1;
    commentMLOpenedCSS = -1;
    commentJSStringML = "";
    commentPHPStringML = "";
    commentJSStringSL = "";
    commentPHPStringSL = "";
    commentCSSStringML = "";
    stringEscStringCSS = "";
    stringEscStringJS = "";
    prevStringEscStringCSS = "";
    prevStringEscStringJS = "";
    stringEscStringPHP = "";
    stringBExpect = -1;
    regexpOpenedJS = -1;
    regexpEscStringJS = "";
    prevRegexpEscStringJS = "";
    regexpPrevCharJS = "";
    keywordStringJS = "";
    keywordStringCSS = "";
    keywordStringPHP = "";
    keywordStringHTML = "";
    keywordJSOpened = -1;
    keywordCSSOpened = -1;
    keywordPHPOpened = -1;
    keywordHTMLOpened = -1;
    exprOpenedPHP = -1;
    exprOpenedJS = -1;
    exprEscStringPHP = "";
    exprEscStringJS = "";
    prevExprEscStringJS = "";
    keywordJSprevChar = "";
    keywordCSSprevChar = "";
    keywordCSSprevPrevChar = "";
    keywordPHPprevChar = "";
    keywordPHPprevPrevChar = "";
    keywordPHPprevString = "";
    keywordPHPprevStringPrevChar = "";
    keywordJSprevString = "";
    keywordJSprevStringPrevChar = "";
    stringEscVariablePHP = "";
    stringEscVariableJS = "";
    prevStringEscVariableJS = "";
    keywordHTMLprevChar = "";
    keywordHTMLprevPrevChar = "";
    bracesCSS = 0;
    bracesJS = 0;
    bracesPHP = 0;
    parensCSS = 0;
    parensJS = 0;
    parensPHP = 0;
    cssMediaScope = false;
    keywordPHPScopedOpened = -1;
    keywordPHPScoped = false;
    keywordJSScopedOpened = -1;
    keywordJSScoped = false;
    specialChars.clear();
    specialCharsPos.clear();
    specialWords.clear();
    specialWordsPos.clear();
    nsNamePHP = "";
    nsChainPHP = "";
    nsScopeChainPHP.clear();
    nsStartsPHP.clear();
    nsEndsPHP.clear();
    nsNamesPHP.clear();
    clsNamePHP = "";
    clsScopeChainPHP.clear();
    clsChainPHP = "";
    clsStartsPHP.clear();
    clsEndsPHP.clear();
    clsNamesPHP.clear();
    funcNamePHP = "";
    funcScopeChainPHP.clear();
    funcChainPHP = "";
    funcStartsPHP.clear();
    funcEndsPHP.clear();
    funcNamesPHP.clear();
    expectedNsNamePHP = "";
    expectedClsNamePHP = "";
    expectedFuncNamePHP = "";
    expectedFuncParsPHP = -1;
    expectedFuncArgsPHP.clear();
    expectedFuncArgsPHPPositions.clear();
    expectedFuncArgsPHPBlocks.clear();
    nsScopePHP = -1;
    clsScopePHP = -1;
    funcScopePHP = -1;
    varsChainsPHP.clear();
    usedVarsChainsPHP.clear();
    varsGlobChainPHP = "";
    usedVarsGlobChainPHP = "";
    varsClsChainPHP = "";
    varsChainPHP = "";
    usedVarsChainPHP = "";
    clsOpenPHP = false;
    clsOpensPHP.clear();
    varsClsOpenChainPHP = "";
    clsPropsChainPHP.clear();
    funcNameJS = "";
    funcScopeChainJS.clear();
    funcChainJS = "";
    funcStartsJS.clear();
    funcEndsJS.clear();
    funcNamesJS.clear();
    expectedFuncNameJS = "";
    expectedFuncVarJS = "";
    expectedFuncParsJS = -1;
    funcScopeJS = -1;
    varsChainJS = "";
    expectedFuncArgsJS.clear();
    mediaNameCSS = "";
    mediaStartsCSS.clear();
    mediaEndsCSS.clear();
    mediaNamesCSS.clear();
    expectedMediaNameCSS = "";
    cssNamesChain = "";
    expectedMediaParsCSS = -1;
    mediaScopeCSS = -1;
    isColorKeyword = true;
    tagChainHTML = "";
    tagChainStartsHTML.clear();
    tagChainEndsHTML.clear();
    tagChainsHTML.clear();
    variables.clear();
    usedVariables.clear();
    clsProps.clear();
    jsNames.clear();
    cssNames.clear();
}

void Highlight::addSpecialChar(QChar c, int pos)
{
    specialChars.append(c);
    specialCharsPos.append(pos);
}

void Highlight::addSpecialWord(QString w, int pos)
{
    specialWords.append(w);
    specialWordsPos.append(pos);
}

void Highlight::resetMode()
{
    enabled = false;
    modeType = MODE_UNKNOWN;
    block_state = 0;
    highlightVarsMode = false;
    firstRunMode = false;
    rehighlightBlockMode = false;
    lastVisibleBlockNumber = -1;
    dirty = false;
    foundModes.clear();
}

void Highlight::setIsBigFile(bool isBig) {
    isBigFile = isBig;
}

void Highlight::initMode(QString ext, int lastBlockNumber)
{
    enabled = true;
    if (ext.size()==0) return;
    modeTypesIterator = modeTypes.find(ext.toLower().toStdString());
    if (modeTypesIterator != modeTypes.end()) {
        modeType = modeTypesIterator->second;
        lastVisibleBlockNumber = lastBlockNumber;
        if (modeType != MODE_MIXED) {
            foundModes.append(QString::fromStdString(modeType));
        }
    }
}

void Highlight::updateBlocks(int lastBlockNumber)
{
    if (!enabled) return;
    lastVisibleBlockNumber = lastBlockNumber;
    QTextCursor curs = QTextCursor(doc);
    curs.movePosition(QTextCursor::Start);
    bool wantUpdate = false;
    do {
        QTextBlock block = curs.block();
        if (!block.isValid()) break;
        if (block.blockNumber() > lastVisibleBlockNumber + EXTRA_HIGHLIGHT_BLOCKS_COUNT) break;
        HighlightData * userData = dynamic_cast<HighlightData *>(block.userData());
        if (userData != nullptr && userData->wantUpdate) {
            wantUpdate = true;
            break;
        }
    } while(curs.movePosition(QTextCursor::NextBlock));
    if (wantUpdate) highlightChanges(curs);
}

std::string Highlight::findModeAtCursor(QTextBlock * block, int pos)
{
    if (modeType != MODE_MIXED) return modeType;
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->modeStarts.size()>0 && blockData->modeEnds.size()>0 && blockData->modeTags.size()>0 &&
        blockData->modeStarts.size()==blockData->modeEnds.size() && blockData->modeStarts.size()==blockData->modeTags.size()
    ) {
        for (int i=0; i<blockData->modeStarts.size(); i++) {
            if (blockData->modeStarts.at(i) <= pos && blockData->modeEnds.at(i) >= pos) {
                return blockData->modeTags.at(i);
            }
        }
    }
    return MODE_HTML;
}

int Highlight::findStateAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->stateStarts.size()>0 && blockData->stateEnds.size()>0 && blockData->stateIds.size()>0 &&
        blockData->stateStarts.size()==blockData->stateEnds.size() && blockData->stateStarts.size()==blockData->stateIds.size()
    ) {
        for (int i=0; i<blockData->stateStarts.size(); i++) {
            if (blockData->stateStarts.at(i) <= pos && blockData->stateEnds.at(i) >= pos) {
                return blockData->stateIds.at(i);
            }
        }
    }
    return STATE_NONE;
}

bool Highlight::isStateOpen(QTextBlock * block, int pos)
{
    int openState = findStateAtCursor(block, pos);
    return (openState != STATE_NONE && openState != STATE_TAG) ? true : false;
}

QString Highlight::findNsPHPAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->nsStartsPHP.size()>0 && blockData->nsEndsPHP.size()>0 && blockData->nsNamesPHP.size()>0 &&
        blockData->nsStartsPHP.size()==blockData->nsEndsPHP.size() && blockData->nsStartsPHP.size()==blockData->nsNamesPHP.size()
    ) {
        for (int i=0; i<blockData->nsStartsPHP.size(); i++) {
            if (blockData->nsStartsPHP.at(i) <= pos && blockData->nsEndsPHP.at(i) >= pos) {
                return blockData->nsNamesPHP.at(i);
            }
        }
    }
    return "";
}

QString Highlight::findClsPHPAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->clsStartsPHP.size()>0 && blockData->clsEndsPHP.size()>0 && blockData->clsNamesPHP.size()>0 &&
        blockData->clsStartsPHP.size()==blockData->clsEndsPHP.size() && blockData->clsStartsPHP.size()==blockData->clsNamesPHP.size()
    ) {
        for (int i=0; i<blockData->clsStartsPHP.size(); i++) {
            if (blockData->clsStartsPHP.at(i) <= pos && blockData->clsEndsPHP.at(i) >= pos) {
                return blockData->clsNamesPHP.at(i);
            }
        }
    }
    return "";
}

QString Highlight::findFuncPHPAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->funcStartsPHP.size()>0 && blockData->funcEndsPHP.size()>0 && blockData->funcNamesPHP.size()>0 &&
        blockData->funcStartsPHP.size()==blockData->funcEndsPHP.size() && blockData->funcStartsPHP.size()==blockData->funcNamesPHP.size()
    ) {
        for (int i=0; i<blockData->funcStartsPHP.size(); i++) {
            if (blockData->funcStartsPHP.at(i) <= pos && blockData->funcEndsPHP.at(i) >= pos) {
                return blockData->funcNamesPHP.at(i);
            }
        }
    }
    return "";
}

QString Highlight::findFuncJSAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->funcStartsJS.size()>0 && blockData->funcEndsJS.size()>0 && blockData->funcNamesJS.size()>0 &&
        blockData->funcStartsJS.size()==blockData->funcEndsJS.size() && blockData->funcStartsJS.size()==blockData->funcNamesJS.size()
    ) {
        for (int i=0; i<blockData->funcStartsJS.size(); i++) {
            if (blockData->funcStartsJS.at(i) <= pos && blockData->funcEndsJS.at(i) >= pos) {
                return blockData->funcNamesJS.at(i);
            }
        }
    }
    return "";
}

QString Highlight::findMediaCSSAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->mediaStartsCSS.size()>0 && blockData->mediaEndsCSS.size()>0 && blockData->mediaNamesCSS.size()>0 &&
        blockData->mediaStartsCSS.size()==blockData->mediaEndsCSS.size() && blockData->mediaStartsCSS.size()==blockData->mediaNamesCSS.size()
    ) {
        for (int i=0; i<blockData->mediaStartsCSS.size(); i++) {
            if (blockData->mediaStartsCSS.at(i) <= pos && blockData->mediaEndsCSS.at(i) >= pos) {
                return blockData->mediaNamesCSS.at(i);
            }
        }
    }
    return "";
}

QString Highlight::findTagChainHTMLAtCursor(QTextBlock * block, int pos)
{
    HighlightData * blockData = dynamic_cast<HighlightData *>(block->userData());
    if (blockData != nullptr && blockData->tagChainStartsHTML.size()>0 && blockData->tagChainEndsHTML.size()>0 && blockData->tagChainsHTML.size()>0 &&
        blockData->tagChainStartsHTML.size()==blockData->tagChainEndsHTML.size() && blockData->tagChainStartsHTML.size()==blockData->tagChainsHTML.size()
    ) {
        for (int i=0; i<blockData->tagChainStartsHTML.size(); i++) {
            if (blockData->tagChainStartsHTML.at(i) <= pos && blockData->tagChainEndsHTML.at(i) >= pos) {
                return blockData->tagChainsHTML.at(i);
            }
        }
    }
    return "";
}

QStringList Highlight::getKnownVars(QString clsName, QString funcName)
{
    QStringList vars;
    if (clsName == "anonymous class" || funcName == "anonymous function") return vars;
    QString k = clsName + "::" + funcName;
    knownVarsIterator = knownVars.find(k.toStdString());
    if (knownVarsIterator != knownVars.end()) {
        QString varsChain = QString::fromStdString(knownVarsIterator->second);
        QStringList varsList = varsChain.split(",");
        for (int i=0; i<varsList.size(); i++) {
            QString varName = varsList.at(i);
            varName = varName.trimmed();
            if (varName.size() == 0) continue;
            vars.append(varName);
        }
    }
    return vars;
}

int Highlight::getKnownVarPosition(QString clsName, QString funcName, QString varName)
{
    if (clsName == "anonymous class" || funcName == "anonymous function") return -1;
    QString k = clsName + "::" + funcName;
    knownVarsIterator = knownVars.find(k.toStdString());
    if (knownVarsIterator != knownVars.end()) {
        QString varsChain = QString::fromStdString(knownVarsIterator->second) + ",";
        if (varsChain.indexOf(varName + ",") < 0) return - 1;
    }
    QString kk = k + "::" + varName;
    knownVarsPositionsIterator = knownVarsPositions.find(kk.toStdString());
    if (knownVarsPositionsIterator != knownVarsPositions.end()) {
        return knownVarsPositionsIterator->second;
    }
    return -1;
}

int Highlight::getKnownVarBlockNumber(QString clsName, QString funcName, QString varName)
{
    if (clsName == "anonymous class" || funcName == "anonymous function") return -1;
    QString k = clsName + "::" + funcName;
    knownVarsIterator = knownVars.find(k.toStdString());
    if (knownVarsIterator != knownVars.end()) {
        QString varsChain = QString::fromStdString(knownVarsIterator->second) + ",";
        if (varsChain.indexOf(varName + ",") < 0) return - 1;
    }
    QString kk = k + "::" + varName;
    knownVarsBlocksIterator = knownVarsBlocks.find(kk.toStdString());
    if (knownVarsBlocksIterator != knownVarsBlocks.end()) {
        return knownVarsBlocksIterator->second;
    }
    return -1;
}

QStringList Highlight::getUsedVars(QString clsName, QString funcName)
{
    QStringList vars;
    if (clsName == "anonymous class" || funcName == "anonymous function") return vars;
    QString k = clsName + "::" + funcName;
    usedVarsIterator = usedVars.find(k.toStdString());
    if (usedVarsIterator != usedVars.end()) {
        QString varsChain = QString::fromStdString(usedVarsIterator->second);
        QStringList varsList = varsChain.split(",");
        for (int i=0; i<varsList.size(); i++) {
            QString varName = varsList.at(i);
            varName = varName.trimmed();
            if (varName.size() == 0) continue;
            vars.append(varName);
        }
    }
    return vars;
}

QStringList Highlight::getKnownFunctions(QString clsName)
{
    QStringList funcs;
    if (clsName == "anonymous class") return funcs;
    for (auto it : knownFunctions) {
        QString k = QString::fromStdString(it.first);
        if (clsName.size() > 0 && k.indexOf(clsName+"::") != 0) continue;
        funcs.append(k);
    }
    return funcs;
}

void Highlight::setHighlightVarsMode(bool varsMode)
{
    highlightVarsMode = varsMode;
}

void Highlight::setFirstRunMode(bool runMode)
{
    firstRunMode = runMode;
}

QStringList Highlight::getFoundModes()
{
    return foundModes;
}

void Highlight::highlightString(int start, int length, const QTextCharFormat format)
{
    setFormat(start, length, format);
}

void Highlight::highlightChar(int start, const QTextCharFormat format)
{
    setFormat(start, 1, format);
}

void Highlight::changeBlockState()
{
    if (highlightVarsMode || firstRunMode) return;
    cBlock.setUserState(static_cast<int>(++block_state));
}

bool Highlight::detectMode(const QChar & c, int pos, bool isWSpace, bool isLast)
{
    bool changed;
    if (mode == MODE_HTML) {
        changed = detectModeOpen(c, pos, isWSpace, isLast);
    } else {
        changed = detectModeOpen(c, pos, isWSpace, isLast);
        if (!changed) changed = detectModeClose(c, pos, isWSpace);
    }
    return changed;
}

bool Highlight::detectModeOpen(const QChar & c, int pos, bool isWSpace, bool isLast)
{
    std::string _mode = mode;

    if (mode!=MODE_PHP && c == "<") {
        // php in opening tag
        prevModeString = modeString;
        prevModeExpect = modeExpect;
        prevModeSkip = modeSkip;
        prevModeSpos = modeSpos;

        modeString = c;
        modeExpect = "";
        modeSkip = false;
        modeSpos = pos;
        return false;
    }

    if (modeString.size()==0) return false;

    if (!isWSpace && !modeSkip) {
        modeString += c;
    }

    if (modeString == "<?") {
        modeExpect = MODE_PHP;
    } else if (modeString == "<?php") {
        modeExpect = MODE_PHP;
    } else if (modeString == "<script") {
        modeExpect = MODE_JS;
    } else if (modeString == "<style") {
        modeExpect = MODE_CSS;
    } else {
        modeExpect = "";
    }

    if (modeString == "<?=") {
        mode = MODE_PHP;
    } else if (modeExpect == MODE_PHP && (isWSpace || isLast)) {
        mode = MODE_PHP;
    } else if (_mode == MODE_HTML && modeString == "<script>") {
        mode = MODE_JS;
    } else if (_mode == MODE_HTML && modeExpect == MODE_JS && c == ">") {
        mode = MODE_JS;
    } else if (_mode == MODE_HTML && modeString == "<style>") {
        mode = MODE_CSS;
    } else if (_mode == MODE_HTML && modeExpect == MODE_CSS && c == ">") {
        mode = MODE_CSS;
    } else if (c == ">" || modeString.size()>8) {
        modeString = "";
        modeExpect = "";
        modeSpos = -1;
    }

    if (isWSpace) {
        modeSkip = true;
    }

    if (mode != _mode) {
        if (mode!=MODE_PHP && ((tagOpened>=0 && tagOpened!=modeSpos) || commentHTMLOpened>=0)) {
            modeString = "";
            modeExpect = "";
            modeSkip = false;
            modeCpos = -1;

            mode = _mode;
            return false;
        } else if (mode!=MODE_PHP && (stringSQOpenedHTML>=0 || stringDQOpenedHTML>=0)) {
            mode = _mode;
            return false;
        } else {
            modeString = "";
            modeExpect = "";
            modeSkip = false;
            modeCpos = -1;

            prevMode = _mode;
        }
        return true;
    }

    return false;
}

bool Highlight::detectModeClose(const QChar & c, int pos, bool isWSpace)
{
    std::string _mode = mode;

    if (mode == MODE_PHP && stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentMLOpenedPHP < 0 && exprOpenedPHP < 0 && c == "?") {
        modeExpectC = "";
        modeStringC = c;
        modeCpos = pos;
        modeCposed = modeCpos;
        modeSkipC = false;
        return false;
    } else if (mode == MODE_JS && stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && exprOpenedJS < 0 && c == "<") {
        // php in closing tag
        prevModeExpectC = modeExpectC;
        prevModeStringC = modeStringC;
        prevModeCpos = modeCpos;
        prevModeSkipC = modeSkipC;

        modeExpectC = "";
        modeStringC = c;
        modeCpos = pos;
        modeCposed = modeCpos;
        modeSkipC = false;
        return false;
    } else if (mode == MODE_CSS && commentMLOpenedCSS < 0 && c == "<") {
        // php in closing tag
        prevModeExpectC = modeExpectC;
        prevModeStringC = modeStringC;
        prevModeCpos = modeCpos;
        prevModeSkipC = modeSkipC;

        modeExpectC = "";
        modeStringC = c;
        modeCpos = pos;
        modeCposed = modeCpos;
        modeSkipC = false;
        return false;
    }

    if (modeStringC.size()==0) return false;

    if (!isWSpace && !modeSkipC) {
        modeStringC += c;
    }

    if (modeStringC == "</script") {
        modeExpectC = MODE_JS;
    } else if (modeStringC == "</style") {
        modeExpectC = MODE_CSS;
    } else {
        modeExpectC = "";
    }

    if (mode == MODE_PHP && modeStringC == "?>") {
        mode = prevMode;
    } else if (mode == MODE_JS && modeStringC == "</script>") {
        mode = prevMode;
    } else if (mode == MODE_JS && modeExpectC == MODE_JS && c == ">") {
        mode = prevMode;
    } else if (mode == MODE_CSS && modeStringC == "</style>") {
        mode = prevMode;
    } else if (mode == MODE_CSS && modeExpectC == MODE_CSS && c == ">") {
        mode = prevMode;
    } else if (c == ">" || modeStringC.size()>9) {
        modeExpectC = "";
        modeStringC = "";
        modeCpos = -1;
    }

    if (isWSpace) {
        modeSkipC = true;
    }

    if (mode != _mode) {
        modeExpectC = "";
        modeStringC = "";
        modeSpos = -1;
        modeSkipC = false;
        prevMode = MODE_HTML;

        return true;
    }

    return false;
}

bool Highlight::detectTag(const QChar c, int pos)
{
    if (stringSQOpenedHTML>=0 || stringDQOpenedHTML>=0) return false;
    if (tagOpened < 0 && c == "<") {
        tagOpened = pos;
        state = STATE_TAG;
        return true;
    } else if (tagOpened >= 0 && c == ">" && commentHTMLOpened < 0) {
        tagOpened = -1;
        state = STATE_NONE;
        return true;
    }
    return false;
}

bool Highlight::detectCommentHTML(const QChar c)
{
    if (tagOpened>=0 && commentHTMLOpened < 0 && commentHTMLString.size()<=4) {
        commentHTMLString += c;
        if (commentHTMLString=="<!--") {
            commentHTMLString = "";
            commentHTMLOpened = tagOpened;
            state = STATE_COMMENT_ML_HTML;
            return true;
        }
    } else if (tagOpened>=0 && commentHTMLOpened >= 0 && c == "-" && commentHTMLString.size()<2) {
        commentHTMLString += c;
    } else if (tagOpened>=0 && commentHTMLOpened >= 0 && c != "-" && c != ">") {
        commentHTMLString = "";
    } else if (tagOpened>=0 && commentHTMLOpened >= 0 && c == ">" && commentHTMLString == "--") {
        commentHTMLString = "";
        commentHTMLOpened = -1;
        tagOpened = -1;
        state = STATE_NONE;
        return true;
    } else if (tagOpened<0) {
        commentHTMLString = "";
    }
    return false;
}

bool Highlight::detectMLCommentCSS(const QChar c, int pos)
{
    bool opened = stringSQOpenedCSS >= 0 || stringDQOpenedCSS >= 0 || commentMLOpenedCSS >= 0;
    if (!opened && commentCSSStringML.size()==0 && c == "/") {
        commentCSSStringML = c;
    } else if (!opened && commentCSSStringML.size()==1) {
        commentCSSStringML += c;
        if (commentCSSStringML=="/*") {
            commentCSSStringML = "";
            commentMLOpenedCSS = pos;
            state = STATE_COMMENT_ML_CSS;
            return true;
        } else {
            commentCSSStringML = "";
        }
    } else if (commentMLOpenedCSS >= 0 && c == "*") {
        commentCSSStringML = c;
    } else if (commentMLOpenedCSS >= 0 && commentCSSStringML.size()==1) {
        commentCSSStringML += c;
        if (commentCSSStringML == "*/") {
            commentCSSStringML = "";
            commentMLOpenedCSS = -1;
            state = STATE_NONE;
            return true;
        } else {
            commentCSSStringML = "";
        }
    }

    return false;
}

bool Highlight::detectSLCommentJS(const QChar c, int pos)
{
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || exprOpenedJS >= 0;
    if (!opened && commentJSStringSL.size()==0 && c == "/" && (regexpOpenedJS < 0 || regexpOpenedJS == pos)) {
        commentJSStringSL = c;
    } else if (!opened && commentJSStringSL.size()==1) {
        commentJSStringSL += c;
        if (commentJSStringSL=="//") {
            commentJSStringSL = "";
            commentSLOpenedJS = pos;
            if (regexpOpenedJS>=0) {
                regexpOpenedJS = -1;
            }
            state = STATE_COMMENT_SL_JS;
            return true;
        } else {
            commentJSStringSL = "";
        }
    }

    return false;
}

bool Highlight::detectMLCommentJS(const QChar c, int pos)
{
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || exprOpenedJS >= 0;
    if (!opened && commentJSStringML.size()==0 && c == "/" && (regexpOpenedJS < 0 || regexpOpenedJS == pos)) {
        commentJSStringML = c;
    } else if (!opened && commentJSStringML.size()==1) {
        commentJSStringML += c;
        if (commentJSStringML=="/*") {
            commentJSStringML = "";
            commentMLOpenedJS = pos;
            if (regexpOpenedJS>=0) regexpOpenedJS = -1;
            state = STATE_COMMENT_ML_JS;
            return true;
        } else {
            commentJSStringML = "";
        }
    } else if (commentMLOpenedJS >= 0 && c == "*") {
        commentJSStringML = c;
    } else if (commentMLOpenedJS >= 0 && commentJSStringML.size()==1) {
        commentJSStringML += c;
        if (commentJSStringML == "*/") {
            commentJSStringML = "";
            commentMLOpenedJS = -1;
            state = STATE_NONE;
            return true;
        } else {
            commentJSStringML = "";
        }
    }

    return false;
}

bool Highlight::detectMLCommentPHP(const QChar c, int pos)
{
    bool opened = stringSQOpenedPHP >= 0 || stringDQOpenedPHP >= 0 || stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || exprOpenedPHP >= 0;
    if (!opened && commentPHPStringML.size()==0 && c == "/") {
        commentPHPStringML = c;
    } else if (!opened && commentPHPStringML.size()==1) {
        commentPHPStringML += c;
        if (commentPHPStringML=="/*") {
            commentPHPStringML = "";
            commentMLOpenedPHP = pos;
            state = STATE_COMMENT_ML_PHP;
            return true;
        } else {
            commentPHPStringML = "";
        }
    } else if (commentMLOpenedPHP >= 0 && c == "*") {
        commentPHPStringML = c;
    } else if (commentMLOpenedPHP >= 0 && commentPHPStringML.size()==1) {
        commentPHPStringML += c;
        if (commentPHPStringML == "*/") {
            commentPHPStringML = "";
            commentMLOpenedPHP = -1;
            state = STATE_NONE;
            return true;
        } else {
            commentPHPStringML = "";
        }
    }

    return false;
}

bool Highlight::detectSLCommentPHP(const QChar c, int pos)
{
    bool opened = stringSQOpenedPHP >= 0 || stringDQOpenedPHP >= 0 || stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || exprOpenedPHP >= 0;
    if (!opened && commentPHPStringSL.size()==0 && (c == "/" || c == "#")) {
        if (c == "#") {
            commentPHPStringSL = "";
            commentSLOpenedPHP = pos;
            state = STATE_COMMENT_SL_PHP;
            return true;
        } else {
            commentPHPStringSL = c;
        }
    } else if (!opened && commentPHPStringSL.size()==1) {
        commentPHPStringSL += c;
        if (commentPHPStringSL=="//") {
            commentPHPStringSL = "";
            commentSLOpenedPHP = pos;
            state = STATE_COMMENT_SL_PHP;
            return true;
        } else {
            commentPHPStringSL = "";
        }
    }

    return false;
}

bool Highlight::detectStringSQHTML(const QChar c, int pos)
{
    bool opened = stringSQOpenedHTML >= 0 || stringDQOpenedHTML >= 0 || commentHTMLOpened >= 0;
    if (!opened && tagOpened >= 0 && c == "'") {
        stringSQOpenedHTML = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_STRING_SQ_HTML;
        return true;
    } else if (stringSQOpenedHTML >= 0 && c == "'") {
        stringSQOpenedHTML = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        return true;
    }
    return false;
}

bool Highlight::detectStringDQHTML(const QChar c, int pos)
{
    bool opened = stringSQOpenedHTML >= 0 || stringDQOpenedHTML >= 0 || commentHTMLOpened >= 0;
    if (!opened && tagOpened >= 0 && c == "\"") {
        stringDQOpenedHTML = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_STRING_DQ_HTML;
        return true;
    } else if (stringDQOpenedHTML >= 0 && c == "\"") {
        stringDQOpenedHTML = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        return true;
    }
    return false;
}

bool Highlight::detectStringSQCSS(const QChar c, int pos)
{
    bool opened = stringSQOpenedCSS >= 0 || stringDQOpenedCSS >= 0 || commentMLOpenedCSS >= 0;
    if (!opened && c == "'") {
        stringSQOpenedCSS = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_STRING_SQ_CSS;
        stringEscStringCSS = "";
        return true;
    } else if (stringSQOpenedCSS >= 0 && c == "'" && stringEscStringCSS.size()%2 == 0) {
        stringSQOpenedCSS = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        stringEscStringCSS = "";
        return true;
    } else if (stringSQOpenedCSS >= 0 && c == "\\") {
        stringEscStringCSS += c;
    } else if (stringSQOpenedCSS >= 0) {
        if (c == "<") prevStringEscStringCSS = stringEscStringCSS;
        stringEscStringCSS = "";
    }
    return false;
}

bool Highlight::detectStringDQCSS(const QChar c, int pos)
{
    bool opened = stringSQOpenedCSS >= 0 || stringDQOpenedCSS >= 0 || commentMLOpenedCSS >= 0;
    if (!opened && c == "\"") {
        stringDQOpenedCSS = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_STRING_DQ_CSS;
        stringEscStringCSS = "";
        return true;
    } else if (stringDQOpenedCSS >= 0 && c == "\"" && stringEscStringCSS.size()%2 == 0) {
        stringDQOpenedCSS = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        stringEscStringCSS = "";
        return true;
    } else if (stringDQOpenedCSS >= 0 && c == "\\") {
        stringEscStringCSS += c;
    } else if (stringDQOpenedCSS >= 0) {
        if (c == "<") prevStringEscStringCSS = stringEscStringCSS;
        stringEscStringCSS = "";
    }
    return false;
}

bool Highlight::detectStringSQJS(const QChar c, int pos)
{
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || regexpOpenedJS >= 0 || exprOpenedJS >= 0;
    if (!opened && c == "'") {
        stringSQOpenedJS = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_STRING_SQ_JS;
        stringEscStringJS = "";
        return true;
    } else if (stringSQOpenedJS >= 0 && c == "'" && stringEscStringJS.size()%2 == 0) {
        stringSQOpenedJS = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        stringEscStringJS = "";
        return true;
    } else if (stringSQOpenedJS >= 0 && c == "\\") {
        stringEscStringJS += c;
    } else if (stringSQOpenedJS >= 0) {
        if (c == "<") prevStringEscStringJS = stringEscStringJS;
        stringEscStringJS = "";
    }
    return false;
}

bool Highlight::detectStringDQJS(const QChar c, int pos)
{
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || regexpOpenedJS >= 0 || exprOpenedJS >= 0;
    if (!opened && c == "\"") {
        stringDQOpenedJS = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_STRING_DQ_JS;
        stringEscStringJS = "";
        return true;
    } else if (stringDQOpenedJS >= 0 && c == "\"" && stringEscStringJS.size()%2 == 0) {
        stringDQOpenedJS = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        stringEscStringJS = "";
        return true;
    } else if (stringDQOpenedJS >= 0 && c == "\\") {
        stringEscStringJS += c;
    } else if (stringDQOpenedJS >= 0) {
        if (c == "<") prevStringEscStringJS = stringEscStringJS;
        stringEscStringJS = "";
    }
    return false;
}

bool Highlight::detectStringSQPHP(const QChar c, int pos)
{
    bool opened = stringSQOpenedPHP >= 0 || stringDQOpenedPHP >= 0 || stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || exprOpenedPHP >= 0;
    if (!opened && c == "'") {
        stringSQOpenedPHP = pos;
        state = STATE_STRING_SQ_PHP;
        stringEscStringPHP = "";
        return true;
    } else if (stringSQOpenedPHP >= 0 && c == "'" && stringEscStringPHP.size()%2 == 0) {
        stringSQOpenedPHP = -1;
        state = STATE_NONE;
        stringEscStringPHP = "";
        return true;
    } else if (stringSQOpenedPHP >= 0 && c == "\\") {
        stringEscStringPHP += c;
    } else if (stringSQOpenedPHP >= 0) {
        stringEscStringPHP = "";
    }
    return false;
}

bool Highlight::detectStringDQPHP(const QChar c, int pos)
{
    bool opened = stringSQOpenedPHP >= 0 || stringDQOpenedPHP >= 0 || stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || exprOpenedPHP >= 0;
    if (!opened && c == "\"") {
        stringDQOpenedPHP = pos;
        state = STATE_STRING_DQ_PHP;
        stringEscStringPHP = "";
        return true;
    } else if (stringDQOpenedPHP >= 0 && c == "\"" && stringEscStringPHP.size()%2 == 0 && !keywordPHPScoped) {
        stringDQOpenedPHP = -1;
        state = STATE_NONE;
        stringEscStringPHP = "";
        return true;
    } else if (stringDQOpenedPHP >= 0 && c == "\\") {
        stringEscStringPHP += c;
    } else if (stringDQOpenedPHP >= 0) {
        stringEscStringPHP = "";
    }
    return false;
}

bool Highlight::detectStringBPHP(const QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast)
{
    bool opened = stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || exprOpenedPHP >= 0;
    if (!opened && stringBstring.size()<=3 && c == "<" && stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0) {
        stringBstring += c;
        if (stringBStart<0) stringBStart = pos;
    } else if (!opened && stringBstring.size()==3 && stringBlock.size()==0 && stringSQOpenedPHP < 0 && c == "\"" && stringBExpect < 0) {
        stringDQOpenedPHP = -1;
        state = STATE_NONE;
        stringEscStringPHP = "";
        stringBExpect = STATE_STRING_HEREDOC;
    }  else if (!opened && stringBstring.size()==3 && stringBlock.size()==0 && stringDQOpenedPHP < 0 && c == "'" && stringBExpect < 0) {
        stringSQOpenedPHP = -1;
        state = STATE_NONE;
        stringEscStringPHP = "";
        stringBExpect = STATE_STRING_NOWDOC;
    } else if (!opened && stringBstring.size()==3 && stringBlock.size()==0 && isAlpha) {
        stringBlock = c;
        if (isLast && stringBExpect < 0) {
            stringBstring = "";
            stringBOpened = pos;
            stringBExpect = -1;
            state = STATE_STRING_HEREDOC;
            return true;
        }
    } else if (!opened && stringBstring.size()==3 && stringBlock.size()>0 && isAlnum) {
        stringBlock += c;
        if (isLast && stringBExpect < 0) {
            stringBstring = "";
            stringBOpened = pos;
            stringBExpect = -1;
            state = STATE_STRING_HEREDOC;
            return true;
        }
    }  else if (!opened && stringBstring.size()==3 && stringBlock.size()>0 && stringSQOpenedPHP < 0 && stringBExpect == STATE_STRING_HEREDOC && c == "\"" && isLast) {
        stringDQOpenedPHP = -1;
        stringEscStringPHP = "";
        stringBExpect = -1;
        stringBstring = "";
        stringBOpened = pos;
        state = STATE_STRING_HEREDOC;
        return true;
    } else if (!opened && stringBstring.size()==3 && stringBlock.size()>0 && stringDQOpenedPHP < 0 && stringBExpect == STATE_STRING_NOWDOC && c == "'" && isLast) {
        stringSQOpenedPHP = -1;
        stringEscStringPHP = "";
        stringBExpect = -1;
        stringBstring = "";
        stringBOpened = pos;
        state = STATE_STRING_NOWDOC;
        return true;
    } else if (stringBOpened>=0 && stringBlock.size()>0) {
        stringBstring += c;
        if (stringBstring == stringBlock+";" && isLast) {
            stringBstring = "";
            stringBlock = "";
            stringBOpened = -1;
            stringBExpect = -1;
            state = STATE_NONE;
            return true;
        }
    } else {
        stringBstring = "";
        stringBlock = "";
        stringBStart = -1;
        stringBExpect = -1;
    }
    return false;
}

bool Highlight::detectRegexpJS(const QChar c, int pos, bool isWSPace, bool isAlnum)
{
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || regexpOpenedJS >= 0 || exprOpenedJS >= 0;
    if (!opened && c == "/" && regexpPrevCharJS.size()==0) {
        regexpOpenedJS = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_REGEXP_JS;
        regexpEscStringJS = "";
        regexpPrevCharJS = "";
        return true;
    } else if (regexpOpenedJS >= 0 && c == "/" && regexpEscStringJS.size()%2 == 0) {
        regexpOpenedJS = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        regexpEscStringJS = "";
        regexpPrevCharJS = "";
        return true;
    } else if (regexpOpenedJS >= 0 && c == "\\") {
        regexpEscStringJS += c;
    } else if (regexpOpenedJS >= 0) {
        if (c == "<") prevRegexpEscStringJS = regexpEscStringJS;
        regexpEscStringJS = "";
    } else if (regexpOpenedJS < 0 && !isWSPace && (isAlnum || c == "$" || c == ")" || c == "]" || c == "<")) {
        regexpPrevCharJS = c;
    } else if (regexpOpenedJS < 0 && !isWSPace) {
        regexpPrevCharJS = "";
    }
    return false;
}

int Highlight::detectKeywordHTML(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast) {
    bool opened = stringSQOpenedHTML >= 0 || stringDQOpenedHTML >= 0 || commentHTMLOpened >= 0 || keywordHTMLOpened >= 0;
    if (!opened && keywordHTMLOpened!=-2 && isAlnum && !isAlpha) {
        keywordHTMLOpened = -2;
    } else if (!opened && keywordHTMLOpened==-2 && !isAlnum) {
        keywordHTMLOpened = -1;
        keywordHTMLprevPrevChar = keywordHTMLprevChar;
        keywordHTMLprevChar = c;
    } else if (!opened && keywordHTMLOpened!=-2 && isAlpha) {
        keywordStringHTML = c;
        keywordHTMLOpened = pos;
    } else if (keywordHTMLOpened>=0 && isAlnum) {
        keywordStringHTML += c;
    } else if (keywordHTMLOpened<0) {
        keywordHTMLprevPrevChar = keywordHTMLprevChar;
        keywordHTMLprevChar = c;
    }
    if (keywordHTMLOpened>=0 && (!isAlnum || isLast)) {
        int kOpened = keywordHTMLOpened;
        keywordHTMLOpened = -1;
        return kOpened;
    }
    return -1;
}

int Highlight::detectKeywordCSS(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast) {
    if (!isAlpha && c == "-") isAlpha = true;
    if (!isAlnum && c == "-") isAlnum = true;
    if (!isAlpha && isAlnum && keywordCSSprevChar == "#") isAlpha = true;
    bool opened = stringSQOpenedCSS >= 0 || stringDQOpenedCSS >= 0 || commentMLOpenedCSS >= 0 || keywordCSSOpened >= 0;
    if (!opened && keywordCSSOpened!=-2 && isAlnum && !isAlpha) {
        keywordCSSOpened = -2;
    } else if (!opened && keywordCSSOpened==-2 && !isAlnum) {
        keywordCSSOpened = -1;
        keywordCSSprevPrevChar = keywordCSSprevChar;
        keywordCSSprevChar = c;
    } else if (!opened && keywordCSSOpened!=-2 && isAlpha) {
        keywordStringCSS = c;
        keywordCSSOpened = pos;
        if (keywordCSSprevChar == "#") isColorKeyword = true;
        else isColorKeyword = false;
    } else if (keywordCSSOpened>=0 && isAlnum) {
        keywordStringCSS += c;
    } else if (keywordCSSOpened<0) {
        keywordCSSprevPrevChar = keywordCSSprevChar;
        keywordCSSprevChar = c;
    }
    if (keywordCSSOpened>=0 && isColorKeyword && isAlnum && !isdigit(c.toLatin1())) {
        QChar _c = c.toLower();
        if (_c != "a" && _c != "b" && _c != "c" && _c != "d" && _c != "e" && _c != "f") isColorKeyword = false;
    }
    if (keywordCSSOpened>=0 && (!isAlnum || isLast)) {
        int kOpened = keywordCSSOpened;
        keywordCSSOpened = -1;
        return kOpened;
    }
    return -1;
}

int Highlight::detectKeywordJS(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast) {
    if (!isAlpha && c == "$") isAlpha = true;
    if (!isAlnum && c == "$") isAlnum = true;
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || regexpOpenedJS >= 0 || keywordJSOpened >= 0 || exprOpenedJS >= 0;
    if (!opened && keywordJSOpened!=-2 && isAlnum && !isAlpha) {
        keywordJSOpened = -2;
    } else if (!opened && keywordJSOpened==-2 && !isAlnum) {
        keywordJSOpened = -1;
        keywordJSprevChar = c;
    } else if (!opened && keywordJSOpened!=-2 && isAlpha) {
        keywordStringJS = c;
        keywordJSOpened = pos;
    } else if (keywordJSOpened>=0 && isAlnum) {
        keywordStringJS += c;
    } else if (keywordJSOpened<0) {
        keywordJSprevChar = c;
    }
    if (keywordJSOpened>=0 && (!isAlnum || isLast)) {
        int kOpened = keywordJSOpened;
        keywordJSOpened = -1;
        return kOpened;
    }
    return -1;
}

int Highlight::detectKeywordPHP(QChar c, int pos, bool isAlpha, bool isAlnum, bool isLast, bool forceDetect) {
    bool opened = stringSQOpenedPHP >= 0 || stringDQOpenedPHP >= 0 || stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || keywordPHPOpened >= 0 || exprOpenedPHP >= 0;
    if (forceDetect && keywordPHPOpened<0) opened = false;
    if (!opened && keywordPHPOpened!=-2 && isAlnum && !isAlpha) {
        keywordPHPOpened = -2;
    } else if (!opened && keywordPHPOpened==-2 && !isAlnum) {
        keywordPHPOpened = -1;
        keywordPHPprevPrevChar = keywordPHPprevChar;
        keywordPHPprevChar = c;
    } else if (!opened && keywordPHPOpened!=-2 && isAlpha) {
        keywordStringPHP = c;
        keywordPHPOpened = pos;
    } else if (keywordPHPOpened>=0 && isAlnum) {
        keywordStringPHP += c;
    } else if (keywordPHPOpened<0) {
        keywordPHPprevPrevChar = keywordPHPprevChar;
        keywordPHPprevChar = c;
    }
    if (keywordPHPOpened>=0 && (!isAlnum || isLast)) {
        int kOpened = keywordPHPOpened;
        keywordPHPOpened = -1;
        return kOpened;
    }
    return -1;
}

bool Highlight::detectExpressionJS(const QChar c, int pos)
{
    bool opened = stringSQOpenedJS >= 0 || stringDQOpenedJS >= 0 || commentSLOpenedJS >= 0 || commentMLOpenedJS >= 0 || regexpOpenedJS >= 0 || exprOpenedJS >= 0;
    if (!opened && c == "`") {
        exprOpenedJS = pos;
        prevPrevState = prevState;
        prevState = state;
        state = STATE_EXPRESSION_JS;
        exprEscStringJS = "";
        return true;
    } else if (exprOpenedJS >= 0 && c == "`" && exprEscStringJS.size()%2 == 0) {
        exprOpenedJS = -1;
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;
        exprEscStringJS = "";
        return true;
    } else if (exprOpenedJS >= 0 && c == "\\") {
        exprEscStringJS += c;
    } else if (exprOpenedJS >= 0) {
        if (c == "<") prevExprEscStringJS = exprEscStringJS;
        exprEscStringJS = "";
    }
    return false;
}

bool Highlight::detectExpressionPHP(const QChar c, int pos)
{
    bool opened = stringSQOpenedPHP >= 0 || stringDQOpenedPHP >= 0 || stringBOpened >= 0 || commentSLOpenedPHP >= 0 || commentMLOpenedPHP >= 0 || exprOpenedPHP >= 0;
    if (!opened && c == "`") {
        exprOpenedPHP = pos;
        state = STATE_EXPRESSION_PHP;
        exprEscStringPHP = "";
        return true;
    } else if (exprOpenedPHP >= 0 && c == "`" && exprEscStringPHP.size()%2 == 0) {
        exprOpenedPHP = -1;
        state = STATE_NONE;
        exprEscStringPHP = "";
        return true;
    } else if (exprOpenedPHP >= 0 && c == "\\") {
        exprEscStringPHP += c;
    } else if (exprOpenedPHP >= 0) {
        exprEscStringPHP = "";
    }
    return false;
}

void Highlight::restoreState() {
    // load previous block data
    QTextCursor curs = QTextCursor(cBlock);
    if (curs.movePosition(QTextCursor::MoveOperation::PreviousBlock)) {
        HighlightData * prevBlockData = dynamic_cast<HighlightData *>(curs.block().userData());
        if (prevBlockData != nullptr) {
            state = prevBlockData->state;
            prevState = prevBlockData->prevState;
            prevPrevState = prevBlockData->prevPrevState;
            mode = prevBlockData->mode.toStdString();
            prevMode = prevBlockData->prevMode.toStdString();
            modeExpect = prevBlockData->modeExpect.toStdString();
            if (modeExpect.size()>0) {
                modeString = prevBlockData->modeString;
                modeSpos = 0;
                modeSkip = true;
            }
            prevModeExpect = prevBlockData->prevModeExpect.toStdString();
            prevModeString = prevBlockData->prevModeString;
            prevModeSkip = prevBlockData->prevModeSkip;
            if (prevModeString.size()>0) {
                prevModeSpos = 0;
            }
            modeExpectC = prevBlockData->modeExpectC.toStdString();
            if (modeExpectC.size()>0) {
                modeStringC = prevBlockData->modeStringC;
                modeCpos = 0;
                modeSkipC = true;
            }
            prevModeExpectC = prevBlockData->prevModeExpectC.toStdString();
            prevModeStringC = prevBlockData->prevModeStringC;
            prevModeSkipC = prevBlockData->prevModeSkipC;
            if (prevModeStringC.size()>0) {
                prevModeCpos = 0;
            }
            stringEscStringCSS = prevBlockData->stringEscStringCSS;
            stringEscStringJS = prevBlockData->stringEscStringJS;
            stringBlock = prevBlockData->stringBlock;
            regexpEscStringJS = prevBlockData->regexpEscStringJS;
            regexpPrevCharJS = prevBlockData->regexpPrevCharJS;
            bracesCSS = prevBlockData->bracesCSS;
            bracesJS = prevBlockData->bracesJS;
            bracesPHP = prevBlockData->bracesPHP;
            parensCSS = prevBlockData->parensCSS;
            parensJS = prevBlockData->parensJS;
            parensPHP = prevBlockData->parensPHP;
            cssMediaScope = prevBlockData->cssMediaScope;
            keywordPHPScoped = prevBlockData->keywordPHPScoped;
            keywordJSScoped = prevBlockData->keywordJSScoped;
            exprEscStringJS = prevBlockData->exprEscStringJS;
            stringEscVariableJS = prevBlockData->stringEscVariableJS;
            nsNamePHP = prevBlockData->nsNamePHP;
            nsChainPHP = prevBlockData->nsChainPHP;
            nsScopeChainPHP = prevBlockData->nsScopeChainPHP;
            clsNamePHP = prevBlockData->clsNamePHP;
            clsChainPHP = prevBlockData->clsChainPHP;
            clsScopeChainPHP = prevBlockData->clsScopeChainPHP;
            funcNamePHP = prevBlockData->funcNamePHP;
            funcChainPHP = prevBlockData->funcChainPHP;
            funcScopeChainPHP = prevBlockData->funcScopeChainPHP;
            expectedNsNamePHP = prevBlockData->expectedNsNamePHP;
            expectedClsNamePHP = prevBlockData->expectedClsNamePHP;
            expectedFuncNamePHP = prevBlockData->expectedFuncNamePHP;
            expectedFuncParsPHP = prevBlockData->expectedFuncParsPHP;
            expectedFuncArgsPHP = prevBlockData->expectedFuncArgsPHP;
            expectedFuncArgsPHPPositions = prevBlockData->expectedFuncArgsPHPPositions;
            expectedFuncArgsPHPBlocks = prevBlockData->expectedFuncArgsPHPBlocks;
            nsScopePHP = prevBlockData->nsScopePHP;
            clsScopePHP = prevBlockData->clsScopePHP;
            funcScopePHP = prevBlockData->funcScopePHP;
            varsChainsPHP = prevBlockData->varsChainsPHP;
            usedVarsChainsPHP = prevBlockData->usedVarsChainsPHP;
            varsGlobChainPHP = prevBlockData->varsGlobChainPHP;
            usedVarsGlobChainPHP = prevBlockData->usedVarsGlobChainPHP;
            varsClsChainPHP = prevBlockData->varsClsChainPHP;
            varsChainPHP = prevBlockData->varsChainPHP;
            usedVarsChainPHP = prevBlockData->usedVarsChainPHP;
            clsOpenPHP = prevBlockData->clsOpenPHP;
            clsOpensPHP = prevBlockData->clsOpensPHP;
            varsClsOpenChainPHP = prevBlockData->varsClsOpenChainPHP;
            clsPropsChainPHP = prevBlockData->clsPropsChainPHP;
            funcNameJS = prevBlockData->funcNameJS;
            funcScopeChainJS = prevBlockData->funcScopeChainJS;
            funcChainJS = prevBlockData->funcChainJS;
            expectedFuncNameJS = prevBlockData->expectedFuncNameJS;
            expectedFuncVarJS = prevBlockData->expectedFuncVarJS;
            expectedFuncParsJS = prevBlockData->expectedFuncParsJS;
            funcScopeJS = prevBlockData->funcScopeJS;
            varsChainJS = prevBlockData->varsChainJS;
            expectedFuncArgsJS = prevBlockData->expectedFuncArgsJS;
            mediaNameCSS = prevBlockData->mediaNameCSS;
            expectedMediaNameCSS = prevBlockData->expectedMediaNameCSS;
            expectedMediaParsCSS = prevBlockData->expectedMediaParsCSS;
            mediaScopeCSS = prevBlockData->mediaScopeCSS;
            tagChainHTML = prevBlockData->tagChainHTML;
            keywordPHPprevString = prevBlockData->keywordPHPprevString;
            keywordPHPprevStringPrevChar = prevBlockData->keywordPHPprevStringPrevChar;
            keywordJSprevString = prevBlockData->keywordJSprevString;
            keywordJSprevStringPrevChar = prevBlockData->keywordJSprevStringPrevChar;
            cssNamesChain = prevBlockData->cssNamesChain;
        }
    }

    if (state == STATE_TAG || prevState == STATE_TAG || prevPrevState == STATE_TAG) {
        tagOpened = 0;
    }
    if (state == STATE_COMMENT_ML_HTML || prevState == STATE_COMMENT_ML_HTML) {
        tagOpened = 0;
        commentHTMLOpened = 0;
    }
    if (state == STATE_STRING_SQ_HTML || prevState == STATE_STRING_SQ_HTML) {
        stringSQOpenedHTML = 0;
    }
    if (state == STATE_STRING_DQ_HTML || prevState == STATE_STRING_DQ_HTML) {
        stringDQOpenedHTML = 0;
    }
    if (state == STATE_STRING_SQ_CSS || prevState == STATE_STRING_SQ_CSS) {
        stringSQOpenedCSS = 0;
    }
    if (state == STATE_STRING_DQ_CSS || prevState == STATE_STRING_DQ_CSS) {
        stringDQOpenedCSS = 0;
    }
    if (state == STATE_STRING_SQ_JS || prevState == STATE_STRING_SQ_JS) {
        stringSQOpenedJS = 0;
    }
    if (state == STATE_STRING_DQ_JS || prevState == STATE_STRING_DQ_JS) {
        stringDQOpenedJS = 0;
    }
    if (state == STATE_REGEXP_JS || prevState == STATE_REGEXP_JS) {
        regexpOpenedJS = 0;
    }
    if (state == STATE_COMMENT_ML_JS || prevState == STATE_COMMENT_ML_JS) {
        commentMLOpenedJS = 0;
    }
    if (state == STATE_COMMENT_SL_JS) {
        state = STATE_NONE;
    }
    if (prevState == STATE_COMMENT_SL_JS) {
        commentSLOpenedJS = 0;
    }
    if (state == STATE_COMMENT_ML_CSS || prevState == STATE_COMMENT_ML_CSS) {
        commentMLOpenedCSS = 0;
    }
    if (state == STATE_STRING_SQ_PHP) {
        stringSQOpenedPHP = 0;
    }
    if (state == STATE_STRING_DQ_PHP) {
        stringDQOpenedPHP = 0;
    }
    if (state == STATE_COMMENT_ML_PHP) {
        commentMLOpenedPHP = 0;
    }
    if (state == STATE_COMMENT_SL_PHP) {
        state = STATE_NONE;
    }
    if (state == STATE_STRING_HEREDOC || state == STATE_STRING_NOWDOC) {
        stringBOpened = 0;
    }
    if (keywordPHPScoped) {
        keywordPHPScopedOpened = 0;
    }
    if (keywordJSScoped) {
        keywordJSScopedOpened = 0;
    }
    if (state == STATE_EXPRESSION_PHP) {
        exprOpenedPHP = 0;
    }
    if (state == STATE_EXPRESSION_JS || prevState == STATE_EXPRESSION_JS) {
        exprOpenedJS = 0;
    }
}

void Highlight::highlightSpell()
{
    if (blockData != nullptr && blockData->spellStarts.size() == blockData->spellLengths.size()) {
        for (int i=0; i<blockData->spellStarts.size(); i++) {
            int start = blockData->spellStarts.at(i);
            int length = blockData->spellLengths.at(i);
            if (start < 0 || length <= 0) continue;
            QTextCharFormat uFormat = format(start);
            uFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            uFormat.setUnderlineColor(spellColor);
            highlightString(start, length, uFormat);
        }
    }
}

bool Highlight::parseMode(const QChar & c, int pos, bool isWSpace, bool isLast, std::string & pMode, int & pState)
{
    if (modeType != MODE_MIXED || !detectMode(c, pos, isWSpace, isLast)) return false;
    if (modeSpos>=0 && modeCpos<0) {
        prevPrevState = prevState;
        prevState = state;
        state = STATE_NONE;
        if (tagOpened == modeSpos) {
            tagOpened = -1;
            if (prevState == STATE_TAG) prevState = STATE_NONE;
        }
        if (prevState == STATE_STRING_SQ_CSS || prevState == STATE_STRING_DQ_CSS) {
            stringEscStringCSS = prevStringEscStringCSS;
            prevStringEscStringCSS = "";
        }
        if (prevState == STATE_STRING_SQ_JS || prevState == STATE_STRING_DQ_JS) {
            stringEscStringJS = prevStringEscStringJS;
            prevStringEscStringJS = "";
        }
        if (prevState == STATE_REGEXP_JS) {
            regexpEscStringJS = prevRegexpEscStringJS;
            prevRegexpEscStringJS = "";
        }
        if (prevState == STATE_EXPRESSION_JS) {
            exprEscStringJS = prevExprEscStringJS;
            prevExprEscStringJS = "";
            stringEscVariableJS = prevStringEscVariableJS;
            prevStringEscVariableJS = "";
        }

        pMode = mode;
        // highlight tag
        if (mode == MODE_PHP) {
            highlightString(modeSpos, pos-modeSpos+1, HW->phpTagFormat);
        } else if (mode == MODE_JS || mode == MODE_CSS) {
            highlightChar(pos, HW->tagFormat);
        }

        QString modeTag = "";
        if (mode == MODE_CSS) modeTag = "style";
        else if (mode == MODE_JS) modeTag = "script";
        if (modeTag.size() > 0) {
            addSpecialWord(modeTag, modeSpos+1);
            // open tag chain
            if (tagChainStartsHTML.size() > tagChainEndsHTML.size()) {
                tagChainEndsHTML.append(modeSpos);
            }
            if (tagChainHTML.size() > 0) tagChainHTML += ",";
            tagChainHTML += modeTag;
            tagChainStartsHTML.append(modeSpos+1);
            tagChainsHTML.append(tagChainHTML);
        }

        if (modeStarts.size()>modeEnds.size()) {
            modeEnds.append(modeSpos);
        }
        modeStarts.append(pos+1);
        modeTags.append(mode);

        if (stateStarts.size()>stateEnds.size()) {
            stateEnds.append(modeSpos);
        }
        pState = state;

        modeSpos = -1;
        modeCpos = -1;

        if (!foundModes.contains(QString::fromStdString(mode))) foundModes.append(QString::fromStdString(mode));
    } else if (modeSpos<0 && modeCpos>=0) {
        state = prevState;
        prevState = prevPrevState;
        prevPrevState = STATE_NONE;

        // highlight tag
        if (pMode == MODE_PHP) {
            highlightString(modeCpos, pos-modeCpos+1, HW->phpTagFormat);
        } else if ((pMode == MODE_JS || pMode == MODE_CSS) && modeCposed == modeCpos) {
            highlightString(modeCpos, pos-modeCpos+1, HW->tagFormat);
        }

        QString modeTag = "";
        if (pMode == MODE_CSS) modeTag = "style";
        else if (pMode == MODE_JS) modeTag = "script";
        if (modeTag.size() > 0) {
            addSpecialWord("/"+modeTag, modeCpos+2);
            // close tag chain
            if (tagChainHTML.size() > 0) {
                QStringList tagChainList = tagChainHTML.split(",");
                QString lastTag = tagChainList.last();
                if (lastTag == modeTag) {
                    tagChainList.removeLast();
                    tagChainEndsHTML.append(modeCpos+modeTag.size()+2);
                    QString tagChainN = "";
                    for (int i=0; i<tagChainList.size(); i++) {
                        if (tagChainN.size() > 0) tagChainN += ",";
                        tagChainN += tagChainList.at(i);
                    }
                    if (tagChainN.size() > 0) {
                        tagChainStartsHTML.append(modeCpos+modeTag.size()+3);
                        tagChainsHTML.append(tagChainN);
                    }
                    tagChainHTML = tagChainN;
                }
            }
        }

        // close single line comments
        if (pMode == MODE_PHP && commentSLOpenedPHP>=0) {
            commentPHPStringSL = "";
            commentSLOpenedPHP = -1;
        }
        if (pMode == MODE_JS && commentSLOpenedJS>=0) {
            commentJSStringSL = "";
            commentSLOpenedJS = -1;
        }
        pMode = mode;

        modeEnds.append(modeCpos);
        if (mode != MODE_HTML) {
            modeStarts.append(pos+1);
            modeTags.append(mode);
        }

        if (stateStarts.size()>stateEnds.size()) {
            stateEnds.append(modeCpos);
        }
        if (state != STATE_NONE) {
            stateStarts.append(pos+1);
            stateIds.append(state);
        }
        pState = state;

        modeString = prevModeString;
        modeExpect = prevModeExpect;
        modeSkip = prevModeSkip;
        modeSpos = prevModeSpos;
        modeStringC = prevModeStringC;
        modeExpectC = prevModeExpectC;
        modeSkipC = prevModeSkipC;
        modeCpos = prevModeCpos;

        if (!foundModes.contains(QString::fromStdString(mode))) foundModes.append(QString::fromStdString(mode));
    } else {
        modeSpos = -1;
        modeCpos = -1;
    }
    return true;
}

void Highlight::parseHTML(const QChar & c, int pos, bool isAlpha, bool isAlnum, bool isLast)
{
    if (mode != MODE_HTML) return;
    bool tagChanged = detectTag(c, pos);
    if (tagOpened>=0 || tagChanged) {
        highlightChar(pos, HW->tagFormat);
    }

    // html comments
    bool commentHTMLchanged = detectCommentHTML(c);
    if (commentHTMLOpened>=0 && commentHTMLchanged) {
        highlightString(commentHTMLOpened, pos-commentHTMLOpened+1, HW->multiLineCommentFormat);
    } else if (commentHTMLOpened<0 && commentHTMLchanged) {
        highlightChar(pos, HW->multiLineCommentFormat);
    } else if (commentHTMLOpened>=0) {
        highlightChar(pos, HW->multiLineCommentFormat);
    }

    // html string (single quote)
    bool stringSQchangedHTML = detectStringSQHTML(c, pos);
    if ((stringSQOpenedHTML>=0 && state == STATE_STRING_SQ_HTML) || stringSQchangedHTML) {
        highlightChar(pos, HW->stringFormat);
    }

    // html string (double quote)
    bool stringDQchangedHTML = detectStringDQHTML(c, pos);
    if ((stringDQOpenedHTML>=0 && state == STATE_STRING_DQ_HTML)|| stringDQchangedHTML) {
        highlightChar(pos, HW->stringFormat);
    }

    // html tag keywords
    int keywordHTMLStart = detectKeywordHTML(c, pos, isAlpha, isAlnum, isLast);
    if (keywordHTMLStart>=0) {
        int keywordHTMLLength = pos-keywordHTMLStart;
        if (isLast && isAlnum) keywordHTMLLength += 1;
        if ((keywordHTMLprevChar=="<" || (keywordHTMLprevChar=="/" && keywordHTMLprevPrevChar=="<")) && keywordStringHTML != "script" && keywordStringHTML != "style") {
            HW->htmlwordsIterator = HW->htmlwords.find(keywordStringHTML.toLower().toStdString());
            if (HW->htmlwordsIterator != HW->htmlwords.end()) {
                QTextCharFormat format = HW->htmlwordsIterator->second;
                highlightString(keywordHTMLStart, keywordHTMLLength, format);
            } else {
                highlightString(keywordHTMLStart, keywordHTMLLength, HW->tagNameFormat);
            }
            if (keywordHTMLprevChar == "/") {
                addSpecialWord(keywordHTMLprevChar+keywordStringHTML, keywordHTMLStart);
                // close tag chain
                if (tagChainHTML.size() > 0) {
                    QStringList tagChainList = tagChainHTML.split(",");
                    QString lastTag = tagChainList.last();
                    if (lastTag.toLower() == keywordStringHTML.toLower()) {
                        tagChainList.removeLast();
                        tagChainEndsHTML.append(keywordHTMLStart+keywordStringHTML.size());
                        QString tagChainN = "";
                        for (int i=0; i<tagChainList.size(); i++) {
                            if (tagChainN.size() > 0) tagChainN += ",";
                            tagChainN += tagChainList.at(i);
                        }
                        if (tagChainN.size() > 0) {
                            tagChainStartsHTML.append(keywordHTMLStart+keywordStringHTML.size()+1);
                            tagChainsHTML.append(tagChainN);
                        }
                        tagChainHTML = tagChainN;
                    }
                }
            } else {
                addSpecialWord(keywordStringHTML, keywordHTMLStart);
                // open tag chain
                if (tagChainStartsHTML.size() > tagChainEndsHTML.size() && tagChainHTML.size() > 0 && keywordHTMLStart > 0) {
                    tagChainEndsHTML.append(keywordHTMLStart-1);
                }
                if (tagChainHTML.size() > 0) tagChainHTML += ",";
                tagChainHTML += keywordStringHTML.toLower();
                tagChainStartsHTML.append(keywordHTMLStart);
                tagChainsHTML.append(tagChainHTML);
            }
        }
        keywordHTMLprevPrevChar = c;
        keywordHTMLprevChar = c;
    }

    // close open short tag
    if (tagChainStartsHTML.size() > tagChainEndsHTML.size() && tagChainHTML.size() > 0 && c == ">" && tagChanged) {
        QStringList tagChainList = tagChainHTML.split(",");
        QString lastTag = tagChainList.last();
        HW->htmlshortsIterator = HW->htmlshorts.find(lastTag.toLower().toStdString());
        if (HW->htmlshortsIterator != HW->htmlshorts.end()) {
            tagChainList.removeLast();
            tagChainEndsHTML.append(pos);
            QString tagChainN = "";
            for (int y=0; y<tagChainList.size(); y++) {
                if (tagChainN.size() > 0) tagChainN += ",";
                tagChainN += tagChainList.at(y);
            }
            if (tagChainN.size() > 0) {
                tagChainStartsHTML.append(pos+1);
                tagChainsHTML.append(tagChainN);
            }
            tagChainHTML = tagChainN;
        }
    }

    // tabs and spaces
    if (stringSQOpenedHTML < 0 && stringDQOpenedHTML < 0 && commentHTMLOpened < 0 && keywordHTMLOpened < 0) {
        if (highlightTabs && c == "\t") {
            highlightChar(pos, HW->tabFormat);
        } else if (highlightSpaces && c == " ") {
            highlightChar(pos, HW->spaceFormat);
        }
    }
}

void Highlight::parseCSS(const QChar & c, int pos, bool isAlpha, bool isAlnum, bool isWSpace, bool isLast, int & keywordCSSStartPrev, int & keywordCSSLengthPrev, bool & cssValuePart)
{
    if (mode != MODE_CSS) return;
    // css string (single quote)
    bool stringSQchangedCSS = detectStringSQCSS(c, pos);
    if ((stringSQOpenedCSS>=0 && state == STATE_STRING_SQ_CSS) || stringSQchangedCSS) {
        highlightChar(pos, HW->stringFormat);
    }

    // css string (double quote)
    bool stringDQchangedCSS = detectStringDQCSS(c, pos);
    if ((stringDQOpenedCSS>=0 && state == STATE_STRING_DQ_CSS)|| stringDQchangedCSS) {
        highlightChar(pos, HW->stringFormat);
    }

    // comments (multi-line)
    bool commentsMLchangedCSS = detectMLCommentCSS(c, pos);
    if ((commentMLOpenedCSS>=0 && state == STATE_COMMENT_ML_CSS)|| commentsMLchangedCSS) {
        highlightChar(pos, HW->multiLineCommentFormat);
        if (commentMLOpenedCSS>=0 && state == STATE_COMMENT_ML_CSS && commentsMLchangedCSS && pos>0) {
            highlightChar(pos-1, HW->multiLineCommentFormat);
        }
    }

    // keywords & selectors && functions
    int keywordCSSStart = detectKeywordCSS(c, pos, isAlpha, isAlnum, isLast);
    if (keywordCSSStart>=0) {
        if (keywordCSSprevChar == "@" && keywordStringCSS == "media") {
            cssMediaScope = true;
            if (mediaNameCSS.size() == 0 && mediaScopeCSS < 0) {
                expectedMediaNameCSS = keywordStringCSS;
                expectedMediaParsCSS = -1;
            }
        }
        int keywordCSSLength = pos-keywordCSSStart;
        if (isLast && isAlnum) keywordCSSLength += 1;
        if ((keywordCSSprevChar == "@" || keywordCSSprevChar == "!") && keywordCSSStart>0) {
            keywordCSSStart -= 1;
            keywordCSSLength += 1;
        }
        if (keywordCSSprevChar != "#" && keywordCSSprevChar != "." && keywordCSSprevPrevChar != "&") {
            bool kFound = false;
            if (keywordCSSprevChar != "$" && keywordCSSprevChar != "%") {
                // css keywords
                if (!isBigFile) {
                    HW->csswordsIterator = HW->csswords.find(keywordStringCSS.toLower().toStdString());
                    if (HW->csswordsIterator != HW->csswords.end()) {
                        QTextCharFormat format = HW->csswordsIterator->second;
                        highlightString(keywordCSSStart, keywordCSSLength, format);
                        keywordCSSStart = -1;
                        kFound = true;
                    }
                }
                if (!kFound && cssValuePart) {
                    // css selectors
                    HW->htmlwordsIterator = HW->htmlwords.find(keywordStringCSS.toLower().toStdString());
                    if (HW->htmlwordsIterator != HW->htmlwords.end()) {
                        highlightString(keywordCSSStart, keywordCSSLength, HW->selectorTagFormat);
                        kFound = true;
                    }
                }
            }
            if (!kFound && !isBigFile) {
                if (keywordCSSprevChar == "$" && keywordCSSStart > 0) {
                    highlightString(keywordCSSStart-1, keywordCSSLength+1, HW->variableFormat);
                } else if (keywordCSSprevChar == "%" && keywordCSSStart > 0) {
                    highlightString(keywordCSSStart-1, keywordCSSLength+1, HW->knownVariableFormat);
                } else if (bracesCSS > 0 && cssValuePart) {
                    // css properties
                    highlightString(keywordCSSStart, keywordCSSLength, HW->propertyFormat);
                }
            }
        } else if ((keywordCSSprevChar == "#" || keywordCSSprevChar == ".") && parensCSS == 0) {
            // css id & class selectors
            if (keywordCSSStart > 0) {
                keywordCSSStart -= 1;
                keywordCSSLength += 1;
            }
            QString cssName = keywordCSSprevChar + keywordStringCSS;
            cssNamesIterator = cssNames.find(cssName.toStdString());
            if (cssNamesIterator != cssNames.end()) {
                highlightString(keywordCSSStart, keywordCSSLength, HW->selectorFormat);
            } else if (!isColorKeyword) {
                if (cssNamesChain.size() > 0) cssNamesChain += ",";
                cssNamesChain += cssName;
                highlightString(keywordCSSStart, keywordCSSLength, HW->selectorTagFormat);
            }
        }
        if (keywordCSSprevChar == "#" && !cssValuePart && isColorKeyword && !isBigFile && (keywordStringCSS.length() == 3 || keywordStringCSS.length() == 6 || keywordStringCSS.length() == 8) && QColor::isValidColor(keywordCSSprevChar+keywordStringCSS)) {
            // colors
            if (keywordCSSStart > 0 && parensCSS == 0) {
                keywordCSSStart += 1;
                keywordCSSLength -= 1;
            }
            HW->colorFormat.setUnderlineColor(QColor(keywordCSSprevChar+keywordStringCSS));
            highlightString(keywordCSSStart, keywordCSSLength, HW->colorFormat);
        }
        keywordCSSprevChar = c;
        keywordCSSprevPrevChar = c;
        keywordCSSStartPrev = keywordCSSStart;
        keywordCSSLengthPrev = keywordCSSLength;
    }
    if (keywordCSSStartPrev>=0 && keywordCSSLengthPrev>0) {
        // css functions
        if (c == "(") {
            highlightString(keywordCSSStartPrev, keywordCSSLengthPrev, HW->functionFormat);
            keywordCSSStartPrev = -1;
            keywordCSSLengthPrev = -1;
        } else if (!isWSpace) {
            keywordCSSStartPrev = -1;
            keywordCSSLengthPrev = -1;
        }
    }

    // css curly brackets
    if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && c == "{") {
        bracesCSS++;
        cssValuePart = true;
        if (expectedMediaNameCSS.size() > 0 && expectedMediaParsCSS < 0) {
            if (mediaStartsCSS.size() > mediaEndsCSS.size()) mediaEndsCSS.append(pos);
            mediaNameCSS = expectedMediaNameCSS;
            mediaScopeCSS = bracesCSS - 1;
            mediaNamesCSS.append(mediaNameCSS);
            mediaStartsCSS.append(pos+1);
            expectedMediaNameCSS = "";
            expectedMediaParsCSS = -1;
        }
    } else if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && c == "}") {
        bracesCSS--;
        if (bracesCSS < 0) bracesCSS = 0;
        if (bracesCSS == 0 && cssMediaScope) {
            cssMediaScope = false;
        }
        if (bracesCSS == 0 && !cssValuePart) {
            cssValuePart = true;
        }
        if (mediaScopeCSS >=0 && mediaScopeCSS == bracesCSS) {
            mediaEndsCSS.append(pos);
            mediaNameCSS = "";
            mediaScopeCSS = -1;
            expectedMediaNameCSS = "";
            expectedMediaParsCSS = -1;
        }
    }
    if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && c == ":") {
        cssValuePart = false;
    } else if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && c == ";") {
        cssValuePart = true;
    }

    // css parentheses
    if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && c == "(") {
        parensCSS++;
        if (expectedMediaNameCSS.size() > 0 && expectedMediaNameCSS == "media") {
            expectedMediaParsCSS = parensCSS-1;
            expectedMediaNameCSS = "";
        }
    } else if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && c == ")") {
        parensCSS--;
        if (parensCSS < 0) parensCSS = 0;
        if (expectedMediaParsCSS >= 0 && expectedMediaParsCSS == parensCSS) {
            expectedMediaParsCSS = -1;
        }
    }

    // expected media
    if (expectedMediaParsCSS >= 0 && c != "(" && c != ")") {
        expectedMediaNameCSS += c;
    }
    // unexpected media
    if (expectedMediaNameCSS.size() > 0 && (c == ";" || c == "{")) {
        expectedMediaNameCSS = "";
        expectedMediaParsCSS = -1;
    }

    // saving special chars data
    if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0 && (c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]")) {
        addSpecialChar(c, pos);
    }

    // tabs, spaces, semicolons, commas
    if (stringSQOpenedCSS < 0 && stringDQOpenedCSS < 0 && commentMLOpenedCSS < 0 && keywordCSSOpened < 0) {
        if (highlightTabs && c == "\t") {
            highlightChar(pos, HW->tabFormat);
        } else if (highlightSpaces && c == " ") {
            highlightChar(pos, HW->spaceFormat);
        }
        if (!isBigFile && (c == ";" || c == "," || c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]")) {
            highlightChar(pos, HW->punctuationFormat);
        }
    }
}

void Highlight::parseJS(const QChar & c, int pos, bool isAlpha, bool isAlnum, bool isWSpace, bool isLast, int & keywordJSStartPrev, int & keywordJSLengthPrev)
{
    if (mode != MODE_JS) return;

    // js string (single quote)
    bool stringSQchangedJS = detectStringSQJS(c, pos);
    if ((stringSQOpenedJS>=0 && state == STATE_STRING_SQ_JS) || stringSQchangedJS) {
        highlightChar(pos, HW->stringFormat);
    }

    // js string (double quote)
    bool stringDQchangedJS = detectStringDQJS(c, pos);
    if ((stringDQOpenedJS>=0 && state == STATE_STRING_DQ_JS) || stringDQchangedJS) {
        highlightChar(pos, HW->stringFormat);
    }

    // js template
    bool exprChangedJS = detectExpressionJS(c, pos);
    if ((exprOpenedJS>=0 && state == STATE_EXPRESSION_JS) || exprChangedJS) {
        highlightChar(pos, HW->knownFormat);
    }

    // js template expression
    if (exprOpenedJS >= 0 && c == "$" && stringEscVariableJS.size()%2 == 0) {
        keywordJSScopedOpened = pos;
        keywordJSScoped = false;
    } else if (exprOpenedJS >= 0 && keywordJSScopedOpened >= 0 && c == "{" && !keywordJSScoped) {
        keywordJSScoped = true;
        highlightString(keywordJSScopedOpened, pos-keywordJSScopedOpened, HW->expressionFormat);
    } else if (exprOpenedJS >= 0 && keywordJSScopedOpened >= 0 && !isWSpace && !keywordJSScoped) {
        keywordJSScopedOpened = -1;
    } else if (exprOpenedJS >= 0 && keywordJSScopedOpened >= 0 && c == "}" && keywordJSScoped) {
        keywordJSScopedOpened = -1;
        keywordJSScoped = false;
        highlightChar(pos, HW->expressionFormat);
    }
    if (exprOpenedJS >= 0 && keywordJSScopedOpened < 0 && c == "\\") {
        stringEscVariableJS += c;
    } else if (c != "$") {
        if (c == "<") prevStringEscVariableJS = stringEscVariableJS;
        stringEscVariableJS = "";
    }
    if (keywordJSScoped) {
        highlightChar(pos, HW->expressionFormat);
    }

    // comments (single-line)
    bool commentsSLchangedJS = detectSLCommentJS(c, pos);
    if (commentSLOpenedJS>=0 || commentsSLchangedJS) {
        highlightChar(pos, HW->singleLineCommentFormat);
        if (commentSLOpenedJS>=0 && commentsSLchangedJS && pos>0) {
            highlightChar(pos-1, HW->singleLineCommentFormat);
        }
    }

    // js regexp
    bool regexpchangedJS = detectRegexpJS(c, pos, isWSpace, isAlnum);
    if ((regexpOpenedJS>=0 && state == STATE_REGEXP_JS)|| regexpchangedJS) {
        highlightChar(pos, HW->stringFormat);
    }

    // comments (multi-line)
    if (!(regexpOpenedJS<0 && regexpchangedJS)) {
        bool commentsMLchangedJS = detectMLCommentJS(c, pos);
        if ((commentMLOpenedJS>=0 && state == STATE_COMMENT_ML_JS)|| commentsMLchangedJS) {
            highlightChar(pos, HW->multiLineCommentFormat);
            if (commentMLOpenedJS>=0 && state == STATE_COMMENT_ML_JS && commentsMLchangedJS && pos>0) {
                highlightChar(pos-1, HW->multiLineCommentFormat);
            }
        }
    } else {
        commentJSStringML = "";
    }

    // keywords & functions
    int keywordJSStart = detectKeywordJS(c, pos, isAlpha, isAlnum, isLast);
    if (keywordJSStart>=0) {
        int keywordJSLength = pos-keywordJSStart;
        if (isLast && isAlnum) keywordJSLength += 1;
        bool known = false;
        if (keywordJSprevChar != "$" && (keywordJSprevChar != "." || keywordStringJS == "prototype") && keywordStringJS.size()>1) {
            // js keywords
            HW->jswordsCSIterator = HW->jswordsCS.find(keywordStringJS.toStdString());
            if (HW->jswordsCSIterator != HW->jswordsCS.end()) {
                QTextCharFormat format = HW->jswordsCSIterator->second;
                highlightString(keywordJSStart, keywordJSLength, format);
                keywordJSStart = -1;
                known = true;
            }
            if (!known && !isBigFile) {
                jsNamesIterator = jsNames.find(keywordStringJS.toStdString());
                if (jsNamesIterator != jsNames.end()) {
                    highlightString(keywordJSStart, keywordJSLength, HW->variableFormat);
                    known = true;
                }
            }
        }
        if (!known && !isBigFile) {
            if ((keywordJSprevString == "var" || keywordJSprevString == "let" || keywordJSprevString == "const") && keywordJSprevStringPrevChar != ".") {
                if (varsChainJS.size() > 0) varsChainJS += ",";
                varsChainJS += keywordStringJS;
                highlightString(keywordJSStart, keywordJSLength, HW->variableFormat);
            } else if (keywordJSprevChar != "." && c == ".") {
                highlightString(keywordJSStart, keywordJSLength, HW->classFormat);
            } else if (keywordJSprevChar == ".") {
                highlightString(keywordJSStart, keywordJSLength, HW->propertyFormat);
            } else if (expectedFuncNameJS.size() > 0 || expectedFuncParsJS >= 0) { // add arg if var is unknown
                expectedFuncArgsJS.append(keywordStringJS);
                highlightString(keywordJSStart, keywordJSLength, HW->variableFormat);
            }
        }
        if (keywordStringJS.size()>0 && !isBigFile) {
            // function scope
            if (expectedFuncNameJS.size() == 0 && keywordStringJS.size() > 0 && keywordStringJS != "function") {
                expectedFuncNameJS = "";
                expectedFuncVarJS = keywordStringJS;
                expectedFuncParsJS = -1;
                expectedFuncArgsJS.clear();
            } else if (expectedFuncNameJS.size() == 0 && expectedFuncVarJS.size() > 0 && keywordStringJS == "function" && expectedFuncVarJS.indexOf("=") > 0) {
                QStringList expectedFuncNameJSList = expectedFuncVarJS.split("=");
                if (expectedFuncNameJSList.size() == 2 && expectedFuncNameJSList.at(1) == "function") {
                    expectedFuncNameJS = expectedFuncNameJSList.at(0);
                    expectedFuncVarJS = "";
                    expectedFuncParsJS = parensJS;
                } else {
                    expectedFuncNameJS = "";
                    expectedFuncVarJS = "";
                    expectedFuncParsJS = -1;
                }
                expectedFuncArgsJS.clear();
            } else if (keywordStringJS.size() > 0 && ((keywordStringJS == "function" && expectedFuncNameJS.size() == 0) || expectedFuncNameJS == "function")) {
                expectedFuncNameJS = keywordStringJS;
                expectedFuncVarJS = "";
                expectedFuncParsJS = parensJS;
                expectedFuncArgsJS.clear();
            } else {
                expectedFuncVarJS = "";
            }
        }
        keywordJSprevString = keywordStringJS;
        keywordJSprevStringPrevChar = keywordJSprevChar;
        keywordJSprevChar = c;
        keywordJSStartPrev = keywordJSStart;
        keywordJSLengthPrev = keywordJSLength;
    }
    if (keywordJSStartPrev>=0 && keywordJSLengthPrev>0) {
        // js functions
        if (c == "(") {
            highlightString(keywordJSStartPrev, keywordJSLengthPrev, HW->functionFormat);
            keywordJSStartPrev = -1;
            keywordJSLengthPrev = -1;
        } else if (!isWSpace) {
            keywordJSStartPrev = -1;
            keywordJSLengthPrev = -1;
        }
    }

    // js curly brackets
    if (stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentSLOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && keywordJSOpened < 0 && c == "{") {
        bracesJS++;
        // open function scope
        if (expectedFuncNameJS.size() > 0 && expectedFuncParsJS == parensJS) {
            QString cFuncNameJS = funcNameJS;
            int cFuncScopeJS = funcScopeJS;
            if (funcScopeJS < 0) cFuncNameJS = "";
            if (funcStartsJS.size() > funcEndsJS.size()) funcEndsJS.append(pos);
            funcNameJS = expectedFuncNameJS;
            funcScopeJS = bracesJS-1;
            funcNamesJS.append(funcNameJS);
            funcStartsJS.append(pos+1);
            expectedFuncNameJS = "";
            expectedFuncParsJS = -1;
            if (funcChainJS.size() > 0) funcChainJS += ",";
            funcChainJS += cFuncNameJS;
            funcScopeChainJS.append(cFuncScopeJS);
            if (expectedFuncArgsJS.size() > 0) {
                for (int i=0; i<expectedFuncArgsJS.size(); i++) {
                    if (varsChainJS.size() > 0) varsChainJS += ",";
                    varsChainJS += expectedFuncArgsJS.at(i);
                }
            }
            expectedFuncArgsJS.clear();
        }
    } else if (stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentSLOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && keywordJSOpened < 0 && c == "}") {
        bracesJS--;
        if (bracesJS < 0) bracesJS = 0;
        // close function scope
        if (funcNameJS.size() > 0 && funcScopeJS >= 0 && funcScopeJS == bracesJS) {
            funcNameJS = "";
            funcScopeJS = -1;
            funcEndsJS.append(pos);
            expectedFuncNameJS = "";
            expectedFuncParsJS = -1;
            expectedFuncArgsJS.clear();
            // set parent function
            int parentFuncScopeJS = -1;
            QString parentFuncJS = "", parentFuncChainJS = "";
            if (!funcScopeChainJS.isEmpty()) {
                parentFuncScopeJS = funcScopeChainJS.last();
                funcScopeChainJS.removeLast();
                QStringList funcChainListJS  = funcChainJS.split(",");
                parentFuncJS = funcChainListJS.last();
                for (int i=0; i<funcChainListJS.size()-1; i++) {
                    if (parentFuncChainJS.size() > 0) parentFuncChainJS += ",";
                    parentFuncChainJS += funcChainListJS.at(i);
                }
                if (parentFuncScopeJS >= 0 && parentFuncJS.size() > 0) {
                    funcNameJS = parentFuncJS;
                    funcChainJS = parentFuncChainJS;
                    funcNamesJS.append(funcNameJS);
                    funcStartsJS.append(pos+1);
                    funcScopeJS = parentFuncScopeJS;
                }
            }
        }
    }

    if (expectedFuncNameJS.size() > 0 && expectedFuncNameJS == "function" && c == "(" && expectedFuncParsJS == parensJS) {
        expectedFuncNameJS = "anonymous function";
    }

    // unexpected function scope
    if (expectedFuncNameJS.size() > 0 && expectedFuncNameJS != "function" && c == ";" && expectedFuncParsJS == parensJS) {
        expectedFuncNameJS = "";
        expectedFuncParsJS = -1;
        expectedFuncArgsJS.clear();
    }

    if (keywordJSStart < 0 && expectedFuncVarJS.size() > 0 && expectedFuncParsJS < 0 && !isWSpace && (isAlnum || c == "=")) {
        expectedFuncVarJS += c;
    } else if (expectedFuncVarJS.size() > 0 && expectedFuncParsJS < 0 && !isWSpace) {
        expectedFuncVarJS = "";
    }

    // js parentheses
    if (stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentSLOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && keywordJSOpened < 0 && c == "(") {
        parensJS++;
    } else if (stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentSLOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && keywordJSOpened < 0 && c == ")") {
        parensJS--;
        if (parensJS < 0) parensJS = 0;
    }

    // saving special chars data
    if (stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentSLOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && keywordJSOpened < 0 && (c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]")) {
        addSpecialChar(c, pos);
    }

    // tabs, spaces, semicolons, commas
    if (stringSQOpenedJS < 0 && stringDQOpenedJS < 0 && commentSLOpenedJS < 0 && commentMLOpenedJS < 0 && regexpOpenedJS < 0 && keywordJSOpened < 0) {
        if (highlightTabs && c == "\t") {
            highlightChar(pos, HW->tabFormat);
        } else if (highlightSpaces && c == " ") {
            highlightChar(pos, HW->spaceFormat);
        }
        if (!isBigFile && (c == ";" || c == "," || c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]")) {
            highlightChar(pos, HW->punctuationFormat);
        }
    }
}

void Highlight::parsePHP(const QChar c, int pos, bool isAlpha, bool isAlnum, bool isWSpace, bool isLast, int & keywordPHPStartPrev, int & keywordPHPLengthPrev)
{
    if (mode != MODE_PHP) return;
    // php string (single quote)
    bool stringSQchangedPHP = detectStringSQPHP(c, pos);
    if ((stringSQOpenedPHP>=0 && state == STATE_STRING_SQ_PHP) || stringSQchangedPHP) {
        highlightChar(pos, HW->stringFormat);
    }

    // php string (double quote)
    bool stringDQchangedPHP = detectStringDQPHP(c, pos);
    if ((stringDQOpenedPHP>=0 && state == STATE_STRING_DQ_PHP)|| stringDQchangedPHP) {
        highlightChar(pos, HW->stringFormat);
    }

    // php shell expression
    bool exprChangedPHP = detectExpressionPHP(c, pos);
    if ((exprOpenedPHP>=0 && state == STATE_EXPRESSION_PHP)|| exprChangedPHP) {
        highlightChar(pos, HW->expressionFormat);
    }

    // comments (single-line)
    bool commentsSLchangedPHP = detectSLCommentPHP(c, pos);
    if (commentSLOpenedPHP>=0 || commentsSLchangedPHP) {
        highlightChar(pos, HW->singleLineCommentFormat);
        if (commentSLOpenedPHP>=0 && commentsSLchangedPHP && pos>0 && c != "#") {
            highlightChar(pos-1, HW->singleLineCommentFormat);
        }
    }

    // comments (multi-line)
    bool commentsMLchangedPHP = detectMLCommentPHP(c, pos);
    if ((commentMLOpenedPHP>=0 && state == STATE_COMMENT_ML_PHP)|| commentsMLchangedPHP) {
        highlightChar(pos, HW->multiLineCommentFormat);
        if (commentMLOpenedPHP>=0 && state == STATE_COMMENT_ML_PHP && commentsMLchangedPHP && pos>0) {
            highlightChar(pos-1, HW->multiLineCommentFormat);
        }
    }

    // heredoc
    detectStringBPHP(c, pos, isAlpha, isAlnum, isLast);
    if (stringBOpened>=0 && stringBStart>=0) {
        highlightString(stringBStart, pos-stringBStart+1, HW->stringFormat);
    } else if (stringBOpened>=0) {
        highlightChar(pos, HW->stringFormat);
    }

    // dq-string variables
    if ((stringDQOpenedPHP >= 0 || (stringBOpened >= 0 && state == STATE_STRING_HEREDOC)) && c == "{") {
        keywordPHPScopedOpened = pos;
        keywordPHPScoped = false;
    } else if ((stringDQOpenedPHP >= 0 || (stringBOpened >= 0 && state == STATE_STRING_HEREDOC)) && keywordPHPScopedOpened >= 0 && c == "$" && !keywordPHPScoped) {
        keywordPHPScoped = true;
        highlightString(keywordPHPScopedOpened, pos-keywordPHPScopedOpened, HW->expressionFormat);
    } else if ((stringDQOpenedPHP >= 0 || (stringBOpened >= 0 && state == STATE_STRING_HEREDOC)) && keywordPHPScopedOpened >= 0 && !isWSpace && !keywordPHPScoped) {
        keywordPHPScopedOpened = -1;
    } else if ((stringDQOpenedPHP >= 0 || (stringBOpened >= 0 && state == STATE_STRING_HEREDOC)) && keywordPHPScopedOpened >= 0 && c == "}" && keywordPHPScoped) {
        keywordPHPScopedOpened = -1;
        keywordPHPScoped = false;
        highlightChar(pos, HW->expressionFormat);
    }
    bool forceDetectKeyword = false;
    if ((stringDQOpenedPHP >= 0 || (stringBOpened >= 0 && state == STATE_STRING_HEREDOC)) && keywordPHPprevChar == "$" && stringEscVariablePHP.size()%2 == 0) {
        forceDetectKeyword = true;
    }
    if ((stringDQOpenedPHP >= 0 || (stringBOpened >= 0 && state == STATE_STRING_HEREDOC)) && keywordPHPScopedOpened < 0 && c == "\\") {
        stringEscVariablePHP += c;
    } else if (c != "$") {
        stringEscVariablePHP = "";
    }
    if (keywordPHPScoped) {
        highlightChar(pos, HW->expressionFormat);
    }

    // keywords & variables & functions
    int keywordPHPStart = detectKeywordPHP(c, pos, isAlpha, isAlnum, isLast, forceDetectKeyword);
    if (keywordPHPStart>=0) {
        int keywordPHPLength = pos-keywordPHPStart;
        if (isLast && isAlnum) keywordPHPLength += 1;
        bool isObjectContext = keywordPHPprevPrevChar == "-" && keywordPHPprevChar == ">";
        if (keywordPHPprevChar != "$" && !isObjectContext && keywordPHPprevChar != ":" && keywordStringPHP.size()>1 && keywordPHPprevString.toLower() != "const" && keywordPHPprevString.toLower() != "function") {
            // php keywords
            HW->phpwordsIterator = HW->phpwords.find(keywordStringPHP.toLower().toStdString());
            if (HW->phpwordsIterator != HW->phpwords.end()) {
                QTextCharFormat format = HW->phpwordsIterator->second;
                highlightString(keywordPHPStart, keywordPHPLength, format);
                keywordPHPStart = -1;
            } else {
                HW->phpwordsCSIterator = HW->phpwordsCS.find(keywordStringPHP.toStdString());
                if (HW->phpwordsCSIterator != HW->phpwordsCS.end()) {
                    QTextCharFormat format = HW->phpwordsCSIterator->second;
                    highlightString(keywordPHPStart, keywordPHPLength, format);
                    keywordPHPStart = -1;
                }
            }
        } else if ((keywordPHPprevChar == "$" || isObjectContext) && !isBigFile) {
            // php variables
            bool known = false;
            if (keywordPHPprevChar == "$" && keywordPHPStart > 0) {
                keywordPHPStart -= 1;
                keywordPHPLength += 1;
                QString varName = keywordPHPprevChar + keywordStringPHP;
                HW->phpwordsCSIterator = HW->phpwordsCS.find(varName.toStdString());
                if (HW->phpwordsCSIterator != HW->phpwordsCS.end()) {
                    known = true;
                } else if (varName == "$this" && clsNamePHP.size() > 0) {
                    known = true;
                } else if (expectedFuncNamePHP.size() > 0) {
                    expectedFuncArgsPHP.append(varName);
                    expectedFuncArgsPHPPositions.append(keywordPHPStart);
                    expectedFuncArgsPHPBlocks.append(cBlock.blockNumber());
                } else if (keywordPHPprevPrevChar != ":" || keywordPHPprevStringPrevChar == "$") {
                    QString ns = "\\";
                    if (nsNamePHP.size() > 0) ns += nsNamePHP + "\\";
                    QString _clsName = clsNamePHP.size() > 0 ? ns + clsNamePHP : "";
                    QString k = _clsName + "::" + funcNamePHP;
                    variablesIterator = variables.find(varName.toStdString());
                    if (variablesIterator == variables.end()) {
                        variables[varName.toStdString()] = varName.toStdString();
                        if (varsChainPHP.size() > 0) varsChainPHP += ",";
                        varsChainPHP += varName;
                        // save to known vars
                        if (highlightVarsMode || firstRunMode) {
                            knownVars[k.toStdString()] = varsChainPHP.toStdString();
                            QString kk = k + "::" + varName;
                            knownVarsPositions[kk.toStdString()] = keywordPHPStart;
                            knownVarsBlocks[kk.toStdString()] = cBlock.blockNumber();
                        }
                    } else {
                        known = true;
                        usedVariablesIterator = usedVariables.find(varName.toStdString());
                        if (usedVariablesIterator == usedVariables.end()) {
                            usedVariables[varName.toStdString()] = varName.toStdString();
                            if (usedVarsChainPHP.size() > 0) usedVarsChainPHP += ",";
                            usedVarsChainPHP += varName;
                        }
                    }
                    if (highlightVarsMode || firstRunMode) {
                        usedVars[k.toStdString()] = usedVarsChainPHP.toStdString();
                    }
                } else if ((keywordPHPprevString.toLower() == "self" || keywordPHPprevString.toLower() == "static") && keywordPHPprevStringPrevChar != "$") {
                    clsPropsIterator = clsProps.find(varName.toStdString());
                    if (clsPropsIterator != clsProps.end()) {
                        known = true;
                    }
                }
            }
            if (isObjectContext) {
                QString varName = "$" + keywordStringPHP;
                if (keywordPHPprevString.toLower() == "this" && keywordPHPprevStringPrevChar == "$") {
                    clsPropsIterator = clsProps.find(varName.toStdString());
                    if (clsPropsIterator != clsProps.end()) {
                        known = true;
                    }
                }
            }
            if (!known) {
                bool unused = false;
                if (keywordPHPprevChar == "$" && ((keywordPHPprevString.toLower() != "self" && keywordPHPprevString.toLower() != "static") || keywordPHPprevStringPrevChar == "$")) {
                    QString ns = "\\";
                    if (nsNamePHP.size() > 0) ns += nsNamePHP + "\\";
                    QString _clsName = clsNamePHP.size() > 0 ? ns + clsNamePHP : "";
                    QString k = _clsName + "::" + funcNamePHP;
                    QString varName = keywordPHPprevChar + keywordStringPHP;
                    unusedVarsIterator = unusedVars.find((k + "::" + varName).toStdString());
                    if (unusedVarsIterator != unusedVars.end() && unusedVarsIterator->second == keywordPHPStart) {
                        unused = true;
                    } else if (expectedFuncNamePHP.size() > 0) {
                        k = _clsName + "::" + expectedFuncNamePHP;
                        unusedVarsIterator = unusedVars.find((k + "::" + varName).toStdString());
                        if (unusedVarsIterator != unusedVars.end() && unusedVarsIterator->second == keywordPHPStart) {
                            unused = true;
                        }
                    }
                }
                if (unused) highlightString(keywordPHPStart, keywordPHPLength, HW->unusedVariableFormat);
                else highlightString(keywordPHPStart, keywordPHPLength, HW->variableFormat);
            } else {
                highlightString(keywordPHPStart, keywordPHPLength, HW->knownVariableFormat);
            }
        } else if (keywordPHPprevChar == ":" && keywordPHPprevString.size() > 0 && !isBigFile) {
            QString k = keywordPHPprevString.toLower()+"::"+keywordStringPHP;
            HW->phpClassWordsCSIterator = HW->phpClassWordsCS.find(k.toStdString());
            if (HW->phpClassWordsCSIterator != HW->phpClassWordsCS.end()) {
                QTextCharFormat format = HW->phpClassWordsCSIterator->second;
                highlightString(keywordPHPStart, keywordPHPLength, format);
            }
        }
        if (keywordPHPprevChar != "$" && !isObjectContext && keywordStringPHP.size()>0 && !isBigFile) {
            // namespace scope
            if (keywordStringPHP.size() > 0 && (keywordStringPHP.toLower() == "namespace" && expectedNsNamePHP.size() == 0)) {
                expectedNsNamePHP = "\\";
            }
            // class scope
            if (keywordStringPHP.size() > 0 && (((keywordStringPHP.toLower() == "class" || keywordStringPHP.toLower() == "interface" || keywordStringPHP.toLower() == "trait") && expectedClsNamePHP.size() == 0) || expectedClsNamePHP.toLower() == "class" || expectedClsNamePHP.toLower() == "interface" || expectedClsNamePHP.toLower() == "trait")) {
                expectedClsNamePHP = keywordStringPHP;
            }
            // function scope
            if (keywordStringPHP.size() > 0 && ((keywordStringPHP.toLower() == "function" && expectedFuncNamePHP.size() == 0) || expectedFuncNamePHP.toLower() == "function")) {
                expectedFuncNamePHP = keywordStringPHP;
                expectedFuncParsPHP = parensPHP;
                expectedFuncArgsPHP.clear();
                expectedFuncArgsPHPPositions.clear();
                expectedFuncArgsPHPBlocks.clear();
            }
        }
        keywordPHPprevString = keywordStringPHP;
        keywordPHPprevStringPrevChar = keywordPHPprevChar;
        keywordPHPprevPrevChar = c;
        keywordPHPprevChar = c;
        keywordPHPStartPrev = keywordPHPStart;
        keywordPHPLengthPrev = keywordPHPLength;
    }
    if (c == ';') {
        keywordPHPprevString = "";
        keywordPHPprevStringPrevChar = "";
    }
    if (keywordPHPStartPrev>=0 && keywordPHPLengthPrev>0) {
        // php functions
        if (c == "(") {
            highlightString(keywordPHPStartPrev, keywordPHPLengthPrev, HW->functionFormat);
            keywordPHPStartPrev = -1;
            keywordPHPLengthPrev = -1;
        } else if (!isWSpace) {
            keywordPHPStartPrev = -1;
            keywordPHPLengthPrev = -1;
        }
    }

    // php curly brackets
    if (stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentSLOpenedPHP < 0 && commentMLOpenedPHP < 0 && keywordPHPOpened < 0 && c == "{") {
        bracesPHP++;
        // open namespace scope
        if (expectedNsNamePHP.size() > 0) {
            if (nsStartsPHP.size() > nsEndsPHP.size()) nsEndsPHP.append(pos);
            QString cNsNamePHP = nsNamePHP;
            int cNsScopePHP = nsScopePHP;
            if (nsScopePHP < 0) cNsNamePHP = "";
            nsNamePHP = expectedNsNamePHP.mid(1).trimmed();
            if (nsNamePHP.size() > 0 && nsNamePHP[0] == "\\") nsNamePHP = nsNamePHP.mid(1).trimmed();
            if (nsNamePHP.size() > 0) {
                if (cNsNamePHP.size() > 0) nsNamePHP = cNsNamePHP + "\\" + nsNamePHP;
                nsNamesPHP.append(nsNamePHP);
                nsStartsPHP.append(pos+1);
                nsScopePHP = bracesPHP-1;
                if (nsChainPHP.size() > 0) nsChainPHP += ",";
                nsChainPHP += cNsNamePHP;
                nsScopeChainPHP.append(cNsScopePHP);
            } else {
                nsScopePHP = -1;
            }
            expectedNsNamePHP = "";
        }
        // open class scope
        if (expectedClsNamePHP.size() > 0) {
            QString cClsNamePHP = clsNamePHP;
            int cClsScopePHP = clsScopePHP;
            if (clsScopePHP < 0) cClsNamePHP = "";
            if (expectedClsNamePHP == "class" || expectedClsNamePHP == "implements" || expectedClsNamePHP == "extends") expectedClsNamePHP = "anonymous class";
            if (clsStartsPHP.size() > clsEndsPHP.size()) clsEndsPHP.append(pos);
            clsNamePHP = expectedClsNamePHP;
            clsScopePHP = bracesPHP-1;
            clsNamesPHP.append(clsNamePHP);
            clsStartsPHP.append(pos+1);
            if (clsChainPHP.size() > 0) clsChainPHP += ",";
            clsChainPHP += cClsNamePHP;
            clsScopeChainPHP.append(cClsScopePHP);
            expectedClsNamePHP = "";
            // variables
            if (cClsNamePHP.size() > 0 || funcNamePHP.size() > 0) {
                varsChainsPHP.append(varsChainPHP);
                usedVarsChainsPHP.append(usedVarsChainPHP);
            } else {
                varsGlobChainPHP = varsChainPHP;
                usedVarsGlobChainPHP = usedVarsChainPHP;
            }
            if (clsOpenPHP) {
                QStringList varsChainList = varsChainPHP.split(",");
                for (int i=0; i<varsChainList.size(); i++) {
                    QString varName = varsChainList.at(i);
                    clsPropsIterator = clsProps.find(varName.toStdString());
                    if (clsPropsIterator == clsProps.end()) {
                        if (varsClsChainPHP.size() > 0) varsClsChainPHP += ",";
                        varsClsChainPHP += varName;
                    }
                }
            }
            varsChainPHP = "";
            usedVarsChainPHP = "";
            variables.clear();
            usedVariables.clear();
            expectedFuncArgsPHP.clear();
            expectedFuncArgsPHPPositions.clear();
            expectedFuncArgsPHPBlocks.clear();
            clsOpensPHP.append(clsOpenPHP);
            clsOpenPHP = true;
            if (varsClsOpenChainPHP.size() > 0 && varsClsChainPHP.size() > 0) varsClsOpenChainPHP += ",";
            varsClsOpenChainPHP += varsClsChainPHP;
            clsPropsChainPHP.append(varsClsChainPHP);
            varsClsChainPHP = "";
        }
        // open function scope
        if (expectedFuncNamePHP.size() > 0 && expectedFuncParsPHP == parensPHP) {
            QString cFuncNamePHP = funcNamePHP;
            int cFuncScopePHP = funcScopePHP;
            if (funcScopePHP < 0) cFuncNamePHP = "";
            if (funcStartsPHP.size() > funcEndsPHP.size()) funcEndsPHP.append(pos);
            funcNamePHP = expectedFuncNamePHP;
            funcScopePHP = bracesPHP-1;
            funcNamesPHP.append(funcNamePHP);
            funcStartsPHP.append(pos+1);
            if (funcChainPHP.size() > 0) funcChainPHP += ",";
            funcChainPHP += cFuncNamePHP;
            funcScopeChainPHP.append(cFuncScopePHP);
            expectedFuncNamePHP = "";
            expectedFuncParsPHP = -1;
            // variables
            if (cFuncNamePHP.size() > 0 || clsNamePHP.size() > 0) {
                varsChainsPHP.append(varsChainPHP);
                usedVarsChainsPHP.append(usedVarsChainPHP);
            } else {
                varsGlobChainPHP = varsChainPHP;
                usedVarsGlobChainPHP = usedVarsChainPHP;
            }
            clsPropsChainPHP.append(varsClsChainPHP);
            if (clsOpenPHP) {
                varsClsChainPHP = varsChainPHP;
                clsProps = variables;
            }
            varsChainPHP = "";
            usedVarsChainPHP = "";
            variables.clear();
            usedVariables.clear();
            for (int i=0; i<expectedFuncArgsPHP.size(); i++) {
                QString varName = expectedFuncArgsPHP.at(i);
                if (varsChainPHP.size() > 0) varsChainPHP += ",";
                varsChainPHP += varName;
                variables[varName.toStdString()] = varName.toStdString();
                QString ns = "\\";
                if (nsNamePHP.size() > 0) ns += nsNamePHP + "\\";
                QString _clsName = clsNamePHP.size() > 0 ? ns + clsNamePHP : "";
                QString k = _clsName + "::" + funcNamePHP;
                // save to known vars
                if (highlightVarsMode || firstRunMode) {
                    knownVars[k.toStdString()] = varsChainPHP.toStdString();
                    QString kk = k + "::" + varName;
                    int eP = -1;
                    if (expectedFuncArgsPHPPositions.size() == expectedFuncArgsPHP.size()) {
                        eP = expectedFuncArgsPHPPositions.at(i);
                    }
                    knownVarsPositions[kk.toStdString()] = eP;
                    int eB = -1;
                    if (expectedFuncArgsPHPBlocks.size() == expectedFuncArgsPHP.size()) {
                        eB = expectedFuncArgsPHPBlocks.at(i);
                    }
                    knownVarsBlocks[kk.toStdString()] = eB;
                    usedVars[k.toStdString()] = usedVarsChainPHP.toStdString();
                }
            }
            expectedFuncArgsPHP.clear();
            expectedFuncArgsPHPPositions.clear();
            expectedFuncArgsPHPBlocks.clear();
            clsOpensPHP.append(clsOpenPHP);
            clsOpenPHP = false;
            // save to known functions
            QString ns = "\\";
            if (nsNamePHP.size() > 0) ns += nsNamePHP + "\\";
            QString _clsName = clsNamePHP.size() > 0 ? ns + clsNamePHP : "";
            QString k = _clsName + "::" + funcNamePHP;
            knownFunctions[k.toStdString()] = funcNamePHP.toStdString();
        }
    } else if (stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentSLOpenedPHP < 0 && commentMLOpenedPHP < 0 && keywordPHPOpened < 0 && c == "}") {
        bracesPHP--;
        if (bracesPHP < 0) bracesPHP = 0;
        // close namespace scope
        if (nsNamePHP.size() > 0 && nsScopePHP >= 0 && nsScopePHP == bracesPHP) {
            nsNamePHP = "";
            nsScopePHP = -1;
            nsEndsPHP.append(pos);
            expectedNsNamePHP = "";
            // set parent namespace
            int parentNsScopePHP = -1;
            QString parentNsPHP = "", parentNsChainPHP = "";
            if (!nsScopeChainPHP.isEmpty()) {
                parentNsScopePHP = nsScopeChainPHP.last();
                nsScopeChainPHP.removeLast();
                QStringList nsChainListPHP  = nsChainPHP.split(",");
                parentNsPHP = nsChainListPHP.last();
                for (int i=0; i<nsChainListPHP.size()-1; i++) {
                    if (parentNsChainPHP.size() > 0) parentNsChainPHP += ",";
                    parentNsChainPHP += nsChainListPHP.at(i);
                }
                if (parentNsScopePHP >= 0 && parentNsPHP.size() > 0) {
                    nsNamePHP = parentNsPHP;
                    nsChainPHP = parentNsChainPHP;
                    nsNamesPHP.append(nsNamePHP);
                    nsStartsPHP.append(pos+1);
                    nsScopePHP = parentNsScopePHP;
                }
            }
        }
        // close class scope
        if (clsNamePHP.size() > 0 && clsScopePHP >= 0 && clsScopePHP == bracesPHP) {
            clsNamePHP = "";
            clsScopePHP = -1;
            clsEndsPHP.append(pos);
            expectedClsNamePHP = "";
            // set parent cls
            int parentClsScopePHP = -1;
            QString parentClsPHP = "", parentClsChainPHP = "";
            if (!clsScopeChainPHP.isEmpty()) {
                parentClsScopePHP = clsScopeChainPHP.last();
                clsScopeChainPHP.removeLast();
                QStringList clsChainListPHP  = clsChainPHP.split(",");
                parentClsPHP = clsChainListPHP.last();
                for (int i=0; i<clsChainListPHP.size()-1; i++) {
                    if (parentClsChainPHP.size() > 0) parentClsChainPHP += ",";
                    parentClsChainPHP += clsChainListPHP.at(i);
                }
                if (parentClsScopePHP >= 0 && parentClsPHP.size() > 0) {
                    clsNamePHP = parentClsPHP;
                    clsChainPHP = parentClsChainPHP;
                    clsNamesPHP.append(clsNamePHP);
                    clsStartsPHP.append(pos+1);
                    clsScopePHP = parentClsScopePHP;
                }
            }
            // restore variables
            varsChainPHP = "";
            usedVarsChainPHP = "";
            variables.clear();
            usedVariables.clear();
            if (!varsChainsPHP.isEmpty()) {
                varsChainPHP = varsChainsPHP.last();
                varsChainsPHP.removeLast();
            } else if (clsNamePHP.size() == 0 && funcNamePHP.size() == 0) {
                varsChainPHP = varsGlobChainPHP;
            }
            if (!usedVarsChainsPHP.isEmpty()) {
                usedVarsChainPHP = usedVarsChainsPHP.last();
                usedVarsChainsPHP.removeLast();
            } else if (clsNamePHP.size() == 0 && funcNamePHP.size() == 0) {
                usedVarsChainPHP = usedVarsGlobChainPHP;
            }
            if (varsChainPHP.size() > 0) {
                QStringList varsChainList = varsChainPHP.split(",");
                for (int i=0; i<varsChainList.size(); i++) {
                    QString varName = varsChainList.at(i);
                    variables[varName.toStdString()] = varName.toStdString();
                }
            }
            if (usedVarsChainPHP.size() > 0) {
                QStringList varsChainList = usedVarsChainPHP.split(",");
                for (int i=0; i<varsChainList.size(); i++) {
                    QString varName = varsChainList.at(i);
                    usedVariables[varName.toStdString()] = varName.toStdString();
                }
            }
            if (!clsPropsChainPHP.isEmpty()) {
                varsClsChainPHP = clsPropsChainPHP.last();
                clsPropsChainPHP.removeLast();
                for (int i=0; i<varsClsChainPHP.size(); i++) {
                    QString varName = varsClsChainPHP.at(i);
                    clsProps[varName.toStdString()] = varName.toStdString();
                }
            } else {
                varsClsChainPHP = "";
                clsProps.clear();
            }
            if (!clsOpensPHP.isEmpty()) {
                clsOpenPHP = clsOpensPHP.last();
                clsOpensPHP.removeLast();
            } else {
                clsOpenPHP = false;
            }
        }
        // close function scope
        if (funcNamePHP.size() > 0 && funcScopePHP >= 0 && funcScopePHP == bracesPHP) {
            funcNamePHP = "";
            funcScopePHP = -1;
            funcEndsPHP.append(pos);
            expectedFuncNamePHP = "";
            expectedFuncParsPHP = -1;
            expectedFuncArgsPHP.clear();
            expectedFuncArgsPHPPositions.clear();
            expectedFuncArgsPHPBlocks.clear();
            // set parent function
            int parentFuncScopePHP = -1;
            QString parentFuncPHP = "", parentFuncChainPHP = "";
            if (!funcScopeChainPHP.isEmpty()) {
                parentFuncScopePHP = funcScopeChainPHP.last();
                funcScopeChainPHP.removeLast();
                QStringList funcChainListPHP  = funcChainPHP.split(",");
                parentFuncPHP = funcChainListPHP.last();
                for (int i=0; i<funcChainListPHP.size()-1; i++) {
                    if (parentFuncChainPHP.size() > 0) parentFuncChainPHP += ",";
                    parentFuncChainPHP += funcChainListPHP.at(i);
                }
                if (parentFuncScopePHP >= 0 && parentFuncPHP.size() > 0) {
                    funcNamePHP = parentFuncPHP;
                    funcChainPHP = parentFuncChainPHP;
                    funcNamesPHP.append(funcNamePHP);
                    funcStartsPHP.append(pos+1);
                    funcScopePHP = parentFuncScopePHP;
                }
            }
            // restore variables
            varsChainPHP = "";
            usedVarsChainPHP = "";
            variables.clear();
            usedVariables.clear();
            if (!varsChainsPHP.isEmpty()) {
                varsChainPHP = varsChainsPHP.last();
                varsChainsPHP.removeLast();
            } else if (clsNamePHP.size() == 0 && funcNamePHP.size() == 0) {
                varsChainPHP = varsGlobChainPHP;
            }
            if (!usedVarsChainsPHP.isEmpty()) {
                usedVarsChainPHP = usedVarsChainsPHP.last();
                usedVarsChainsPHP.removeLast();
            } else if (clsNamePHP.size() == 0 && funcNamePHP.size() == 0) {
                usedVarsChainPHP = usedVarsGlobChainPHP;
            }
            if (varsChainPHP.size() > 0) {
                QStringList varsChainList = varsChainPHP.split(",");
                for (int i=0; i<varsChainList.size(); i++) {
                    QString varName = varsChainList.at(i);
                    variables[varName.toStdString()] = varName.toStdString();
                }
            }
            if (usedVarsChainPHP.size() > 0) {
                QStringList varsChainList = usedVarsChainPHP.split(",");
                for (int i=0; i<varsChainList.size(); i++) {
                    QString varName = varsChainList.at(i);
                    usedVariables[varName.toStdString()] = varName.toStdString();
                }
            }
            if (!clsPropsChainPHP.isEmpty()) {
                varsClsChainPHP = clsPropsChainPHP.last();
                clsPropsChainPHP.removeLast();
                for (int i=0; i<varsClsChainPHP.size(); i++) {
                    QString varName = varsClsChainPHP.at(i);
                    clsProps[varName.toStdString()] = varName.toStdString();
                }
            } else {
                varsClsChainPHP = "";
                clsProps.clear();
            }
            if (!clsOpensPHP.isEmpty()) {
                clsOpenPHP = clsOpensPHP.last();
                clsOpensPHP.removeLast();
            } else {
                clsOpenPHP = false;
            }
        }
    }

    // expected namespace scope
    if (expectedNsNamePHP.size() > 0 && c != ";") {
        expectedNsNamePHP += c;
    } else if (expectedNsNamePHP.size() > 0 && c == ";") {
        // open namespace without scope
        if (nsStartsPHP.size() > nsEndsPHP.size()) {
            nsEndsPHP.append(pos);
        }
        nsNamePHP = expectedNsNamePHP.mid(1).trimmed();
        if (nsNamePHP.size() > 0 && nsNamePHP[0] == "\\") nsNamePHP = nsNamePHP.mid(1).trimmed();
        nsScopePHP = -1;
        if (nsNamePHP.size() > 0) {
            nsNamesPHP.append(nsNamePHP);
            nsChainPHP = "";
            nsScopeChainPHP.clear();
            nsStartsPHP.append(pos+1);
        }
        expectedNsNamePHP = "";
    }
    // unexpected class scope
    if (expectedClsNamePHP.size() > 0 && expectedClsNamePHP.toLower() == "class" && c == "(") {
        expectedClsNamePHP = "anonymous class";
    } else if (expectedClsNamePHP.size() > 0 && c == ";") {
        expectedClsNamePHP = "";
    }
    // unexpected function scope
    if (expectedFuncNamePHP.size() > 0 && expectedFuncNamePHP.toLower() == "function" && c == "(" && expectedFuncParsPHP == parensPHP) {
        expectedFuncNamePHP = "anonymous function";
    } else if (expectedFuncNamePHP.size() > 0 && c == ";" && expectedFuncParsPHP == parensPHP) {
        expectedFuncNamePHP = "";
        expectedFuncParsPHP = -1;
    }

    // php parentheses
    if (stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentSLOpenedPHP < 0 && commentMLOpenedPHP < 0 && keywordPHPOpened < 0 && c == "(") {
        parensPHP++;
    } else if (stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentSLOpenedPHP < 0 && commentMLOpenedPHP < 0 && keywordPHPOpened < 0 && c == ")") {
        parensPHP--;
        if (parensPHP < 0) parensPHP = 0;
    }

    // saving special chars data
    if (stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentSLOpenedPHP < 0 && commentMLOpenedPHP < 0 && keywordPHPOpened < 0 && (c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]")) {
        addSpecialChar(c, pos);
    }

    // tabs, spaces, semicolons, commas
    if (stringSQOpenedPHP < 0 && stringDQOpenedPHP < 0 && stringBOpened < 0 && commentSLOpenedPHP < 0 && commentMLOpenedPHP < 0 && keywordPHPOpened < 0) {
        if (highlightTabs && c == "\t") {
            highlightChar(pos, HW->tabFormat);
        } else if (highlightSpaces && c == " ") {
            highlightChar(pos, HW->spaceFormat);
        }
        if (!isBigFile && (c == ";" || c == "," || c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]" || c == "\\" || c == "@")) {
            highlightChar(pos, HW->punctuationFormat);
        }
    }
}

void Highlight::parseUnknown(const QChar &c, int pos)
{
    if (mode != MODE_UNKNOWN) return;

    if (highlightTabs && c == "\t") {
        highlightChar(pos, HW->tabFormat);
    } else if (highlightSpaces && c == " ") {
        highlightChar(pos, HW->spaceFormat);
    }
    if (!isBigFile && (c == ";" || c == "," || c == "{" || c == "}" || c == "(" || c == ")" || c == "[" || c == "]" || c == "\\" || c == "@" || c == "&" || c == "<" || c == ">" || c == "." || c == "*" || c == "/" || c == "?" || c == ":" || c == "%" || c == "$" || c == "^" || c == "#" || c == "@" || c == "!" || c == "-" || c == "=" || c == "+" || c == "'" || c == "\"" || c == "|" || c == "~" || c == "`")) {
        highlightChar(pos, HW->punctuationFormat);
    } else if (!isBigFile && c.isUpper()) {
        highlightChar(pos, HW->constFormat);
    }
}

void Highlight::updateState(const QChar & c, int pos, int & pState)
{
    if (state == pState) return;
    if (stateStarts.size()>stateEnds.size()) {
        stateEnds.append(pos);
    }
    if (state != STATE_NONE) {
        int stateStart = pos;
        if (pos > 0 && (
            state == STATE_COMMENT_ML_CSS ||
            state == STATE_COMMENT_ML_JS ||
            state == STATE_COMMENT_ML_PHP ||
            state == STATE_COMMENT_SL_JS
        )) {
            stateStart = pos - 1;
        }
        if (state == STATE_COMMENT_SL_PHP && c != "#" && pos > 0) stateStart = pos - 1;
        if (state == STATE_COMMENT_ML_HTML) stateStart = commentHTMLOpened;
        if (state == STATE_STRING_HEREDOC || state == STATE_STRING_NOWDOC) stateStart = stringBStart;
        stateStart++;
        if (stateStarts.size() > 0 && stateStarts.last() == stateStart &&
            stateStarts.size() == stateIds.size() && stateStarts.size() == stateEnds.size()
        ) {
            stateStarts.removeLast();
            stateIds.removeLast();
            stateEnds.removeLast();
        }
        stateStarts.append(stateStart);
        stateIds.append(state);
    }
    pState = state;
}

void Highlight::openBlockDataLists()
{
    if (modeType == MODE_MIXED && mode != MODE_HTML) {
        modeStarts.append(0);
        modeTags.append(mode);
    }
    if (state != STATE_NONE) {
        stateStarts.append(0);
        stateIds.append(state);
    }
    if (nsNamePHP.size() > 0) {
        nsStartsPHP.append(0);
        nsNamesPHP.append(nsNamePHP);
    }
    if (clsNamePHP.size() > 0 && clsScopePHP >= 0) {
        clsStartsPHP.append(0);
        clsNamesPHP.append(clsNamePHP);
    }
    if (funcNamePHP.size() > 0 && funcScopePHP >= 0) {
        funcStartsPHP.append(0);
        funcNamesPHP.append(funcNamePHP);
    }
    if (varsChainPHP.size() > 0) {
        QStringList varsChainList = varsChainPHP.split(",");
        for (int i=0; i<varsChainList.size(); i++) {
            QString varName = varsChainList.at(i);
            variables[varName.toStdString()] = varName.toStdString();
        }
    }
    if (usedVarsChainPHP.size() > 0) {
        QStringList varsChainList = usedVarsChainPHP.split(",");
        for (int i=0; i<varsChainList.size(); i++) {
            QString varName = varsChainList.at(i);
            usedVariables[varName.toStdString()] = varName.toStdString();
        }
    }
    if (varsClsChainPHP.size() > 0) {
        QStringList varsClsChainList = varsClsChainPHP.split(",");
        for (int i=0; i<varsClsChainList.size(); i++) {
            QString varName = varsClsChainList.at(i);
            clsProps[varName.toStdString()] = varName.toStdString();
        }
    }
    if (funcNameJS.size() > 0 && funcScopeJS >= 0) {
        funcStartsJS.append(0);
        funcNamesJS.append(funcNameJS);
    }
    if (mediaNameCSS.size() > 0 && mediaScopeCSS >= 0) {
        mediaStartsCSS.append(0);
        mediaNamesCSS.append(mediaNameCSS);
    }
    if (tagChainHTML.size() > 0) {
        tagChainStartsHTML.append(0);
        tagChainsHTML.append(tagChainHTML);
    }
    if (varsChainJS.size() > 0) {
        QStringList varsChainList = varsChainJS.split(",");
        for (int i=0; i<varsChainList.size(); i++) {
            QString varName = varsChainList.at(i);
            jsNames[varName.toStdString()] = varName.toStdString();
        }
    }
    if (cssNamesChain.size() > 0) {
        QStringList cssNamesChainList = cssNamesChain.split(",");
        for (int i=0; i<cssNamesChainList.size(); i++) {
            QString cssName = cssNamesChainList.at(i);
            cssNames[cssName.toStdString()] = cssName.toStdString();
        }
    }
}

void Highlight::closeBlockDataLists(int textSize)
{
    if (modeType == MODE_MIXED && mode != MODE_HTML && modeStarts.size() > modeEnds.size()) {
        modeEnds.append(textSize);
    }
    if (stateStarts.size()>stateEnds.size()) {
        stateEnds.append(textSize);
    }
    if (nsStartsPHP.size()>nsEndsPHP.size()) {
        nsEndsPHP.append(textSize);
    }
    if (clsStartsPHP.size()>clsEndsPHP.size()) {
        clsEndsPHP.append(textSize);
    }
    if (funcStartsPHP.size()>funcEndsPHP.size()) {
        funcEndsPHP.append(textSize);
    }
    if (funcStartsJS.size()>funcEndsJS.size()) {
        funcEndsJS.append(textSize);
    }
    if (mediaStartsCSS.size()>mediaEndsCSS.size()) {
        mediaEndsCSS.append(textSize);
    }
    if (tagChainStartsHTML.size()>tagChainEndsHTML.size()) {
        tagChainEndsHTML.append(textSize);
    }
}

bool Highlight::isDirty()
{
    return dirty;
}

void Highlight::rehighlight()
{
    rehighlightBlockMode = true;
    int blocksCount = doc->blockCount();
    QTextBlock block = doc->findBlockByNumber(0);
    if (!block.isValid()) return;
    int startPos = block.position();
    int progressPercent = 0;
    do {
        dirty = true;
        highlightBlock(block, false);
        if (highlightVarsMode || firstRunMode) {
            int percent = (block.blockNumber()+1)*100 / blocksCount;
            if (percent - progressPercent > 10) {
                progressPercent = percent;
                doc->markContentsDirty(startPos, block.position() - startPos + block.length());
                startPos = block.position() + block.length();
                dirty = false;
                emit progressChanged(percent);
            }
        }
        block = block.next();
    } while(block.isValid());
    if (!block.isValid()) block = block.previous();
    doc->markContentsDirty(startPos, block.position() - startPos + block.length());
    dirty = false;
    if (highlightVarsMode || firstRunMode) {
        emit progressChanged(100);
    }
    rehighlightBlockMode = false;
}

void Highlight::rehighlightBlock(QTextBlock & block)
{
    rehighlightBlockMode = true;
    highlightBlock(block, true);
    rehighlightBlockMode = false;
}

void Highlight::highlightChanges(QTextCursor curs)
{
    QTextBlock block = curs.block();
    if (!block.isValid()) return;
    int startPos = block.position();
    do {
        dirty = true;
        const int stateBeforeHighlight = block.userState();
        highlightBlock(block, false);
        if (block.userState() == stateBeforeHighlight) break;
        block = block.next();
    } while(block.isValid());
    if (!block.isValid()) block = block.previous();
    doc->markContentsDirty(startPos, block.position() - startPos + block.length());
    dirty = false;
}

void Highlight::highlightBlock(QTextBlock & block, bool markDirty)
{
    formatChanges.clear();
    formatChanges.fill(QTextCharFormat(), block.text().size());
    cBlock = block;
    blockData = dynamic_cast<HighlightData *>(block.userData());
    QString text = block.text();
    if (parseBlock(text)) {
        applyFormatChanges(markDirty);
    }
    formatChanges.clear();
}

bool Highlight::parseBlock(const QString & text)
{
    if (!enabled) return false;

    reset();
    if (modeType != MODE_MIXED) {
        mode = modeType;
    }

    std::string _mode = mode;
    std::string _prevMode = prevMode;
    QString _stringBlock = stringBlock;
    int _state = state;
    int _prevState = prevState;
    int _prevPrevState = prevPrevState;
    QString _stringEscStringCSS = stringEscStringCSS;
    QString _stringEscStringJS = stringEscStringJS;
    QString _regexpEscStringJS = regexpEscStringJS;
    QString _regexpPrevCharJs = regexpPrevCharJS;
    int _bracesCSS = bracesCSS;
    int _bracesJS = bracesJS;
    int _bracesPHP = bracesPHP;
    //int _parensCSS = parensCSS;
    int _parensJS = parensJS;
    int _parensPHP = parensPHP;
    bool _keywordPHPScoped = keywordPHPScoped;
    bool _keywordJSScoped = keywordJSScoped;
    QString _exprEscStringJS = exprEscStringJS;
    QString _stringEscVariableJS = stringEscVariableJS;
    bool _hasMarkPoint = false; // used by editor
    bool _isModified = false; // used by editor
    QString _nsNamePHP = nsNamePHP;
    QString _nsChainPHP = nsChainPHP;
    QString _clsNamePHP = clsNamePHP;
    QString _clsChainPHP = clsChainPHP;
    QString _funcNamePHP = funcNamePHP;
    QString _funcChainPHP = funcChainPHP;
    QString _varsChainPHP = varsChainPHP;
    QString _usedVarsChainPHP = usedVarsChainPHP;
    QString _varsGlobChainPHP = varsGlobChainPHP;
    QString _usedVarsGlobChainPHP = usedVarsGlobChainPHP;
    QString _varsClsChainPHP = varsClsChainPHP;
    QString _varsClsOpenChainPHP = varsClsOpenChainPHP;
    QString _expectedNsNamePHP = expectedNsNamePHP;
    QString _expectedClsNamePHP = expectedClsNamePHP;
    QString _expectedFuncNamePHP = expectedFuncNamePHP;
    QString _expectedFuncArgsPHPChain = expectedFuncArgsPHP.join(",");
    int _expectedFuncParsPHP = expectedFuncParsPHP;
    QString _expectedFuncNameJS = expectedFuncNameJS;
    int _expectedFuncParsJS = expectedFuncParsJS;
    QString _expectedFuncVarJS = expectedFuncVarJS;
    QString _varsChainJS = varsChainJS;
    QString _expectedMediaNameCSS = expectedMediaNameCSS;
    int _expectedMediaParsCSS = expectedMediaParsCSS;
    QString _funcNameJS = funcNameJS;
    QString _funcChainJS = funcChainJS;
    QString _expectedFuncArgsJSChain = expectedFuncArgsJS.join(",");
    QString _mediaNameCSS = mediaNameCSS;
    QString _tagChainHTML = tagChainHTML;
    QString _cssNamesChain = cssNamesChain;

    // load current block data
    if (blockData == nullptr) {
        blockData = new HighlightData();
    } else {
        _mode = blockData->mode.toStdString();
        _prevMode = blockData->prevMode.toStdString();
        _stringBlock = blockData->stringBlock;
        _state = blockData->state;
        _prevState = blockData->prevState;
        _prevPrevState = blockData->prevPrevState;
        _stringEscStringCSS = blockData->stringEscStringCSS;
        _stringEscStringJS = blockData->stringEscStringJS;
        _regexpEscStringJS = blockData->regexpEscStringJS;
        _regexpPrevCharJs = blockData->regexpPrevCharJS;
        _bracesCSS = blockData->bracesCSS;
        _bracesJS = blockData->bracesJS;
        _bracesPHP = blockData->bracesPHP;
        //_parensCSS = blockData->parensCSS;
        _parensJS = blockData->parensJS;
        _parensPHP = blockData->parensPHP;
        _keywordPHPScoped = blockData->keywordPHPScoped;
        _keywordJSScoped = blockData->keywordJSScoped;
        _exprEscStringJS = blockData->exprEscStringJS;
        _stringEscVariableJS = blockData->stringEscVariableJS;
        _hasMarkPoint = blockData->hasMarkPoint;
        _isModified = blockData->isModified;
        _nsNamePHP = blockData->nsNamePHP;
        _nsChainPHP = blockData->nsChainPHP;
        _clsNamePHP = blockData->clsNamePHP;
        _clsChainPHP = blockData->clsChainPHP;
        _funcNamePHP = blockData->funcNamePHP;
        _funcChainPHP = blockData->funcChainPHP;
        _varsChainPHP = blockData->varsChainPHP;
        _usedVarsChainPHP = blockData->usedVarsChainPHP;
        _varsGlobChainPHP = blockData->varsGlobChainPHP;
        _usedVarsGlobChainPHP = blockData->usedVarsGlobChainPHP;
        _varsClsChainPHP = blockData->varsClsChainPHP;
        _varsClsOpenChainPHP = blockData->varsClsOpenChainPHP;
        _expectedNsNamePHP = blockData->expectedNsNamePHP;
        _expectedClsNamePHP = blockData->expectedClsNamePHP;
        _expectedFuncNamePHP = blockData->expectedFuncNamePHP;
        _expectedFuncArgsPHPChain = blockData->expectedFuncArgsPHP.join(",");
        _expectedFuncParsPHP = blockData->expectedFuncParsPHP;
        _expectedFuncNameJS = blockData->expectedFuncNameJS;
        _expectedFuncParsJS = blockData->expectedFuncParsJS;
        _expectedFuncVarJS = blockData->expectedFuncVarJS;
        _varsChainJS = blockData->varsChainJS;
        _expectedMediaNameCSS = blockData->expectedMediaNameCSS;
        _expectedMediaParsCSS = blockData->expectedMediaParsCSS;
        _funcNameJS = blockData->funcNameJS;
        _funcChainJS = blockData->funcChainJS;
        _expectedFuncArgsJSChain = blockData->expectedFuncArgsJS.join(",");
        _mediaNameCSS = blockData->mediaNameCSS;
        _tagChainHTML = blockData->tagChainHTML;
        _cssNamesChain = blockData->cssNamesChain;
    }

    if (!highlightVarsMode && !firstRunMode && !rehighlightBlockMode && lastVisibleBlockNumber >= 0 && cBlock.blockNumber() > lastVisibleBlockNumber + EXTRA_HIGHLIGHT_BLOCKS_COUNT) {
        blockData->wantUpdate = true;
        cBlock.setUserData(blockData);
        return false;
    }

    blockData->reset();
    restoreState();

    // open data lists
    openBlockDataLists();

    std::string pMode = mode;
    int pState = state;
    int keywordCSSStartPrev = -1, keywordCSSLengthPrev = -1, keywordJSStartPrev = -1, keywordJSLengthPrev = -1, keywordPHPStartPrev = -1, keywordPHPLengthPrev = -1;
    bool cssValuePart = true;
    for (int i=0; i<text.size(); i++) {
        QChar c = text[i];
        bool isLast = (i == text.size()-1) ? true : false;
        bool isAlpha = isalpha(c.toLatin1()) > 0 || c == "_" ? true : false;
        bool isAlnum = isalnum(c.toLatin1()) > 0 || c == "_" ? true : false;
        bool isWSpace = c.isSpace();

        // detect highlight mode
        if (parseMode(c, i, isWSpace, isLast, pMode, pState)) continue;

        // html mode
        parseHTML(c, i, isAlpha, isAlnum, isLast);

        // css mode
        parseCSS(c, i, isAlpha, isAlnum, isWSpace, isLast, keywordCSSStartPrev, keywordCSSLengthPrev, cssValuePart);

        // js mode
        parseJS(c, i, isAlpha, isAlnum, isWSpace, isLast, keywordJSStartPrev, keywordJSLengthPrev);

        // php mode
        parsePHP(c, i, isAlpha, isAlnum, isWSpace, isLast, keywordPHPStartPrev, keywordPHPLengthPrev);

        // unknown
        parseUnknown(c, i);

        // state changes
        updateState(c, i, pState);
    }

    // draw spell underline
    highlightSpell();

    // close data lists
    closeBlockDataLists(text.size());

    // Helper::log(Helper::intToStr(cBlock.blockNumber())+": "+QString::fromStdString(prevMode)+", "+QString::fromStdString(mode)+" : "+Helper::intToStr(prevState)+", "+Helper::intToStr(state)+"\n");

    if (mode != _mode ||
        prevMode != _prevMode ||
        state != _state ||
        prevState != _prevState ||
        prevPrevState != _prevPrevState ||
        stringEscStringCSS != _stringEscStringCSS ||
        stringEscStringJS != _stringEscStringJS ||
        regexpEscStringJS != _regexpEscStringJS ||
        regexpPrevCharJS != _regexpPrevCharJs ||
        stringBlock != _stringBlock ||
        _bracesCSS != bracesCSS ||
        _bracesJS != bracesJS ||
        _bracesPHP != bracesPHP ||
        //_parensCSS != parensCSS ||
        _parensJS != parensJS ||
        _parensPHP != parensPHP ||
        _keywordPHPScoped != keywordPHPScoped ||
        _keywordJSScoped != keywordJSScoped ||
        _exprEscStringJS != exprEscStringJS ||
        _stringEscVariableJS != stringEscVariableJS ||
        _nsNamePHP != nsNamePHP ||
        _nsChainPHP != nsChainPHP ||
        _clsNamePHP != clsNamePHP ||
        _clsChainPHP != clsChainPHP ||
        _funcNamePHP != funcNamePHP ||
        _funcChainPHP != funcChainPHP ||
        _varsChainPHP != varsChainPHP ||
        _usedVarsChainPHP != usedVarsChainPHP ||
        _varsGlobChainPHP != varsGlobChainPHP ||
        _usedVarsGlobChainPHP != usedVarsGlobChainPHP ||
        _varsClsChainPHP != varsClsChainPHP ||
        _varsClsOpenChainPHP != varsClsOpenChainPHP ||
        _expectedNsNamePHP != expectedNsNamePHP ||
        _expectedClsNamePHP != expectedClsNamePHP ||
        _expectedFuncNamePHP != expectedFuncNamePHP ||
        _expectedFuncArgsPHPChain != expectedFuncArgsPHP.join(",") ||
        _expectedFuncParsPHP != expectedFuncParsPHP ||
        _expectedFuncNameJS != expectedFuncNameJS ||
        _expectedFuncParsJS != expectedFuncParsJS ||
        _expectedFuncVarJS != expectedFuncVarJS ||
        _varsChainJS != varsChainJS ||
        _expectedFuncArgsJSChain != expectedFuncArgsJS.join(",") ||
        _expectedMediaNameCSS != expectedMediaNameCSS ||
        _expectedMediaParsCSS != expectedMediaParsCSS ||
        _funcNameJS != funcNameJS ||
        _funcChainJS != funcChainJS ||
        _mediaNameCSS != mediaNameCSS ||
        _tagChainHTML != tagChainHTML ||
        _cssNamesChain != cssNamesChain
    ) {
        changeBlockState();
    }

    // save current block data
    blockData->state = state;
    blockData->prevState = prevState;
    blockData->prevPrevState = prevPrevState;
    blockData->mode = QString::fromStdString(mode);
    blockData->prevMode = QString::fromStdString(prevMode);
    blockData->modeExpect = QString::fromStdString(modeExpect);
    blockData->modeExpectC = QString::fromStdString(modeExpectC);
    blockData->modeString = modeString;
    blockData->modeStringC = modeStringC;
    blockData->prevModeExpect = QString::fromStdString(prevModeExpect);
    blockData->prevModeString = prevModeString;
    blockData->prevModeExpectC = QString::fromStdString(prevModeExpectC);
    blockData->prevModeStringC = prevModeStringC;
    blockData->prevModeSkip = prevModeSkip;
    blockData->prevModeSkipC = prevModeSkipC;
    blockData->stringBlock = stringBlock;
    blockData->stringEscStringCSS = stringEscStringCSS;
    blockData->stringEscStringJS = stringEscStringJS;
    blockData->regexpEscStringJS = regexpEscStringJS;
    blockData->regexpPrevCharJS = regexpPrevCharJS;
    blockData->bracesCSS = bracesCSS;
    blockData->bracesJS = bracesJS;
    blockData->bracesPHP = bracesPHP;
    blockData->parensCSS = parensCSS;
    blockData->parensJS = parensJS;
    blockData->parensPHP = parensPHP;
    blockData->cssMediaScope = cssMediaScope;
    blockData->keywordPHPScoped = keywordPHPScoped;
    blockData->keywordJSScoped = keywordJSScoped;
    blockData->exprEscStringJS = exprEscStringJS;
    blockData->stringEscVariableJS = stringEscVariableJS;
    blockData->specialChars = specialChars;
    blockData->specialCharsPos = specialCharsPos;
    blockData->specialWords = specialWords;
    blockData->specialWordsPos = specialWordsPos;
    blockData->modeStarts = modeStarts;
    blockData->modeEnds = modeEnds;
    blockData->modeTags = modeTags;
    blockData->stateStarts = stateStarts;
    blockData->stateEnds = stateEnds;
    blockData->stateIds = stateIds;
    blockData->hasMarkPoint = _hasMarkPoint; // used by editor
    blockData->isModified = _isModified; // used by editor
    blockData->nsNamePHP = nsNamePHP;
    blockData->nsChainPHP = nsChainPHP;
    blockData->nsScopeChainPHP = nsScopeChainPHP;
    blockData->nsStartsPHP = nsStartsPHP;
    blockData->nsEndsPHP = nsEndsPHP;
    blockData->nsNamesPHP = nsNamesPHP;
    blockData->clsNamePHP = clsNamePHP;
    blockData->clsChainPHP = clsChainPHP;
    blockData->clsScopeChainPHP = clsScopeChainPHP;
    blockData->clsStartsPHP = clsStartsPHP;
    blockData->clsEndsPHP = clsEndsPHP;
    blockData->clsNamesPHP = clsNamesPHP;
    blockData->funcNamePHP = funcNamePHP;
    blockData->funcChainPHP = funcChainPHP;
    blockData->funcScopeChainPHP = funcScopeChainPHP;
    blockData->funcStartsPHP = funcStartsPHP;
    blockData->funcEndsPHP = funcEndsPHP;
    blockData->funcNamesPHP = funcNamesPHP;
    blockData->expectedNsNamePHP = expectedNsNamePHP;
    blockData->expectedClsNamePHP = expectedClsNamePHP;
    blockData->expectedFuncNamePHP = expectedFuncNamePHP;
    blockData->expectedFuncParsPHP = expectedFuncParsPHP;
    blockData->expectedFuncArgsPHP = expectedFuncArgsPHP;
    blockData->expectedFuncArgsPHPPositions = expectedFuncArgsPHPPositions;
    blockData->expectedFuncArgsPHPBlocks = expectedFuncArgsPHPBlocks;
    blockData->nsScopePHP = nsScopePHP;
    blockData->clsScopePHP = clsScopePHP;
    blockData->funcScopePHP = funcScopePHP;
    blockData->varsChainsPHP = varsChainsPHP;
    blockData->usedVarsChainsPHP = usedVarsChainsPHP;
    blockData->varsGlobChainPHP = varsGlobChainPHP;
    blockData->usedVarsGlobChainPHP = usedVarsGlobChainPHP;
    blockData->varsClsChainPHP = varsClsChainPHP;
    blockData->varsChainPHP = varsChainPHP;
    blockData->usedVarsChainPHP = usedVarsChainPHP;
    blockData->clsOpenPHP = clsOpenPHP;
    blockData->clsOpensPHP = clsOpensPHP;
    blockData->varsClsOpenChainPHP = varsClsOpenChainPHP;
    blockData->clsPropsChainPHP = clsPropsChainPHP;
    blockData->funcNameJS = funcNameJS;
    blockData->funcScopeChainJS = funcScopeChainJS;
    blockData->funcChainJS = funcChainJS;
    blockData->funcStartsJS= funcStartsJS;
    blockData->funcEndsJS = funcEndsJS;
    blockData->funcNamesJS = funcNamesJS;
    blockData->expectedFuncArgsJS = expectedFuncArgsJS;
    blockData->expectedFuncNameJS = expectedFuncNameJS;
    blockData->expectedFuncVarJS = expectedFuncVarJS;
    blockData->expectedFuncParsJS = expectedFuncParsJS;
    blockData->funcScopeJS = funcScopeJS;
    blockData->varsChainJS = varsChainJS;
    blockData->mediaNameCSS = mediaNameCSS;
    blockData->mediaStartsCSS = mediaStartsCSS;
    blockData->mediaEndsCSS = mediaEndsCSS;
    blockData->mediaNamesCSS = mediaNamesCSS;
    blockData->expectedMediaNameCSS = expectedMediaNameCSS;
    blockData->expectedMediaParsCSS = expectedMediaParsCSS;
    blockData->mediaScopeCSS = mediaScopeCSS;
    blockData->tagChainHTML = tagChainHTML;
    blockData->tagChainStartsHTML = tagChainStartsHTML;
    blockData->tagChainEndsHTML = tagChainEndsHTML;
    blockData->tagChainsHTML = tagChainsHTML;
    blockData->keywordPHPprevString = keywordPHPprevString;
    blockData->keywordPHPprevStringPrevChar = keywordPHPprevStringPrevChar;
    blockData->keywordJSprevString = keywordJSprevString;
    blockData->keywordJSprevStringPrevChar = keywordJSprevStringPrevChar;
    blockData->cssNamesChain = cssNamesChain;
    cBlock.setUserData(blockData);

    return true;
}
