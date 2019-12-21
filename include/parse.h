/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef PARSE_H
#define PARSE_H

#include <QRegularExpression>

class Parse
{
public:
    Parse();
    virtual ~Parse();
    virtual void prepare(QString & text);
    virtual QString strip(QRegularExpressionMatch & match, QString & text, int group);
    virtual int getLine(QString & text, int offset);
    virtual QString getLineText(QString & text, int offset);
    virtual int getFirstNotEmptyLineTo(QString & text, int offset);
    virtual int findOpenScope(QVector<int> list);
    virtual int findCloseScope(QVector<int> list);
protected:
    QRegularExpression stringDQExpression;
    QRegularExpression stringSQExpression;
    QRegularExpression commentMLExpression;
    QRegularExpression backtickExpression;
};

#endif // PARSE_H
