/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef PARSERWORKER_H
#define PARSERWORKER_H

#include <QObject>
#include <QDir>
#include "settings.h"
#include "parsephp.h"
#include "parsejs.h"
#include "parsecss.h"
#include "types.h"

extern const QString PHP_WEBSERVER_URI;

class ParserWorker : public QObject
{
    Q_OBJECT
public:
    explicit ParserWorker(Settings * settings, QObject *parent = nullptr);
    ~ParserWorker();
protected:
    void parseProjectDir(QString dir, QStringList & files);
    void parseProjectFile(QString file, QVariantMap & map);
    void searchInDir(QString searchDirectory, QString searchText, QString searchExtensions, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp, QStringList excludeDirs);
    void searchInFile(QString file, QString searchText, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp);
    void searchInFilesResultFound(QString file, QString lineText, int line, int symbol);
    void quickFindInDir(QString startDir, QString dir, QString text);
    bool createAndroidDirectory(QDir rootDir, QString path);
    bool setAndroidFilePermissions(QFile &f);
    bool setAndroidFilePermissions(QString path);
    bool installAndroidFile(QString fileName, QString installDir);
    bool installAndroidPackFiles();
    bool isAndroidPackInstalled();
    void setAndroidBinPaths();
private:
    QString phpPath;
    QString gitPath;
    QString bashPath;
    QString sasscPath;
    QString phpcsPath;
    QString phpcsStandard;
    int phpcsErrorSeverity;
    int phpcsWarningSeverity;
    std::string encoding;
    std::string encodingFallback;
    bool enabled;
    int searchResultsCount;
    bool searchBreaked;
    bool isBusy;
    int quickResultsCount;
    bool quickBreaked;
    bool wantStop;
    QString androidHomePath;
    QStringList androidBinFiles;
    QStringList androidGitFiles;
    QStringList androidOtherFiles;
    qint64 phpWebServerPid;
signals:
    void lintFinished(int tabIndex, QStringList errorTexts, QStringList errorLines, QString output);
    void execPHPFinished(int tabIndex, QString output);
    void execPHPWebServerFinished(bool success, QString output);
    void phpcsFinished(int tabIndex, QStringList errorTexts, QStringList errorLines);
    void parseMixedFinished(int tabIndex, ParsePHP::ParseResult result);
    void parseJSFinished(int tabIndex, ParseJS::ParseResult result);
    void parseCSSFinished(int tabIndex, ParseCSS::ParseResult result);
    void parseProjectFinished(bool success = true, bool isModified = true);
    void parseProjectProgress(int v);
    void searchInFilesFound(QString file, QString lineText, int line, int symbol);
    void searchInFilesFinished();
    void message(QString text);
    void gitCommandFinished(QString command, QString output, bool outputResult = true);
    void serversCommandFinished(QString output);
    void sassCommandFinished(QString output);
    void quickFound(QString text, QString info, QString file, int line);
    void activateProgress();
    void deactivateProgress();
    void activateProgressInfo(QString text);
    void deactivateProgressInfo();
    void updateProgressInfo(QString text);
    void installAndroidPackFinished(QString result);
public slots:
    void disable();
    void lint(int tabIndex, QString path);
    void execPHP(int tabIndex, QString path);
    void execSelection(int tabIndex, QString text);
    void startPHPWebServer(QString path);
    void stopPHPWebServer();
    void phpcs(int tabIndex, QString path);
    void parseMixed(int tabIndex, QString text);
    void parseJS(int tabIndex, QString text);
    void parseCSS(int tabIndex, QString text);
    void parseProject(QString path);
    void searchInFiles(QString searchDirectory, QString searchText, QString searchExtensions, bool searchOptionCase, bool searchOptionWord, bool searchOptionRegexp, QStringList excludeDirs);
    void gitCommand(QString path, QString command, QStringList attrs, bool outputResult = true, bool silent = false);
    void serversCommand(QString command, QString pwd);
    void sassCommand(QString src, QString dst);
    void quickFind(QString dir, QString text, WordsMapList words, QStringList wordPrefixes);
    void cancelRequested();
    void installAndroidPack();
};

#endif // PARSERWORKER_H
