/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "parsephp.h"
#include <QVector>
#include <QFile>
#include <QTextStream>
#include "helper.h"

const int EXPECT_NAMESPACE = 0;
const int EXPECT_CLASS = 1;
const int EXPECT_CLASS_EXTENDED = 2;
const int EXPECT_CLASS_IMPLEMENTED = 3;
const int EXPECT_CLASS_EXTENDED_IMPLEMENTED = 4;
const int EXPECT_INTERFACE = 5;
const int EXPECT_INTERFACE_EXTENDED = 6;
const int EXPECT_TRAIT = 7;
const int EXPECT_USE = 8;
const int EXPECT_USE_FUNCTION = 9;
const int EXPECT_USE_CONSTANT = 10;
const int EXPECT_FUNCTION = 11;
const int EXPECT_FUNCTION_RETURN_TYPE = 12;
const int EXPECT_VARIABLE = 13;
const int EXPECT_CONST = 14;
const int EXPECT_CONST_VALUE = 15;
const int EXPECT_COMMENT = 16;

const QString IMPORT_TYPE_CLASS = "class";
const QString IMPORT_TYPE_FUNCTION = "function";
const QString IMPORT_TYPE_CONSTANT = "constant";

std::unordered_map<std::string, std::string> ParsePHP::dataTypes = {};

ParsePHP::ParsePHP()
{
    phpExpression = QRegularExpression("[<][?](?:php)?[\\s](.+?)([?][>]|$)", QRegularExpression::DotMatchesEverythingOption);
    phpStartExpression = QRegularExpression("[<][?](php)?[\\s]", QRegularExpression::DotMatchesEverythingOption);
    phpEndExpression = QRegularExpression("[?][>]", QRegularExpression::DotMatchesEverythingOption);
    stringHeredocExpression = QRegularExpression("[<][<][<]([_a-zA-Z][_a-zA-Z0-9]*)[\n](.+?)[\n](\\1)[;]([\n]|$)", QRegularExpression::DotMatchesEverythingOption);
    stringHeredocDQExpression = QRegularExpression("[<][<][<][\"]([_a-zA-Z][_a-zA-Z0-9]*)[\"][\n](.+?)[\n](\\1)[;]([\n]|$)", QRegularExpression::DotMatchesEverythingOption);
    stringNowdocExpression = QRegularExpression("[<][<][<][']([_a-zA-Z][_a-zA-Z0-9]*)['][\n](.+?)[\n](\\1)[;]([\n]|$)", QRegularExpression::DotMatchesEverythingOption);
    commentSLExpression = QRegularExpression("[/][/]([^\n]+?)([?][>]|[\n]|$)", QRegularExpression::DotMatchesEverythingOption);
    commentSLAExpression = QRegularExpression("[#]([^\n]+?)([?][>]|[\n]|$)", QRegularExpression::DotMatchesEverythingOption);
    parseExpression = QRegularExpression("([a-zA-Z0-9_\\$\\\\]+|[\\(\\)\\{\\}\\[\\],=;:!@#%^&*\\-+/\\|<>\\?])", QRegularExpression::DotMatchesEverythingOption);
    nameExpression = QRegularExpression("^[\\$\\\\]?[a-zA-Z_][a-zA-Z0-9_\\\\]*$");

    if (dataTypes.size() == 0) {
        QFile tf(":/highlight/php_types");
        tf.open(QIODevice::ReadOnly);
        QTextStream tin(&tf);
        QString k;
        while (!tin.atEnd()) {
            k = tin.readLine();
            if (k == "") continue;
            dataTypes[k.toStdString()] = k.toStdString();
        }
        tf.close();
    }
}

QString ParsePHP::cleanUp(QString text)
{
    comments.clear();
    prepare(text);
    // strip strings & comments
    bool phpOpened = false;
    int offset = 0;
    QRegularExpressionMatch phpEndMatch;
    QRegularExpressionMatch stringDQMatch;
    QRegularExpressionMatch stringSQMatch;
    QRegularExpressionMatch stringHeredocMatch;
    QRegularExpressionMatch stringHeredocDQMatch;
    QRegularExpressionMatch stringNowdocMatch;
    QRegularExpressionMatch commentMLMatch;
    QRegularExpressionMatch commentSLMatch;
    QRegularExpressionMatch commentSLAMatch;
    QRegularExpressionMatch backtickMatch;
    QList<int> matchesPos;
    int phpEndPos = -2,
        stringDQPos = -2,
        stringSQPos = -2,
        stringHeredocPos = -2,
        stringHeredocDQPos = -2,
        stringNowdocPos = -2,
        commentMLPos = -2,
        commentSLPos = -2,
        commentSLAPos = -2,
        backtickPos = -2;
    do {
        if (!phpOpened) {
            QRegularExpressionMatch match = phpStartExpression.match(text, offset);
            if (match.capturedStart() < 0) break;
            offset = match.capturedStart() + match.capturedLength();
            phpOpened = true;
        }
        matchesPos.clear();
        if (phpEndPos != -1 && phpEndPos < offset) {
            phpEndMatch = phpEndExpression.match(text, offset);
            phpEndPos = phpEndMatch.capturedStart();
        }
        if (phpEndPos >= 0) matchesPos.append(phpEndPos);
        if (stringDQPos != -1 && stringDQPos < offset) {
            stringDQMatch = stringDQExpression.match(text, offset);
            stringDQPos = stringDQMatch.capturedStart();
        }
        if (stringDQPos >= 0) matchesPos.append(stringDQPos);
        if (stringSQPos != -1 && stringSQPos < offset) {
            stringSQMatch = stringSQExpression.match(text, offset);
            stringSQPos = stringSQMatch.capturedStart();
        }
        if (stringSQPos >= 0) matchesPos.append(stringSQPos);
        if (stringHeredocPos != -1 && stringHeredocPos < offset) {
            stringHeredocMatch = stringHeredocExpression.match(text, offset);
            stringHeredocPos = stringHeredocMatch.capturedStart();
        }
        if (stringHeredocPos >= 0) matchesPos.append(stringHeredocPos);
        if (stringHeredocDQPos != -1 && stringHeredocDQPos < offset) {
            stringHeredocDQMatch = stringHeredocDQExpression.match(text, offset);
            stringHeredocDQPos = stringHeredocDQMatch.capturedStart();
        }
        if (stringHeredocDQPos >= 0) matchesPos.append(stringHeredocDQPos);
        if (stringNowdocPos != -1 && stringNowdocPos < offset) {
            stringNowdocMatch = stringNowdocExpression.match(text, offset);
            stringNowdocPos = stringNowdocMatch.capturedStart();
        }
        if (stringNowdocPos >= 0) matchesPos.append(stringNowdocPos);
        if (commentMLPos != -1 && commentMLPos < offset) {
            commentMLMatch = commentMLExpression.match(text, offset);
            commentMLPos = commentMLMatch.capturedStart();
        }
        if (commentMLPos >= 0) matchesPos.append(commentMLPos);
        if (commentSLPos != -1 && commentSLPos < offset) {
            commentSLMatch = commentSLExpression.match(text, offset);
            commentSLPos = commentSLMatch.capturedStart();
        }
        if (commentSLPos >= 0) matchesPos.append(commentSLPos);
        if (commentSLAPos != -1 && commentSLAPos < offset) {
            commentSLAMatch = commentSLAExpression.match(text, offset);
            commentSLAPos = commentSLAMatch.capturedStart();
        }
        if (commentSLAPos >= 0) matchesPos.append(commentSLAPos);
        if (backtickPos != -1 && backtickPos < offset) {
            backtickMatch = backtickExpression.match(text, offset);
            backtickPos = backtickMatch.capturedStart();
        }
        if (backtickPos >= 0) matchesPos.append(backtickPos);
        if (matchesPos.size() == 0) break;
        std::sort(matchesPos.begin(), matchesPos.end());
        int pos = matchesPos.at(0);
        if (phpEndPos == pos) {
            offset = phpEndMatch.capturedStart() + phpEndMatch.capturedLength();
            phpOpened = false;
            continue;
        }
        if (stringDQPos == pos) {
            offset = stringDQMatch.capturedStart() + stringDQMatch.capturedLength();
            strip(stringDQMatch, text, 1);
            continue;
        }
        if (stringSQPos == pos) {
            offset = stringSQMatch.capturedStart() + stringSQMatch.capturedLength();
            strip(stringSQMatch, text, 1);
            continue;
        }
        if (stringHeredocPos == pos) {
            offset = stringHeredocMatch.capturedStart() + stringHeredocMatch.capturedLength();
            strip(stringHeredocMatch, text, 2);
            continue;
        }
        if (stringHeredocDQPos == pos) {
            offset = stringHeredocDQMatch.capturedStart() + stringHeredocDQMatch.capturedLength();
            strip(stringHeredocDQMatch, text, 2);
            continue;
        }
        if (stringNowdocPos == pos) {
            offset = stringNowdocMatch.capturedStart() + stringNowdocMatch.capturedLength();
            strip(stringNowdocMatch, text, 2);
            continue;
        }
        if (commentMLPos == pos) {
            offset = commentMLMatch.capturedStart() + commentMLMatch.capturedLength();
            QString stripped = strip(commentMLMatch, text, 0); // group 0
            comments[getLine(text, offset)] = stripped.toStdString();
            continue;
        }
        if (commentSLPos == pos) {
            offset = commentSLMatch.capturedStart(1) + commentSLMatch.capturedLength(1);
            QString stripped = strip(commentSLMatch, text, 0); // group 0
            comments[getLine(text, offset)] = stripped.toStdString();
            continue;
        }
        if (commentSLAPos == pos) {
            offset = commentSLAMatch.capturedStart(1) + commentSLAMatch.capturedLength(1);
            QString stripped = strip(commentSLAMatch, text, 0); // group 0
            comments[getLine(text, offset)] = stripped.toStdString();
            continue;
        }
        if (backtickPos == pos) {
            offset = backtickMatch.capturedStart() + backtickMatch.capturedLength();
            strip(backtickMatch, text, 1);
            continue;
        }
    } while (matchesPos.size() > 0);
    return text;
}

bool ParsePHP::isValidName(QString name)
{
    QRegularExpressionMatch m = nameExpression.match(name);
    return (m.capturedStart()==0);
}

QString ParsePHP::toAbs(QString ns, QString path, QString type)
{
    if (type.size() == 0) type = IMPORT_TYPE_CLASS;
    QString name = path;
    int p = path.indexOf("\\");
    if (p > 0) {
        if (path.indexOf("namespace\\")==0) {
            if (ns.size() > 0 && ns[0] != "\\") ns = "\\" + ns;
            path = ns + path.mid(p);
            p = 0;
        } else {
            name = path.mid(0, p);
        }
    }
    if (p != 0) {
        std::unordered_map<std::string, std::string> aliases;
        std::unordered_map<std::string, std::string>::iterator aliasesIterator;
        if (type == IMPORT_TYPE_FUNCTION) {
            aliases = functionAliases;
            aliasesIterator = functionAliasesIterator;
        } else if (type == IMPORT_TYPE_CONSTANT) {
            aliases = constantAliases;
            aliasesIterator = constantAliasesIterator;
        } else  {
            aliases = classAliases;
            aliasesIterator = classAliasesIterator;
        }
        aliasesIterator = aliases.find(name.toStdString());
        if (aliasesIterator != aliases.end()) {
            QString use_path = QString::fromStdString(aliasesIterator->second);
            if (use_path.size() > 0 && use_path[0] != "\\") use_path = "\\" + use_path;
            if (p > 0) {
                return use_path + path.mid(p);
            } else {
                return use_path;
            }
        } else {
            if (ns.size() > 0 && ns[0] != "\\") ns = "\\" + ns;
            return ns + "\\" + path;
        }
    }
    return path;
}

void ParsePHP::addImport(QString nsName, QString name, QString path, QString type, int line) {
    if (!isValidName(name) || !isValidName(path)) return;
    ParseResultImport imp;
    imp.name = name;
    imp.path = path;
    imp.type = type;
    imp.line = line;
    result.imports.append(imp);
    importIndexes[name.toStdString()] = result.imports.size() - 1;
    if (nsName.size() > 0) {
        namespaceIndexesIterator = namespaceIndexes.find(nsName.toStdString());
        if (namespaceIndexesIterator != namespaceIndexes.end()) {
            int i = namespaceIndexesIterator->second;
            if (result.namespaces.size() > i) {
                ParsePHP::ParseResultNamespace ns = result.namespaces.at(i);
                ns.importsIndexes.append(result.imports.size() - 1);
                result.namespaces.replace(i, ns);
            }
        }
    }
}

void ParsePHP::addNamespace(QString name, int line) {
    if (!isValidName(name)) return;
    ParseResultNamespace ns;
    ns.name = name;
    ns.line = line;
    result.namespaces.append(ns);
    namespaceIndexes[name.toStdString()] = result.namespaces.size() - 1;
}

void ParsePHP::addClass(QString name, bool isAbstract, QString extend, QStringList implements, int line) {
    if (!isValidName(name)) return;
    ParseResultClass cls;
    cls.name = name;
    cls.isAbstract = isAbstract;
    cls.isInterface = false;
    cls.isTrait = false;
    cls.parent = extend;
    cls.interfaces = implements;
    cls.line = line;
    result.classes.append(cls);
    classIndexes[name.toStdString()] = result.classes.size() - 1;
}

void ParsePHP::addInterface(QString name, QString extend, int line) {
    if (!isValidName(name)) return;
    ParseResultClass cls;
    cls.name = name;
    cls.isAbstract = false;
    cls.isInterface = true;
    cls.isTrait = false;
    cls.parent = extend;
    cls.interfaces.clear();
    cls.line = line;
    result.classes.append(cls);
    classIndexes[name.toStdString()] = result.classes.size() - 1;
}

void ParsePHP::addTrait(QString name, int line) {
    if (!isValidName(name)) return;
    ParseResultClass cls;
    cls.name = name;
    cls.isAbstract = false;
    cls.isInterface = false;
    cls.isTrait = true;
    cls.parent = "";
    cls.interfaces.clear();
    cls.line = line;
    result.classes.append(cls);
    classIndexes[name.toStdString()] = result.classes.size() - 1;
}

void ParsePHP::addFunction(QString clsName, QString name, QString args, bool isStatic, bool isAbstract, QString visibility, int minArgs, int maxArgs, QString returnType, QString comment, int line) {
    if (!isValidName(name)) return;
    ParseResultFunction func;
    func.name = name;
    func.clsName = clsName;
    func.args = args;
    func.isStatic = isStatic;
    func.isAbstract = isAbstract;
    func.visibility = visibility;
    func.minArgs = minArgs;
    func.maxArgs = maxArgs;
    func.returnType = returnType;
    func.comment = comment;
    func.line = line;
    result.functions.append(func);
    functionIndexes[clsName.toStdString() + "::" + name.toStdString()] = result.functions.size() - 1;
    if (clsName.size() > 0) {
        classIndexesIterator = classIndexes.find(clsName.toStdString());
        if (classIndexesIterator != classIndexes.end()) {
            int i = classIndexesIterator->second;
            if (result.classes.size() > i) {
                ParsePHP::ParseResultClass cls = result.classes.at(i);
                cls.functionIndexes.append(result.functions.size() - 1);
                result.classes.replace(i, cls);
            }
        }
    }
}

void ParsePHP::updateFunctionReturnType(QString clsName, QString funcName, QString returnType)
{
    functionIndexesIterator = functionIndexes.find(clsName.toStdString() + "::" + funcName.toStdString());
    if (functionIndexesIterator != functionIndexes.end()) {
        int i = functionIndexesIterator->second;
        if (result.functions.size() > i) {
            ParseResultFunction func = result.functions.at(i);
            func.returnType = returnType;
            result.functions.replace(i, func);
        }
    }
}

void ParsePHP::addVariable(QString clsName, QString funcName, QString name, bool isStatic, QString visibility, QString type, int line)
{
    if (!isValidName(name)) return;
    variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + funcName.toStdString() + "::" + name.toStdString());
    if (variableIndexesIterator != variableIndexes.end()) return;
    ParseResultVariable variable;
    variable.name = name;
    variable.clsName = clsName;
    variable.funcName = funcName;
    variable.isStatic = isStatic;
    variable.visibility =visibility;
    variable.type = type;
    variable.line = line;
    result.variables.append(variable);
    variableIndexes[clsName.toStdString() + "::" + funcName.toStdString() + "::" + name.toStdString()] = result.variables.size() - 1;
    if (clsName.size() > 0 && funcName.size() == 0) {
        classIndexesIterator = classIndexes.find(clsName.toStdString());
        if (classIndexesIterator != classIndexes.end()) {
            int i = classIndexesIterator->second;
            if (result.classes.size() > i) {
                ParsePHP::ParseResultClass cls = result.classes.at(i);
                cls.variableIndexes.append(result.variables.size() - 1);
                result.classes.replace(i, cls);
            }
        }
    } else if (funcName.size() > 0) {
        functionIndexesIterator = functionIndexes.find(clsName.toStdString() + "::" + funcName.toStdString());
        if (functionIndexesIterator != functionIndexes.end()) {
            int i = functionIndexesIterator->second;
            if (result.functions.size() > i) {
                ParsePHP::ParseResultFunction func = result.functions.at(i);
                func.variableIndexes.append(result.variables.size() - 1);
                result.functions.replace(i, func);
            }
        }
    }
}

void ParsePHP::updateVariableType(QString clsName, QString funcName, QString varName, QString type)
{
    variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + funcName.toStdString() + "::" + varName.toStdString());
    if (variableIndexesIterator != variableIndexes.end()) {
        int i = variableIndexesIterator->second;
        if (result.variables.size() > i) {
            ParseResultVariable variable = result.variables.at(i);
            variable.type = type;
            result.variables.replace(i, variable);
        }
    }
}

void ParsePHP::addConstant(QString clsName, QString name, QString value, int line)
{
    if (!isValidName(name)) return;
    ParseResultConstant constant;
    constant.name = name;
    constant.clsName = clsName;
    constant.value = value;
    constant.line = line;
    result.constants.append(constant);
    constantIndexes[clsName.toStdString() + "::" + name.toStdString()] = result.constants.size() - 1;
    if (clsName.size() > 0) {
        classIndexesIterator = classIndexes.find(clsName.toStdString());
        if (classIndexesIterator != classIndexes.end()) {
            int i = classIndexesIterator->second;
            if (result.classes.size() > i) {
                ParsePHP::ParseResultClass cls = result.classes.at(i);
                cls.constantIndexes.append(result.constants.size() - 1);
                result.classes.replace(i, cls);
            }
        }
    }
}

void ParsePHP::addComment(QString text, int line) {
    QString name = "";
    if (text.size() > 0) {
        if (text.indexOf("//")==0) text = text.mid(2);
        else if (text.indexOf("#")==0) text = text.mid(1);
        else text.replace(QRegularExpression("^[/][*](.*)[*][/]$", QRegularExpression::DotMatchesEverythingOption), "\\1");
        QString text_clean = "";
        QStringList textList = text.split("\n");
        for (int i=0; i<textList.size(); i++) {
            QString text_line = textList.at(i).trimmed().replace(QRegularExpression("^[*]+[\\s]*"), "").replace(QRegularExpression("[\\s]*[*]+$"), "");
            if (text_line.size() == 0) continue;
            if (text_clean.size() > 0) text_clean += "\n";
            if (name.size() == 0) name = text_line;
            text_clean += text_line;
        }
        text = text_clean;
    }
    if (text.size() == 0 || name.size() == 0) return;
    ParseResultComment comment;
    comment.name = name;
    comment.text = text;
    comment.line = line;
    result.comments.append(comment);
}

void ParsePHP::parseCode(QString & code, QString & origText, int textOffset)
{
    // parse data
    QString current_namespace = "";
    QString current_class = "";
    bool current_class_is_abstract = false;
    QString current_class_parent = "";
    QStringList current_class_interfaces;
    QString current_interface = "";
    QString current_interface_parent = "";
    QString current_trait = "";
    QString current_function = "";
    QString current_function_args = "";
    bool current_function_is_static = false;
    bool current_function_is_abstract = false;
    QString current_function_visibility = "";
    int current_function_min_args = 0;
    int current_function_max_args = 0;
    QString current_function_return_type = "";
    QString current_variable = "";
    bool current_variable_is_static = false;
    QString current_variable_visibility = "";
    QString current_variable_type = "";
    QString current_constant = "";
    QString current_constant_value = "";

    QStringList parent_namespaces;
    QVector<int> parent_namespace_scopes;
    QString expected_function_name = "";
    QStringList expected_function_args;
    QStringList expected_function_arg_types;
    QString expected_class_name = "";
    int scope = 0;
    int namespaceScope = -1, classScope = -1, interfaceScope = -1, traitScope = -1, functionScope = -1, anonymFunctionScope = -1, anonymClassScope = -1;
    int pars = 0;
    int functionArgPars = -1;
    int functionArgsStart = -1;
    int constantValueStart = -1;
    bool functionParsFound = false, classParsFound = false;
    int expect = -1;
    QString expectName = "";
    QString prevK = "", prevPrevK = "", prevPrevPrevK = "", prevPrevPrevPrevK = "", prevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevPrevK = "";
    int useStart = -1, namespaceStart = -1, classStart = -1, interfaceStart = -1, traitStart = -1, functionStart = -1, variableStart = -1, constantStart = -1;
    QString class_static_variable = "", class_variable = "";
    QStringList functionChainList, classChainList, traitChainList;

    QRegularExpressionMatchIterator mi = parseExpression.globalMatch(code);
    while(mi.hasNext()){
        QRegularExpressionMatch m = mi.next();
        if (m.capturedStart(1) < 0) continue;
        QString k = m.captured(1).trimmed();
        if (k.size() == 0) continue;

        // uses
        if (expect < 0 && (((namespaceScope < 0 && scope == 0) || scope == namespaceScope+1) && (prevK == ";" || prevK == "{" || prevK == "}" || prevK.size() == 0)) && k.toLower() == "use" && current_class.size() == 0 && current_interface.size() == 0 && current_trait.size() == 0 && current_function.size() == 0 && anonymFunctionScope < 0) {
            expect = EXPECT_USE;
            expectName = "";
            useStart = m.capturedStart(1);
        } else if (expect == EXPECT_USE && expectName.size() == 0 && k.toLower() == "function") {
            expect = EXPECT_USE_FUNCTION;
        } else if (expect == EXPECT_USE && expectName.size() == 0 && k.toLower() == "const") {
            expect = EXPECT_USE_CONSTANT;
        } else if ((expect == EXPECT_USE || expect == EXPECT_USE_FUNCTION || expect == EXPECT_USE_CONSTANT) && expectName.size() == 0) {
            expectName = k;
        } else if ((expect == EXPECT_USE || expect == EXPECT_USE_FUNCTION || expect == EXPECT_USE_CONSTANT) && expectName.size() > 0 && k.toLower() == "as") {
            expectName += " as ";
        } else if ((expect == EXPECT_USE || expect == EXPECT_USE_FUNCTION || expect == EXPECT_USE_CONSTANT) && expectName.size() > 0 && k != ";") {
            expectName += k;
        } else if ((expect == EXPECT_USE || expect == EXPECT_USE_FUNCTION || expect == EXPECT_USE_CONSTANT) && expectName.size() > 0 && k == ";") {
            QString useType = IMPORT_TYPE_CLASS;
            if (expect == EXPECT_USE_FUNCTION) useType = IMPORT_TYPE_FUNCTION;
            else if (expect == EXPECT_USE_CONSTANT) useType = IMPORT_TYPE_CONSTANT;
            if (expectName.indexOf("{") > 0 && expectName.indexOf("}") == expectName.size() - 1 && expectName.size() - expectName.indexOf("{") > 2) {
                QString expectNameUnscoped = "";
                QString usePre = expectName.mid(0, expectName.indexOf("{"));
                if (usePre.size() > 1 && usePre.mid(usePre.size()-1) == "\\") {
                    QString useScope = expectName.mid(expectName.indexOf("{")+1, expectName.size() - expectName.indexOf("{") - 2);
                    QStringList useScopeList = useScope.split(",");
                    for (int i=0; i<useScopeList.size(); i++) {
                        QString useScopePart = useScopeList.at(i);
                        if (expectNameUnscoped.size() > 0) expectNameUnscoped += ",";
                        expectNameUnscoped += usePre+useScopePart;
                    }
                }
                expectName = expectNameUnscoped;
            }
            QStringList useList = expectName.split(",");
            for (int i=0; i<useList.size(); i++) {
                QString usePath = "", useName = "";
                QString usePart = useList.at(i);
                if (usePart.indexOf("{") >= 0 || usePart.indexOf("}") >= 0) continue;
                QStringList usePartList = usePart.split(" as ");
                if (usePartList.size() == 2) {
                    usePath = usePartList.at(0);
                    useName = usePartList.at(1);
                } else if (usePartList.size() == 1) {
                    usePath = usePartList.at(0);
                    useName = usePath;
                    int p = useName.lastIndexOf("\\");
                    if (p >= 0) useName = useName.mid(p+1);
                }
                if (usePath.size() > 0 && useName.size() > 0 && isValidName(usePath) && isValidName(useName)) {
                    if (usePath[0] != "\\") usePath = "\\" + usePath;
                    if (useType == IMPORT_TYPE_CLASS) {
                        classAliases[useName.toStdString()] = usePath.toStdString();
                    } else if (useType == IMPORT_TYPE_FUNCTION) {
                        functionAliases[useName.toStdString()] = usePath.toStdString();
                    } else if (useType == IMPORT_TYPE_CONSTANT) {
                        constantAliases[useName.toStdString()] = usePath.toStdString();
                    }
                    int line = 0;
                    //if (useStart >= 0) line = getLine(origText, textOffset + useStart);
                    if (useStart >= 0) line = getLine(origText, textOffset + m.capturedStart(1)); // line at end
                    addImport(current_namespace, useName, usePath, useType, line);
                }
            }
            expect = -1;
            expectName = "";
        }

        // namespaces
        if (expect < 0 && k.toLower() == "namespace") {
            expect = EXPECT_NAMESPACE;
            expectName = "";
            namespaceStart = m.capturedStart(1);
        } else if (expect == EXPECT_NAMESPACE && expectName.size() == 0) {
            expectName = k;
        } else if (expect == EXPECT_NAMESPACE && expectName.size() > 0 && (k == ";" || k == "{")) {
            QString parent = "";
            if (namespaceScope >= 0 && current_namespace.size() > 0) {
                parent_namespaces.append(current_namespace);
                parent_namespace_scopes.append(namespaceScope);
                // namespace cannot be nested
                //if (expectName[0] != "\\") parent = current_namespace + "\\";
            }
            current_namespace = parent + expectName;
            if (current_namespace[0] == "\\") current_namespace = current_namespace.mid(1);
            int line = 0;
            //if (namespaceStart >= 0) line = getLine(origText, textOffset + namespaceStart);
            if (namespaceStart >= 0) line = getLine(origText, textOffset + m.capturedStart(1)); // line at end
            addNamespace(current_namespace, line);
            expect = -1;
            expectName = "";
            if (k == "{") namespaceScope = scope;
            if (k == ";") {
                parent_namespaces.clear();
                parent_namespace_scopes.clear();
            }
            // reset imports
            classAliases.clear();
            functionAliases.clear();
            constantAliases.clear();
        }

        // classes
        if ((((prevPrevPrevK.size() == 0 || prevPrevPrevK == ";" || prevPrevPrevK == "{" || prevPrevPrevK == "}" || prevPrevPrevK == ">" || prevPrevPrevK == ":") && prevPrevK.size() > 0 && prevK == "=") || prevK.toLower() == "return") && k == "new" && anonymClassScope < 0) {
            expected_class_name = prevPrevK;
        }
        if (expect < 0 && k.toLower() == "class" && (prevK == ";" || prevK == "{" || prevK == "}" || prevK == "=" || prevK.size() == 0 || prevK.toLower() == "new" || prevK.toLower() == "abstract" || prevK.toLower() == "final") && anonymClassScope < 0) {
            expect = EXPECT_CLASS;
            expectName = "";
            current_class_is_abstract = (prevK.toLower() == "abstract");
            current_class_parent = "";
            current_class_interfaces.clear();
            classStart =  m.capturedStart(1);
            classParsFound = false;
        } else if (expect == EXPECT_CLASS && expectName.size() == 0 && k != "{" && !classParsFound) {
            if (k != "(" && k != ")" && k != "{" && k != "extends" && k != "implements" && k.indexOf("$") < 0 && (current_class.size() == 0 && current_interface.size() == 0 && current_trait.size() == 0)) {
                expectName = k;
            } else {
                classParsFound = true;
            }
        } else if (expect == EXPECT_CLASS && expectName.size() > 0 && k.toLower() == "extends") {
            expect = EXPECT_CLASS_EXTENDED;
        } else if (expect == EXPECT_CLASS_EXTENDED && expectName.size() > 0 && current_class_parent.size() == 0 && k != "{") {
            current_class_parent = toAbs(current_namespace, k);
        } else if ((expect == EXPECT_CLASS || expect == EXPECT_CLASS_EXTENDED) && expectName.size() > 0 && k.toLower() == "implements") {
            if (expect == EXPECT_CLASS) expect = EXPECT_CLASS_IMPLEMENTED;
            if (expect == EXPECT_CLASS_EXTENDED) expect = EXPECT_CLASS_EXTENDED_IMPLEMENTED;
        } else if ((expect == EXPECT_CLASS_IMPLEMENTED || expect == EXPECT_CLASS_EXTENDED_IMPLEMENTED) && expectName.size() > 0 && k != "{" && k != ",") {
            current_class_interfaces.append(toAbs(current_namespace, k));
        } else if ((expect == EXPECT_CLASS || expect == EXPECT_CLASS_EXTENDED || expect == EXPECT_CLASS_IMPLEMENTED || expect == EXPECT_CLASS_EXTENDED_IMPLEMENTED) && (expectName.size() > 0 || expected_class_name.size() > 0) && k == "{") {
            if (expectName.size() > 0 && (current_class.size() == 0 && current_interface.size() == 0 && current_trait.size() == 0)) {
                QString ns = "\\";
                if (current_namespace.size() > 0) ns += current_namespace + "\\";
                current_class = ns + expectName;
                int line = 0;
                if (classStart >= 0) line = getLine(origText, textOffset + classStart);
                addClass(current_class, current_class_is_abstract, current_class_parent, current_class_interfaces, line);
                classScope = scope;
            } else {
                anonymClassScope = scope;
                if (current_class.size() > 0) {
                    classChainList.append(current_class);
                    current_class = "";
                } else if (current_trait.size() > 0) {
                    traitChainList.append(current_trait);
                    current_trait = "";
                }
            }
            expect = -1;
            expectName = "";
            current_interface = "";
            current_trait = "";
            expected_class_name = "";
            classParsFound = false;
        }

        // interfaces
        if (expect < 0 && k.toLower() == "interface" && (current_class.size() == 0 && current_interface.size() == 0 && current_trait.size() == 0)) {
            expect = EXPECT_INTERFACE;
            expectName = "";
            current_interface_parent = "";
            interfaceStart = m.capturedStart(1);
        } else if (expect == EXPECT_INTERFACE && expectName.size() == 0) {
            expectName = k;
        } else if (expect == EXPECT_INTERFACE && expectName.size() > 0 && k.toLower() == "extends") {
            expect = EXPECT_INTERFACE_EXTENDED;
        } else if (expect == EXPECT_INTERFACE_EXTENDED && expectName.size() > 0 && current_interface_parent.size() == 0 && k != "{") {
            current_interface_parent = toAbs(current_namespace, k);
        } else if ((expect == EXPECT_INTERFACE || expect == EXPECT_INTERFACE_EXTENDED) && expectName.size() > 0 && k == "{") {
            QString ns = "\\";
            if (current_namespace.size() > 0) ns += current_namespace + "\\";
            current_interface = ns + expectName;
            int line = 0;
            if (interfaceStart >= 0) line = getLine(origText, textOffset + interfaceStart);
            addInterface(current_interface, current_interface_parent, line);
            expect = -1;
            expectName = "";
            interfaceScope = scope;
            current_class = "";
            current_trait = "";
        }

        // traits
        if (expect < 0 && k.toLower() == "trait" && (current_class.size() == 0 && current_interface.size() == 0 && current_trait.size() == 0) && anonymClassScope < 0) {
            expect = EXPECT_TRAIT;
            expectName = "";
            traitStart = m.capturedStart(1);
        } else if (expect == EXPECT_TRAIT && expectName.size() == 0) {
            expectName = k;
        } else if (expect == EXPECT_TRAIT && expectName.size() > 0 && k == "{") {
            QString ns = "\\";
            if (current_namespace.size() > 0) ns += current_namespace + "\\";
            current_trait = ns + expectName;
            int line = 0;
            if (traitStart >= 0) line = getLine(origText, textOffset + traitStart);
            addTrait(current_trait, line);
            expect = -1;
            expectName = "";
            traitScope = scope;
            current_class = "";
            current_interface = "";
        }

        // functions
        if ((prevPrevK.size() == 0 || prevPrevK == ";" || prevPrevK == "{" || prevPrevK == "}" || prevPrevK == ">" || prevPrevK == ":") && prevK.size() > 0 && k == "=" && functionArgsStart < 0 && anonymFunctionScope < 0) {
            expected_function_name = prevK;
        }
        if (expect < 0 && k.toLower() == "function" && (prevK == ";" || prevK == "{" || prevK == "}" || prevK == "=" || prevK.size() == 0 || prevK.toLower() == "public" || prevK.toLower() == "protected" || prevK.toLower() == "private" || prevK.toLower() == "static" || prevK.toLower() == "abstract") && functionArgsStart < 0 && anonymFunctionScope < 0 && anonymClassScope < 0) {
            expect = EXPECT_FUNCTION;
            current_function_args = "";
            expected_function_args.clear();
            expected_function_arg_types.clear();
            current_function_is_static = (prevK.toLower() == "static" || (prevK.size() > 1 && prevPrevK.toLower() == "static") || (prevK.size() > 1 && prevPrevK.size() > 1 && prevPrevPrevK.toLower() == "static"));
            current_function_is_abstract = (prevK.toLower() == "abstract" || (prevK.size() > 1 && prevPrevK.toLower() == "abstract") || (prevK.size() > 1 && prevPrevK.size() > 1 && prevPrevPrevK.toLower() == "abstract"));
            current_function_visibility = "";
            if (prevK.toLower() == "public" || prevK.toLower() == "protected" || prevK.toLower() == "private") current_function_visibility = prevK.toLower();
            if (prevK.size() > 1 && (prevPrevK.toLower() == "public" || prevPrevK.toLower() == "protected" || prevPrevK.toLower() == "private")) current_function_visibility = prevPrevK.toLower();
            if (prevK.size() > 1 && prevPrevK.size() > 1 && (prevPrevPrevK.toLower() == "public" || prevPrevPrevK.toLower() == "protected" || prevPrevPrevK.toLower() == "private")) current_function_visibility = prevPrevPrevK.toLower();
            current_function_min_args = 0;
            current_function_max_args = 0;
            current_function_return_type = "";
            expectName = "";
            functionArgPars = -1;
            functionArgsStart = -1;
            functionParsFound = false;
            functionStart = m.capturedStart(1);
        } else if (expect == EXPECT_FUNCTION && expectName.size() == 0 && k != "&" && k != "(" && k != ")" && k != "{" && k != "use" && functionArgsStart < 0 && current_function_args.size() == 0 && !functionParsFound) {
            expectName = k;
        } else if (expect == EXPECT_FUNCTION && functionArgPars < 0 && k == "(" && !functionParsFound) {
            functionArgPars = pars;
            functionArgsStart = m.capturedStart(1);
            functionParsFound = true;
        } else if (expect == EXPECT_FUNCTION && expectName.size() > 0 && functionArgPars < 0 && k == ":") {
            expect = EXPECT_FUNCTION_RETURN_TYPE;
        } else if (expect == EXPECT_FUNCTION_RETURN_TYPE && expectName.size() > 0 && (current_function_return_type.size() == 0 || current_function_return_type == "?") && k != "{" && k != ";") {
            if (current_function_return_type == "?") current_function_return_type += k;
            else current_function_return_type = k;
        } else if ((expect == EXPECT_FUNCTION || expect == EXPECT_FUNCTION_RETURN_TYPE) && (expectName.size() > 0 || expected_function_name.size() > 0) && (k == "{" || (current_interface.size() > 0 && k == ";") || (current_function_is_abstract && k == ";")) && functionArgsStart < 0) {
            if (expectName.size() > 0) {
                current_function = expectName;
                QString current_function_return_type_clean = current_function_return_type;
                if (current_function_return_type_clean.size() > 0 && current_function_return_type_clean.mid(0, 1) == "?") {
                    current_function_return_type_clean = current_function_return_type_clean.mid(1);
                }
                if (current_function_return_type_clean.size() > 0) {
                    dataTypesIterator = dataTypes.find(current_function_return_type_clean.toLower().toStdString());
                    if (dataTypesIterator == dataTypes.end()) {
                        current_function_return_type_clean = toAbs(current_namespace, current_function_return_type_clean);
                        if (current_function_return_type.mid(0, 1) == "?") current_function_return_type = "?"+current_function_return_type_clean;
                        else current_function_return_type = current_function_return_type_clean;
                    }
                }
                QString clsName = "";
                if (current_class.size() > 0) clsName = current_class;
                else if (current_interface.size() > 0) clsName = current_interface;
                else if (current_trait.size() > 0) clsName = current_trait;
                if (current_function_visibility.size() == 0 && clsName.size() > 0) current_function_visibility = "public";
                int line = 0;
                if (functionStart >= 0) line = getLine(origText, textOffset + functionStart);
                QString current_comment = "";
                int comment_line = getFirstNotEmptyLineTo(origText, textOffset + functionStart);
                if (comment_line > 0) {
                    commentsIterator = comments.find(comment_line);
                    if (commentsIterator != comments.end()) {
                        current_comment = QString::fromStdString(commentsIterator->second);
                    }
                }
                if (current_comment.size() > 0) {
                    if (current_comment.indexOf("//")==0) current_comment = current_comment.mid(2);
                    else if (current_comment.indexOf("#")==0) current_comment = current_comment.mid(1);
                    else current_comment.replace(QRegularExpression("^[/][*](.*)[*][/]$", QRegularExpression::DotMatchesEverythingOption), "\\1");
                    QString current_comment_clean = "";
                    QStringList commentList = current_comment.split("\n");
                    for (int i=0; i<commentList.size(); i++) {
                        //QString current_comment_line = commentList.at(i).trimmed().replace(QRegularExpression("^[*]+[\\s]*"), "").replace(QRegularExpression("^[@]"), "- ");
                        QString current_comment_line = commentList.at(i).trimmed().replace(QRegularExpression("^[*]+[\\s]*"), "");
                        if (current_comment_line.size() == 0) continue;
                        if (current_comment_clean.size() > 0) current_comment_clean += "\n";
                        current_comment_clean += current_comment_line;
                    }
                    current_comment = current_comment_clean;
                }
                if (clsName.size() == 0) {
                    QString ns = "\\";
                    if (current_namespace.size() > 0) ns += current_namespace + "\\";
                    current_function = ns + current_function;
                }
                addFunction(clsName, current_function, current_function_args, current_function_is_static, current_function_is_abstract, current_function_visibility, current_function_min_args, current_function_max_args, current_function_return_type, current_comment, line);
                if (expected_function_args.size() > 0 && expected_function_args.size() == expected_function_arg_types.size()) {
                    for (int i=0; i<expected_function_args.size(); i++) {
                        QString argName = expected_function_args.at(i);
                        QString argType = expected_function_arg_types.at(i);
                        QString clsName = "";
                        if (current_class.size() > 0) clsName = current_class;
                        else if (current_interface.size() > 0) clsName = current_interface;
                        else if (current_trait.size() > 0) clsName = current_trait;
                        addVariable(clsName, current_function, argName, false, "", argType, line);
                    }
                }
                functionScope = scope;
                if ((current_interface.size() > 0 && k == ";") || (current_function_is_abstract && k == ";")) {
                    current_function = "";
                    current_function_args = "";
                    current_function_is_static = false;
                    current_function_is_abstract = false;
                    current_function_visibility = "";
                    current_function_min_args = 0;
                    current_function_max_args = 0;
                    current_function_return_type = "";
                    expected_function_args.clear();
                    expected_function_arg_types.clear();
                    functionScope = -1;
                    functionArgPars = -1;
                    functionArgsStart = -1;
                    functionStart = -1;
                    functionParsFound = false;
                    expected_function_name = "";
                }
            } else {
                anonymFunctionScope = scope;
                if (current_function.size() > 0) {
                    functionChainList.append(current_function);
                    current_function = "";
                }
            }
            expect = -1;
            expectName = "";
            functionArgPars = -1;
            functionArgsStart = -1;
            functionParsFound = false;
            expected_function_name = "";
        }

        // class method return "$this" type
        if (current_function.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevK.toLower() == "return" && prevK == "$this" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            current_function_return_type = current_class;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            updateFunctionReturnType(clsName, current_function, current_function_return_type);
        } else if (current_function.size() > 0 && prevPrevK.toLower() == "return" && prevK.indexOf("$") == 0 && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "$var" type
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + current_function.toStdString() + "::" + prevK.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.type.size() > 0) {
                        current_function_return_type = variable.type;
                        updateFunctionReturnType(clsName, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && prevPrevPrevPrevK.toLower() == "return" && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "function()" type
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if (prevPrevPrevK[0] != "\\") {
                QString ns = "\\";
                if (current_namespace.size() > 0) ns += current_namespace + "\\";
                prevPrevPrevK = ns + prevPrevPrevK;
            }
            functionIndexesIterator = functionIndexes.find("::" + prevPrevPrevK.toStdString());
            if (functionIndexesIterator != functionIndexes.end()) {
                int i = functionIndexesIterator->second;
                if (result.functions.size() > i) {
                    ParseResultFunction func = result.functions.at(i);
                    if (func.returnType.size() > 0) {
                        current_function_return_type = func.returnType;
                        updateFunctionReturnType(clsName, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && prevPrevPrevPrevPrevK.toLower() == "return" && prevPrevPrevPrevK == "new" && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "new Class()" type
            QString type = prevPrevPrevK;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((type.toLower() == "self" || type.toLower() == "static") && clsName.size() > 0) {
                type = clsName;
            }
            dataTypesIterator = dataTypes.find(type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                type = toAbs(current_namespace, type);
            }
            updateFunctionReturnType(clsName, current_function, type);
        } else if (current_function.size() > 0 && prevPrevPrevK.toLower() == "return" && prevPrevK == "new" && prevK.size() > 0 && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "new Class" type
            QString type = prevK;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((type.toLower() == "self" || type.toLower() == "static") && clsName.size() > 0) {
                type = clsName;
            }
            dataTypesIterator = dataTypes.find(type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                type = toAbs(current_namespace, type);
            }
            updateFunctionReturnType(clsName, current_function, type);
        } else if (current_function.size() > 0 && prevPrevPrevPrevPrevPrevK.toLower() == "return" && prevPrevPrevPrevPrevK == "new" && prevPrevPrevPrevK.size() > 0 && prevPrevPrevK == "(" && prevPrevK.size() > 0 && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "new Class($var)" type
            QString type = prevPrevPrevPrevK;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((type.toLower() == "self" || type.toLower() == "static") && clsName.size() > 0) {
                type = clsName;
            }
            dataTypesIterator = dataTypes.find(type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                type = toAbs(current_namespace, type);
            }
            updateFunctionReturnType(clsName, current_function, type);
        } else if (current_function.size() > 0 && prevPrevPrevPrevPrevPrevPrevPrevK.toLower() == "return" && prevPrevPrevPrevPrevPrevPrevK == "new" && prevPrevPrevPrevPrevPrevK.size() > 0 && prevPrevPrevPrevPrevK == "(" && prevPrevPrevPrevK.size() > 0 && prevPrevPrevK == "," && prevPrevK.size() > 0 && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "new Class($var1, $var2)" type
            QString type = prevPrevPrevPrevPrevPrevK;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((type.toLower() == "self" || type.toLower() == "static") && clsName.size() > 0) {
                type = clsName;
            }
            dataTypesIterator = dataTypes.find(type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                type = toAbs(current_namespace, type);
            }
            updateFunctionReturnType(clsName, current_function, type);
        } else if (current_function.size() > 0 && prevPrevPrevPrevPrevPrevPrevPrevK.toLower() == "return" && prevPrevPrevPrevPrevPrevPrevK == "new" && prevPrevPrevPrevPrevPrevK.size() > 0 && prevPrevPrevPrevPrevK == "(" && prevPrevPrevPrevK.size() > 0 && prevPrevPrevK == "(" && prevPrevK == ")" && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "new Class(func())" type
            QString type = prevPrevPrevPrevPrevPrevK;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((type.toLower() == "self" || type.toLower() == "static") && clsName.size() > 0) {
                type = clsName;
            }
            dataTypesIterator = dataTypes.find(type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                type = toAbs(current_namespace, type);
            }
            updateFunctionReturnType(clsName, current_function, type);
        } else if (current_function.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevPrevPrevPrevK.toLower() == "return" && (prevPrevPrevPrevK.toLower() == "self" || prevPrevPrevPrevK.toLower() == "static") && prevPrevPrevK == ":" && prevPrevK == ":" && prevK.indexOf("$") == 0 && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "self::$var" type
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + "::" + prevK.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.isStatic && variable.type.size() > 0) {
                        current_function_return_type = variable.type;
                        updateFunctionReturnType(clsName, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevPrevPrevPrevK.toLower() == "return" && prevPrevPrevPrevK == "$this" && prevPrevPrevK == "-" && prevPrevK == ">" && prevK.indexOf("$") < 0 && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "$this->var" type
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + "::" + "$" + prevK.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.type.size() > 0) {
                        current_function_return_type = variable.type;
                        updateFunctionReturnType(clsName, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevPrevPrevPrevPrevPrevK.toLower() == "return" && (prevPrevPrevPrevPrevPrevK.toLower() == "self" || prevPrevPrevPrevPrevPrevK.toLower() == "static") && prevPrevPrevPrevPrevK == ":" && prevPrevPrevPrevK == ":" && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "self::function()" type
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            functionIndexesIterator = functionIndexes.find(clsName.toStdString() + "::" + prevPrevPrevK.toStdString());
            if (functionIndexesIterator != functionIndexes.end()) {
                int i = functionIndexesIterator->second;
                if (result.functions.size() > i) {
                    ParseResultFunction func = result.functions.at(i);
                    if (func.isStatic && func.returnType.size() > 0) {
                        current_function_return_type = func.returnType;
                        updateFunctionReturnType(clsName, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevPrevPrevPrevPrevPrevK.toLower() == "return" && prevPrevPrevPrevPrevPrevK == "$this" && prevPrevPrevPrevPrevK == "-" && prevPrevPrevPrevK == ">" && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // function return "$this->function()" type
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            functionIndexesIterator = functionIndexes.find(clsName.toStdString() + "::" + prevPrevPrevK.toStdString());
            if (functionIndexesIterator != functionIndexes.end()) {
                int i = functionIndexesIterator->second;
                if (result.functions.size() > i) {
                    ParseResultFunction func = result.functions.at(i);
                    if (func.returnType.size() > 0) {
                        current_function_return_type = func.returnType;
                        updateFunctionReturnType(clsName, current_function, current_function_return_type);
                    }
                }
            }
        }

        // variables
        if (expect < 0 && functionArgPars < 0 && k.indexOf("$") == 0 && prevK != ":" && prevK != ">" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            expect = EXPECT_VARIABLE;
            expectName = k;
            current_variable = "";
            current_variable_type = "";
            class_static_variable = "";
            class_variable = "";
            if ((current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && current_function.size() == 0) {
                current_variable_is_static = (prevK.toLower() == "static" || (prevK.size() > 1 && prevPrevK.toLower() == "static"));
                current_variable_visibility = "public";
                if (prevK.toLower() == "public" || prevK.toLower() == "protected" || prevK.toLower() == "private") current_variable_visibility = prevK.toLower();
                if (prevK.size() > 1 && (prevPrevK.toLower() == "public" || prevPrevK.toLower() == "protected" || prevPrevK.toLower() == "private")) current_variable_visibility = prevPrevK.toLower();
            } else {
                current_variable_is_static = false;
                current_variable_visibility = "";
            }
            variableStart = m.capturedStart(1);
        } else if (expect == EXPECT_VARIABLE && expectName.size() > 0 && current_variable.size() == 0 && (k == "=" || (k == ";" && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && current_function.size() == 0))) {
            current_variable = expectName;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            int line = 0;
            if (variableStart >= 0) line = getLine(origText, textOffset + variableStart);
            addVariable(clsName, current_function, current_variable, current_variable_is_static, current_variable_visibility, current_variable_type, line);
            expect = -1;
            expectName = "";
            class_static_variable = "";
            class_variable = "";
        }

        // class static variable
        if (class_static_variable.size() == 0 && functionArgPars < 0 && k.indexOf("$") == 0 && prevK == ":" && prevPrevK == ":" && (prevPrevPrevK.toLower() == "self" || prevPrevPrevK.toLower() == "static") && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && anonymFunctionScope < 0 && anonymClassScope < 0) {
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + "::" + k.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.isStatic && variable.type.size() == 0) {
                        class_static_variable = k;
                        current_variable = "";
                        current_variable_type = "";
                    }
                }
            }
        }

        // class property
        if (class_variable.size() == 0 && functionArgPars < 0 && k.indexOf("$") < 0 && prevK == ">" && prevPrevK == "-" && prevPrevPrevK == "$this" && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && anonymFunctionScope < 0 && anonymClassScope < 0) {
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + "::" + "$" + k.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.type.size() == 0) {
                        class_variable = k;
                        current_variable = "";
                        current_variable_type = "";
                    }
                }
            }
        }

        // variable type
        if (current_variable.size() > 0 && prevPrevK == "=" && prevK.toLower() == "new" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            current_variable_type = k;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((current_variable_type.toLower() == "self" || current_variable_type.toLower() == "static") && clsName.size() > 0) {
                current_variable_type = clsName;
            }
            dataTypesIterator = dataTypes.find(current_variable_type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                current_variable_type = toAbs(current_namespace, current_variable_type);
            }
            updateVariableType(clsName, current_function, current_variable, current_variable_type);
        } else if (class_static_variable.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevK == "=" && prevK.toLower() == "new" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // class static variable type
            QString class_static_variable_type = k;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((class_static_variable_type.toLower() == "self" || class_static_variable_type.toLower() == "static") && clsName.size() > 0) {
                class_static_variable_type = clsName;
            }
            dataTypesIterator = dataTypes.find(class_static_variable_type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                class_static_variable_type = toAbs(current_namespace, class_static_variable_type);
            }
            updateVariableType(clsName, "", class_static_variable, class_static_variable_type);
        } else if (class_variable.size() > 0 && (current_class.size() > 0 || current_interface.size() > 0 || current_trait.size() > 0) && prevPrevK == "=" && prevK.toLower() == "new" && anonymFunctionScope < 0 && anonymClassScope < 0) {
            // class property type
            QString class_variable_type = k;
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            if ((class_variable_type.toLower() == "self" || class_variable_type.toLower() == "static") && clsName.size() > 0) {
                class_variable_type = clsName;
            }
            dataTypesIterator = dataTypes.find(class_variable_type.toLower().toStdString());
            if (dataTypesIterator == dataTypes.end()) {
                class_variable_type = toAbs(current_namespace, class_variable_type);
            }
            updateVariableType(clsName, "", "$" + class_variable, class_variable_type);
        }

        // constants
        if (expect < 0 && k.toLower() == "const" && functionArgsStart < 0 && (prevK == ";" || prevK == "{" || prevK == "}" || prevK.size() == 0) && anonymFunctionScope < 0 && (((namespaceScope < 0 && scope == 0) || scope == namespaceScope+1) || (current_class.size() > 0 && scope == classScope + 1) || (current_interface.size() > 0 && scope == interfaceScope + 1) || (current_trait.size() > 0 && scope == traitScope + 1))) {
            expect = EXPECT_CONST;
            expectName = "";
            current_constant = "";
            current_constant_value = "";
            constantValueStart = -1;
            constantStart = m.capturedStart(1);
        } else if (expect == EXPECT_CONST && expectName.size() == 0 && k.size() > 0) {
            expectName = k;
        } else if (expect == EXPECT_CONST && expectName.size() > 0 && k == "=") {
            expect = EXPECT_CONST_VALUE;
            constantValueStart = m.capturedStart(1) + 1;
        } else if (expect == EXPECT_CONST_VALUE && expectName.size() > 0 && k == ";") {
            current_constant = expectName;
            current_constant_value = origText.mid(textOffset+constantValueStart, m.capturedStart(1)-constantValueStart).trimmed();
            QString clsName = "";
            if (current_class.size() > 0) clsName = current_class;
            else if (current_interface.size() > 0) clsName = current_interface;
            else if (current_trait.size() > 0) clsName = current_trait;
            int line = 0;
            if (constantStart >= 0) line = getLine(origText, textOffset + constantStart);
            if (clsName.size() == 0) {
                QString ns = "\\";
                if (current_namespace.size() > 0) ns += current_namespace + "\\";
                current_constant = ns + current_constant;
            }
            addConstant(clsName, current_constant, current_constant_value, line);
            expect = -1;
            expectName = "";
            constantValueStart = -1;
        }

        if ((expect == EXPECT_VARIABLE ||
             expect == EXPECT_USE ||
             expect == EXPECT_USE_FUNCTION ||
             expect == EXPECT_USE_CONSTANT ||
             expect == EXPECT_NAMESPACE ||
             expect == EXPECT_CLASS_EXTENDED ||
             expect == EXPECT_CLASS_IMPLEMENTED ||
             expect == EXPECT_CLASS_EXTENDED_IMPLEMENTED ||
             expect == EXPECT_INTERFACE ||
             expect == EXPECT_INTERFACE_EXTENDED ||
             expect == EXPECT_TRAIT ||
             class_static_variable.size() > 0 || class_variable.size() > 0
             ) && (k == "-" || k == "+" || k == "*" || k == "/" || k == "%" || k == "&" || k == "|" || k == ":" || k == ">" || k == "<" || k == "?" || k == "[" || k == "]" || k == "(" || k == ")") && functionArgsStart < 0
        ) {
            expect = -1;
            expectName = "";
            class_static_variable = "";
            class_variable = "";
        }

        if ((k == ";" || ((k == "{" || k == "}") && expect != EXPECT_USE)) && functionArgsStart < 0) {
            expect = -1;
            expectName = "";
            class_static_variable = "";
            class_variable = "";
            expected_function_name = "";
            expected_class_name = "";
        }
        // braces
        if (k == "{") {
            scope++;
        }
        if (k == "}") {
            scope--;
            if (scope < 0) scope = 0;
            // namespace close
            if (current_namespace.size() > 0 && namespaceScope >= 0 && namespaceScope == scope) {
                current_namespace = "";
                namespaceScope = -1;
                if (parent_namespaces.size() > 0 && parent_namespaces.size() == parent_namespace_scopes.size()) {
                    current_namespace = parent_namespaces.last();
                    namespaceScope = parent_namespace_scopes.last();
                    parent_namespaces.removeLast();
                    parent_namespace_scopes.removeLast();
                }
                namespaceStart = -1;
            }
            // class close
            if (current_class.size() > 0 && classScope >= 0 && classScope == scope) {
                current_class = "";
                current_class_is_abstract = false;
                current_class_parent = "";
                current_class_interfaces.clear();
                classScope = -1;
                classStart = -1;
                expected_class_name = "";
                classParsFound = false;
            }
            // anonymous class close
            if (anonymClassScope >= 0 && anonymClassScope == scope) {
                anonymClassScope = -1;
                if (!classChainList.isEmpty()) {
                    current_class = classChainList.last();
                    classChainList.removeLast();
                } else if (!traitChainList.isEmpty()) {
                    current_trait = traitChainList.last();
                    traitChainList.removeLast();
                }
            }
            // interface close
            if (current_interface.size() > 0 && interfaceScope >= 0 && interfaceScope == scope) {
                current_interface = "";
                current_interface_parent = "";
                interfaceScope = -1;
                interfaceStart = -1;
            }
            // trait close
            if (current_trait.size() > 0 && traitScope >= 0 && traitScope == scope) {
                current_trait = "";
                traitScope = -1;
                traitStart = -1;
            }
            // function close
            if (current_function.size() > 0 && functionScope >= 0 && functionScope == scope) {
                current_function = "";
                current_function_args = "";
                current_function_is_static = false;
                current_function_is_abstract = false;
                current_function_visibility = "";
                current_function_min_args = 0;
                current_function_max_args = 0;
                current_function_return_type = "";
                expected_function_args.clear();
                expected_function_arg_types.clear();
                functionScope = -1;
                functionArgPars = -1;
                functionArgsStart = -1;
                functionStart = -1;
                functionParsFound = false;
                expected_function_name = "";
            }
            // anonymous function close
            if (anonymFunctionScope >= 0 && anonymFunctionScope == scope) {
                anonymFunctionScope = -1;
                if (!functionChainList.isEmpty()) {
                    current_function = functionChainList.last();
                    functionChainList.removeLast();
                }
            }
        }
        // parens
        if (k == "(") {
            pars++;
        }
        if (k == ")") {
            pars--;
            if (pars < 0) pars = 0;
            // function args
            if (functionArgPars >= 0 && functionArgPars == pars && functionArgsStart >= 0) {
                current_function_args = origText.mid(textOffset+functionArgsStart+1, m.capturedStart(1)-functionArgsStart-1).trimmed();
                current_function_args = Helper::stripScopedText(current_function_args);
                if (current_function_args.size() > 0) {
                    QString current_function_args_cleaned = "";
                    QStringList argsList, argsDefaultsList, argsTypeList;
                    QString argType, argName, argDefault;
                    argsList = current_function_args.split(",");
                    for (int i=0; i<argsList.size(); i++) {
                        argType = "", argName = "", argDefault = "";
                        argsDefaultsList = argsList.at(i).trimmed().split("=");
                        if (argsDefaultsList.size() == 2) {
                            argDefault = argsDefaultsList.at(1).trimmed();
                        } else if (argsDefaultsList.size() > 2) {
                            continue;
                        }
                        argsTypeList = argsDefaultsList.at(0).trimmed().replace(QRegularExpression("[&][\\s]+"), "&").split(QRegularExpression("[\\s]"));
                        if (argsTypeList.size() == 2) {
                            argType = argsTypeList.at(0).trimmed();
                            argName = argsTypeList.at(1).trimmed();
                        } else if (argsTypeList.size() == 1) {
                            argName = argsTypeList.at(0).trimmed();
                        } else {
                            continue;
                        }
                        if (argName.size() == 0 || (argName.indexOf("$") != 0 && argName.indexOf("&$") != 0)) {
                            continue;
                        }
                        QString argNameClean = argName;
                        if (argNameClean.mid(0, 1) == "&") argNameClean = argNameClean.mid(1);
                        if (argType.size() > 0) {
                            dataTypesIterator = dataTypes.find(argType.toLower().toStdString());
                            if (dataTypesIterator == dataTypes.end()) {
                                argType = toAbs(current_namespace, argType);
                            }
                        } else {
                            argType = "mixed";
                        }
                        current_function_max_args++;
                        if (argDefault.size() == 0) {
                            current_function_min_args++;
                        }
                        expected_function_args.append(argNameClean);
                        expected_function_arg_types.append(argType);
                        if (argType.size() > 0) {
                            argType += " ";
                        }
                        if (current_function_args_cleaned.size() > 0) {
                            current_function_args_cleaned += ", ";
                        }
                        if (argDefault.size() > 0) argDefault = " = "+argDefault;
                        current_function_args_cleaned += argType + argName + argDefault;
                    }
                    current_function_args = current_function_args_cleaned;
                }
                functionArgPars = -1;
                functionArgsStart = -1;
            }
        }
        prevPrevPrevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevK = prevPrevPrevPrevK;
        prevPrevPrevPrevK = prevPrevPrevK;
        prevPrevPrevK = prevPrevK;
        prevPrevK = prevK;
        prevK = k;
    }
}

void ParsePHP::reset()
{
    classAliases.clear();
    functionAliases.clear();
    constantAliases.clear();
    functionIndexes.clear();
    variableIndexes.clear();
    constantIndexes.clear();
    namespaceIndexes.clear();
    importIndexes.clear();
    classIndexes.clear();
    comments.clear();
}

ParsePHP::ParseResult ParsePHP::parse(QString text)
{
    result = ParseResult();
    reset();
    QString cleanText = cleanUp(text);
    QRegularExpressionMatchIterator mi = phpExpression.globalMatch(cleanText);
    while(mi.hasNext()){
        QRegularExpressionMatch m = mi.next();
        if (m.capturedStart(1) < 0) continue;
        if (m.captured(1).trimmed().size() == 0) continue;
        QString code = m.captured(1);
        parseCode(code, text, m.capturedStart(1));
    }
    // comments
    std::map<int, std::string> orderedComments(comments.begin(), comments.end());
    for (auto & commentsIterator : orderedComments) {
        addComment(QString::fromStdString(commentsIterator.second), commentsIterator.first);
    }
    return result;
}
