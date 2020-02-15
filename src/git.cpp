/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "git.h"
#include <QRegularExpression>
#include <QDateTime>

const QString GIT_DIRECTORY = ".git";

const QString GIT_STATUS_COMMAND = "status";
const QString GIT_ANNOTATION_COMMAND = "blame";
const QString GIT_DIFF_COMMAND = "diff";
const QString GIT_COMMIT_COMMAND = "commit";
const QString GIT_PUSH_COMMAND = "push";
const QString GIT_PULL_COMMAND = "pull";

Git::Git(Settings * settings, QObject *parent) : QObject(parent)
{
    cmdTpl = "<div><b>%1</b> %2</div><div>&nbsp;</div>";
    lineTpl = "<div>%1</div>";
    errorTpl = "<span style=\"color:"+QString::fromStdString(settings->get("git_output_error_color"))+"\">%1</span>";
    msgTpl = "<span style=\"color:"+QString::fromStdString(settings->get("git_output_message_color"))+"\">%1</span>";
    infoTpl = "<span style=\"color:"+QString::fromStdString(settings->get("git_output_info_color"))+"\">%1</span>";
}

bool Git::isCommandSafe(QString command)
{
    if (command == "reset" || command == "revert" || command == "pull") return false;
    return true;
}

void Git::showStatus(QString path)
{
    emit runGitCommand(path, "status", QStringList());
}

void Git::showStatusShort(QString path, bool outputResult)
{
    emit runGitCommand(path, "status", QStringList() << "--short" << "--porcelain", outputResult);
}

void Git::showLog(QString path)
{
    emit runGitCommand(path, "log", QStringList() << "-n" << "100");
}

void Git::showLastCommitDiffTree(QString path)
{
    emit runGitCommand(path, "diff-tree", QStringList() << "--name-status" << "--no-commit-id" << "-r" << "HEAD");
}

void Git::showUncommittedDiffAll(QString path)
{
    emit runGitCommand(path, "diff", QStringList());
}

void Git::showUncommittedDiffCurrent(QString path, QString fileName)
{
    emit runGitCommand(path, "diff", QStringList() << fileName);
}

void Git::showUncommittedDiffCurrentUnified(QString path, QString fileName, bool outputResult)
{
    emit runGitCommand(path, "diff", QStringList() << "--unified=0" << "--no-color" << fileName, outputResult);
}

void Git::showLastCommitDiffAll(QString path)
{
    emit runGitCommand(path, "diff", QStringList() << "-r" << "HEAD~" << "HEAD");
}

void Git::showLastCommitDiffCurrent(QString path, QString fileName)
{
    emit runGitCommand(path, "diff", QStringList() <<  "-r" << "HEAD~" << "HEAD" << fileName);
}

void Git::resetAll(QString path)
{
    emit runGitCommand(path, "reset", QStringList());
}

void Git::resetCurrent(QString path, QString fileName)
{
    emit runGitCommand(path, "reset", QStringList() << fileName);
}

void Git::resetHardUncommitted(QString path)
{
    emit runGitCommand(path, "reset", QStringList() << "--hard" << "HEAD");
}

void Git::resetToPreviousCommit(QString path)
{
    emit runGitCommand(path, "reset", QStringList() << "HEAD~");
}

void Git::resetHardToPreviousCommit(QString path)
{
    emit runGitCommand(path, "reset", QStringList() << "--hard" << "HEAD~");
}

void Git::revertLastCommit(QString path)
{
    emit runGitCommand(path, "revert", QStringList() << "HEAD");
}

void Git::addAll(QString path)
{
    emit runGitCommand(path, "add", QStringList() << ".");
}

void Git::addCurrent(QString path, QString fileName)
{
    emit runGitCommand(path, "add", QStringList() << fileName);
}

void Git::commit(QString path, QString msg)
{
    emit runGitCommand(path, "commit", QStringList() << "-m" << msg);
}

void Git::addAndCommit(QString path, QString msg)
{
    addAll(path);
    commit(path, msg);
}

void Git::pushOriginMaster(QString path)
{
    emit runGitCommand(path, "push", QStringList() << "origin" << "master");
}

void Git::pullOriginMaster(QString path)
{
    emit runGitCommand(path, "pull", QStringList() << "origin" << "master");
}

void Git::showAnnotation(QString path, QString fileName, bool outputResult)
{
    emit runGitCommand(path, "blame", QStringList() <<  "--line-porcelain" << fileName, outputResult);
}

QString Git::highlightCommand(QString & text)
{
    int p = text.indexOf(" ");
    if (p <= 0) return text;
    QString path = text.mid(0, p);
    QString cmd = text.mid(p+1);
    return cmdTpl.arg(path).arg(infoTpl.arg(cmd));
}

QString Git::highlightOutput(QString & output)
{
    QString result = "";
    QStringList outputList = output.split("\n");
    for (int i=0; i<outputList.size(); i++) {
        QString lineText = outputList.at(i);
        lineText.replace("<", "&lt;");
        lineText.replace(">", "&gt;");
        // skip qt messages
        if (lineText == "Icon theme \"ubuntu-mono-dark\" not found.") continue;
        if (lineText == "Icon theme \"Mint-X\" not found.") continue;
        if (lineText == "Icon theme \"elementary\" not found.") continue;
        if (lineText == "Icon theme \"gnome\" not found.") continue;

        if (lineText == "Changes not staged for commit:" || lineText == "Untracked files:" || lineText == "Unstaged changes after reset:") {
            lineText = errorTpl.arg(lineText);
        } else if (lineText == "Changes to be committed:") {
            lineText = msgTpl.arg(lineText);
        } else if (lineText.indexOf("---") == 0) {
            lineText = infoTpl.arg(lineText);
        } else if (lineText.indexOf("+++") == 0) {
            lineText = infoTpl.arg(lineText);
        } else if (lineText.indexOf("-") == 0) {
            lineText = errorTpl.arg(lineText);
        } else if (lineText.indexOf("+") == 0) {
            lineText = msgTpl.arg(lineText);
        } else if (lineText.indexOf("commit ") == 0) {
            lineText = infoTpl.arg(lineText);
        } else if (lineText.indexOf("Author: ") == 0 || lineText.indexOf("Date: ") == 0) {
            lineText = msgTpl.arg(lineText);
        } else if (lineText.indexOf("no changes added to commit") == 0) {
            lineText = infoTpl.arg(lineText);
        } else if (lineText.indexOf("fatal:") == 0 || lineText.indexOf("error:") == 0 || lineText.indexOf("CONFLICT") == 0 || lineText.indexOf("failed") >= 0) {
            lineText = errorTpl.arg(lineText);
        } else if (lineText.indexOf("hint:") == 0) {
            lineText = infoTpl.arg(lineText);
        }
        lineText.replace("\t", "    ");
        lineText.replace("  ", "&nbsp;&nbsp;");
        if (lineText.isEmpty()) lineText = "&nbsp;";
        result += lineTpl.arg(lineText);
    }
    return result;
}

QHash<int, Git::Annotation> Git::parseAnnotationOutput(QString & output)
{
    QHash<int, Git::Annotation> annotations;
    if (output.size() > 0) {
        int p = 0, offset = 0;
        QString lineStr;
        bool isAnnotationStart = true, isAnnotationEnd = false;
        int line  = -1;
        QString k = "", v = "";
        QDateTime ts;
        QString author = "", authorDate = "", committer = "", committerDate = "", comment = "", commitID = "", file = "";
        do {
            p = output.indexOf("\n", offset);
            if (p < 0) {
                lineStr = output.mid(offset).trimmed();
            } else {
                lineStr = output.mid(offset, p-offset).trimmed();
                offset = p + 1;
            }
            if (!isAnnotationStart && isAnnotationEnd) {
                isAnnotationStart = true;
                isAnnotationEnd = false;
                continue;
            }
            int s = lineStr.indexOf(" ");
            if (s < 0) continue; // boundary ?
            if (isAnnotationStart) {
                isAnnotationStart = false;
                author = ""; authorDate = ""; committer = ""; committerDate = ""; comment = ""; commitID = ""; file = "";
                QStringList lineStrParts = lineStr.split(" ");
                if (lineStrParts.size() < 3 || lineStrParts.at(0).size() != 40) break; // something wrong
                commitID = lineStrParts.at(0);
                line = lineStrParts.at(2).toInt();
                continue;
            }
            k = lineStr.mid(0, s);
            v = lineStr.mid(s+1);
            if (k == "author") {
                author += v;
            } else if (k == "author-mail") {
                author += v;
            } else if (k == "author-time") {
                ts.setTime_t(v.toUInt());
                authorDate += ts.toString("yyyy, MMMM d");
            } else if (k == "author-tz") {
                //authorDate += " "+v;
            } else if (k == "committer") {
                committer += v;
            } else if (k == "committer-mail") {
                committer += v;
                if (v == "<not.committed.yet>") {
                    committer = "<"+tr("you")+">";
                    comment = tr("not committed yet");
                }
            } else if (k == "committer-time") {
                ts.setTime_t(v.toUInt());
                committerDate += ts.toString("yyyy, MMMM d");
            } else if (k == "committer-tz") {
                //committerDate += " "+v;
            } else if (k == "summary" && comment.size() == 0) {
                comment = v;
            } else if (k == "filename") {
                file = v;
                isAnnotationEnd = true;
            }
            if (isAnnotationEnd) {
                Git::Annotation annotation;
                annotation.line = line;
                annotation.author = author;
                annotation.authorDate = authorDate;
                annotation.committer = committer;
                annotation.committerDate = committerDate;
                annotation.comment = comment;
                annotation.commitID = commitID;
                annotation.file = file;
                annotations.insert(line, annotation);
                continue;
            }
        } while(p >= 0);
    }
    return annotations;
}

QHash<int, Git::DiffLine> Git::parseDiffUnifiedOutput(QString & output, QString & fileName)
{
    QHash<int,Git::DiffLine> mLines;
    QStringList lines = output.split("\n");
    QRegularExpression expr("^[@][@]\\s([-][0-9,]+)\\s([+][0-9,]+)\\s[@][@]");
    QRegularExpressionMatch m;
    QString delPart = "", addPart = "";
    int delLine = 0, addLine = 0, delCount = 0, addCount = 0, p = -1, lineInt = 0;
    QString fileA = "", fileB = "";
    for (QString line : lines) {
        if (line.indexOf("--- a/") == 0 && line.size() > 6 && fileA.size() == 0) fileA = line.mid(6);
        if (line.indexOf("+++ b/") == 0 && line.size() > 6 && fileB.size() == 0) fileB = line.mid(6);
        if (fileA.size() == 0 || fileB.size() == 0) continue;
        if (fileA != fileB) break; // something wrong
        fileName = fileB;
        if (line.indexOf("@@") != 0) continue;
        m = expr.match(line);
        if (m.capturedStart(0) != 0) continue;
        delPart = line.mid(m.capturedStart(1), m.capturedLength(1));
        addPart = line.mid(m.capturedStart(2), m.capturedLength(2));
        if (delPart.indexOf("-") != 0) continue;
        delPart = delPart.mid(1);
        if (addPart.indexOf("+") != 0) continue;
        addPart = addPart.mid(1);
        p = delPart.indexOf(",");
        if (p > 0) {
            delLine = delPart.mid(0, p).toInt();
            delCount = delPart.mid(p+1).toInt();
        } else {
            delLine = delPart.toInt();
            delCount = 1;
        }
        p = addPart.indexOf(",");
        if (p > 0) {
            addLine = addPart.mid(0, p).toInt();
            addCount = addPart.mid(p+1).toInt();
        } else {
            addLine = addPart.toInt();
            addCount = 1;
        }
        if (delLine <= 0 || addLine <= 0) continue;
        if (addCount > 0) {
            for (int i=0; i<addCount; i++) {
                lineInt = addLine + i;
                Git::DiffLine mLine;
                mLine.line = lineInt;
                mLine.isDeleted = false;
                mLine.isModified = true;
                mLine.file = fileB;
                mLines.insert(lineInt, mLine);
            }
        } else if (delCount > addCount) {
            lineInt = delLine;
            Git::DiffLine mLine;
            mLine.line = lineInt;
            mLine.isDeleted = true;
            mLine.isModified = false;
            mLine.file = fileB;
            mLines.insert(lineInt, mLine);
        }
    }
    return mLines;
}
