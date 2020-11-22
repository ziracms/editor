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
    static Settings& instance();
    static void applyLightColors();
    static void applyDarkColors();
    static void applyCustomColors(QString path);
    static void set(std::string k, std::string v);
    static std::string get(std::string k);
    static void load();
    static void reset();
    static void change(std::unordered_map<std::string, std::string> map);
    static void save();
    static QString getDefaultSnippets();
    static void initApplicationScaling();
protected:
    void initData();
    void _applyLightColors();
    void _applyDarkColors();
    void _applyCustomColors(QString path);
    void _set(std::string k, std::string v);
    std::string _get(std::string k);
    void _load();
    void _reset();
    void _change(std::unordered_map<std::string, std::string> map);
    void _save();
private:
    Settings(QObject *parent = nullptr);
    std::unordered_map<std::string, std::string> data;
    std::unordered_map<std::string, std::string>::iterator iterator;
    std::unordered_map<std::string, std::string> changesData;
signals:
    void restartApp();
};

#endif // SETTINGS_H
