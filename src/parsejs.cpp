/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "parsejs.h"
#include <QVector>
#include "helper.h"

const int EXPECT_FUNCTION = 0;
const int EXPECT_VARIABLE = 1;
const int EXPECT_CONST = 2;
const int EXPECT_CONST_VALUE = 3;
const int EXPECT_CLASS_ES6 = 4;
const int EXPECT_CLASS_ES6_EXTENDED = 5;

ParseJS::ParseJS()
{
    commentSLExpression = QRegularExpression("[/][/]([^\n]+?)(?:[\n]|$)", QRegularExpression::DotMatchesEverythingOption);
    regexpExpression = QRegularExpression("(?:[^\\sa-zA-Z0-9_\\$\\)\\]<\\*\\~\\\\][\\s]*)[/](.*?[^\\\\])[/]", QRegularExpression::DotMatchesEverythingOption);
    parseExpression = QRegularExpression("([a-zA-Z0-9_\\$]+|[\\(\\)\\{\\}\\[\\]\\.,=;:!@#%^&*\\-+/\\|<>\\?\\\\])", QRegularExpression::DotMatchesEverythingOption);
    nameExpression = QRegularExpression("^[a-zA-Z_\\$][a-zA-Z0-9_\\$]*$");
}

QString ParseJS::cleanUp(QString text)
{
    comments.clear();
    prepare(text);
    // strip strings & comments
    int offset = 0;
    QList<int> matchesPos;
    QRegularExpressionMatch stringDQMatch;
    QRegularExpressionMatch stringSQMatch;
    QRegularExpressionMatch commentMLMatch;
    QRegularExpressionMatch commentSLMatch;
    QRegularExpressionMatch regexpMatch;
    QRegularExpressionMatch backtickMatch;
    int stringDQPos = -2,
        stringSQPos = -2,
        commentMLPos = -2,
        commentSLPos = -2,
        regexpPos = -2,
        backtickPos = -2;
    do {
        matchesPos.clear();
        if (stringDQPos != -1 && stringDQPos < offset) {
            stringDQMatch = stringDQExpression.match(text, offset);
            stringDQPos = stringDQMatch.capturedStart(1)-1;
        }
        if (stringDQPos >= 0) matchesPos.append(stringDQPos);
        if (stringSQPos != -1 && stringSQPos < offset) {
            stringSQMatch = stringSQExpression.match(text, offset);
            stringSQPos = stringSQMatch.capturedStart(1)-1;
        }
        if (stringSQPos >= 0) matchesPos.append(stringSQPos);
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
        if (regexpPos != -1 && regexpPos < offset) {
            regexpMatch = regexpExpression.match(text, offset);
            regexpPos = regexpMatch.capturedStart(1)-1;
        }
        if (regexpPos >= 0) matchesPos.append(regexpPos);
        if (backtickPos != -1 && backtickPos < offset) {
            backtickMatch = backtickExpression.match(text, offset);
            backtickPos = backtickMatch.capturedStart(1)-1;
        }
        if (backtickPos >= 0) matchesPos.append(backtickPos);
        if (matchesPos.size() == 0) break;
        std::sort(matchesPos.begin(), matchesPos.end());
        int pos = matchesPos.at(0);
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
        if (regexpPos == pos) {
            offset = regexpMatch.capturedStart(1) + regexpMatch.capturedLength(1);
            strip(regexpMatch, text, 1);
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

bool ParseJS::isValidName(QString name)
{
    QRegularExpressionMatch m = nameExpression.match(name);
    return (m.capturedStart()==0);
}

void ParseJS::addClass(QString name, int line) {
    if (!isValidName(name)) return;
    ParseResultClass cls;
    cls.name = name;
    cls.line = line;
    result.classes.append(cls);
    classIndexes[name.toStdString()] = result.classes.size() - 1;
}

void ParseJS::addFunction(QString clsName, QString name, QString args, int minArgs, int maxArgs, bool isGlobal, QString returnType, QString comment, int line) {
    if (!isValidName(name)) return;
    ParseResultFunction func;
    func.name = name;
    func.clsName = clsName;
    func.args = args;
    func.minArgs = minArgs;
    func.maxArgs = maxArgs;
    func.isGlobal = isGlobal;
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
                ParseJS::ParseResultClass cls = result.classes.at(i);
                cls.functionIndexes.append(result.functions.size() - 1);
                result.classes.replace(i, cls);
            }
        }
    }
}

void ParseJS::updateFunctionReturnType(QString clsName, QString funcName, QString returnType)
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

void ParseJS::addVariable(QString clsName, QString funcName, QString name, QString type, int line)
{
    if (!isValidName(name)) return;
    variableIndexesIterator = variableIndexes.find(clsName.toStdString() + "::" + funcName.toStdString() + "::" + name.toStdString());
    if (variableIndexesIterator != variableIndexes.end()) return;
    ParseResultVariable variable;
    variable.name = name;
    variable.clsName = clsName;
    variable.funcName = funcName;
    variable.type = type;
    variable.line = line;
    result.variables.append(variable);
    variableIndexes[clsName.toStdString() + "::" + funcName.toStdString() + "::" + name.toStdString()] = result.variables.size() - 1;
    if (clsName.size() > 0 && funcName.size() == 0) {
        classIndexesIterator = classIndexes.find(clsName.toStdString());
        if (classIndexesIterator != classIndexes.end()) {
            int i = classIndexesIterator->second;
            if (result.classes.size() > i) {
                ParseJS::ParseResultClass cls = result.classes.at(i);
                cls.variableIndexes.append(result.variables.size() - 1);
                result.classes.replace(i, cls);
            }
        }
    } else if (funcName.size() > 0) {
        functionIndexesIterator = functionIndexes.find(clsName.toStdString() + "::" + funcName.toStdString());
        if (functionIndexesIterator != functionIndexes.end()) {
            int i = functionIndexesIterator->second;
            if (result.functions.size() > i) {
                ParseJS::ParseResultFunction func = result.functions.at(i);
                func.variableIndexes.append(result.variables.size() - 1);
                result.functions.replace(i, func);
            }
        }
    }
}

void ParseJS::updateVariableType(QString clsName, QString funcName, QString varName, QString type)
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

void ParseJS::addConstant(QString clsName, QString funcName, QString name, QString value, int line)
{
    if (!isValidName(name)) return;
    ParseResultConstant constant;
    constant.name = name;
    constant.clsName = clsName;
    constant.funcName = funcName;
    constant.value = value;
    constant.line = line;
    result.constants.append(constant);
    constantIndexes[clsName.toStdString() + "::" + funcName.toStdString() + "::" + name.toStdString()] = result.constants.size() - 1;
    if (funcName.size() > 0) {
        functionIndexesIterator = functionIndexes.find(clsName.toStdString()+"::"+funcName.toStdString());
        if (functionIndexesIterator != functionIndexes.end()) {
            int i = functionIndexesIterator->second;
            if (result.functions.size() > i) {
                ParseJS::ParseResultFunction func = result.functions.at(i);
                func.constantIndexes.append(result.constants.size() - 1);
                result.functions.replace(i, func);
            }
        }
    }
}

void ParseJS::addComment(QString text, int line) {
    QString name = "";
    if (text.size() > 0) {
        if (text.indexOf("//")==0) text = text.mid(2);
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

void ParseJS::addError(QString text, int line, int symbol) {
    ParseResultError error;
    error.text = text;
    error.line = line;
    error.symbol = symbol;
    result.errors.append(error);
}

void ParseJS::parseCode(QString & code, QString & origText)
{
    // parse data
    QString current_class = "";
    QString current_function = "";
    QString current_function_args = "";
    int current_function_min_args = 0;
    int current_function_max_args = 0;
    QString current_function_return_type = "";
    QString current_variable = "";
    QString current_variable_type = "";
    QString current_constant = "";
    QString current_constant_value = "";
    QString current_class_es6 = "";
    QString current_class_es6_parent = "";

    QString expected_function_name = "";
    QStringList expected_function_args;
    int scope = 0;
    int functionScope = -1, classES6Scope = -1;
    int pars = 0;
    int curlyBrackets = 0, roundBrackets = 0, squareBrackets = 0;
    QVector<int> curlyBracketsList, roundBracketsList, squareBracketsList;
    int functionArgPars = -1;
    int functionArgsStart = -1;
    int constantValueStart = -1;
    bool functionParsFound = false, classES6ParsFound = false;
    int expect = -1;
    QString expectName = "";
    QString prevK = "", prevPrevK = "", prevPrevPrevK = "", prevPrevPrevPrevK = "", prevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevPrevPrevK = "";
    int functionStart = -1, variableStart = -1, constantStart = -1, classES6Start = -1;
    QString class_variable = "";
    QString expected_class_es6_name = "";

    QRegularExpressionMatchIterator mi = parseExpression.globalMatch(code);
    while(mi.hasNext()){
        QRegularExpressionMatch m = mi.next();
        if (m.capturedStart(1) < 0) continue;
        QString k = m.captured(1).trimmed();
        if (k.size() == 0) continue;

        // classes ES6
        if ((prevPrevK.size() == 0 || prevPrevK == ";" || prevPrevK == "{" || prevPrevK == "}" || prevPrevK == "var" || prevPrevK == "let" || prevPrevK == "const" || prevPrevK == "final") && prevK.size() > 0 && k == "=" && functionArgsStart < 0 && ((prevPrevK != "var" && prevPrevK != "let" && prevPrevK != "const" && prevPrevK != "final") || (current_function.size() == 0 && scope == 0) || (functionScope >= 0 && functionScope == scope - 1))) {
            expected_class_es6_name = prevK;
        }
        if ((expect < 0 || expect == EXPECT_VARIABLE) && k.toLower() == "class" && (prevK == ";" || prevK == "{" || prevK == "}" || prevK == "=" || prevK == "final" || prevK.size() == 0) && current_function.size() == 0) {
            expect = EXPECT_CLASS_ES6;
            expectName = "";
            current_class_es6_parent = "";
            classES6Start =  m.capturedStart(1);
            classES6ParsFound = false;
        } else if (expect == EXPECT_CLASS_ES6 && expectName.size() == 0 && k != "{" && !classES6ParsFound) {
            if (k != "(" && k != ")" && k != "{" && k != "extends" && current_class_es6.size() == 0) {
                expectName = k;
            } else {
                classES6ParsFound = true;
            }
        } else if (expect == EXPECT_CLASS_ES6 && expectName.size() > 0 && k.toLower() == "extends") {
            expect = EXPECT_CLASS_ES6_EXTENDED;
        } else if (expect == EXPECT_CLASS_ES6_EXTENDED && expectName.size() > 0 && current_class_es6_parent.size() == 0 && k != "{") {
            current_class_es6_parent = k;
        } else if ((expect == EXPECT_CLASS_ES6 || expect == EXPECT_CLASS_ES6_EXTENDED) && (expectName.size() > 0 || expected_class_es6_name.size() > 0) && k == "{") {
            current_class_es6 = expectName.size() > 0 ? expectName : expected_class_es6_name;
            int line = 0;
            if (classES6Start >= 0) line = getLine(origText, classES6Start);
            addClass(current_class_es6, line);
            classES6Scope = scope;
            expect = -1;
            expectName = "";
            expected_class_es6_name = "";
            classES6ParsFound = false;
            expected_function_name = "";
        }

        // classes
        if (prevPrevPrevPrevPrevK.size() > 0 && prevPrevPrevPrevK == "." && prevPrevPrevK == "prototype" && prevPrevK == "." && prevK.size() > 0 && k == "=" && functionArgsStart < 0) {
            QString clsName = prevPrevPrevPrevPrevK;
            functionIndexesIterator = functionIndexes.find("::"+clsName.toStdString());
            if (functionIndexesIterator != functionIndexes.end()) {
                int i = functionIndexesIterator->second;
                if (result.functions.size() > i) {
                    current_class = clsName;
                    ParseJS::ParseResultFunction func = result.functions.at(i);
                    classIndexesIterator = classIndexes.find(clsName.toStdString());
                    if (classIndexesIterator == classIndexes.end()) {
                        addClass(clsName, func.line);
                    }
                }
            }
        }

        // functions
        if ((prevPrevK.size() == 0 || prevPrevK == ";" || prevPrevK == "{" || prevPrevK == "}" || prevPrevK == "var" || prevPrevK == "let" || prevPrevK == "const" || prevPrevK == "final" || (prevPrevK == "." && prevPrevPrevK == "prototype" && prevPrevPrevPrevK == ".")) && prevK.size() > 0 && k == "=" && functionArgsStart < 0 && ((prevPrevK != "var" && prevPrevK != "let" && prevPrevK != "const" && prevPrevK != "final") || (current_function.size() == 0 && scope == 0) || (functionScope >= 0 && functionScope == scope - 1))) {
            expected_function_name = prevK;
        }
        if (expect != EXPECT_CLASS_ES6 && expect != EXPECT_CLASS_ES6_EXTENDED && current_class_es6.size() > 0 && classES6Scope == scope - 1 && k.size() > 0 && k != "(" && k != ")" && k != "{" && k != "}" && current_function.size() == 0 && functionArgPars < 0) {
            expect = EXPECT_FUNCTION;
            current_function_args = "";
            expected_function_args.clear();
            current_function_min_args = 0;
            current_function_max_args = 0;
            current_function_return_type = "";
            expectName = "";
            functionArgPars = -1;
            functionArgsStart = -1;
            functionParsFound = false;
            functionStart = m.capturedStart(1);
            expectName = k;
        } else if ((expect < 0 || expect == EXPECT_VARIABLE) && current_function.size() == 0 && k == "function" && functionArgsStart < 0) {
            expect = EXPECT_FUNCTION;
            current_function_args = "";
            expected_function_args.clear();
            current_function_min_args = 0;
            current_function_max_args = 0;
            current_function_return_type = "";
            expectName = "";
            functionArgPars = -1;
            functionArgsStart = -1;
            functionParsFound = false;
            functionStart = m.capturedStart(1);
        } else if (expect == EXPECT_FUNCTION && expectName.size() == 0 && k != "(" && k != ")" && k != "{" && functionArgsStart < 0 && current_function_args.size() == 0 && !functionParsFound) {
            expectName = k;
        } else if (expect == EXPECT_FUNCTION && functionArgPars < 0 && k == "(") {
            functionArgPars = pars;
            functionArgsStart = m.capturedStart(1);
            functionParsFound = true;
        } else if (expect == EXPECT_FUNCTION && (expectName.size() > 0 || expected_function_name.size() > 0) && k == "{" && functionArgsStart < 0) {
            current_function = expectName.size() > 0 ? expectName : expected_function_name;
            int line = 0;
            if (functionStart >= 0) line = getLine(origText, functionStart);
            QString current_comment = "";
            int comment_line = getFirstNotEmptyLineTo(origText, functionStart);
            if (comment_line > 0) {
                commentsIterator = comments.find(comment_line);
                if (commentsIterator != comments.end()) {
                    current_comment = QString::fromStdString(commentsIterator->second);
                }
            }
            if (current_comment.size() > 0) {
                if (current_comment.indexOf("//")==0) current_comment = current_comment.mid(2);
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
            bool isGlobal = (scope == 0);
            QString cls = current_class_es6.size() > 0 ? current_class_es6 : current_class;
            addFunction(cls, current_function, current_function_args, current_function_min_args, current_function_max_args, isGlobal, current_function_return_type, current_comment, line);
            if (expected_function_args.size() > 0) {
                for (int i=0; i<expected_function_args.size(); i++) {
                    QString argName = expected_function_args.at(i);
                    QString cls = current_class_es6.size() > 0 ? current_class_es6 : current_class;
                    addVariable(cls, current_function, argName, "", line);
                }
            }
            expect = -1;
            expectName = "";
            functionScope = scope;
            functionArgPars = -1;
            functionArgsStart = -1;
            expected_function_name = "";
            functionParsFound = false;
            expected_class_es6_name = "";
        }

        // class method return "this" type
        if (current_function.size() > 0 && current_class.size() > 0 && current_function_return_type.size() == 0 && prevPrevK == "return" && prevK == "this" && k == ";" && functionScope == scope - 1) {
            current_function_return_type = current_class;
            updateFunctionReturnType(current_class, current_function, current_function_return_type);
        } else if (current_function.size() > 0 && current_function_return_type.size() == 0 && prevPrevK == "return" && prevK.size() > 0 && k == ";" && functionScope == scope - 1) {
            // function return "var" type
            variableIndexesIterator = variableIndexes.find(current_class.toStdString() + "::" + current_function.toStdString() + "::" + prevK.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.type.size() > 0) {
                        current_function_return_type = variable.type;
                        updateFunctionReturnType(current_class, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && current_function_return_type.size() == 0 && prevPrevPrevPrevK == "return" && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && functionScope == scope - 1) {
            // function return "function()" type
            functionIndexesIterator = functionIndexes.find("::" + prevPrevPrevK.toStdString());
            if (functionIndexesIterator != functionIndexes.end()) {
                int i = functionIndexesIterator->second;
                if (result.functions.size() > i) {
                    ParseResultFunction func = result.functions.at(i);
                    if (func.returnType.size() > 0) {
                        current_function_return_type = func.returnType;
                        updateFunctionReturnType(current_class, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && current_function_return_type.size() == 0 && prevPrevPrevPrevPrevK == "return" && prevPrevPrevPrevK == "new" && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && functionScope == scope - 1) {
            // function return "new Class()" type
            QString type = prevPrevPrevK;
            updateFunctionReturnType(current_class, current_function, type);
        } else if (current_function.size() > 0 && current_function_return_type.size() == 0 && prevPrevPrevK == "return" && prevPrevK == "new" && prevK.size() > 0 && k == ";" && functionScope == scope - 1) {
            // function return "new Class" type
            QString type = prevK;
            updateFunctionReturnType(current_class, current_function, type);
        } else if (current_function.size() > 0 && current_function_return_type.size() == 0 && current_class.size() > 0 && prevPrevPrevPrevK == "return" && prevPrevPrevK == "this" && prevPrevK == "." && prevK.size() > 0 && k == ";" && functionScope == scope - 1) {
            // function return "this.var" type
            variableIndexesIterator = variableIndexes.find(current_class.toStdString() + "::" + "::" + prevK.toStdString());
            if (variableIndexesIterator != variableIndexes.end()) {
                int i = variableIndexesIterator->second;
                if (result.variables.size() > i) {
                    ParseResultVariable variable = result.variables.at(i);
                    if (variable.type.size() > 0) {
                        current_function_return_type = variable.type;
                        updateFunctionReturnType(current_class, current_function, current_function_return_type);
                    }
                }
            }
        } else if (current_function.size() > 0 && current_function_return_type.size() == 0 && current_class.size() > 0 && prevPrevPrevPrevPrevPrevK == "return" && prevPrevPrevPrevPrevK == "this" && prevPrevPrevPrevK == "." && prevPrevPrevK.size() > 0 && prevPrevK == "(" && prevK == ")" && k == ";" && functionScope == scope - 1) {
            // function return "this.function()" type
            functionIndexesIterator = functionIndexes.find(current_class.toStdString() + "::" + prevPrevPrevK.toStdString());
            if (functionIndexesIterator != functionIndexes.end()) {
                int i = functionIndexesIterator->second;
                if (result.functions.size() > i) {
                    ParseResultFunction func = result.functions.at(i);
                    if (func.returnType.size() > 0) {
                        current_function_return_type = func.returnType;
                        updateFunctionReturnType(current_class, current_function, current_function_return_type);
                    }
                }
            }
        }

        // variables
        if (expect < 0 && functionArgPars < 0 && k.size() > 0 && (prevK == "var" || prevK == "let" || prevK == "const" || prevK.size() > 1 || (prevK.size() == 1 && ((prevK[0]).isLetter() || prevK[0] == '$'))) && ((current_function.size() == 0 && scope == 0) || (functionScope >= 0 && functionScope == scope - 1))) {
            expect = EXPECT_VARIABLE;
            expectName = k;
            current_variable = "";
            current_variable_type = "";
            class_variable = "";
            variableStart = m.capturedStart(1);
        } else if (expect == EXPECT_VARIABLE && expectName.size() > 0 && current_variable.size() == 0 && prevK == "=" && k.size() > 0 && (k != "function" || current_function.size() > 0) && (k != "class" || current_function.size() > 0)) {
            current_variable = expectName;
            int line = 0;
            if (variableStart >= 0) line = getLine(origText, variableStart);
            QString cls = current_class_es6.size() > 0 ? current_class_es6 : current_class;
            addVariable(cls, current_function, current_variable, current_variable_type, line);
            expect = -1;
            expectName = "";
            class_variable = "";
        } else if (functionScope >= 0 && functionScope == scope - 1 && functionArgPars < 0 && current_function.size() > 0 && prevK == "function" && k.size() > 0 && k != "(" && k != ")" && k != "{") {
            QString cls = current_class_es6.size() > 0 ? current_class_es6 : current_class;
            addVariable(cls, current_function, k, "", getLine(origText, m.capturedStart(1)));
        } else if (functionScope >= 0 && functionScope == scope - 1 && functionArgPars < 0 && current_function.size() > 0 && prevK == "class" && k.size() > 0 && k != "(" && k != ")" && k != "{") {
            QString cls = current_class_es6.size() > 0 ? current_class_es6 : current_class;
            addVariable(cls, current_function, k, "", getLine(origText, m.capturedStart(1)));
        } else if (scope == 0 && (prevPrevPrevK.size() == 0 || prevPrevPrevK == ";" || prevPrevPrevK == "{" || prevPrevPrevK == "}") && prevPrevK.size() > 0 && prevK == "=" && k.size() > 0 && k != "function" && k != "class") {
            current_variable = prevPrevK;
            int line = getLine(origText, m.capturedStart(1));
            addVariable("", "", current_variable, "", line);
            expect = -1;
            expectName = "";
            class_variable = "";
        }

        // class property
        if (class_variable.size() == 0 && functionArgPars < 0 && k == "=" && prevK.size() > 0 && prevPrevK == "." && prevPrevPrevK == "this" && current_class.size() == 0 && current_function.size() > 0 && functionArgsStart < 0 && functionScope == scope - 1 && current_class_es6.size() == 0) {
            variableIndexesIterator = variableIndexes.find(current_function.toStdString() + "::" + "::" + prevK.toStdString());
            if (variableIndexesIterator == variableIndexes.end()) {
                classIndexesIterator = classIndexes.find(current_function.toStdString());
                if (classIndexesIterator == classIndexes.end()) {
                    functionIndexesIterator = functionIndexes.find("::"+current_function.toStdString());
                    if (functionIndexesIterator != functionIndexes.end()) {
                        int i = functionIndexesIterator->second;
                        if (result.functions.size() > i) {
                            ParseJS::ParseResultFunction func = result.functions.at(i);
                            addClass(current_function, func.line);
                        }
                    }
                }
                class_variable = prevK;
                current_variable = "";
                current_variable_type = "";
                addVariable(current_function, "", class_variable, "", getLine(origText, m.capturedStart(1)));
            }
        } else if (class_variable.size() == 0 && functionArgPars < 0 && k == "=" && prevK.size() > 0 && prevPrevK == "." && prevPrevPrevK == "this" && current_class_es6.size() > 0 && current_function.size() > 0 && current_function == "constructor" && functionArgsStart < 0 && functionScope == scope - 1) {
            variableIndexesIterator = variableIndexes.find(current_class_es6.toStdString() + "::" + "::" + prevK.toStdString());
            if (variableIndexesIterator == variableIndexes.end()) {
                class_variable = prevK;
                current_variable = "";
                current_variable_type = "";
                addVariable(current_class_es6, "", class_variable, "", getLine(origText, m.capturedStart(1)));
            }
        }

        // variable type
        if (current_variable.size() > 0 && current_variable_type.size() == 0 && prevPrevK == "=" && prevK == "new" && ((current_function.size() == 0 && scope == 0) || (functionScope >= 0 && functionScope == scope - 1))) {
            current_variable_type = k;
            updateVariableType(current_class, current_function, current_variable, current_variable_type);
        } else if (class_variable.size() > 0 && current_class.size() == 0 && current_function.size() > 0 && prevPrevK == "=" && prevK == "new" && functionScope == scope - 1) {
            // class property type
            QString class_variable_type = k;
            updateVariableType(current_function, "", class_variable, class_variable_type);
        }

        // constants
        if (expect < 0 && prevK == "const" && ((current_function.size() == 0 && scope == 0) || (functionScope >= 0 && functionScope == scope - 1)) && functionArgsStart < 0) {
            expect = EXPECT_CONST;
            expectName = k;
            current_constant = "";
            current_constant_value = "";
            constantValueStart = -1;
            constantStart = m.capturedStart(1);
        } else if (expect == EXPECT_CONST && expectName.size() > 0 && k == "=") {
            expect = EXPECT_CONST_VALUE;
            constantValueStart = m.capturedStart(1) + 1;
        } else if (expect == EXPECT_CONST_VALUE && expectName.size() > 0 && k == ";") {
            current_constant = expectName;
            current_constant_value = origText.mid(constantValueStart, m.capturedStart(1)-constantValueStart).trimmed();
            int line = 0;
            if (constantStart >= 0) line = getLine(origText, constantStart);
            addConstant(current_class, current_function, current_constant, current_constant_value, line);
            expect = -1;
            expectName = "";
            constantValueStart = -1;
        }

        if ((expect == EXPECT_VARIABLE || expect == EXPECT_CONST || class_variable.size() > 0) && (k == "-" || k == "+" || k == "*" || k == "/" || k == "%" || k == "&" || k == "|" || k == ":" || k == ">" || k == "<" || k == "?" || k == "[" || k == "]" || k == "(" || k == ")" || k == ".")) {
            expect = -1;
            expectName = "";
            class_variable = "";
        }

        if ((k == ";" || k == "{" || k == "}") && functionArgsStart < 0) {
            expect = -1;
            expectName = "";
            class_variable = "";
            expected_function_name = "";
            expected_class_es6_name = "";
        }
        // braces
        if (k == "{") {
            scope++;
            curlyBrackets++;
            curlyBracketsList.append(m.capturedStart(1)+1);
        }
        if (k == "}") {
            scope--;
            if (scope < 0) scope = 0;
            curlyBrackets--;
            curlyBracketsList.append(-1 * (m.capturedStart(1)+1));
            // class ES6 close
            if (current_class_es6.size() > 0 && classES6Scope >= 0 && classES6Scope == scope) {
                current_class_es6 = "";
                current_class_es6_parent = "";
                classES6Scope = -1;
                classES6Start = -1;
                expected_class_es6_name = "";
                classES6ParsFound = false;
                expected_function_name = "";
            }
            // function close
            if (current_function.size() > 0 && functionScope >= 0 && functionScope == scope) {
                current_function = "";
                current_function_args = "";
                current_function_min_args = 0;
                current_function_max_args = 0;
                current_function_return_type = "";
                expected_function_args.clear();
                functionScope = -1;
                functionArgPars = -1;
                functionArgsStart = -1;
                functionStart = -1;
                expected_function_name = "";
                current_class = "";
                functionParsFound = false;
                expected_class_es6_name = "";
            }
        }
        // parens
        if (k == "(") {
            pars++;
            roundBrackets++;
            roundBracketsList.append(m.capturedStart(1)+1);
        }
        if (k == ")") {
            pars--;
            if (pars < 0) pars = 0;
            roundBrackets--;
            roundBracketsList.append(-1 * (m.capturedStart(1)+1));
            // function args
            if (functionArgPars >= 0 && functionArgPars == pars && functionArgsStart >= 0) {
                current_function_args = origText.mid(functionArgsStart+1, m.capturedStart(1)-functionArgsStart-1).trimmed();
                current_function_args = Helper::stripScopedText(current_function_args);
                if (current_function_args.size() > 0) {
                    QString current_function_args_cleaned = "";
                    QStringList argsList, argsDefaultsList;
                    QString argName, argDefault;
                    argsList = current_function_args.split(",");
                    for (int i=0; i<argsList.size(); i++) {
                        argName = ""; argDefault = "";
                        argsDefaultsList = argsList.at(i).trimmed().split("=");
                        if (argsDefaultsList.size() == 2) {
                            argDefault = argsDefaultsList.at(1).trimmed();
                        } else if (argsDefaultsList.size() > 2) {
                            continue;
                        }
                        argName = argsDefaultsList.at(0).trimmed();
                        if (argName.indexOf(" ") >= 0) {
                            argName = argName.mid(argName.lastIndexOf(" ")+1);
                        }
                        if (argName.size() == 0) {
                            continue;
                        }
                        current_function_max_args++;
                        if (argDefault.size() == 0) {
                            current_function_min_args++;
                        }
                        expected_function_args.append(argName);
                        if (current_function_args_cleaned.size() > 0) {
                            current_function_args_cleaned += ", ";
                        }
                        if (argDefault.size() > 0) argDefault = " = "+argDefault;
                        current_function_args_cleaned += argName + argDefault;
                    }
                    current_function_args = current_function_args_cleaned;
                }
                functionArgPars = -1;
                functionArgsStart = -1;
            }
        }
        // brackets
        if (k == "[") {
            squareBrackets++;
            squareBracketsList.append(m.capturedStart(1)+1);
        }
        if (k == "]") {
            squareBrackets--;
            squareBracketsList.append(-1 * (m.capturedStart(1)+1));
        }
        prevPrevPrevPrevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevPrevK = prevPrevPrevPrevPrevK;
        prevPrevPrevPrevPrevK = prevPrevPrevPrevK;
        prevPrevPrevPrevK = prevPrevPrevK;
        prevPrevPrevK = prevPrevK;
        prevPrevK = prevK;
        prevK = k;
    }
    if (curlyBrackets > 0) {
        int offset = findOpenScope(curlyBracketsList);
        if (offset != 0) offset = abs(offset) - 1;
        int line = getLine(origText, offset);
        addError(QObject::tr("Unclosed brace"), line, offset);
    } else if (curlyBrackets < 0) {
        int offset = findCloseScope(curlyBracketsList);
        if (offset != 0) offset = abs(offset) - 1;
        int line = getLine(origText, offset);
        addError(QObject::tr("Excess brace"), line, offset);
    }
    if (roundBrackets > 0) {
        int offset = findOpenScope(roundBracketsList);
        if (offset != 0) offset = abs(offset) - 1;
        int line = getLine(origText, offset);
        addError(QObject::tr("Unclosed parenthesis"), line, offset);
    } else if (roundBrackets < 0) {
        int offset = findCloseScope(roundBracketsList);
        if (offset != 0) offset = abs(offset) - 1;
        int line = getLine(origText, offset);
        addError(QObject::tr("Excess parenthesis"), line, offset);
    }
    if (squareBrackets > 0) {
        int offset = findOpenScope(squareBracketsList);
        if (offset != 0) offset = abs(offset) - 1;
        int line = getLine(origText, offset);
        addError(QObject::tr("Unclosed bracket"), line, offset);
    } else if (squareBrackets < 0) {
        int offset = findCloseScope(squareBracketsList);
        if (offset != 0) offset = abs(offset) - 1;
        int line = getLine(origText, offset);
        addError(QObject::tr("Excess bracket"), line, offset);
    }
}

void ParseJS::reset()
{
    functionIndexes.clear();
    variableIndexes.clear();
    constantIndexes.clear();
    classIndexes.clear();
    comments.clear();
}

ParseJS::ParseResult ParseJS::parse(QString text)
{
    result = ParseResult();
    reset();
    QString cleanText = cleanUp(text);
    parseCode(cleanText, text);
    // comments
    std::map<int, std::string> orderedComments(comments.begin(), comments.end());
    for (auto & commentsIterator : orderedComments) {
        addComment(QString::fromStdString(commentsIterator.second), commentsIterator.first);
    }
    return result;
}
