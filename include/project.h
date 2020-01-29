/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include "parsephp.h"
#include "completewords.h"
#include "highlightwords.h"
#include "helpwords.h"

extern const QString PROJECT_SUBDIR;
extern const int PROJECT_LOAD_DELAY;

class Project : public QObject
{
    Q_OBJECT
public:
    explicit Project(QObject *parent = nullptr);
    bool create(QString name, QString path, bool lintEnabled, bool csEnabled, bool gitEnabled);
    bool edit(QString name, QString path, bool lintEnabled, bool csEnabled, QStringList openTabFiles, QList<int> openTabLines, int currentTabIndex, QString todo);
    static bool exists(QString path);
    bool open(QString path);
    void save(QStringList openTabFiles, QList<int> openTabLines, int currentTabIndex, QString todo);
    void close();
    bool isOpen();
    QString getName();
    QString getPath();
    bool isPHPLintEnabled();
    bool isPHPCSEnabled();
    void loadWords(CompleteWords * CW, HighlightWords * HW, HelpWords * HPW);
    void deleteDataFile();
    void findDeclaration(QString name, QString & path, int & line);
    static QVariantMap createPHPResultMap();
    static QVariantMap loadPHPDataMap(QString path);
    static bool isProjectFile(QString path);
    static void checkParsePHPFilesModified(QStringList & files, QVariantMap & data, QVariantMap & map);
    static void parsePHPResult(ParsePHP::ParseResult result, QVariantMap & map, QString path, QString lastModified);
    static void savePHPResults(QString path, QVariantMap & map);
    std::unordered_map<std::string, std::string> phpFunctionDeclarations;
    std::unordered_map<std::string, std::string>::iterator phpFunctionDeclarationsIterator;
    std::unordered_map<std::string, std::string> phpClassMethodDeclarations;
    std::unordered_map<std::string, std::string>::iterator phpClassMethodDeclarationsIterator;
    std::unordered_map<std::string, std::string> phpClassDeclarations;
    std::unordered_map<std::string, std::string>::iterator phpClassDeclarationsIterator;
protected:
    void reset();
    void loadPHPWords(QString project_dir, CompleteWords * CW, HighlightWords * HW, HelpWords * HPW);
    bool updateMetaFile(QString name, QString path, bool lintEnabled, bool csEnabled, QString time_created, QString time_modified, QStringList openTabFiles, QList<int> openTabLines, int currentTabIndex, QString todo);
    static std::unordered_map<std::string, std::string> getPHPFilesMapFromData(QVariantMap & data);
    static void preparePHPResults(QString project_dir, QVariantMap & map);
    static void preparePHPClasses(QString cls_name, QVariantMap cls, QString & cls_args, QString & class_consts_str, QString & class_properties_str, QString & class_methods_str, QString & class_method_types_str, QString & class_methods_search_str, QString & class_methods_help_str,std::unordered_map<std::string, std::string> & cls_methods_map, std::unordered_map<std::string, std::string> & cls_props_map, std::unordered_map<std::string, std::string> & cls_consts_map);
private:
    QString projectName;
    QString projectPath;
    QString projectCreated;
    QString projectModified;
    bool projectPHPLintEnabled;
    bool projectPHPCSEnabled;
signals:
    void openTabsRequested(QStringList openTabFiles, bool initHighlight);
    void gotoTabLinesRequested(QList<int> openTabLines);
    void switchToTabRequested(int currentTabIndex);
    void closeAllTabsRequested(void);
    void showTodoRequested(QString text);
public slots:
};

#endif // PROJECT_H
