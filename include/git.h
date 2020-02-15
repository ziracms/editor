/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef GIT_H
#define GIT_H

#include <QObject>
#include <QHash>
#include "settings.h"

extern const QString GIT_DIRECTORY;
extern const QString GIT_STATUS_COMMAND;
extern const QString GIT_ANNOTATION_COMMAND;
extern const QString GIT_DIFF_COMMAND;
extern const QString GIT_COMMIT_COMMAND;
extern const QString GIT_PUSH_COMMAND;
extern const QString GIT_PULL_COMMAND;

class Git : public QObject
{
    Q_OBJECT
public:
    explicit Git(Settings * settings, QObject *parent = nullptr);
    bool isCommandSafe(QString command);
    void showStatus(QString path);
    void showStatusShort(QString path, bool outputResult = true);
    void showLog(QString path);
    void showLastCommitDiffTree(QString path);
    void showUncommittedDiffAll(QString path);
    void showUncommittedDiffCurrent(QString path, QString fileName);
    void showUncommittedDiffCurrentUnified(QString path, QString fileName, bool outputResult = true);
    void showLastCommitDiffAll(QString path);
    void showLastCommitDiffCurrent(QString path, QString fileName);
    void resetAll(QString path);
    void resetCurrent(QString path, QString fileName);
    void resetHardUncommitted(QString path);
    void resetToPreviousCommit(QString path);
    void resetHardToPreviousCommit(QString path);
    void revertLastCommit(QString path);
    void addAll(QString path);
    void addCurrent(QString path, QString fileName);
    void commit(QString path, QString msg);
    void addAndCommit(QString path, QString msg);
    void pushOriginMaster(QString path);
    void pullOriginMaster(QString path);
    void showAnnotation(QString path, QString fileName, bool outputResult = true);
    QString highlightCommand(QString & text);
    QString highlightOutput(QString & output);
    struct Annotation {
        int line;
        QString author;
        QString authorDate;
        QString committer;
        QString committerDate;
        QString comment;
        QString commitID;
        QString file;
    };
    QHash<int,Annotation> parseAnnotationOutput(QString & output);
    struct DiffLine {
        int line;
        bool isDeleted;
        bool isModified;
        QString file;
    };
    QHash<int,DiffLine> parseDiffUnifiedOutput(QString & output, QString & fileName);
protected:
    QString cmdTpl;
    QString lineTpl;
    QString errorTpl;
    QString msgTpl;
    QString infoTpl;
signals:
    void runGitCommand(QString path, QString command, QStringList attrs, bool outputResult = true);
public slots:
};

#endif // GIT_H
