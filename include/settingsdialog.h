/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settings.h"
#include <QDialog>
#include "settings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(Settings * settings, QWidget * parent);
    ~SettingsDialog() override;
    std::unordered_map<std::string, std::string> getData();
private:
    Ui::SettingsDialog * ui;
    Settings * settings;
    std::unordered_map<std::string, std::string> dataMap;
    QFont appFont;
    QFont editorFont;
    std::string tabsType;
    std::string newLineMode;
private slots:
    void projectHomeButtonPressed();
    void editorTabTypeTabsToggled(bool checked);
    void editorTabTypeSpacesToggled(bool checked);
    void editorNewLineLFToggled(bool checked);
    void editorNewLineCRToggled(bool checked);
    void editorNewLineCRLFToggled(bool checked);
    void resetButtonPressed();
};

#endif // SETTINGSDIALOG_H
