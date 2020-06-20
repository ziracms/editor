/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef HIGHLIGHTDATA_H
#define HIGHLIGHTDATA_H

#include <QTextBlockUserData>
#include <unordered_map>

class HighlightData : public QTextBlockUserData
{
public:
    HighlightData();
    void reset();
    QString stringBlock;
    QString mode;
    QString prevMode;
    QString modeExpect;
    QString modeExpectC;
    QString modeString;
    QString modeStringC;
    QString prevModeExpect;
    QString prevModeString;
    QString prevModeExpectC;
    QString prevModeStringC;
    bool prevModeSkip;
    bool prevModeSkipC;
    int state;
    int prevState;
    int prevPrevState;
    QString stringEscStringCSS;
    QString stringEscStringJS;
    QString regexpEscStringJS;
    QString regexpPrevCharJS;
    int bracesCSS;
    int bracesJS;
    int bracesPHP;
    int parensCSS;
    int parensJS;
    int parensPHP;
    bool cssMediaScope;
    bool keywordPHPScoped;
    bool keywordJSScoped;
    QString exprEscStringJS;
    QString stringEscVariableJS;
    QVector<QChar> specialChars;
    QVector<int> specialCharsPos;
    QVector<QString> specialWords;
    QVector<int> specialWordsPos;
    QVector<int> modeStarts;
    QVector<int> modeEnds;
    QVector<std::string> modeTags;
    QVector<int> stateStarts;
    QVector<int> stateEnds;
    QVector<int> stateIds;
    bool hasMarkPoint;
    bool isModified;
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
    QString tagChainHTML;
    QVector<int> tagChainStartsHTML;
    QVector<int> tagChainEndsHTML;
    QVector<QString> tagChainsHTML;
    QString keywordPHPprevString;
    QString keywordPHPprevStringPrevChar;
    QString keywordJSprevString;
    QString keywordJSprevStringPrevChar;
    QVector<int> spellStarts;
    QVector<int> spellLengths;
    QString operatorsChainPHP;
    std::unordered_map<int, std::string> operatorsPHP;
    QString operatorsChainJS;
    std::unordered_map<int, std::string> operatorsJS;
    bool wantUpdate;
};

#endif // HIGHLIGHTDATA_H
