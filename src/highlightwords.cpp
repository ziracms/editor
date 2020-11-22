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

HighlightWords::HighlightWords(){}

HighlightWords& HighlightWords::instance()
{
    static HighlightWords _instance;
    return _instance;
}

void HighlightWords::loadDelayed()
{
    instance()._loadDelayed();
}

void HighlightWords::_loadDelayed()
{
    QTimer::singleShot(LOAD_DELAY, this, SLOT(load()));
}

void HighlightWords::load()
{
    instance()._load();
}

void HighlightWords::_load()
{
    loadPHPWords();
    loadJSWords();
    loadCSSWords();
    loadGeneralWords();
}

void HighlightWords::reload()
{
    instance()._reload();
}

void HighlightWords::_reload()
{
    reset();
    load();
}

void HighlightWords::reset()
{
    instance()._reset();
}

void HighlightWords::_reset()
{
    phpwords.clear();
    phpwordsCS.clear();
    phpClassWordsCS.clear();
    jswordsCS.clear();
    csswords.clear();
    htmlwords.clear();
    htmlshorts.clear();
}

void HighlightWords::setColors()
{
    instance()._setColors();
}

void HighlightWords::_setColors()
{
    // highlight colors
    std::string keywordColorStr = Settings::get("highlight_keyword_color");
    QColor keywordColor(keywordColorStr.c_str());
    std::string classColorStr = Settings::get("highlight_class_color");
    QColor classColor(classColorStr.c_str());
    std::string functionColorStr = Settings::get("highlight_function_color");
    QColor functionColor(functionColorStr.c_str());
    std::string knownFunctionColorStr = Settings::get("highlight_known_function_color");
    QColor knownFunctionColor(knownFunctionColorStr.c_str());
    std::string variableColorStr = Settings::get("highlight_variable_color");
    QColor variableColor(variableColorStr.c_str());
    std::string knownVariableColorStr = Settings::get("highlight_known_variable_color");
    QColor knownVariableColor(knownVariableColorStr.c_str());
    std::string unusedVariableColorStr = Settings::get("highlight_unused_variable_color");
    QColor unusedVariableColor(unusedVariableColorStr.c_str());
    std::string singleLineCommentColorStr = Settings::get("highlight_single_line_comment_color");
    QColor singleLineCommentColor(singleLineCommentColorStr.c_str());
    std::string multiLineCommentColorStr = Settings::get("highlight_multi_line_comment_color");
    QColor multiLineCommentColor(multiLineCommentColorStr.c_str());
    std::string stringColorStr = Settings::get("highlight_string_color");
    QColor stringColor(stringColorStr.c_str());
    std::string tagColorStr = Settings::get("highlight_tag_color");
    QColor tagColor(tagColorStr.c_str());
    std::string tagNameColorStr = Settings::get("highlight_tag_name_color");
    QColor tagNameColor(tagNameColorStr.c_str());
    std::string phpTagColorStr = Settings::get("highlight_php_tag_color");
    QColor phpTagColor(phpTagColorStr.c_str());
    std::string selectorColorStr = Settings::get("highlight_selector_color");
    QColor selectorColor(selectorColorStr.c_str());
    std::string selectorTagColorStr = Settings::get("highlight_selector_tag_color");
    QColor selectorTagColor(selectorTagColorStr.c_str());
    std::string propertyColorStr = Settings::get("highlight_property_color");
    QColor propertyColor(propertyColorStr.c_str());
    std::string pseudoClassColorStr = Settings::get("highlight_pseudo_class_color");
    QColor pseudoClassColor(pseudoClassColorStr.c_str());
    std::string cssSpecialColorStr = Settings::get("highlight_css_special_color");
    QColor cssSpecialColor(cssSpecialColorStr.c_str());
    std::string knownColorStr = Settings::get("highlight_known_color");
    QColor knownColor(knownColorStr.c_str());
    std::string exprColorStr = Settings::get("highlight_expression_color");
    QColor exprColor(exprColorStr.c_str());
    std::string spaceColorStr = Settings::get("highlight_space_color");
    QColor spaceColor(spaceColorStr.c_str());
    std::string tabColorStr = Settings::get("highlight_tab_color");
    QColor tabColor(tabColorStr.c_str());
    std::string punctuationColorStr = Settings::get("highlight_punctuation_color");
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

void HighlightWords::loadGeneralWords()
{
    QString k;

    // keywords
    QFile kf(":/highlight/general_keys");
    kf.open(QIODevice::ReadOnly);
    QTextStream kin(&kf);
    while (!kin.atEnd()) {
        k = kin.readLine();
        if (k == "") continue;
        generalwords[k.toStdString()] = keywordFormat;
    }
    kf.close();
}

void HighlightWords::addPHPClass(QString k)
{
    instance()._addPHPClass(k);
}

void HighlightWords::_addPHPClass(QString k)
{
    phpwords[k.toLower().toStdString()] = classFormat;
}

void HighlightWords::addPHPFunction(QString k)
{
    instance()._addPHPFunction(k);
}

void HighlightWords::_addPHPFunction(QString k)
{
    phpwords[k.toLower().toStdString()] = knownFunctionFormat;
}

void HighlightWords::addPHPVariable(QString k)
{
    instance()._addPHPVariable(k);
}

void HighlightWords::_addPHPVariable(QString k)
{
    phpwordsCS[k.toStdString()] = knownVariableFormat;
}

void HighlightWords::addPHPConstant(QString k)
{
    instance()._addPHPConstant(k);
}

void HighlightWords::_addPHPConstant(QString k)
{
    phpwordsCS[k.toStdString()] = constFormat;
}

void HighlightWords::addPHPClassConstant(QString cls, QString c)
{
    instance()._addPHPClassConstant(cls, c);
}

void HighlightWords::_addPHPClassConstant(QString cls, QString c)
{
    if (cls.indexOf("\\") >= 0) cls = cls.mid(cls.lastIndexOf("\\")+1);
    QString k = cls.toLower() + "::" + c;
    phpClassWordsCS[k.toStdString()] = constFormat;
}

void HighlightWords::addJSFunction(QString k)
{
    instance()._addJSFunction(k);
}

void HighlightWords::_addJSFunction(QString k)
{
    jswordsCS[k.toStdString()] = knownFunctionFormat;
}

void HighlightWords::addJSInterface(QString k)
{
    instance()._addJSInterface(k);
}

void HighlightWords::_addJSInterface(QString k)
{
    jswordsCS[k.toStdString()] = classFormat;
}

void HighlightWords::addJSObject(QString k)
{
    instance()._addJSObject(k);
}

void HighlightWords::_addJSObject(QString k)
{
    jswordsCS[k.toStdString()] = classFormat;
}

void HighlightWords::addJSExtDartObject(QString k)
{
    instance()._addJSExtDartObject(k);
}

void HighlightWords::_addJSExtDartObject(QString k)
{
    jsExtDartWordsCS[k.toStdString()] = classFormat;
}

void HighlightWords::addJSExtDartFunction(QString k)
{
    instance()._addJSExtDartFunction(k);
}

void HighlightWords::_addJSExtDartFunction(QString k)
{
    jsExtDartWordsCS[k.toStdString()] = knownFunctionFormat;
}

void HighlightWords::addCSSProperty(QString k)
{
    instance()._addCSSProperty(k);
}

void HighlightWords::_addCSSProperty(QString k)
{
    csswords[k.toStdString()] = knownFormat;
}

void HighlightWords::addHTMLTag(QString k)
{
    instance()._addHTMLTag(k);
}

void HighlightWords::_addHTMLTag(QString k)
{
    htmlwords[k.toStdString()] = knownFormat;
}

void HighlightWords::addHTMLShortTag(QString k)
{
    instance()._addHTMLShortTag(k);
}

void HighlightWords::_addHTMLShortTag(QString k)
{
    htmlshorts[k.toStdString()] = knownFormat;
}
