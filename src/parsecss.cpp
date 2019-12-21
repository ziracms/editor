/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "parsecss.h"
#include <QFile>
#include <QTextStream>
#include "helper.h"

const int EXPECT_SELECTOR = 0;
const int EXPECT_MEDIA = 1;
const int EXPECT_KEYFRAMES = 2;
const int EXPECT_FONT_FACE = 3;
const int EXPECT_FONT_FAMILY = 4;

std::unordered_map<std::string, std::string> ParseCSS::mainTags = {};

ParseCSS::ParseCSS()
{
    parseExpression = QRegularExpression("([a-zA-Z0-9_\\-]+|[\\$\\(\\)\\{\\}\\[\\]\\.,=;:!@#%^&*+/\\|<>\\?\\\\])", QRegularExpression::DotMatchesEverythingOption);
    nameExpression = QRegularExpression("^[#\\.]?[a-zA-Z_][a-zA-Z0-9_\\-#\\.: ]*$");
    colorExpression = QRegularExpression("^[#](?:[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9])(?:[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9])?(?:[a-fA-F0-9][a-fA-F0-9])?$");

    if (mainTags.size() == 0) {
        QFile sf(":/syntax/html_alltags");
        sf.open(QIODevice::ReadOnly);
        QTextStream sin(&sf);
        QString k;
        while (!sin.atEnd()) {
            k = sin.readLine();
            if (k == "") continue;
            mainTags[k.toStdString()] = k.toStdString();
        }
        sf.close();
    }
}

QString ParseCSS::cleanUp(QString text)
{
    comments.clear();
    prepare(text);
    // strip strings & comments
    int offset = 0;
    QList<int> matchesPos;
    QRegularExpressionMatch stringDQMatch;
    QRegularExpressionMatch stringSQMatch;
    QRegularExpressionMatch commentMLMatch;
    int stringDQPos = -2,
        stringSQPos = -2,
        commentMLPos = -2;
    do {
        matchesPos.clear();
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
        if (commentMLPos != -1 && commentMLPos < offset) {
            commentMLMatch = commentMLExpression.match(text, offset);
            commentMLPos = commentMLMatch.capturedStart();
        }
        if (commentMLPos >= 0) matchesPos.append(commentMLPos);
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
    } while (matchesPos.size() > 0);
    return text;
}

bool ParseCSS::isValidName(QString name)
{
    QRegularExpressionMatch m = nameExpression.match(name);
    return (m.capturedStart()==0);
}

bool ParseCSS::isColor(QString name)
{
    QRegularExpressionMatch m = colorExpression.match(name);
    return (m.capturedStart()==0);
}

void ParseCSS::addSelector(QString name, int line) {
    if (!isValidName(name)) return;
    selectorIndexesIterator = selectorIndexes.find(name.toStdString());
    if (selectorIndexesIterator != selectorIndexes.end()) return;
    ParseResultSelector selector;
    selector.name = name;
    selector.line = line;
    result.selectors.append(selector);
    selectorIndexes[name.toStdString()] = result.selectors.size() - 1;
}

void ParseCSS::addName(QString name, int line) {
    if (!isValidName(name) || isColor(name)) return;
    nameIndexesIterator = nameIndexes.find(name.toStdString());
    if (nameIndexesIterator != nameIndexes.end()) return;
    ParseResultName nm;
    nm.name = name;
    nm.line = line;
    result.names.append(nm);
    nameIndexes[name.toStdString()] = result.names.size() - 1;
}

void ParseCSS::addMedia(QString name, int line) {
    if (!isValidName(name)) return;
    mediaIndexesIterator = mediaIndexes.find(name.toStdString());
    if (mediaIndexesIterator != mediaIndexes.end()) return;
    ParseResultMedia media;
    media.name = name;
    media.line = line;
    result.medias.append(media);
    mediaIndexes[name.toStdString()] = result.medias.size() - 1;
}

void ParseCSS::addKeyframe(QString name, int line) {
    if (!isValidName(name)) return;
    keyframeIndexesIterator = keyframeIndexes.find(name.toStdString());
    if (keyframeIndexesIterator != keyframeIndexes.end()) return;
    ParseResultKeyframe keyframe;
    keyframe.name = name;
    keyframe.line = line;
    result.keyframes.append(keyframe);
    keyframeIndexes[name.toStdString()] = result.keyframes.size() - 1;
}

void ParseCSS::addFont(QString name, int line) {
    if (!isValidName(name)) return;
    fontIndexesIterator = fontIndexes.find(name.toStdString());
    if (fontIndexesIterator != fontIndexes.end()) return;
    ParseResultFont font;
    font.name = name;
    font.line = line;
    result.fonts.append(font);
    fontIndexes[name.toStdString()] = result.fonts.size() - 1;
}

void ParseCSS::addComment(QString text, int line) {
    QString name = "";
    if (text.size() > 0) {
        text.replace(QRegularExpression("^[/][*](.*)[*][/]$", QRegularExpression::DotMatchesEverythingOption), "\\1");
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

void ParseCSS::addError(QString text, int line, int symbol) {
    ParseResultError error;
    error.text = text;
    error.line = line;
    error.symbol = symbol;
    result.errors.append(error);
}

void ParseCSS::parseCode(QString & code, QString & origText)
{
    // parse data
    QString current_selector = "";
    QString current_media = "";
    QString current_keyframe = "";
    QString current_font = "";

    int scope = 0;
    int selectorScope = -1, mediaScope = -1, keyframeScope = -1, fontScope = -1;
    int pars = 0;
    int curlyBrackets = 0, roundBrackets = 0, squareBrackets = 0;
    QVector<int> curlyBracketsList, roundBracketsList, squareBracketsList;
    int mediaArgPars = -1;
    int mediaArgsStart = -1;
    int fontFamilyStart = -1;
    int expect = -1;
    QString expectName = "";
    QString prevK = "", prevPrevK = "", prevPrevPrevK = "", prevPrevPrevPrevK = "", prevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevPrevK = "", prevPrevPrevPrevPrevPrevPrevPrevPrevK = "";
    int selectorStart = -1, mediaStart = -1, keyframeStart = -1, fontStart = -1;

    QRegularExpressionMatchIterator mi = parseExpression.globalMatch(code);
    while(mi.hasNext()){
        QRegularExpressionMatch m = mi.next();
        if (m.capturedStart(1) < 0) continue;
        QString k = m.captured(1).trimmed();
        if (k.size() == 0) continue;

        // main tags selectors
        if (expect < 0 && scope == 0 && expect != EXPECT_SELECTOR && k.size() > 1 && (prevK.size() == 0 || prevK == "}") && current_selector.size() == 0) {
            mainTagsIterator = mainTags.find(k.toLower().toStdString());
            if (mainTagsIterator != mainTags.end()) {
                expect = EXPECT_SELECTOR;
                expectName = k;
                selectorStart = m.capturedStart(1);
                current_selector = "";
            }
        } else if (expect == EXPECT_SELECTOR && expectName.size() > 0 && prevK == expectName && (k == "#" || k == ".") && m.capturedStart(1) == selectorStart+expectName.size()) {
            expectName += k;
        } else if (expect == EXPECT_SELECTOR && expectName.size() > 0 && prevPrevK + prevK == expectName && (prevK == "#" || prevK == ".") && k != "," && k != "{") {
            expectName += k;
        } else if (expect == EXPECT_SELECTOR && expectName.size() > 0 && k == "{" && mediaArgPars < 0) {
            current_selector = expectName;
            int line = 0;
            if (selectorStart >= 0) line = getLine(origText, selectorStart);
            addSelector(current_selector, line);
            selectorScope = scope;
            expect = -1;
            expectName = "";
        }

        // ids & classes
        if ((prevK == "#" || prevK == ".") && k.size() > 0 && pars == 0) {
            int line = getLine(origText, m.capturedStart(1));
            QString name = prevK + k;
            addName(name, line);
        }

        // media
        if (expect < 0 && scope == 0 && expect != EXPECT_MEDIA && k.toLower() == "media" && prevK == "@" && current_media.size() == 0) {
            expect = EXPECT_MEDIA;
            expectName = "";
            mediaStart = m.capturedStart(1);
            mediaArgPars = -1;
            mediaArgsStart = -1;
            current_media = "";
        } else if (expect == EXPECT_MEDIA && expectName.size() == 0 && k == "(") {
            mediaArgPars = pars;
            mediaArgsStart = m.capturedStart(1);
        } else if (expect == EXPECT_MEDIA && expectName.size() > 0 && k == "{" && mediaArgPars < 0) {
            current_media = expectName;
            int line = 0;
            if (mediaStart >= 0) line = getLine(origText, mediaStart);
            addMedia(current_media, line);
            mediaScope = scope;
            mediaArgPars = -1;
            mediaArgsStart = -1;
            expect = -1;
            expectName = "";
        }

        // keyframes
        if (expect < 0 && scope == 0 && expect != EXPECT_KEYFRAMES && prevK.toLower() == "keyframes" && prevPrevK == "@" && k.size() > 0 && current_keyframe.size() == 0) {
            expect = EXPECT_KEYFRAMES;
            expectName = k;
            keyframeStart = m.capturedStart(1);
            current_keyframe = "";
        } else if (expect == EXPECT_KEYFRAMES && expectName.size() > 0 && k == "{" && mediaArgPars < 0) {
            current_keyframe = expectName;
            int line = 0;
            if (keyframeStart >= 0) line = getLine(origText, keyframeStart);
            addKeyframe(current_keyframe, line);
            keyframeScope = scope;
            expect = -1;
            expectName = "";
        }

        // font-face
        if (expect < 0 && scope == 0 && expect != EXPECT_FONT_FACE && prevK.toLower() == "font-face" && prevPrevK == "@" && k == "{" && current_font.size() == 0) {
            expect = EXPECT_FONT_FACE;
            expectName = "";
            fontStart = m.capturedStart(1);
            fontScope = scope;
            fontFamilyStart = -1;
            current_font = "";
        } else if (expect != EXPECT_FONT_FACE && current_font.size() == 0 && fontScope >= 0 && fontFamilyStart < 0 && prevK.toLower() == "font-family" && k == ":") {
            expect = EXPECT_FONT_FAMILY;
            fontFamilyStart = m.capturedStart(1);
        } else if (expect == EXPECT_FONT_FAMILY && current_font.size() == 0 && k == ";" && fontFamilyStart >= 0 &&  mediaArgPars < 0) {
            current_font = origText.mid(fontFamilyStart+1, m.capturedStart(1)-fontFamilyStart-1).trimmed().replace("\"","").replace("'","").replace(QRegularExpression("[\\s]+"), " ");
            int line = 0;
            if (fontStart >= 0) line = getLine(origText, fontStart);
            addFont(current_font, line);
            expect = -1;
            expectName = "";
        }

        if ((k == ";" || k == "{" || k == "}") && mediaArgsStart < 0) {
            expect = -1;
            expectName = "";
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
            // selector close
            if (current_selector.size() > 0 && selectorScope >= 0 && selectorScope == scope) {
                current_selector = "";
                selectorScope = -1;
            }
            // media close
            if (current_media.size() > 0 && mediaScope >= 0 && mediaScope == scope) {
                current_media = "";
                mediaScope = -1;
            }
            // keyframes close
            if (current_keyframe.size() > 0 && keyframeScope >= 0 && keyframeScope == scope) {
                current_keyframe = "";
                keyframeScope = -1;
            }
            // font-face close
            if (fontScope >= 0 && fontScope == scope) {
                current_font = "";
                fontScope = -1;
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
            // media args
            if (mediaArgPars >= 0 && mediaArgPars == pars && mediaArgsStart >= 0 && expectName.size() == 0) {
                expectName = origText.mid(mediaArgsStart+1, m.capturedStart(1)-mediaArgsStart-1).trimmed().replace(QRegularExpression("[\\s]+"), " ");
                mediaArgPars = -1;
                mediaArgsStart = -1;
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

void ParseCSS::reset()
{
    selectorIndexes.clear();
    mediaIndexes.clear();
    keyframeIndexes.clear();
    fontIndexes.clear();
    comments.clear();
}

ParseCSS::ParseResult ParseCSS::parse(QString text)
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
