/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "highlightwords.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QTimer>

const int LOAD_DELAY = 250; // should not be less then PROJECT_LOAD_DELAY

HighlightWords::HighlightWords(Settings * settings)
{
    // highlight colors
    std::string keywordColorStr = settings->get("highlight_keyword_color");
    QColor keywordColor(keywordColorStr.c_str());
    std::string classColorStr = settings->get("highlight_class_color");
    QColor classColor(classColorStr.c_str());
    std::string functionColorStr = settings->get("highlight_function_color");
    QColor functionColor(functionColorStr.c_str());
    std::string knownFunctionColorStr = settings->get("highlight_known_function_color");
    QColor knownFunctionColor(knownFunctionColorStr.c_str());
    std::string variableColorStr = settings->get("highlight_variable_color");
    QColor variableColor(variableColorStr.c_str());
    std::string knownVariableColorStr = settings->get("highlight_known_variable_color");
    QColor knownVariableColor(knownVariableColorStr.c_str());
    std::string unusedVariableColorStr = settings->get("highlight_unused_variable_color");
    QColor unusedVariableColor(unusedVariableColorStr.c_str());
    std::string singleLineCommentColorStr = settings->get("highlight_single_line_comment_color");
    QColor singleLineCommentColor(singleLineCommentColorStr.c_str());
    std::string multiLineCommentColorStr = settings->get("highlight_multi_line_comment_color");
    QColor multiLineCommentColor(multiLineCommentColorStr.c_str());
    std::string stringColorStr = settings->get("highlight_string_color");
    QColor stringColor(stringColorStr.c_str());
    std::string tagColorStr = settings->get("highlight_tag_color");
    QColor tagColor(tagColorStr.c_str());
    std::string tagNameColorStr = settings->get("highlight_tag_name_color");
    QColor tagNameColor(tagNameColorStr.c_str());
    std::string phpTagColorStr = settings->get("highlight_php_tag_color");
    QColor phpTagColor(phpTagColorStr.c_str());
    std::string selectorColorStr = settings->get("highlight_selector_color");
    QColor selectorColor(selectorColorStr.c_str());
    std::string selectorTagColorStr = settings->get("highlight_selector_tag_color");
    QColor selectorTagColor(selectorTagColorStr.c_str());
    std::string propertyColorStr = settings->get("highlight_property_color");
    QColor propertyColor(propertyColorStr.c_str());
    std::string pseudoClassColorStr = settings->get("highlight_pseudo_class_color");
    QColor pseudoClassColor(pseudoClassColorStr.c_str());
    std::string cssSpecialColorStr = settings->get("highlight_css_special_color");
    QColor cssSpecialColor(cssSpecialColorStr.c_str());
    std::string knownColorStr = settings->get("highlight_known_color");
    QColor knownColor(knownColorStr.c_str());
    std::string exprColorStr = settings->get("highlight_expression_color");
    QColor exprColor(exprColorStr.c_str());
    std::string spaceColorStr = settings->get("highlight_space_color");
    QColor spaceColor(spaceColorStr.c_str());
    std::string tabColorStr = settings->get("highlight_tab_color");
    QColor tabColor(tabColorStr.c_str());
    std::string punctuationColorStr = settings->get("highlight_punctuation_color");
    QColor punctuationColor(punctuationColorStr.c_str());

    // highlight formats
    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(classColor);
    classFormat.setFontWeight(QFont::Normal);
    functionFormat.setForeground(functionColor);
    functionFormat.setFontWeight(QFont::Normal);
    knownFunctionFormat.setForeground(knownFunctionColor);
    knownFunctionFormat.setFontWeight(QFont::Normal);
    variableFormat.setForeground(variableColor);
    variableFormat.setFontWeight(QFont::Normal);
    knownVariableFormat.setForeground(knownVariableColor);
    knownVariableFormat.setFontWeight(QFont::Normal);
    unusedVariableFormat.setForeground(unusedVariableColor);
    unusedVariableFormat.setFontWeight(QFont::Normal);
    //unusedVariableFormat.setFontItalic(true);
    constFormat.setFontWeight(QFont::Bold);
    singleLineCommentFormat.setForeground(singleLineCommentColor);
    singleLineCommentFormat.setFontWeight(QFont::Normal);
    multiLineCommentFormat.setForeground(multiLineCommentColor);
    multiLineCommentFormat.setFontWeight(QFont::Normal);
    stringFormat.setForeground(stringColor);
    stringFormat.setFontWeight(QFont::Normal);
    tagFormat.setForeground(tagColor);
    tagFormat.setFontWeight(QFont::Normal);
    tagNameFormat.setForeground(tagNameColor);
    tagNameFormat.setFontWeight(QFont::Normal);
    phpTagFormat.setForeground(phpTagColor);
    phpTagFormat.setFontWeight(QFont::Normal);
    selectorFormat.setForeground(selectorColor);
    selectorFormat.setFontWeight(QFont::Normal);
    selectorTagFormat.setForeground(selectorTagColor);
    selectorTagFormat.setFontWeight(QFont::Normal);
    propertyFormat.setForeground(propertyColor);
    propertyFormat.setFontWeight(QFont::Normal);
    pseudoClassFormat.setForeground(pseudoClassColor);
    pseudoClassFormat.setFontWeight(QFont::Normal);
    cssSpecialFormat.setForeground(cssSpecialColor);
    cssSpecialFormat.setFontWeight(QFont::Normal);
    knownFormat.setForeground(knownColor);
    knownFormat.setFontWeight(QFont::Normal);
    expressionFormat.setForeground(exprColor);
    expressionFormat.setFontWeight(QFont::Normal);
    spaceFormat.setUnderlineColor(spaceColor);
    spaceFormat.setUnderlineStyle(QTextCharFormat::DotLine);
    tabFormat.setUnderlineColor(tabColor);
    tabFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    colorFormat.setUnderlineColor(tabColor);
    colorFormat.setUnderlineStyle(QTextCharFormat::DashUnderline);
    punctuationFormat.setForeground(punctuationColor);
    punctuationFormat.setFontWeight(QFont::Bold);

    QTimer::singleShot(LOAD_DELAY, this, SLOT(load()));
}

void HighlightWords::load()
{
    loadPHPWords();
    loadJSWords();
    loadCSSWords();
}

void HighlightWords::reload()
{
    reset();
    load();
}

void HighlightWords::reset()
{
    phpwords.clear();
    phpwordsCS.clear();
    phpClassWordsCS.clear();
    jswordsCS.clear();
    csswords.clear();
    htmlwords.clear();
    htmlshorts.clear();
}

void HighlightWords::loadPHPWords()
{
    QString k;

    // keywords
    QFile kf(":/highlight/php_keys");
    kf.open(QIODevice::ReadOnly);
    QTextStream kin(&kf);
    while (!kin.atEnd()) {
        k = kin.readLine();
        if (k == "") continue;
        phpwords[k.toStdString()] = keywordFormat;
    }
    kf.close();

    // consts
    QFile cf(":/highlight/php_consts");
    cf.open(QIODevice::ReadOnly);
    QTextStream cin(&cf);
    while (!cin.atEnd()) {
        k = cin.readLine();
        if (k == "") continue;
        phpwordsCS[k.toStdString()] = keywordFormat;
    }
    cf.close();

    // types
    QFile tf(":/highlight/php_types");
    tf.open(QIODevice::ReadOnly);
    QTextStream tin(&tf);
    while (!tin.atEnd()) {
        k = tin.readLine();
        if (k == "") continue;
        phpwords[k.toStdString()] = knownFormat;
    }
    tf.close();

    // classes
    QFile clf(":/highlight/php_classes");
    clf.open(QIODevice::ReadOnly);
    QTextStream clin(&clf);
    while (!clin.atEnd()) {
        k = clin.readLine();
        if (k == "") continue;
        phpwords[k.toStdString()] = knownFormat;
    }
    clf.close();
}

void HighlightWords::loadJSWords()
{
    QString k;

    // keywords
    QFile kf(":/highlight/js_keys");
    kf.open(QIODevice::ReadOnly);
    QTextStream kin(&kf);
    while (!kin.atEnd()) {
        k = kin.readLine();
        if (k == "") continue;
        jswordsCS[k.toStdString()] = keywordFormat;
    }
    kf.close();
}

void HighlightWords::loadCSSWords()
{
    QString k;

    // keywords
    QFile kf(":/highlight/css_keys");
    kf.open(QIODevice::ReadOnly);
    QTextStream kin(&kf);
    while (!kin.atEnd()) {
        k = kin.readLine();
        if (k == "") continue;
        csswords[k.toStdString()] = keywordFormat;
    }
    kf.close();

    // special
    QFile sf(":/highlight/css_spec");
    sf.open(QIODevice::ReadOnly);
    QTextStream sin(&sf);
    while (!sin.atEnd()) {
        k = sin.readLine();
        if (k == "") continue;
        csswords[k.toStdString()] = cssSpecialFormat;
    }
    sf.close();

    // types
    QFile tf(":/highlight/css_pseudo");
    tf.open(QIODevice::ReadOnly);
    QTextStream tin(&tf);
    while (!tin.atEnd()) {
        k = tin.readLine();
        if (k == "") continue;
        csswords[k.toStdString()] = pseudoClassFormat;
    }
    tf.close();
}

void HighlightWords::addPHPClass(QString k)
{
    phpwords[k.toLower().toStdString()] = classFormat;
}

void HighlightWords::addPHPFunction(QString k)
{
    phpwords[k.toLower().toStdString()] = knownFunctionFormat;
}

void HighlightWords::addPHPVariable(QString k)
{
    phpwordsCS[k.toStdString()] = knownVariableFormat;
}

void HighlightWords::addPHPConstant(QString k)
{
    phpwordsCS[k.toStdString()] = constFormat;
}

void HighlightWords::addPHPClassConstant(QString cls, QString c)
{
    if (cls.indexOf("\\") >= 0) cls = cls.mid(cls.lastIndexOf("\\")+1);
    QString k = cls.toLower() + "::" + c;
    phpClassWordsCS[k.toStdString()] = constFormat;
}

void HighlightWords::addJSFunction(QString k)
{
    jswordsCS[k.toStdString()] = knownFunctionFormat;
}

void HighlightWords::addJSInterface(QString k)
{
    jswordsCS[k.toStdString()] = classFormat;
}

void HighlightWords::addJSObject(QString k)
{
    jswordsCS[k.toStdString()] = classFormat;
}

void HighlightWords::addJSExtDartObject(QString k)
{
    jsExtDartWordsCS[k.toStdString()] = classFormat;
}

void HighlightWords::addJSExtDartFunction(QString k)
{
    jsExtDartWordsCS[k.toStdString()] = knownFunctionFormat;
}

void HighlightWords::addCSSProperty(QString k)
{
    csswords[k.toStdString()] = knownFormat;
}

void HighlightWords::addHTMLTag(QString k)
{
    htmlwords[k.toStdString()] = knownFormat;
}

void HighlightWords::addHTMLShortTag(QString k) {
    htmlshorts[k.toStdString()] = knownFormat;
}
