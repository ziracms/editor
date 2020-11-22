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
    static HighlightWords& instance();
    static void loadDelayed();
    static void setColors();
    static void reload();
    static void reset();
    static void addPHPClass(QString k);
    static void addPHPFunction(QString k);
    static void addPHPVariable(QString k);
    static void addPHPConstant(QString k);
    static void addPHPClassConstant(QString cls, QString c);
    static void addJSFunction(QString k);
    static void addJSInterface(QString k);
    static void addJSObject(QString k);
    static void addJSExtDartObject(QString k);
    static void addJSExtDartFunction(QString k);
    static void addCSSProperty(QString k);
    static void addHTMLTag(QString k);
    static void addHTMLShortTag(QString k);
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
    void _loadDelayed();
    void _setColors();
    void _reload();
    void _reset();
    void _addPHPClass(QString k);
    void _addPHPFunction(QString k);
    void _addPHPVariable(QString k);
    void _addPHPConstant(QString k);
    void _addPHPClassConstant(QString cls, QString c);
    void _addJSFunction(QString k);
    void _addJSInterface(QString k);
    void _addJSObject(QString k);
    void _addJSExtDartObject(QString k);
    void _addJSExtDartFunction(QString k);
    void _addCSSProperty(QString k);
    void _addHTMLTag(QString k);
    void _addHTMLShortTag(QString k);
    void _load();
private:
    HighlightWords();
public slots:
    static void load();
};

#endif // HIGHLIGHTWORDS_H
