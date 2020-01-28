/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef HELPER_H
#define HELPER_H

#include <QList>
#include "plugininterface.h"
#include "spellcheckerinterface.h"

extern const QString APPLICATION_NAME;
extern const QString APPLICATION_VERSION;
extern const QString ORGANIZATION_NAME;
extern const QString AUTHOR_EMAIL_USERNAME;
extern const QString AUTHOR_EMAIL_DOMAIN;
extern const QString AUTHOR_CARD_URL;
extern const QString AUTHOR_CARD_ID;
extern const QString AUTHOR_CMS_URL;
extern const QString GITHUB_EDITOR_URL;

class Helper
{
public:
    static QString loadFile(QString path, std::string encoding, std::string fallbackEncoding, bool silent = false);
    static QString loadTextFile(QString path, std::string encoding, std::string fallbackEncoding, bool silent = false);
    static bool saveTextFile(QString path, const QString & text, std::string encoding);
    static bool createFile(QString path);
    static bool createDir(QString path);
    static bool deleteFile(QString path);
    static bool deleteFolder(QString path);
    static bool renameFile(QString path, QString newpath);
    static bool renameDir(QString path, QString newpath);
    static bool renameFileOrFolder(QString path, QString newpath);
    static bool copyFile(QString path, QString newpath);
    static bool fileExists(QString path);
    static bool folderExists(QString path);
    static bool fileOrFolderExists(QString path);
    static QString intToStr(int n);
    static QString doubleToStr(double n);
    static QString stripScopedText(QString scopedText);
    static void log(int n);
    static void log(QString str);
    static void log(const char * str);
    static void log(std::string str);
    static QString getPluginFile(QString name, QString path);
    static QObject * loadPlugin(QString name, QString path = "");
    static bool isPluginExists(QString name, QString path = "");
    static SpellCheckerInterface * loadSpellChecker(QString path = "");
    static QString getExistingDirectory(QWidget * parent, QString title, QString directory);
};

#endif // HELPER_H
