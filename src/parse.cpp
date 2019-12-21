/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "parse.h"
#include "helper.h"
#include <QVector>

Parse::Parse()
{
    stringDQExpression = QRegularExpression("(?:^|[^\\\\])[\"](.*?[^\\\\])[\"]", QRegularExpression::DotMatchesEverythingOption);
    stringSQExpression = QRegularExpression("(?:^|[^\\\\])[\'](.*?[^\\\\])[\']", QRegularExpression::DotMatchesEverythingOption);
    commentMLExpression = QRegularExpression("[/][*](.+?)[*][/]", QRegularExpression::DotMatchesEverythingOption);
    backtickExpression = QRegularExpression("(?:^|[^\\\\])[`](.*?[^\\\\])[`]", QRegularExpression::DotMatchesEverythingOption);
}

Parse::~Parse()
{

}

void Parse::prepare(QString & text)
{
    text.replace("\\\\", "  ");
    text.replace(QRegularExpression("(^|[^\\\\])['][']"), "\\1  ");
    text.replace(QRegularExpression("(^|[^\\\\])[\"][\"]"), "\\1  ");
    text.replace(QRegularExpression("(^|[^\\\\])[`][`]"), "\\1  ");
}

QString Parse::strip(QRegularExpressionMatch & match, QString & text, int group)
{
    QString stripped = "";
    if (match.capturedStart(group) >= 0) {
        QString r = match.captured(group);
        r.replace(QRegularExpression("[^\n]"), " ");
        stripped = text.mid(match.capturedStart(group), match.capturedLength(group));
        text.replace(match.capturedStart(group), match.capturedLength(group), r);
    }
    return stripped;
}

int Parse::getLine(QString & text, int offset)
{
    QString textPart = text.mid(0, offset);
    return textPart.count("\n") + 1;
}

QString Parse::getLineText(QString & text, int offset)
{
    int preP = text.lastIndexOf("\n", offset-text.size());
    int postP = text.indexOf("\n", offset);
    if (postP < 0) postP = text.size();
    return text.mid(preP + 1, postP - preP - 1);
}

int Parse::getFirstNotEmptyLineTo(QString & text, int offset)
{
    QString textPart = text.mid(0, offset);
    int offsetPart = textPart.lastIndexOf("\n");
    int line = 0;
    while(offsetPart > 0) {
        offsetPart--;
        QChar c = textPart[offsetPart];
        if (!c.isSpace()) {
            line = getLine(textPart, offsetPart+1);
            break;
        }
    }
    return line;
}

int Parse::findOpenScope(QVector<int> list)
{
    int s = 0;
    for (int i=list.size()-1; i>=0; i--) {
        int v = list.at(i);
        if (v < 0) {
            s--;
        } else if (v > 0) {
            s++;
        }
        if (s > 0) {
            return v;
        }
    }
    return !list.isEmpty() ? list.last() : 0;
}

int Parse::findCloseScope(QVector<int> list)
{
    int s = 0;
    for (int i=0; i<list.size(); i++) {
        int v = list.at(i);
        if (v > 0) {
            s++;
        } else if (v < 0) {
            s--;
        }
        if (s < 0) {
            return v;
        }
    }
    return !list.isEmpty() ? list.last() : 0;
}
