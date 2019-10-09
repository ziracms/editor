/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef GIT_H
#define GIT_H

#include <QObject>
#include "settings.h"

class Git : public QObject
{
    Q_OBJECT
public:
    explicit Git(Settings * settings, QObject *parent = nullptr);
    bool isCommandSafe(QString command);
    void showStatus(QString path);
    void showLog(QString path);
    void showLastCommitDiffTree(QString path);
    void showUncommittedDiffAll(QString path);
    void showUncommittedDiffCurrent(QString path, QString fileName);
    void showLastCommitDiffAll(QString path);
    void showLastCommitDiffCurrent(QString path, QString fileName);
    void resetAll(QString path);
    void resetCurrent(QString path, QString fileName);
    void resetHardUncommitted(QString path);
    void resetHardToPreviousCommit(QString path);
    void revertLastCommit(QString path);
    void addAll(QString path);
    void addCurrent(QString path, QString fileName);
    void commit(QString path, QString msg);
    void pushOriginMaster(QString path);
    void pullOriginMaster(QString path);
    QString highlightCommand(QString & text);
    QString highlightOutput(QString & output);
protected:
    QString cmdTpl;
    QString lineTpl;
    QString errorTpl;
    QString msgTpl;
    QString infoTpl;
signals:
    void runGitCommand(QString path, QString command, QStringList attrs);
public slots:
};

#endif // GIT_H
