/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef PARSEPHP_H
#define PARSEPHP_H

#include "parse.h"
#include <unordered_map>
#include <QVector>

extern const QString IMPORT_TYPE_CLASS;
extern const QString IMPORT_TYPE_FUNCTION;
extern const QString IMPORT_TYPE_CONSTANT;

class ParsePHP : public Parse
{
public:
    ParsePHP();

    struct ParseResultVariable {
        QString name;
        QString clsName;
        QString funcName;
        bool isStatic;
        QString visibility;
        QString type;
        int line;
    };
    struct ParseResultFunction {
        QString name;
        QString args;
        QString clsName;
        bool isStatic;
        bool isAbstract;
        QString visibility;
        int minArgs;
        int maxArgs;
        QString returnType;
        QVector<int> variableIndexes;
        int line;
        QString comment;
    };
    struct ParseResultConstant {
        QString name;
        QString clsName;
        QString value;
        int line;
    };
    struct ParseResultClass {
        QString name;
        bool isAbstract;
        bool isInterface;
        bool isTrait;
        QString parent;
        QStringList interfaces;
        QVector<int> functionIndexes;
        QVector<int> variableIndexes;
        QVector<int> constantIndexes;
        int line;
    };
    struct ParseResultNamespace {
        QString name;
        QVector<int> importsIndexes;
        int line;
    };
    struct ParseResultImport {
        QString name;
        QString path;
        QString type;
        int line;
    };
    struct ParseResultComment {
        QString name;
        QString text;
        int line;
    };
    struct ParseResultError {
        QString text;
        int line;
    };
    struct ParseResult
    {
        QVector<ParseResultImport> imports;
        QVector<ParseResultNamespace> namespaces;
        QVector<ParseResultClass> classes;
        QVector<ParseResultFunction> functions;
        QVector<ParseResultVariable> variables;
        QVector<ParseResultConstant> constants;
        QVector<ParseResultComment> comments;
        QVector<ParseResultError> errors;
    };

    ParsePHP::ParseResult parse(QString text);
    static std::unordered_map<std::string, std::string> dataTypes;
protected:
    void reset();
    QString cleanUp(QString text);
    bool isValidName(QString name);
    void parseCode(QString & code, QString & origText, int textOffset);
    QString toAbs(QString ns, QString path, QString type = "");
    void addImport(QString nsName, QString name, QString path, QString type, int line);
    void addNamespace(QString name, int line);
    void addClass(QString name, bool isAbstract, QString extend, QStringList implements, int line);
    void addInterface(QString name, QString extend, int line);
    void addTrait(QString name, int line);
    void addFunction(QString clsName, QString name, QString args, bool isStatic, bool isAbstract, QString visibility, int minArgs, int maxArgs, QString returnType, QString comment, int line);
    void updateFunctionReturnType(QString clsName, QString funcName, QString returnType);
    void addVariable(QString clsName, QString funcName, QString name, bool isStatic, QString visibility, QString type, int line);
    void updateVariableType(QString clsName, QString funcName, QString varName, QString type);
    void addConstant(QString clsName, QString name, QString value, int line);
    void addComment(QString text, int line);
    void addError(QString text, int line);

    QRegularExpression phpExpression;
    QRegularExpression phpStartExpression;
    QRegularExpression phpEndExpression;
    QRegularExpression stringHeredocExpression;
    QRegularExpression stringHeredocDQExpression;
    QRegularExpression stringNowdocExpression;
    QRegularExpression commentSLExpression;
    QRegularExpression commentSLAExpression;
    QRegularExpression parseExpression;
    QRegularExpression nameExpression;
private:
    std::unordered_map<std::string, std::string> classAliases;
    std::unordered_map<std::string, std::string>::iterator classAliasesIterator;
    std::unordered_map<std::string, std::string> functionAliases;
    std::unordered_map<std::string, std::string>::iterator functionAliasesIterator;
    std::unordered_map<std::string, std::string> constantAliases;
    std::unordered_map<std::string, std::string>::iterator constantAliasesIterator;

    std::unordered_map<std::string, int> functionIndexes;
    std::unordered_map<std::string, int>::iterator functionIndexesIterator;
    std::unordered_map<std::string, int> variableIndexes;
    std::unordered_map<std::string, int>::iterator variableIndexesIterator;
    std::unordered_map<std::string, int> constantIndexes;
    std::unordered_map<std::string, int>::iterator constantIndexesIterator;
    std::unordered_map<std::string, int> namespaceIndexes;
    std::unordered_map<std::string, int>::iterator namespaceIndexesIterator;
    std::unordered_map<std::string, int> importIndexes;
    std::unordered_map<std::string, int>::iterator importIndexesIterator;
    std::unordered_map<std::string, int> classIndexes;
    std::unordered_map<std::string, int>::iterator classIndexesIterator;

    std::unordered_map<int, std::string> comments;
    std::unordered_map<int, std::string>::iterator commentsIterator;

    std::unordered_map<std::string, std::string>::iterator dataTypesIterator;

    ParsePHP::ParseResult result;
};

#endif // PARSEPHP_H
