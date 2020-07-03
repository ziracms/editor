/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <unordered_map>

extern const QString COLOR_SCHEME_TYPE;
extern const QString COLOR_SCHEME_LIGHT;
extern const QString COLOR_SCHEME_DARK;

extern const QString THEME_SYSTEM;
extern const QString THEME_LIGHT;
extern const QString THEME_DARK;

extern const QString CUSTOM_THEME_CSS_FILE;
extern const QString CUSTOM_THEME_SCHEME_FILE;
extern const QString CUSTOM_THEME_COLORS_FILE;
extern const QString CUSTOM_THEME_ICONS_FOLDER;

extern const QString CUSTOM_THEMES_FALLBACK_FOLDER;
extern const QString PHP_MANUAL_FALLBACK_FOLDER;

extern const std::string PHP_MANUAL_ENCODING;

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = nullptr);
    void applyLightColors();
    void applyDarkColors();
    void applyCustomColors(QString path);
    void set(std::string k, std::string v);
    std::string get(std::string k);
    void load();
    void reset();
    void change(std::unordered_map<std::string, std::string> map);
    void save();
    QString getDefaultSnippets();
private:
    std::unordered_map<std::string, std::string> data;
    std::unordered_map<std::string, std::string>::iterator iterator;
    std::unordered_map<std::string, std::string> changesData;
signals:
    void restartApp();
};

#endif // SETTINGS_H
