/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "highlightdata.h"

HighlightData::HighlightData()
{
    reset();
}

void HighlightData::reset()
{
    stringBlock = "";
    mode = "";
    prevMode = "";
    modeExpect = "";
    modeExpectC = "";
    modeString = "";
    modeStringC = "";
    prevModeExpect = "";
    prevModeString = "";
    prevModeExpectC = "";
    prevModeStringC = "";
    prevModeSkip = false;
    prevModeSkipC = false;
    state = -1;
    prevState = -1;
    prevPrevState = -1;
    stringEscStringCSS = "";
    stringEscStringJS = "";
    regexpEscStringJS = "";
    regexpPrevCharJS = "";
    bracesCSS = 0;
    bracesJS = 0;
    bracesPHP = 0;
    parensCSS = 0;
    parensJS = 0;
    parensPHP = 0;
    cssMediaScope = false;
    keywordPHPScoped = false;
    keywordJSScoped = false;
    exprEscStringJS = "";
    stringEscVariableJS = "";
    specialChars.clear();
    specialCharsPos.clear();
    specialWords.clear();
    specialWordsPos.clear();
    modeStarts.clear();
    modeEnds.clear();
    modeTags.clear();
    stateStarts.clear();
    stateEnds.clear();
    stateIds.clear();
    hasMarkPoint = false;
    isModified = false;
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
    tagChainHTML = "";
    tagChainStartsHTML.clear();
    tagChainEndsHTML.clear();
    tagChainsHTML.clear();
    keywordPHPprevString = "";
    keywordPHPprevStringPrevChar = "";
    keywordJSprevString = "";
    keywordJSprevStringPrevChar = "";
    wantUpdate = false;
}
