/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef QUICKACCESS_H
#define QUICKACCESS_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include "settings.h"
#include "parsephp.h"
#include "parsejs.h"
#include "parsecss.h"

class QuickAccess : public QFrame
{
    Q_OBJECT
public:
    explicit QuickAccess(Settings * settings, QWidget *parent = nullptr);
    ~QuickAccess();
    QSize sizeHint() const override;
    void display(int x, int y, int width, int height);
    void setParseResult(ParsePHP::ParseResult result, QString file);
    void setParseResult(ParseJS::ParseResult result, QString file);
    void setParseResult(ParseCSS::ParseResult result, QString file);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void restoreResults();
    QLineEdit * findEdit;
    QListWidget * resultsList;
    QVBoxLayout * vLayout;
private:
    QString lastSearch;
    ParsePHP::ParseResult parseResultPHP;
    ParseJS::ParseResult parseResultJS;
    ParseCSS::ParseResult parseResultCSS;
    QString parseResultFile;
    int parseResultType;
    bool findLocked;
signals:
    void quickAccessRequested(QString file, int line);
    void quickFindRequested(QString text);
public slots:
    void resultsListItemActivated(QListWidgetItem * item);
    void findTextReturned();
    void findTextChanged(QString text);
    void quickFound(QString text, QString info, QString file, int line);
    void findTextDelayed();
};

#endif // QUICKACCESS_H
