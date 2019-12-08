/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef PARSECSS_H
#define PARSECSS_H

#include "parse.h"
#include <QVector>
#include <unordered_map>

class ParseCSS : public Parse
{
public:
    ParseCSS();

    struct ParseResultSelector {
        QString name;
        int line;
    };
    struct ParseResultName {
        QString name;
        int line;
    };
    struct ParseResultMedia {
        QString name;
        int line;
    };
    struct ParseResultKeyframe {
        QString name;
        int line;
    };
    struct ParseResultFont {
        QString name;
        int line;
    };
    struct ParseResultComment {
        QString name;
        QString text;
        int line;
    };
    struct ParseResult
    {
        QVector<ParseResultSelector> selectors;
        QVector<ParseResultName> names;
        QVector<ParseResultMedia> medias;
        QVector<ParseResultKeyframe> keyframes;
        QVector<ParseResultFont> fonts;
        QVector<ParseResultComment> comments;
    };

    ParseCSS::ParseResult parse(QString text);

    static std::unordered_map<std::string, std::string> mainTags;
protected:
    void reset();
    QString cleanUp(QString text);
    bool isValidName(QString name);
    bool isColor(QString name);
    void addSelector(QString name, int line);
    void addName(QString name, int line);
    void addMedia(QString name, int line);
    void addKeyframe(QString name, int line);
    void addFont(QString name, int line);
    void addComment(QString text, int line);
    void parseCode(QString & code, QString & origText);
private:
    ParseCSS::ParseResult result;

    QRegularExpression parseExpression;
    QRegularExpression nameExpression;
    QRegularExpression colorExpression;

    std::unordered_map<std::string, std::string>::iterator mainTagsIterator;

    std::unordered_map<std::string, int> selectorIndexes;
    std::unordered_map<std::string, int>::iterator selectorIndexesIterator;
    std::unordered_map<std::string, int> nameIndexes;
    std::unordered_map<std::string, int>::iterator nameIndexesIterator;
    std::unordered_map<std::string, int> mediaIndexes;
    std::unordered_map<std::string, int>::iterator mediaIndexesIterator;
    std::unordered_map<std::string, int> keyframeIndexes;
    std::unordered_map<std::string, int>::iterator keyframeIndexesIterator;
    std::unordered_map<std::string, int> fontIndexes;
    std::unordered_map<std::string, int>::iterator fontIndexesIterator;

    std::unordered_map<int, std::string> comments;
    std::unordered_map<int, std::string>::iterator commentsIterator;
};

#endif // PARSECSS_H
