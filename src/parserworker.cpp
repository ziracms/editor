/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "parserworker.h"
#include <QProcess>
#include <QRegularExpression>
#include <QDirIterator>
#include <QDateTime>
#include <QCoreApplication>
#include "helper.h"
#include "project.h"
#include "servers.h"
#include "git.h"

ParserWorker::ParserWorker(Settings * settings, QObject *parent) : QObject(parent)
{
    // php path
    phpPath = "";
    QString phpPathStr = QString::fromStdString(settings->get("parser_php_path"));
    if (phpPathStr.size() == 0) {
        QProcess process(this);
        process.start("which", QStringList() << "php");
        if (!process.waitForFinished()) return;
        QByteArray result = process.readAllStandardOutput();
        phpPathStr = QString(result).trimmed();
    }
    if (phpPathStr.size() > 0 && Helper::fileOrFolderExists(phpPathStr)) {
        phpPath = phpPathStr;
    }
    // git path
    gitPath = "";
    QString gitPathStr = QString::fromStdString(settings->get("parser_git_path"));
    if (gitPathStr.size() == 0) {
        QProcess process(this);
        process.start("which", QStringList() << "git");
        if (!process.waitForFinished()) return;
        QByteArray result = process.readAllStandardOutput();
        gitPathStr = QString(result).trimmed();
    }
    if (gitPathStr.size() > 0 && Helper::fileOrFolderExists(gitPathStr)) {
        gitPath = gitPathStr;
    }
    // bash path
    bashPath = "";
    QString bashPathStr = QString::fromStdString(settings->get("parser_bash_path"));
    if (bashPathStr.size() == 0) {
        QProcess process(this);
        process.start("which", QStringList() << "bash");
        if (!process.waitForFinished()) return;
        QByteArray result = process.readAllStandardOutput();
        bashPathStr = QString(result).trimmed();
    }
    if (bashPathStr.size() > 0 && Helper::fileOrFolderExists(bashPathStr)) {
        bashPath = bashPathStr;
    }
    // sassc path
    sasscPath = "";
    QString sasscPathStr = QString::fromStdString(settings->get("parser_sassc_path"));
    if (sasscPathStr.size() == 0) {
        QProcess process(this);
        process.start("which", QStringList() << "sassc");
        if (!process.waitForFinished()) return;
        QByteArray result = process.readAllStandardOutput();
        sasscPathStr = QString(result).trimmed();
    }
    if (sasscPathStr.size() > 0 && Helper::fileOrFolderExists(sasscPathStr)) {
        sasscPath = sasscPathStr;
    }
    // phpcs path
    phpcsPath = "";
    QString phpcsPathStr = QString::fromStdString(settings->get("parser_phpcs_path"));
    if (phpcsPathStr.size() == 0) {
        QProcess process(this);
        process.start("which", QStringList() << "phpcs");
        if (!process.waitForFinished()) return;
        QByteArray result = process.readAllStandardOutput();
        phpcsPathStr = QString(result).trimmed();
    }
    if (phpcsPathStr.size() > 0 && Helper::fileOrFolderExists(phpcsPathStr)) {
        phpcsPath = phpcsPathStr;
    }
    phpcsStandard = QString::fromStdString(settings->get("parser_phpcs_standard"));
    if (phpcsStandard.size() == 0) phpcsStandard = "PEAR";
    phpcsErrorSeverity = std::stoi(settings->get("parser_phpcs_error_severity"));
    if (phpcsErrorSeverity < 0) phpcsErrorSeverity = 0;
    phpcsWarningSeverity = std::stoi(settings->get("parser_phpcs_warning_severity"));
    if (phpcsWarningSeverity < 0) phpcsWarningSeverity = 0;
    encoding = settings->get("editor_encoding");
    encodingFallback = settings->get("editor_fallback_encoding");
    enabled = true;
    searchResultsCount = 0;
    searchBreaked = false;
    isBusy = false;
    quickResultsCount = 0;
    quickBreaked = false;
}

void ParserWorker::disable()
{
    enabled = false;
}

void ParserWorker::lint(int tabIndex, QString path)
{
    if (phpPath.size() == 0) return; // silence
    QStringList errorTexts, errorLines;
    QProcess process(this);
    process.start(phpPath, QStringList() << "-n" << "-l" << "-f" << path);
    if (!process.waitForFinished()) return;
    QByteArray result = process.readAllStandardOutput();
    QString errors = QString(result).trimmed();
    if (errors.size() > 0 && errors.indexOf("No syntax errors")==0) errors = "";
    if (errors.size() > 0) {
        QRegularExpression errReg = QRegularExpression("(.+?)[ ][i][n][ ].+?[ ][o][n][ ][l][i][n][e][ ](\\d+)");
        QRegularExpressionMatch errMatch;
        int offset = 0;
        do {
            errMatch = errReg.match(errors, offset);
            if (errMatch.capturedStart() >= 0) {
                errorTexts.append(errMatch.captured(1));
                errorLines.append(errMatch.captured(2));
                offset = errMatch.capturedStart() + errMatch.capturedLength();
            }
        } while(errMatch.capturedStart() >= 0);
    }
    emit lintFinished(tabIndex, errorTexts, errorLines, errors);
}

void ParserWorker::phpcs(int tabIndex, QString path)
{
    if (phpcsPath.size() == 0) return; //silence
    QStringList errorTexts, errorLines;
    QProcess process(this);
    process.start(phpcsPath, QStringList() << "--standard="+phpcsStandard << "--error-severity="+Helper::intToStr(phpcsErrorSeverity) << "--warning-severity="+Helper::intToStr(phpcsWarningSeverity) << "--report=csv" << path);
    if (!process.waitForFinished()) return;
    QByteArray result = process.readAllStandardOutput();
    QString errors = QString(result).trimmed();
    if (errors.indexOf("ERROR:")==0) return;
    if (errors.size() > 0) {
        errors.replace("\"\"", "'");
        QRegularExpression errReg = QRegularExpression("[\"](.+?)[\"]");
        QRegularExpressionMatch errMatch;
        int offset = 0;
        do {
            errMatch = errReg.match(errors, offset);
            if (errMatch.capturedStart() >= 0) {
                errors.replace(errMatch.capturedStart(), errMatch.capturedLength(), errMatch.captured(1).replace(",",";"));
                offset = errMatch.capturedStart() + errMatch.capturedLength() - 2;
            }
        } while(errMatch.capturedStart() >= 0);
        QStringList errorsList = errors.split("\n");
        if (errorsList.size() > 1) {
            int lineIndex = -1, messageIndex = -1;
            QString errorsListHeader = errorsList.at(0);
            QStringList errorsListHeaderList = errorsListHeader.split(",");
            for (int i=0; i<errorsListHeaderList.size(); i++) {
                if (errorsListHeaderList.at(i) == "Line") lineIndex = i;
                //if (errorsListHeaderList.at(i) == "Column") columnIndex = i;
                if (errorsListHeaderList.at(i) == "Message") messageIndex = i;
                //if (errorsListHeaderList.at(i) == "Severity") severityIndex = i;
            }
            for (int i=1; i<errorsList.size(); i++) {
                QString errorsListLine = errorsList.at(i);
                QStringList errorsListLineList = errorsListLine.split(",");
                if (lineIndex >= 0 && lineIndex < errorsListLineList.size()) errorLines.append(errorsListLineList.at(lineIndex));
                if (messageIndex >= 0 && messageIndex < errorsListLineList.size()) errorTexts.append(errorsListLineList.at(messageIndex));
            }
        }
    }
    emit phpcsFinished(tabIndex, errorTexts, errorLines);
}

void ParserWorker::parseMixed(int tabIndex, QString text)
{
    ParsePHP parser;
    ParsePHP::ParseResult result = parser.parse(text);
    emit parseMixedFinished(tabIndex, result);
}

void ParserWorker::parseJS(int tabIndex, QString text)
{
    ParseJS parser;
    ParseJS::ParseResult result = parser.parse(text);
    emit parseJSFinished(tabIndex, result);
}

void ParserWorker::parseCSS(int tabIndex, QString text)
{
    ParseCSS parser;
    ParseCSS::ParseResult result = parser.parse(text);
    emit parseCSSFinished(tabIndex, result);
}

void ParserWorker::parseProject(QString path)
{
    if (isBusy) {
        emit message(tr("Worker is busy. Please wait..."));
        return;
    }
    isBusy = true;
    QStringList files;
    emit activateProgress();
    parseProjectDir(path, files);
    QVariantMap map = Project::createPHPResultMap();
    QVariantMap data = Project::loadPHPDataMap(path);
    Project::checkParsePHPFilesModified(files, data, map);
    for (int i=0; i<files.size(); i++) {
        if (!enabled) break;
        QString file = files.at(i);
        parseProjectFile(file, map);
        int v = (i + 1) * 100 / files.size();
        emit parseProjectProgress(v);
    }
    Project::savePHPResults(path, map);
    map.clear();
    data.clear();
    files.clear();
    emit parseProjectFinished();
    emit deactivateProgress();
    isBusy = false;
}

void ParserWorker::parseProjectDir(QString dir, QStringList & files)
{
    QDirIterator it(dir, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fInfo(path);
        if (!fInfo.exists() || !fInfo.isReadable()) continue;
        if (fInfo.isDir()) {
            parseProjectDir(path, files);
        } else if (fInfo.isFile() && Project::isProjectFile(path)) {
            files.append(path);
        }
    }
}

void ParserWorker::parseProjectFile(QString file, QVariantMap & map)
{
    QCoreApplication::processEvents();
    if (!Helper::fileExists(file)) return;
    QString content = Helper::loadTextFile(file, encoding, encodingFallback, true);
    ParsePHP parser;
    ParsePHP::ParseResult result = parser.parse(content);
    QFileInfo fInfo(file);
    QDateTime dt = fInfo.lastModified();
    QString dts = QString::number(dt.toMSecsSinceEpoch());
    Project::parsePHPResult(result, map, file, dts);
}

void ParserWorker::searchInFiles(QString searchDirectory, QString searchText, QString searchExtensions, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp, QStringList excludeDirs)
{
    if (isBusy) {
        emit message(tr("Worker is busy. Please wait..."));
        return;
    }
    isBusy = true;
    if (!Helper::folderExists(searchDirectory) || searchText.size() == 0) return;
    emit activateProgress();
    QString allowedExtensions = "";
    QStringList searchExtensionsList = searchExtensions.split(",");
    for (int i=0; i<searchExtensionsList.size(); i++) {
        QString searchExtension = searchExtensionsList.at(i);
        searchExtension = searchExtension.trimmed();
        if (searchExtension.size() > 0 && searchExtension[0] == "*") searchExtension = searchExtension.mid(1);
        if (searchExtension == ".*") continue;
        if (searchExtension.size() > 1 && searchExtension[0] == "." && allowedExtensions.indexOf("*"+searchExtension+";") < 0) {
            allowedExtensions += "*"+searchExtension+";";
        }
    }
    searchResultsCount = 0;
    searchBreaked = false;
    searchInDir(searchDirectory, searchText, allowedExtensions, searchOptionCase, searchOptionWord, searchOptionRegexp, excludeDirs);
    emit searchInFilesFinished();
    emit deactivateProgress();
    isBusy = false;
}

void ParserWorker::searchInDir(QString searchDirectory, QString searchText, QString searchExtensions, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp, QStringList excludeDirs)
{
    QDirIterator it(searchDirectory, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        if (!enabled) break;
        if (searchBreaked) break;
        QString path = it.next();
        QFileInfo fInfo(path);
        if (!fInfo.exists() || !fInfo.isReadable()) continue;
        if (fInfo.isDir() && (fInfo.fileName() == ".git" || fInfo.fileName() == PROJECT_SUBDIR || fInfo.fileName() == ".idea" || fInfo.fileName() == ".vscode" || fInfo.fileName() == "nbproject")) continue;
        if (fInfo.isDir() && excludeDirs.contains(fInfo.absoluteFilePath())) continue;
        if (fInfo.isDir()) {
            searchInDir(path, searchText, searchExtensions, searchOptionCase, searchOptionWord, searchOptionRegexp, excludeDirs);
        } else if (fInfo.isFile()) {
            if (searchExtensions.size() > 0) {
                int p = path.lastIndexOf(".");
                if (p < 0) continue;
                QString ext = path.mid(p);
                if (ext.size() == 0) continue;
                if (searchExtensions.indexOf("*"+ext+";") < 0) continue;
            }
            searchInFile(path, searchText, searchOptionCase, searchOptionWord, searchOptionRegexp);
        }
    }
}

void ParserWorker::searchInFile(QString file, QString searchText, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp)
{
    QCoreApplication::processEvents();
    if (!Helper::fileExists(file) || searchText.size() == 0) return;
    QString content = Helper::loadTextFile(file, encoding, encodingFallback, true);
    Parse parser;
    if (!searchOptionWord && !searchOptionRegexp) {
        // regular search
        int p = -1, offset = 0;
        Qt::CaseSensitivity cs = searchOptionCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
        do {
            if (searchBreaked) break;
            p = content.indexOf(searchText, offset, cs);
            if (p >= 0) {
                offset = p + searchText.size();
                int line = parser.getLine(content, p);
                QString lineText = parser.getLineText(content, p);
                searchInFilesResultFound(file, lineText, line, p);
            }
        } while (p >= 0);
    } else if (searchOptionWord) {
        // search whole words
        int p = -1, offset = 0;
        QRegularExpression::PatternOptions opt = QRegularExpression::NoPatternOption;
        if (!searchOptionCase) opt = QRegularExpression::CaseInsensitiveOption;
        QRegularExpression regexp = QRegularExpression("\\b"+QRegularExpression::escape(searchText)+"\\b", opt);
        if (!regexp.isValid()) return;
        do {
            if (searchBreaked) break;
            QRegularExpressionMatch match = regexp.match(content, offset);
            p = match.capturedStart();
            if (p >= 0) {
                offset = p + match.capturedLength();
                int line = parser.getLine(content, p);
                QString lineText = parser.getLineText(content, p);
                searchInFilesResultFound(file, lineText, line, p);
            }
        } while (p >= 0);
    } else if (searchOptionRegexp) {
        // search regexp
        int p = -1, offset = 0;
        QRegularExpression::PatternOptions opt = QRegularExpression::NoPatternOption;
        if (!searchOptionCase) opt = QRegularExpression::CaseInsensitiveOption;
        QRegularExpression regexp = QRegularExpression(searchText, opt);
        if (!regexp.isValid()) return;
        do {
            if (searchBreaked) break;
            QRegularExpressionMatch match = regexp.match(content, offset);
            p = match.capturedStart();
            if (p >= 0) {
                offset = p + match.capturedLength();
                int line = parser.getLine(content, p);
                QString lineText = parser.getLineText(content, p);
                searchInFilesResultFound(file, lineText, line, p);
            }
        } while (p >= 0);
    }
}

void ParserWorker::searchInFilesResultFound(QString file, QString lineText, int line, int symbol)
{
    searchResultsCount++;
    emit searchInFilesFound(file, lineText, line, symbol);
    if (searchResultsCount >= 1000) {
        searchBreaked = true;
        emit searchInFilesFound("", tr("Too many results. Search stopped."), -1, -1);
    }
}

void ParserWorker::gitCommand(QString path, QString command, QStringList attrs, bool outputResult)
{
    if (gitPath.size() == 0) {
        emit message(tr("Git not found."));
        return;
    }
    if (path.size() == 0 || !Helper::folderExists(path)) return;
    bool useProgress = false;
    if (command == GIT_PUSH_COMMAND || command == GIT_PULL_COMMAND) useProgress = true;
    if (useProgress && !isBusy) emit activateProgress();
    QProcess process(this);
    process.setWorkingDirectory(path);
    process.start(gitPath, QStringList() << command << attrs);
    if (!process.waitForFinished(300000)) {
        if (useProgress && !isBusy) emit deactivateProgress();
        return;
    }
    QString result = QString(process.readAllStandardOutput());
    if (result.size() == 0) result = QString(process.readAllStandardError());
    emit gitCommandFinished(command, result, outputResult);
    if (useProgress && !isBusy) emit deactivateProgress();
}

void ParserWorker::serversCommand(QString command, QString pwd)
{
    if (bashPath.size() == 0) {
        emit message(tr("Bash not found."));
        return;
    }
    if (command.size() == 0) return;
    if (!isBusy) emit activateProgress();
    QString errorApache = "";
    // apache2
    QProcess processApache(this);
    processApache.start(bashPath, QStringList() << "-c" << Servers::generateApacheServiceCommand(command, pwd));
    if (!processApache.waitForFinished(300000)) {
        if (!isBusy) emit deactivateProgress();
        return;
    }
    QString outputApache = QString(processApache.readAllStandardOutput());
    if (outputApache.size() == 0) errorApache = QString(processApache.readAllStandardError());
    if (errorApache.size() > 0) {
        QStringList errorApacheList = errorApache.split("\n");
        errorApache = "";
        for (int i=0; i<errorApacheList.size(); i++) {
            QString error = errorApacheList.at(i).trimmed();
            if (error == "[sudo] password for root:") continue;
            if (error == "sudo: 1 incorrect password attempt") continue;
            if (error.indexOf("[sudo] password for root:") == 0) error = error.mid(25).trimmed();
            if (error.size() == 0) continue;
            if (errorApache.size() > 0)  errorApache += "\n";
            errorApache += error;
        }
    }
    if (errorApache.size() > 0) {
        emit serversCommandFinished(errorApache.trimmed());
        if (!isBusy) emit deactivateProgress();
        return;
    }
    // mariadb
    QString errorMariadb = "";
    QProcess processMariadb(this);
    processMariadb.start(bashPath, QStringList() << "-c" << Servers::generateMariaDBServiceCommand(command, pwd));
    if (!processMariadb.waitForFinished(300000)) {
        if (!isBusy) emit deactivateProgress();
        return;
    }
    QString outputMariadb = QString(processMariadb.readAllStandardOutput());
    if (outputMariadb.size() == 0) errorMariadb = QString(processMariadb.readAllStandardError());
    if (errorMariadb.size() > 0) {
        QStringList outputMariadbList = errorMariadb.split("\n");
        errorMariadb = "";
        for (int i=0; i<outputMariadbList.size(); i++) {
            QString error = outputMariadbList.at(i).trimmed();
            if (error == "[sudo] password for root:") continue;
            if (error == "sudo: 1 incorrect password attempt") continue;
            if (error.indexOf("[sudo] password for root:") == 0) error = error.mid(25).trimmed();
            if (error.size() == 0) continue;
            if (errorMariadb.size() > 0)  errorMariadb += "\n";
            errorMariadb += error;
        }
    }
    if (errorMariadb.size() > 0) {
        emit serversCommandFinished(outputApache.trimmed() + "\n\n" + errorMariadb.trimmed());
        if (!isBusy) emit deactivateProgress();
        return;
    }
    if (command != SERVERS_STATUS_CMD) {
        //if (!isBusy) emit deactivateProgress();
        serversCommand(SERVERS_STATUS_CMD, pwd);
        return;
    }
    emit serversCommandFinished(outputApache.trimmed() + "\n\n" + outputMariadb.trimmed());
    if (!isBusy) emit deactivateProgress();
}

void ParserWorker::sassCommand(QString src, QString dst)
{
    if (sasscPath.size() == 0) {
        emit message(tr("Sassc not found."));
        return;
    }
    if (src.size() == 0 || dst.size() == 0) return;

    if (!isBusy) emit activateProgress();
    QProcess process(this);
    process.start(sasscPath, QStringList() << src << dst);
    if (!process.waitForFinished()) {
        if (!isBusy) emit deactivateProgress();
        return;
    }
    QString error = QString(process.readAllStandardError());

    emit sassCommandFinished(error);
    if (!isBusy) emit deactivateProgress();
}

void ParserWorker::quickFind(QString dir, QString text, WordsMapList words, QStringList wordPrefixes)
{
    if (!isBusy) emit activateProgress();
    quickResultsCount = 0;
    quickBreaked = false;
    // words
    int it = 0;
    for (auto wordsList : words) {
        //QCoreApplication::processEvents();
        if (!enabled) break;
        QString prefix = "";
        if (it < wordPrefixes.size()) prefix = wordPrefixes.at(it);
        it++;
        int co = 0;
        // words
        for (auto it : wordsList) {
            QString name = QString::fromStdString(it.first);
            if (name.indexOf(text, 0, Qt::CaseInsensitive) >= 0) {
                QString dec_str = QString::fromStdString(it.second);
                int p = dec_str.lastIndexOf(":");
                if (p >= 0) {
                    QString path = dec_str.mid(0, p).trimmed();
                    int line = dec_str.mid(p+1).toInt();
                    emit quickFound(text, prefix+name, path, line);
                    co++;
                    if (co >= 100) break;
                }
            }
        }
    }
    // search files
    quickFindInDir(dir, dir, text);
    if (!isBusy) emit deactivateProgress();
}

void ParserWorker::quickFindInDir(QString startDir, QString dir, QString text)
{
    QString prefix = "file: ";
    QDirIterator it(dir, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        //QCoreApplication::processEvents();
        if (!enabled) break;
        if (quickBreaked) break;
        QString path = it.next();
        QFileInfo fInfo(path);
        if (!fInfo.exists() || !fInfo.isReadable()) continue;
        if (fInfo.isDir() && (fInfo.fileName() == ".git" || fInfo.fileName() == PROJECT_SUBDIR || fInfo.fileName() == ".idea" || fInfo.fileName() == ".vscode" || fInfo.fileName() == "nbproject")) continue;
        if (fInfo.isDir()) {
            quickFindInDir(startDir, path, text);
        } else if (fInfo.isFile()) {
            if (fInfo.fileName().indexOf(text, 0, Qt::CaseInsensitive) >= 0) {
                QString info = path;
                if (info.size() > startDir.size()) info = info.mid(startDir.size()+1);
                emit quickFound(text, prefix+info, path, 1);
                quickResultsCount++;
                if (quickResultsCount >= 100) {
                    quickBreaked = true;
                    break;
                }
            }
        }
    }
}
