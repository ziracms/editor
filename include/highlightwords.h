/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef HIGHLIGHTWORDS_H
#define HIGHLIGHTWORDS_H

#include <QObject>
#include <unordered_map>
#include <QTextCharFormat>
#include "settings.h"

class HighlightWords : public QObject
{
    Q_OBJECT
public:
    HighlightWords();
    void reload();
    void reset();
    void addPHPClass(QString k);
    void addPHPFunction(QString k);
    void addPHPVariable(QString k);
    void addPHPConstant(QString k);
    void addPHPClassConstant(QString cls, QString c);
    void addJSFunction(QString k);
    void addJSInterface(QString k);
    void addJSObject(QString k);
    void addJSExtDartObject(QString k);
    void addJSExtDartFunction(QString k);
    void addCSSProperty(QString k);
    void addHTMLTag(QString k);
    void addHTMLShortTag(QString k);
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat variableFormat;
    QTextCharFormat knownVariableFormat;
    QTextCharFormat unusedVariableFormat;
    QTextCharFormat constFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat knownFunctionFormat;
    QTextCharFormat phpTagFormat;
    QTextCharFormat tagFormat;
    QTextCharFormat tagNameFormat;
    QTextCharFormat selectorFormat;
    QTextCharFormat selectorTagFormat;
    QTextCharFormat propertyFormat;
    QTextCharFormat pseudoClassFormat;
    QTextCharFormat cssSpecialFormat;
    QTextCharFormat knownFormat;
    QTextCharFormat expressionFormat;
    QTextCharFormat spaceFormat;
    QTextCharFormat tabFormat;
    QTextCharFormat colorFormat;
    QTextCharFormat punctuationFormat;
    std::unordered_map<std::string, QTextCharFormat> phpwords;
    std::unordered_map<std::string, QTextCharFormat>::iterator phpwordsIterator;
    std::unordered_map<std::string, QTextCharFormat> phpwordsCS;
    std::unordered_map<std::string, QTextCharFormat>::iterator phpwordsCSIterator;
    std::unordered_map<std::string, QTextCharFormat> phpClassWordsCS;
    std::unordered_map<std::string, QTextCharFormat>::iterator phpClassWordsCSIterator;
    std::unordered_map<std::string, QTextCharFormat> jswordsCS;
    std::unordered_map<std::string, QTextCharFormat>::iterator jswordsCSIterator;
    std::unordered_map<std::string, QTextCharFormat> jsExtDartWordsCS;
    std::unordered_map<std::string, QTextCharFormat>::iterator jsExtDartWordsCSIterator;
    std::unordered_map<std::string, QTextCharFormat> csswords;
    std::unordered_map<std::string, QTextCharFormat>::iterator csswordsIterator;
    std::unordered_map<std::string, QTextCharFormat> htmlwords;
    std::unordered_map<std::string, QTextCharFormat>::iterator htmlwordsIterator;
    std::unordered_map<std::string, QTextCharFormat> htmlshorts;
    std::unordered_map<std::string, QTextCharFormat>::iterator htmlshortsIterator;
    std::unordered_map<std::string, QTextCharFormat> generalwords;
    std::unordered_map<std::string, QTextCharFormat>::iterator generalwordsIterator;
protected:
    void loadPHPWords();
    void loadJSWords();
    void loadCSSWords();
    void loadGeneralWords();
public slots:
    void load();
};

#endif // HIGHLIGHTWORDS_H
