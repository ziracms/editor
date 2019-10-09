/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef PARSEJS_H
#define PARSEJS_H

#include "parse.h"
#include <QVector>
#include <unordered_map>

class ParseJS : public Parse
{
public:
    ParseJS();

    struct ParseResultVariable {
        QString name;
        QString clsName;
        QString funcName;
        QString type;
        int line;
    };
    struct ParseResultFunction {
        QString name;
        QString args;
        QString clsName;
        bool isGlobal;
        int minArgs;
        int maxArgs;
        QString returnType;
        QVector<int> variableIndexes;
        QVector<int> constantIndexes;
        int line;
        QString comment;
    };
    struct ParseResultConstant {
        QString name;
        QString clsName;
        QString funcName;
        QString value;
        int line;
    };
    struct ParseResultClass {
        QString name;
        QVector<int> functionIndexes;
        QVector<int> variableIndexes;
        int line;
    };
    struct ParseResultComment {
        QString name;
        QString text;
        int line;
    };
    struct ParseResult
    {
        QVector<ParseResultClass> classes;
        QVector<ParseResultFunction> functions;
        QVector<ParseResultVariable> variables;
        QVector<ParseResultConstant> constants;
        QVector<ParseResultComment> comments;
    };

    ParseJS::ParseResult parse(QString text);
protected:
    void reset();
    QString cleanUp(QString text);
    bool isValidName(QString name);
    void parseCode(QString & code, QString & origText);
    void addClass(QString name, int line);
    void addFunction(QString clsName, QString name, QString args, int minArgs, int maxArgs, bool isGlobal, QString returnType, QString comment, int line);
    void updateFunctionReturnType(QString clsName, QString funcName, QString returnType);
    void addVariable(QString clsName, QString funcName, QString name, QString type, int line);
    void updateVariableType(QString clsName, QString funcName, QString varName, QString type);
    void addConstant(QString clsName, QString funcName, QString name, QString value, int line);
    void addComment(QString text, int line);

    QRegularExpression commentSLExpression;
    QRegularExpression regexpExpression;
    QRegularExpression parseExpression;
    QRegularExpression nameExpression;
private:
    ParseJS::ParseResult result;

    std::unordered_map<std::string, int> functionIndexes;
    std::unordered_map<std::string, int>::iterator functionIndexesIterator;
    std::unordered_map<std::string, int> variableIndexes;
    std::unordered_map<std::string, int>::iterator variableIndexesIterator;
    std::unordered_map<std::string, int> constantIndexes;
    std::unordered_map<std::string, int>::iterator constantIndexesIterator;
    std::unordered_map<std::string, int> classIndexes;
    std::unordered_map<std::string, int>::iterator classIndexesIterator;

    std::unordered_map<int, std::string> comments;
    std::unordered_map<int, std::string>::iterator commentsIterator;
};

#endif // PARSEJS_H
