/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "git.h"
#include <QRegularExpression>
#include "helper.h"

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

void Git::pushOriginMaster(QString path)
{
    emit runGitCommand(path, "push", QStringList() << "origin" << "master");
}

void Git::pullOriginMaster(QString path)
{
    emit runGitCommand(path, "pull", QStringList() << "origin" << "master");
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
