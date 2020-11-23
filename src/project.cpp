/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "project.h"
#include "parsephp.h"
#include "helper.h"
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>

const std::string PROJECT_DATA_ENCODING = "UTF-8";
const QString PROJECT_SUBDIR = ".zira";
const QString PROJECT_META_FILE = "meta.json";
const QString PROJECT_PHP_DATA_FILE = "php_data.json";
const QString PROJECT_PHP_CONSTS_FILE = "php_consts";
const QString PROJECT_PHP_CLASS_CONSTS_FILE = "php_class_consts";
const QString PROJECT_PHP_VARS_FILE = "php_globals";
const QString PROJECT_PHP_CLASS_PROPS_FILE = "php_class_props";
const QString PROJECT_PHP_FUNCTIONS_FILE = "php_functions";
const QString PROJECT_PHP_CLASS_METHODS_FILE = "php_class_methods";
const QString PROJECT_PHP_CLASSES_FILE = "php_classes";
const QString PROJECT_PHP_CLASS_PARENTS_FILE = "php_class_parents";
const QString PROJECT_PHP_CLASS_METHOD_TYPES_FILE = "php_class_method_types";
const QString PROJECT_PHP_FUNCTION_TYPES_FILE = "php_function_types";
const QString PROJECT_PHP_FUNCTIONS_SEARCH_FILE = "php_functions_search";
const QString PROJECT_PHP_CLASS_METHODS_SEARCH_FILE = "php_class_methods_search";
const QString PROJECT_PHP_CLASSES_SEARCH_FILE = "php_classes_search";
const QString PROJECT_PHP_CLASS_METHODS_HELP_FILE = "php_class_methods_help";
const QString PROJECT_PHP_FUNCTIONS_HELP_FILE = "php_functions_help";

const QString PARSE_PROJECT_PHP_EXT = "php";
const QString GIT_IGNORE_FILE = ".gitignore";

const int PROJECT_LOAD_DELAY = 500;

Project::Project(){}

Project& Project::instance()
{
    static Project _instance;
    return _instance;
}

void Project::init()
{
    CW = &CompleteWords::instance();
    HPW = &HelpWords::instance();
    reset();
}

void Project::reset()
{
    projectName = "";
    projectPath = "";
    projectCreated = "";
    projectModified = "";
    projectPHPLintEnabled = false;
    projectPHPCSEnabled = false;
    phpFunctionDeclarations.clear();
    phpClassMethodDeclarations.clear();
    phpClassDeclarations.clear();
}

bool Project::isOpen()
{
    if (projectName.size() > 0 && projectPath.size() > 0) return true;
    return false;
}

void Project::deleteDataFile()
{
    if (!isOpen()) return;
    QString project_dir = projectPath + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return;
    }
    QString data_filename = project_dir + "/" + PROJECT_PHP_DATA_FILE;
    if (!Helper::fileExists(data_filename)) {
        return;
    }
    Helper::deleteFile(data_filename);
}

bool Project::updateMetaFile(QString name, QString path, bool lintEnabled, bool csEnabled, QString time_created, QString time_modified, QStringList openTabFiles, QList<int> openTabLines, int currentTabIndex, QString todo)
{
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return false;
    }
    QString meta_filename = project_dir + "/" + PROJECT_META_FILE;
    if (!Helper::fileExists(meta_filename)) {
        return false;
    }
    QVariantMap m;
    m.insert("name", name);
    m.insert("path", path);
    m.insert("phplint_enabled", lintEnabled);
    m.insert("phpcs_enabled", csEnabled);
    m.insert("time_created", time_created);
    m.insert("time_modified", time_modified);
    m.insert("tabs", openTabFiles);
    QList<QVariant> lines;
    for (int i=0; i< openTabLines.size(); i++) {
        int line = openTabLines.at(i);
        lines.append(QVariant(line));
    }
    m.insert("tab_lines", QVariant(lines));
    m.insert("tab_index", currentTabIndex);
    m.insert("todo", todo);
    QJsonObject o = QJsonObject::fromVariantMap(m);
    QJsonDocument d;
    d.setObject(o);
    QString data = QString(d.toJson());
    if (!Helper::saveTextFile(meta_filename, data, PROJECT_DATA_ENCODING)) {
        return false;
    }
    return true;
}

bool Project::create(QString name, QString path, bool lintEnabled, bool csEnabled, bool gitEnabled)
{
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (Helper::folderExists(project_dir) || !Helper::createDir(project_dir)) {
        return false;
    }
    QString meta_filename = project_dir + "/" + PROJECT_META_FILE;
    if (Helper::fileExists(meta_filename) || !Helper::createFile(meta_filename)) {
        return false;
    }
    if (!Helper::folderExists(project_dir)) {
        return false;
    }
    if (!Helper::fileExists(meta_filename)) {
        return false;
    }
    QDateTime dt = QDateTime::currentDateTime();
    QString time = dt.toString("MMMM d, yyyy - hh:mm:ss");
    QStringList openTabFiles;
    QList<int> openTabLines;
    int currentTabIndex = -1;
    QString todo = "";
    if (!updateMetaFile(name, path, lintEnabled, csEnabled, time, time, openTabFiles, openTabLines, currentTabIndex, todo)) {
        return false;
    }
    // creating .gitignore
    if (gitEnabled) {
        if (!Helper::fileExists(path + "/" + GIT_IGNORE_FILE)) {
            QString gitIgnore = PROJECT_SUBDIR + "/\n";
            gitIgnore += ".idea/\n";
            gitIgnore += ".vscode/\n";
            gitIgnore += "nbproject/\n";
            Helper::saveTextFile(path + "/" + GIT_IGNORE_FILE, gitIgnore, PROJECT_DATA_ENCODING);
        } else {
            QString gitIgnore = Helper::loadTextFile(path + "/" + GIT_IGNORE_FILE, PROJECT_DATA_ENCODING, PROJECT_DATA_ENCODING, true);
            QRegularExpression expr("(?:^|\n)"+QRegularExpression::escape(PROJECT_SUBDIR+"/")+"(?:\n|$)");
            QRegularExpressionMatch m = expr.match(gitIgnore);
            if (m.capturedStart() < 0) {
                gitIgnore = gitIgnore.trimmed();
                gitIgnore += "\n" + PROJECT_SUBDIR + "/\n";
                Helper::saveTextFile(path + "/" + GIT_IGNORE_FILE, gitIgnore, PROJECT_DATA_ENCODING);
            }
        }
    }
    return true;
}

bool Project::edit(QString name, QString path, bool lintEnabled, bool csEnabled, QStringList openTabFiles, QList<int> openTabLines, int currentTabIndex, QString todo)
{
    if (!isOpen()) return false;
    if (name.size() == 0 || projectPath != path) return false;
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return false;
    }
    QString meta_filename = project_dir + "/" + PROJECT_META_FILE;
    if (!Helper::fileExists(meta_filename)) {
        return false;
    }
    QDateTime dt = QDateTime::currentDateTime();
    QString time = dt.toString("MMMM d, yyyy - hh:mm:ss");
    if (!updateMetaFile(name, projectPath, lintEnabled, csEnabled, projectCreated, time, openTabFiles, openTabLines, currentTabIndex, todo)) {
        return false;
    }
    projectName = name;
    projectPHPLintEnabled = lintEnabled;
    projectPHPCSEnabled = csEnabled;
    projectModified = time;
    return true;
}

bool Project::exists(QString path)
{
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return false;
    }
    QString meta_filename = project_dir + "/" + PROJECT_META_FILE;
    if (!Helper::fileExists(meta_filename)) {
        return false;
    }
    return true;
}

bool Project::open(QString path)
{
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return false;
    }
    QString meta_filename = project_dir + "/" + PROJECT_META_FILE;
    if (!Helper::fileExists(meta_filename)) {
        return false;
    }
    QString data = Helper::loadTextFile(meta_filename, PROJECT_DATA_ENCODING, PROJECT_DATA_ENCODING, true);
    QJsonDocument d = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject o = d.object();
    QJsonValue nameVal = o.value("name");
    QString name = nameVal.toString();
    QJsonValue pathVal = o.value("path");
    QString _path = pathVal.toString();
    if (path != _path || name.size() == 0) {
        return false;
    }
    QJsonValue lintEnabledVal = o.value("phplint_enabled");
    bool lintEnabled = lintEnabledVal.toBool();
    QJsonValue csEnabledVal = o.value("phpcs_enabled");
    bool csEnabled = csEnabledVal.toBool();
    QJsonValue createdVal = o.value("time_created");
    QString time_created = createdVal.toString();
    QJsonValue modifiedVal = o.value("time_modified");
    QString time_modified = modifiedVal.toString();
    QJsonValue tabsVal = o.value("tabs");
    QJsonArray tabsArr = tabsVal.toArray();
    QStringList openTabFiles;
    for (int i=0; i< tabsArr.size(); i++) {
        QJsonValue tabVal = tabsArr.at(i);
        QString tabStr = tabVal.toString();
        openTabFiles.append(tabStr);
    }
    QJsonValue linesVal = o.value("tab_lines");
    QJsonArray linesArr = linesVal.toArray();
    QList<int> openTabLines;
    for (int i=0; i< linesArr.size(); i++) {
        QJsonValue lineVal = linesArr.at(i);
        int line= lineVal.toInt();
        openTabLines.append(line);
    }
    QJsonValue tabIndexVal = o.value("tab_index");
    int tabIndex = tabIndexVal.toInt();
    QJsonValue todoVal = o.value("todo");
    QString todo = todoVal.toString();
    reset();
    projectName = name;
    projectPath = path;
    projectPHPLintEnabled = lintEnabled;
    projectPHPCSEnabled = csEnabled;
    projectCreated = time_created;
    projectModified = time_modified;
    emit openTabsRequested(openTabFiles, false);
    if (openTabLines.size() == openTabFiles.size()) {
        emit gotoTabLinesRequested(openTabLines);
    }
    if (tabIndex >= 0 && tabIndex < openTabFiles.size()) {
        emit switchToTabRequested(tabIndex);
    }
    emit showTodoRequested(todo);
    return true;
}

void Project::close()
{
    if (!isOpen()) return;
    reset();
    emit closeAllTabsRequested();
}

void Project::save(QStringList openTabFiles, QList<int> openTabLines, int currentTabIndex, QString todo)
{
    if (!isOpen()) return;
    QDateTime dt = QDateTime::currentDateTime();
    QString time = dt.toString("MMMM d, yyyy - hh:mm:ss");
    updateMetaFile(projectName, projectPath, projectPHPLintEnabled, projectPHPCSEnabled, projectCreated, time, openTabFiles, openTabLines, currentTabIndex, todo);
}

QString Project::getName()
{
    return projectName;
}

QString Project::getPath()
{
    return projectPath;
}

bool Project::isPHPLintEnabled()
{
    return projectPHPLintEnabled;
}

bool Project::isPHPCSEnabled()
{
    return projectPHPCSEnabled;
}

void Project::loadWords()
{
    if (!isOpen()) return;
    QString project_dir = projectPath + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return;
    }
    loadPHPWords(project_dir);
}

void Project::loadPHPWords(QString project_dir)
{
    QString k;

    // php functions
    QFile ff(project_dir + "/" + PROJECT_PHP_FUNCTIONS_FILE);
    ff.open(QIODevice::ReadOnly);
    QTextStream fin(&ff);
    while (!fin.atEnd()) {
        k = fin.readLine();
        if (k == "") continue;
        QString kName = "", kParams = "";
        int kSep = k.indexOf("(");
        if (kSep > 0) {
            kName = k.mid(0, kSep).trimmed();
            kParams = k.mid(kSep).trimmed();
            CW->phpFunctionsComplete[kName.toStdString()] = kParams.toStdString();
            CW->tooltipsPHP[kName.toStdString()] = kParams.replace("<", "&lt;").replace(">", "&gt;").toStdString();
            //HighlightWords::addPHPFunction(kName);
        } else {
            CW->phpFunctionsComplete[k.toStdString()] = k.toStdString();
            //HighlightWords::addPHPFunction(k);
        }
    }
    ff.close();

    // php consts
    QFile cnf(project_dir + "/" + PROJECT_PHP_CONSTS_FILE);
    cnf.open(QIODevice::ReadOnly);
    QTextStream cnin(&cnf);
    while (!cnin.atEnd()) {
        k = cnin.readLine();
        if (k == "") continue;
        CW->phpConstsComplete[k.toStdString()] = k.toStdString();
        HighlightWords::addPHPConstant(k);
    }
    cnf.close();

    // php classes
    QFile cf(project_dir + "/" + PROJECT_PHP_CLASSES_FILE);
    cf.open(QIODevice::ReadOnly);
    QTextStream cin(&cf);
    while (!cin.atEnd()) {
        k = cin.readLine();
        if (k == "") continue;
        QString kName = "", kParams = "";
        int kSep = k.indexOf("(");
        if (kSep > 0) {
            kName = k.mid(0, kSep).trimmed();
            kParams = k.mid(kSep).trimmed();
            CW->phpClassesComplete[kName.toStdString()] = kParams.toStdString();
            CW->tooltipsPHP[kName.toStdString()] = kParams.replace("<", "&lt;").replace(">", "&gt;").toStdString();
            QStringList classParts = kName.split("\\");
            for (int i=0; i<classParts.size(); i++) {
                QString classPart = classParts.at(i);
                if (classPart.size() == 0) continue;
                HighlightWords::addPHPClass(classPart);
            }
        } else {
            CW->phpClassesComplete[k.toStdString()] = k.toStdString();
            QStringList classParts = k.split("\\");
            for (int i=0; i<classParts.size(); i++) {
                QString classPart = classParts.at(i);
                if (classPart.size() == 0) continue;
                HighlightWords::addPHPClass(classPart);
            }
        }
    }
    cf.close();

    // php class methods
    QFile mf(project_dir + "/" + PROJECT_PHP_CLASS_METHODS_FILE);
    mf.open(QIODevice::ReadOnly);
    QTextStream min(&mf);
    while (!min.atEnd()) {
        k = min.readLine();
        if (k == "") continue;
        QString kName = "", kParams = "";
        int kSep = k.indexOf("(");
        if (kSep > 0) {
            kName = k.mid(0, kSep).trimmed();
            kParams = k.mid(kSep).trimmed();
            CW->phpClassMethodsComplete[kName.toStdString()] = kParams.toStdString();
            CW->tooltipsPHP[kName.toStdString()] = kParams.replace("<", "&lt;").replace(">", "&gt;").toStdString();
            //HighlightWords::addPHPFunction(kName);
        } else {
            CW->phpClassMethodsComplete[k.toStdString()] = k.toStdString();
            //HighlightWords::addPHPFunction(k);
        }
    }
    mf.close();

    // php class consts
    QFile cof(project_dir + "/" + PROJECT_PHP_CLASS_CONSTS_FILE);
    cof.open(QIODevice::ReadOnly);
    QTextStream coin(&cof);
    while (!coin.atEnd()) {
        k = coin.readLine();
        if (k == "") continue;
        CW->phpClassConstsComplete[k.toStdString()] = k.toStdString();
        QStringList kParts = k.split("::");
        if (kParts.size() == 2) HighlightWords::addPHPClassConstant(kParts.at(0), kParts.at(1));
    }
    cof.close();

    // php class props
    QFile pof(project_dir + "/" + PROJECT_PHP_CLASS_PROPS_FILE);
    pof.open(QIODevice::ReadOnly);
    QTextStream poin(&pof);
    while (!poin.atEnd()) {
        k = poin.readLine();
        if (k == "") continue;
        CW->phpClassPropsComplete[k.toStdString()] = k.toStdString();
    }
    pof.close();

    // php class parents
    QFile cpf(project_dir + "/" + PROJECT_PHP_CLASS_PARENTS_FILE);
    cpf.open(QIODevice::ReadOnly);
    QTextStream cpin(&cpf);
    while (!cpin.atEnd()) {
        k = cpin.readLine();
        if (k == "") continue;
        QStringList kList = k.split(" ");
        if (kList.size() != 2) continue;
        CW->phpClassParents[kList.at(0).toStdString()] = kList.at(1).toStdString();
    }
    cpf.close();

    // php function types
    QFile ftf(project_dir + "/" + PROJECT_PHP_FUNCTION_TYPES_FILE);
    ftf.open(QIODevice::ReadOnly);
    QTextStream ftin(&ftf);
    while (!ftin.atEnd()) {
        k = ftin.readLine();
        if (k == "") continue;
        QStringList kList = k.split(" ");
        if (kList.size() != 2) continue;
        CW->phpFunctionTypes[kList.at(0).toStdString()] = kList.at(1).toStdString();
    }
    ftf.close();

    // php class method types
    QFile mtf(project_dir + "/" + PROJECT_PHP_CLASS_METHOD_TYPES_FILE);
    mtf.open(QIODevice::ReadOnly);
    QTextStream mtin(&mtf);
    while (!mtin.atEnd()) {
        k = mtin.readLine();
        if (k == "") continue;
        QStringList kList = k.split(" ");
        if (kList.size() != 2) continue;
        CW->phpClassMethodTypes[kList.at(0).toStdString()] = kList.at(1).toStdString();
    }
    mtf.close();

    QString func, desc;

    // php function descriptions
    QFile hpf(project_dir + "/" + PROJECT_PHP_FUNCTIONS_HELP_FILE);
    hpf.open(QIODevice::ReadOnly);
    QTextStream hpin(&hpf);
    while (!hpin.atEnd()) {
        k = hpin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        HPW->phpFunctionDescs[func.toStdString()] = desc.replace("<br />", "\n").toStdString();
    }
    hpf.close();

    // php class method descriptions
    QFile hcf(project_dir + "/" + PROJECT_PHP_CLASS_METHODS_HELP_FILE);
    hcf.open(QIODevice::ReadOnly);
    QTextStream hcin(&hcf);
    while (!hcin.atEnd()) {
        k = hcin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        HPW->phpClassMethodDescs[func.toStdString()] = desc.replace("<br />", "\n").toStdString();
    }
    hcf.close();

    // php function declarations
    QFile spf(project_dir + "/" + PROJECT_PHP_FUNCTIONS_SEARCH_FILE);
    spf.open(QIODevice::ReadOnly);
    QTextStream spin(&spf);
    while (!spin.atEnd()) {
        k = spin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        phpFunctionDeclarations[func.toStdString()] = desc.toStdString();
    }
    spf.close();

    // php class method declarations
    QFile scf(project_dir + "/" + PROJECT_PHP_CLASS_METHODS_SEARCH_FILE);
    scf.open(QIODevice::ReadOnly);
    QTextStream scin(&scf);
    while (!scin.atEnd()) {
        k = scin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        phpClassMethodDeclarations[func.toStdString()] = desc.toStdString();
    }
    scf.close();

    // php class declarations
    QFile lcf(project_dir + "/" + PROJECT_PHP_CLASSES_SEARCH_FILE);
    lcf.open(QIODevice::ReadOnly);
    QTextStream lcin(&lcf);
    while (!lcin.atEnd()) {
        k = lcin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        phpClassDeclarations[func.toStdString()] = desc.toStdString();
    }
    lcf.close();
}

void Project::findDeclaration(QString name, QString & path, int & line)
{
    if (name.indexOf("::") >= 0) {
        phpClassMethodDeclarationsIterator = phpClassMethodDeclarations.find(name.toStdString());
        if (phpClassMethodDeclarationsIterator != phpClassMethodDeclarations.end()) {
            QString dec_str = QString::fromStdString(phpClassMethodDeclarationsIterator->second);
            int p = dec_str.lastIndexOf(":");
            if (p >= 0) {
                path = dec_str.mid(0, p).trimmed();
                line = dec_str.mid(p+1).toInt();
            }
        }
    } else {
        phpClassDeclarationsIterator = phpClassDeclarations.find(name.toStdString());
        if (phpClassDeclarationsIterator != phpClassDeclarations.end()) {
            QString dec_str = QString::fromStdString(phpClassDeclarationsIterator->second);
            int p = dec_str.lastIndexOf(":");
            if (p >= 0) {
                path = dec_str.mid(0, p).trimmed();
                line = dec_str.mid(p+1).toInt();
            }
        }
        if (path.size() == 0 || line <= 0) {
            if (name.indexOf("\\") >= 0) name = name.mid(name.lastIndexOf("\\")+1);
            phpFunctionDeclarationsIterator = phpFunctionDeclarations.find(name.toStdString());
            if (phpFunctionDeclarationsIterator != phpFunctionDeclarations.end()) {
                QString dec_str = QString::fromStdString(phpFunctionDeclarationsIterator->second);
                int p = dec_str.lastIndexOf(":");
                if (p >= 0) {
                    path = dec_str.mid(0, p).trimmed();
                    line = dec_str.mid(p+1).toInt();
                }
            }
        }
    }
}

QVariantMap Project::createPHPResultMap()
{
    QVariantMap map;
    QVariant php_consts;
    QVariantMap php_globals;
    QVariantMap php_functions;
    QVariantMap php_classes;
    QVariantMap php_files;
    map.insert("php_consts", QVariant(php_consts));
    map.insert("php_globals", QVariant(php_globals));
    map.insert("php_functions", QVariant(php_functions));
    map.insert("php_classes", QVariant(php_classes));
    map.insert("php_files", QVariant(php_files));
    return map;
}

QVariantMap Project::loadPHPDataMap(QString path)
{
    QVariantMap defaultMap;
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return defaultMap;
    }
    QString data_filename = project_dir + "/" + PROJECT_PHP_DATA_FILE;
    if (!Helper::fileExists(data_filename)) {
        return defaultMap;
    }
    QString data = Helper::loadTextFile(data_filename, PROJECT_DATA_ENCODING, PROJECT_DATA_ENCODING, true);
    QJsonDocument d = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject o = d.object();
    return o.toVariantMap();
}

bool Project::isProjectFile(QString path)
{
    int p1 = path.lastIndexOf(".");
    if (p1 < 0) return false;
    QString ext = path.mid(p1+1);
    if (ext.size() == 0) return false;
    int p2 = path.lastIndexOf(".", p1-1);
    if (p2 >= 0) {
        QString _ext = path.mid(p2+1, p1-p2-1);
        if (_ext.size() > 0 && _ext.toLower() == "min") {
            return false;
        }
    }
    if (ext.toLower() != PARSE_PROJECT_PHP_EXT) return false;
    return true;
}

std::unordered_map<std::string, std::string> Project::getPHPFilesMapFromData(QVariantMap & data)
{
    std::unordered_map<std::string, std::string> filesMap;
    QVariant php_files_v = data.value("php_files");
    QVariantMap php_files = qvariant_cast<QVariantMap>(php_files_v);
    for (auto k : php_files.keys()) {
        QVariant lastModifiedV = php_files.value(k);
        QString lastModified = lastModifiedV.toString();
        filesMap[k.toStdString()] = lastModified.toStdString();
    }
    return filesMap;
}

void Project::checkParsePHPFilesModified(QStringList & files, QVariantMap & data, QVariantMap & map)
{
    QVariant php_consts_v = map.value("php_consts");
    QVariantMap php_consts = qvariant_cast<QVariantMap>(php_consts_v);
    QVariant php_globals_v = map.value("php_globals");
    QVariantMap php_globals = qvariant_cast<QVariantMap>(php_globals_v);
    QVariant php_functions_v = map.value("php_functions");
    QVariantMap php_functions = qvariant_cast<QVariantMap>(php_functions_v);
    QVariant php_classes_v = map.value("php_classes");
    QVariantMap php_classes = qvariant_cast<QVariantMap>(php_classes_v);
    QVariant php_files_v = map.value("php_files");
    QVariantMap php_files = qvariant_cast<QVariantMap>(php_files_v);

    std::unordered_map<std::string, std::string> filesMap = getPHPFilesMapFromData(data);
    std::unordered_map<std::string, std::string> filesNotModifiedMap;
    int i=0;
    while (i<files.size()) {
        QString file = files.at(i);
        std::unordered_map<std::string, std::string>::iterator filesMapIt = filesMap.find(file.toStdString());
        if (filesMapIt != filesMap.end()) {
            QFileInfo fInfo(file);
            QDateTime dt = fInfo.lastModified();
            QString dts = QString::number(dt.toMSecsSinceEpoch());
            if (dts.toStdString() == filesMapIt->second) {
                files.removeAt(i);
                php_files.insert(file, QVariant(dts));
                filesNotModifiedMap[file.toStdString()] = dts.toStdString();
                continue;
            }
        }
        i++;
    }

    QVariant _php_consts_v = data.value("php_consts");
    QVariantMap _php_consts = qvariant_cast<QVariantMap>(_php_consts_v);
    QVariant _php_globals_v = data.value("php_globals");
    QVariantMap _php_globals = qvariant_cast<QVariantMap>(_php_globals_v);
    QVariant _php_functions_v = data.value("php_functions");
    QVariantMap _php_functions = qvariant_cast<QVariantMap>(_php_functions_v);
    QVariant _php_classes_v = data.value("php_classes");
    QVariantMap _php_classes = qvariant_cast<QVariantMap>(_php_classes_v);

    for (auto k : _php_consts.keys()) {
        QVariant v = _php_consts.value(k);
        QVariantMap m = qvariant_cast<QVariantMap>(v);
        QString name = m.value("name").toString();
        QString file = m.value("file").toString();
        std::unordered_map<std::string, std::string>::iterator filesNotModifiedMapIt = filesNotModifiedMap.find(file.toStdString());
        if (filesNotModifiedMapIt != filesNotModifiedMap.end()) {
            php_consts.insert(name, m);
        }
    }

    for (auto k : _php_globals.keys()) {
        QVariant v = _php_globals.value(k);
        QVariantMap m = qvariant_cast<QVariantMap>(v);
        QString name = m.value("name").toString();
        QString file = m.value("file").toString();
        std::unordered_map<std::string, std::string>::iterator filesNotModifiedMapIt = filesNotModifiedMap.find(file.toStdString());
        if (filesNotModifiedMapIt != filesNotModifiedMap.end()) {
            php_globals.insert(name, m);
        }
    }

    for (auto k : _php_functions.keys()) {
        QVariant v = _php_functions.value(k);
        QVariantMap m = qvariant_cast<QVariantMap>(v);
        QString name = m.value("name").toString();
        QString file = m.value("file").toString();
        std::unordered_map<std::string, std::string>::iterator filesNotModifiedMapIt = filesNotModifiedMap.find(file.toStdString());
        if (filesNotModifiedMapIt != filesNotModifiedMap.end()) {
            php_functions.insert(name, m);
        }
    }

    for (auto k : _php_classes.keys()) {
        QVariant v = _php_classes.value(k);
        QVariantMap m = qvariant_cast<QVariantMap>(v);
        QString name = m.value("name").toString();
        QString file = m.value("file").toString();
        std::unordered_map<std::string, std::string>::iterator filesNotModifiedMapIt = filesNotModifiedMap.find(file.toStdString());
        if (filesNotModifiedMapIt != filesNotModifiedMap.end()) {
            php_classes.insert(name, m);
        }
    }

    map.insert("php_consts", QVariant(php_consts));
    map.insert("php_globals", QVariant(php_globals));
    map.insert("php_functions", QVariant(php_functions));
    map.insert("php_classes", QVariant(php_classes));
    map.insert("php_files", QVariant(php_files));
}

void Project::parsePHPResult(ParsePHP::ParseResult result, QVariantMap & map, QString path, QString lastModified)
{
    QVariant php_consts_v = map.value("php_consts");
    QVariantMap php_consts = qvariant_cast<QVariantMap>(php_consts_v);
    QVariant php_globals_v = map.value("php_globals");
    QVariantMap php_globals = qvariant_cast<QVariantMap>(php_globals_v);
    QVariant php_functions_v = map.value("php_functions");
    QVariantMap php_functions = qvariant_cast<QVariantMap>(php_functions_v);
    QVariant php_classes_v = map.value("php_classes");
    QVariantMap php_classes = qvariant_cast<QVariantMap>(php_classes_v);
    QVariant php_files_v = map.value("php_files");
    QVariantMap php_files = qvariant_cast<QVariantMap>(php_files_v);

    php_files.insert(path, QVariant(lastModified));

    for (int c=0; c<result.constants.size(); c++) {
        ParsePHP::ParseResultConstant constant = result.constants.at(c);
        if (constant.clsName.size() > 0) continue;
        QString name = constant.name;
        int p = constant.name.lastIndexOf("\\");
        if (p >= 0) name = constant.name.mid(p+1);
        QVariantMap data;
        data.insert("name", constant.name);
        data.insert("value", constant.value);
        data.insert("line", constant.line);
        data.insert("file", path);
        php_consts.insert(constant.name, QVariant(data));
    }
    // variables
    for (int v=0; v<result.variables.size(); v++) {
        ParsePHP::ParseResultVariable variable = result.variables.at(v);
        if (variable.clsName.size() > 0 || variable.funcName.size() > 0) continue;
        QVariantMap data;
        data.insert("name", variable.name);
        data.insert("type", variable.type);
        data.insert("line", variable.line);
        data.insert("visibility", variable.visibility);
        data.insert("file", path);
        php_globals.insert(variable.name, QVariant(data));
    }
    // functions
    for (int f=0; f<result.functions.size(); f++) {
        ParsePHP::ParseResultFunction func = result.functions.at(f);
        if (func.clsName.size() > 0) continue;

        QString synopsis = func.name;
        //if (synopsis.size() > 0 && synopsis.at(0) == "\\") synopsis = synopsis.mid(1);
        int p = synopsis.lastIndexOf("\\");
        if (p >= 0) synopsis = synopsis.mid(p+1);
        if (func.args.size() > 0) synopsis += " ( "+func.args+" )";
        else synopsis += "()";

        QVariantMap data;
        data.insert("name", func.name);
        data.insert("args", func.args);
        data.insert("synopsis", synopsis);
        data.insert("return_type", func.returnType);
        data.insert("line", func.line);
        data.insert("comment", func.comment);
        data.insert("isStatic", func.isStatic);
        data.insert("isAbstract", func.isAbstract);
        data.insert("visibility", func.visibility);
        data.insert("file", path);
        php_functions.insert(func.name, QVariant(data));
    }
    // classes
    for (int i=0; i<result.classes.size(); i++) {
        ParsePHP::ParseResultClass cls = result.classes.at(i);

        QVariantMap data;
        data.insert("name", cls.name);
        data.insert("parent", cls.parent);
        data.insert("isInterface", cls.isInterface);
        data.insert("isTrait", cls.isTrait);
        data.insert("line", cls.line);
        data.insert("isAbstract", cls.isAbstract);
        data.insert("file", path);

        QVariantMap interfacesMap;
        for (int y=0; y<cls.interfaces.size(); y++) {
            interfacesMap.insert(QString::fromStdString(std::to_string(y)), cls.interfaces.at(y));
        }
        data.insert("interfaces", QVariant(interfacesMap));

        // class constants
        QVariantMap constantsMap;
        for (int c=0; c<cls.constantIndexes.size(); c++) {
            if (result.constants.size() <= cls.constantIndexes.at(c)) break;
            ParsePHP::ParseResultConstant constant = result.constants.at(cls.constantIndexes.at(c));

            QVariantMap constantsData;
            constantsData.insert("name", constant.name);
            constantsData.insert("value", constant.value);
            constantsData.insert("line", constant.line);
            constantsData.insert("file", path);
            constantsMap.insert(constant.name, QVariant(constantsData));
        }
        data.insert("constants", QVariant(constantsMap));

        // class variables
        QVariantMap variablesMap;
        for (int v=0; v<cls.variableIndexes.size(); v++) {
            if (result.variables.size() <= cls.variableIndexes.at(v)) break;
            ParsePHP::ParseResultVariable variable = result.variables.at(cls.variableIndexes.at(v));

            QVariantMap variablesData;
            variablesData.insert("name", variable.name);
            variablesData.insert("type", variable.type);
            variablesData.insert("line", variable.line);
            variablesData.insert("visibility", variable.visibility);
            variablesData.insert("file", path);
            variablesMap.insert(variable.name, QVariant(variablesData));
        }
        data.insert("properties", QVariant(variablesMap));

        // class methods
        QVariantMap methodsMap;
        for (int f=0; f<cls.functionIndexes.size(); f++) {
            if (result.functions.size() <= cls.functionIndexes.at(f)) break;
            ParsePHP::ParseResultFunction func = result.functions.at(cls.functionIndexes.at(f));

            QString synopsis = func.name;
            if (func.args.size() > 0) synopsis += " ( "+func.args+" )";
            else synopsis += "()";

            QString returnType = func.returnType;
            if (returnType.size() == 0 && func.comment.size() > 0) {
                QRegularExpression returnExpr = QRegularExpression("[\n]@return (?:[\\?][\\s]*)?([a-zA-Z0-9_\\\\]+)");
                QRegularExpressionMatch returnMatch = returnExpr.match(func.comment);
//                if (returnMatch.capturedStart(1) < 0) {
//                    returnExpr = QRegularExpression("[\n]@return .*[\\[][\\[]([a-zA-Z0-9_\\\\]+?)[\\]][\\]]");
//                    returnMatch = returnExpr.match(func.comment);
//                }
                if (returnMatch.capturedStart(1) > 0) {
                    QString returnStr = returnMatch.captured(1);
                    if (returnStr.at(0) != "\\" && cls.name.indexOf("\\") >= 0) {
                        returnType = cls.name.mid(0, cls.name.lastIndexOf("\\")+1) + returnStr;
                    } else {
                        returnType = returnStr;
                    }
                }
            }

            QVariantMap methodData;
            methodData.insert("name", func.name);
            methodData.insert("args", func.args);
            methodData.insert("synopsis", synopsis);
            methodData.insert("return_type", returnType);
            methodData.insert("line", func.line);
            methodData.insert("comment", func.comment);
            methodData.insert("isStatic", func.isStatic);
            methodData.insert("isAbstract", func.isAbstract);
            methodData.insert("visibility", func.visibility);
            methodData.insert("file", path);
            methodsMap.insert(func.name, QVariant(methodData));
        }
        data.insert("methods", QVariant(methodsMap));

        php_classes.insert(cls.name, QVariant(data));
    }

    map.insert("php_consts", QVariant(php_consts));
    map.insert("php_globals", QVariant(php_globals));
    map.insert("php_functions", QVariant(php_functions));
    map.insert("php_classes", QVariant(php_classes));
    map.insert("php_files", QVariant(php_files));
}

void Project::savePHPResults(QString path, QVariantMap & map)
{
    QString project_dir = path + "/" + PROJECT_SUBDIR;
    if (!Helper::folderExists(project_dir)) {
        return;
    }
    QString meta_filename = project_dir + "/" + PROJECT_META_FILE;
    if (!Helper::fileExists(meta_filename)) {
        return;
    }
    preparePHPResults(project_dir, map);
    QJsonObject o = QJsonObject::fromVariantMap(map);
    QJsonDocument d;
    d.setObject(o);
    QString data = QString(d.toJson());
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_DATA_FILE, data, PROJECT_DATA_ENCODING);
}

void Project::preparePHPResults(QString project_dir, QVariantMap & map)
{
    QString consts_str = "";
    QString globals_str = "";
    QString functions_str = "";
    QString function_types_str = "";
    QString functions_search_str = "";
    QString functions_help_str = "";
    QString classes_str = "";
    QString class_parents_str = "";
    QString classes_search_str = "";
    QString class_consts_str = "";
    QString class_properties_str = "";
    QString class_methods_str = "";
    QString class_method_types_str = "";
    QString class_methods_search_str = "";
    QString class_methods_help_str = "";

    // consts
    QVariant consts_v = map.value("php_consts");
    QVariantMap consts = qvariant_cast<QVariantMap>(consts_v);
    for (auto c : consts.keys()) {
        QVariant const_v = consts.value(c);
        QVariantMap cons = qvariant_cast<QVariantMap>(const_v);
        QString const_name = cons.value("name").toString();
        //if (const_name.size() > 0 && const_name.at(0) == "\\") const_name = const_name.mid(1);
        int p = const_name.lastIndexOf("\\");
        if (p >= 0) const_name = const_name.mid(p+1);
        consts_str += const_name + "\n";
    }
    // variables
    QVariant vars_v = map.value("php_globals");
    QVariantMap vars = qvariant_cast<QVariantMap>(vars_v);
    for (auto v : vars.keys()) {
        QVariant var_v = vars.value(v);
        QVariantMap var = qvariant_cast<QVariantMap>(var_v);
        globals_str += var.value("name").toString() + "\n";
    }
    // functions
    QVariant methods_v = map.value("php_functions");
    QVariantMap methods = qvariant_cast<QVariantMap>(methods_v);
    for (auto m : methods.keys()) {
        QVariant method_v = methods.value(m);
        QVariantMap method = qvariant_cast<QVariantMap>(method_v);
        functions_str += method.value("synopsis").toString() + "\n";
        QString func_name = method.value("name").toString();
        if (func_name.size() > 0 && func_name.at(0) == "\\") func_name = func_name.mid(1);
        QString method_type = method.value("return_type").toString();
        if (method_type.size() > 0 && method_type.at(0) == "\\") method_type = method_type.mid(1);
        if (method_type.size() > 0) {
            function_types_str += func_name + " " + method_type + "\n";
        }
        functions_search_str += func_name + " " + method.value("file").toString() + ":" + method.value("line").toString() + "\n";
        QString function_help = method.value("comment").toString();
        if (function_help.size() > 0) {
            functions_help_str += func_name + " " + function_help.replace(QRegularExpression("[\r\n]+"), "<br />") + "\n";
        }
    }
    // classes
    std::unordered_map<std::string, QVariantMap> cls_map;
    std::unordered_map<std::string, std::string> cls_methods_map;
    std::unordered_map<std::string, std::string> cls_props_map;
    std::unordered_map<std::string, std::string> cls_consts_map;
    QVariant php_classes_v = map.value("php_classes");
    QVariantMap php_classes = qvariant_cast<QVariantMap>(php_classes_v);
    for (auto k : php_classes.keys()) {
        QVariant cls_v = php_classes.value(k);
        QVariantMap cls = qvariant_cast<QVariantMap>(cls_v);
        //if (cls.value("isAbstract").toBool() || cls.value("isInterface").toBool() || cls.value("isTrait").toBool()) continue;
        QString cls_name = cls.value("name").toString();
        if (cls_name.size() > 0 && cls_name.at(0) == "\\") cls_name = cls_name.mid(1);
        cls_map[cls_name.toStdString()] = cls;
    }
    for (auto k : php_classes.keys()) {
        QVariant cls_v = php_classes.value(k);
        QVariantMap cls = qvariant_cast<QVariantMap>(cls_v);
        //if (cls.value("isAbstract").toBool() || cls.value("isInterface").toBool() || cls.value("isTrait").toBool()) continue;
        QString cls_name = cls.value("name").toString();
        if (cls_name.size() > 0 && cls_name.at(0) == "\\") cls_name = cls_name.mid(1);
        QString cls_args = "()";

        preparePHPClasses(cls_name, cls, cls_args, class_consts_str, class_properties_str, class_methods_str, class_method_types_str, class_methods_search_str, class_methods_help_str, cls_methods_map, cls_props_map, cls_consts_map);
        QString cls_parent = "";
        QString cls_parents = "";
        QVariantMap _cls = cls;
        do {
            cls_parent = _cls.value("parent").toString();
            if (cls_parent.size() > 0 && cls_parent.at(0) == "\\") cls_parent = cls_parent.mid(1);
            if (cls_parent.size() > 0) {
                std::unordered_map<std::string, QVariantMap>::iterator cls_map_it = cls_map.find(cls_parent.toStdString());
                if (cls_map_it != cls_map.end()) {
                    _cls = cls_map_it->second;
                    preparePHPClasses(cls_name, _cls, cls_args, class_consts_str, class_properties_str, class_methods_str, class_method_types_str, class_methods_search_str, class_methods_help_str, cls_methods_map, cls_props_map, cls_consts_map);
                    if (cls_parent.size() > 0) {
                        if (cls_parents.size() > 0) cls_parents += ",";
                        cls_parents += cls_parent;
                    }
                } else {
                    cls_parent = "";
                }
            }
        } while(cls_parent.size() > 0);

        QString cls_synopsis = cls_name + cls_args;
        classes_str += cls_synopsis + "\n";
        if (cls_parents.size() > 0) class_parents_str += cls_name + " " + cls_parents + "\n";
        classes_search_str += cls_name + " " + cls.value("file").toString() + ":" + cls.value("line").toString() + "\n";
    }

    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CONSTS_FILE, consts_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_VARS_FILE, globals_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_FUNCTIONS_FILE, functions_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_FUNCTION_TYPES_FILE, function_types_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASSES_FILE, classes_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_PARENTS_FILE, class_parents_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_CONSTS_FILE, class_consts_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_PROPS_FILE, class_properties_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_METHODS_FILE, class_methods_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_METHOD_TYPES_FILE, class_method_types_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASSES_SEARCH_FILE, classes_search_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_METHODS_SEARCH_FILE, class_methods_search_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_FUNCTIONS_SEARCH_FILE, functions_search_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_FUNCTIONS_HELP_FILE, functions_help_str, PROJECT_DATA_ENCODING);
    Helper::saveTextFile(project_dir + "/" + PROJECT_PHP_CLASS_METHODS_HELP_FILE, class_methods_help_str, PROJECT_DATA_ENCODING);
}

void Project::preparePHPClasses(QString cls_name,
                                QVariantMap cls,
                                QString & cls_args,
                                QString & class_consts_str,
                                QString & class_properties_str,
                                QString & class_methods_str,
                                QString & class_method_types_str,
                                QString & class_methods_search_str,
                                QString & class_methods_help_str,
                                std::unordered_map<std::string, std::string> & cls_methods_map,
                                std::unordered_map<std::string, std::string> & cls_props_map,
                                std::unordered_map<std::string, std::string> & cls_consts_map
                                )
{
    // class consts
    QVariant cls_consts_v = cls.value("constants");
    QVariantMap cls_consts = qvariant_cast<QVariantMap>(cls_consts_v);
    for (auto kc : cls_consts.keys()) {
        QVariant cls_const_v = cls_consts.value(kc);
        QVariantMap cls_const = qvariant_cast<QVariantMap>(cls_const_v);
        QString _class_consts_str = cls_name + "::" + cls_const.value("name").toString();
        std::unordered_map<std::string, std::string>::iterator cls_consts_map_it = cls_consts_map.find(_class_consts_str.toStdString());
        if (cls_consts_map_it == cls_consts_map.end()) {
            class_consts_str += _class_consts_str + "\n";
            cls_consts_map[_class_consts_str.toStdString()] = _class_consts_str.toStdString();
        }
    }
    // class variables
    QVariant cls_vars_v = cls.value("properties");
    QVariantMap cls_vars = qvariant_cast<QVariantMap>(cls_vars_v);
    for (auto kv : cls_vars.keys()) {
        QVariant cls_var_v = cls_vars.value(kv);
        QVariantMap cls_var = qvariant_cast<QVariantMap>(cls_var_v);
        QString _class_properties_str = cls_name + "::" + cls_var.value("name").toString();
        std::unordered_map<std::string, std::string>::iterator cls_props_map_it = cls_props_map.find(_class_properties_str.toStdString());
        if (cls_props_map_it == cls_props_map.end()) {
            class_properties_str += _class_properties_str + "\n";
            cls_props_map[_class_properties_str.toStdString()] = _class_properties_str.toStdString();
        }
    }
    // class methods
    QVariant cls_methods_v = cls.value("methods");
    QVariantMap cls_methods = qvariant_cast<QVariantMap>(cls_methods_v);
    for (auto kk : cls_methods.keys()) {
        QVariant cls_method_v = cls_methods.value(kk);
        QVariantMap cls_method = qvariant_cast<QVariantMap>(cls_method_v);
        //if (cls_method.value("isAbstract").toBool()) continue;
        if (cls_method.value("name").toString() == "__construct") {
            cls_args = " ( "+cls_method.value("args").toString()+" )";
        }
        QString _class_methods_str = cls_name + "::" + cls_method.value("name").toString();
        std::unordered_map<std::string, std::string>::iterator cls_methods_map_it = cls_methods_map.find(_class_methods_str.toStdString());
        if (cls_methods_map_it != cls_methods_map.end()) continue;
        cls_methods_map[_class_methods_str.toStdString()] = _class_methods_str.toStdString();
        class_methods_str += cls_name + "::" + cls_method.value("synopsis").toString() + "\n";
        QString cls_method_type = cls_method.value("return_type").toString();
        if (cls_method_type.size() > 0 && cls_method_type.at(0) == "\\") cls_method_type = cls_method_type.mid(1);
        if (cls_method_type.size() > 0) {
            class_method_types_str += cls_name + "::" + cls_method.value("name").toString() + " " + cls_method_type + "\n";
        }
        class_methods_search_str += cls_name + "::" + cls_method.value("name").toString() + " " + cls_method.value("file").toString() + ":" + cls_method.value("line").toString() + "\n";
        QString cls_method_help = cls_method.value("comment").toString();
        if (cls_method_help.size() > 0) {
            class_methods_help_str += cls_name + "::" + cls_method.value("name").toString() + " " + cls_method_help.replace(QRegularExpression("[\r\n]+"), "<br />") + "\n";
        }
    }
}
