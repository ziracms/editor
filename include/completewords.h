/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef COMPLETEWORDS_H
#define COMPLETEWORDS_H

#include <unordered_map>
#include <map>
#include "highlightwords.h"

class CompleteWords : public QObject
{
    Q_OBJECT
public:
    static CompleteWords& instance();
    static void loadDelayed();
    static void reload();
    static void reset();
    std::unordered_map<std::string, std::string> tooltipsPHP;
    std::unordered_map<std::string, std::string>::iterator tooltipsIteratorPHP;
    std::unordered_map<std::string, std::string> htmlTags;
    std::unordered_map<std::string, std::string>::iterator htmlTagsIterator;

    std::unordered_map<std::string, std::string> phpFunctionTypes;
    std::unordered_map<std::string, std::string>::iterator phpFunctionTypesIterator;
    std::unordered_map<std::string, std::string> phpClassMethodTypes;
    std::unordered_map<std::string, std::string>::iterator phpClassMethodTypesIterator;
    std::unordered_map<std::string, std::string> phpClassParents;
    std::unordered_map<std::string, std::string>::iterator phpClassParentsIterator;

    std::map<std::string, std::string> htmlAllTagsComplete;
    std::map<std::string, std::string> cssPropertiesComplete;
    std::map<std::string, std::string> cssPseudoComplete;
    std::map<std::string, std::string> cssValuesComplete;
    std::map<std::string, std::string> jsObjectsComplete;
    std::map<std::string, std::string> jsSpecialsComplete;
    std::map<std::string, std::string> jsFunctionsComplete;
    std::map<std::string, std::string> jsInterfacesComplete;
    std::map<std::string, std::string> jsMethodsComplete;
    std::map<std::string, std::string> jsEventsComplete;
    std::map<std::string, std::string> phpFunctionsComplete;
    std::map<std::string, std::string> phpConstsComplete;
    std::map<std::string, std::string> phpClassesComplete;
    std::map<std::string, std::string> phpClassConstsComplete;
    std::map<std::string, std::string> phpClassPropsComplete;
    std::map<std::string, std::string> phpClassMethodsComplete;
    std::map<std::string, std::string> phpGlobalsComplete;
    std::map<std::string, std::string> phpSpecialsComplete;
    std::map<std::string, std::string> phpMagicComplete;
    std::map<std::string, std::string> dartObjectsComplete;
    std::map<std::string, std::string> flutterObjectsComplete;
    std::map<std::string, std::string> dartFunctionsComplete;
    std::map<std::string, std::string> flutterFunctionsComplete;
protected:
    void loadCSSWords();
    void loadHTMLWords();
    void loadJSWords();
    void loadPHPWords();
    void loadFlutterWords();
    void _loadDelayed();
    void _reload();
    void _reset();
    void _load();
private:
    CompleteWords();
public slots:
    static void load();
};

#endif // COMPLETEWORDS_H
