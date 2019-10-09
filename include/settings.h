/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <unordered_map>

extern const QString COLOR_SCHEME_LIGHT;
extern const QString COLOR_SCHEME_DARK;

extern const QString THEME_SYSTEM;
extern const QString THEME_LIGHT;
extern const QString THEME_DARK;

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = nullptr);
    ~Settings();
    void applyLightColors();
    void applyDarkColors();
    void set(std::string k, std::string v);
    std::string get(std::string k);
    void load();
    void reset();
    void change(std::unordered_map<std::string, std::string> map);
    void save();
private:
    std::unordered_map<std::string, std::string> data;
    std::unordered_map<std::string, std::string>::iterator iterator;
    std::unordered_map<std::string, std::string> changesData;
signals:
    void restartApp();
};

#endif // SETTINGS_H
