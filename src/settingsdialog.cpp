/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "settingsdialog.h"
#include <QFontDialog>
#include <QFontDatabase>
#include <QTextCodec>
#include <QFileDialog>
#include <QDirIterator>
#include "helper.h"

const std::string CHECKED_YES = "yes";
const std::string CHECKED_NO = "no";
const std::string TABS_TYPE_TABS = "tabs";
const std::string TABS_TYPE_SPACES = "spaces";
const std::string NEW_LINE_LF = "lf";
const std::string NEW_LINE_CR = "cr";
const std::string NEW_LINE_CRLF = "crlf";

SettingsDialog::SettingsDialog(Settings * settings, QWidget * parent):
    QDialog(parent),
    ui(new Ui::SettingsDialog()),
    settings(settings)
{
    ui->setupUi(this);
    setModal(true);

    std::string fontFamily = settings->get("editor_font_family");
    std::string fontSize = settings->get("editor_font_size");
    if (fontFamily=="") {
        QFont sysFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        editorFont.setFamily(sysFont.family());
    } else {
        editorFont.setStyleHint(QFont::Monospace);
        editorFont.setFamily(QString::fromStdString(fontFamily));
    }
    editorFont.setPointSize(std::stoi(fontSize));

    tabsType = settings->get("editor_tab_type");
    newLineMode = settings->get("editor_new_line_mode");

    ui->settingsTabWidget->setFocusPolicy(Qt::NoFocus);
    ui->projectsHomeLineEdit->setText(QString::fromStdString(settings->get("file_browser_home")));
    if (settings->get("experimental_mode_enabled") == CHECKED_YES) ui->experimentalModeCheckbox->setChecked(true);
    ui->editorFontComboBox->setCurrentFont(editorFont);
    ui->editorFontSpinBox->setValue(editorFont.pointSize());
    if (tabsType == TABS_TYPE_TABS) ui->editorTabTypeTabsRadio->setChecked(true);
    else if (tabsType == TABS_TYPE_SPACES) ui->editorTabTypeSpacesRadio->setChecked(true);
    ui->editorTabWidthSpinBox->setValue(std::stoi(settings->get("editor_tab_width")));
    if (settings->get("editor_tab_type_detect") == CHECKED_YES) ui->editorDetectTabTypeCheckbox->setChecked(true);
    ui->filesEncodingLineEdit->setText(QString::fromStdString(settings->get("editor_encoding")));
    ui->filesFallbackEncodingLineEdit->setText(QString::fromStdString(settings->get("editor_fallback_encoding")));
    if (newLineMode == NEW_LINE_LF) ui->filesNewLineLFRadio->setChecked(true);
    else if (newLineMode == NEW_LINE_CR) ui->filesNewLineCRRadio->setChecked(true);
    else if (newLineMode == NEW_LINE_CRLF) ui->filesNewLineCRLFRadio->setChecked(true);
    if (settings->get("editor_clean_before_save") == CHECKED_YES) ui->filesCleanForSaveCheckbox->setChecked(true);
    if (settings->get("editor_breadcrumbs_enabled") == CHECKED_YES) ui->breadcrumbsCheckbox->setChecked(true);
    if (settings->get("highlight_spaces") == CHECKED_YES) ui->highlightSpacesCheckbox->setChecked(true);
    if (settings->get("editor_show_annotations") == CHECKED_YES) ui->editorAnnotationsCheckBox->setChecked(true);
    ui->phpTypesEdit->setPlainText(QString::fromStdString(settings->get("highlight_php_extensions")).replace(" ", "").replace(",","\n"));
    ui->jsTypesEdit->setPlainText(QString::fromStdString(settings->get("highlight_js_extensions")).replace(" ", "").replace(",","\n"));
    ui->cssTypesEdit->setPlainText(QString::fromStdString(settings->get("highlight_css_extensions")).replace(" ", "").replace(",","\n"));
    ui->htmlTypesEdit->setPlainText(QString::fromStdString(settings->get("highlight_html_extensions")).replace(" ", "").replace(",","\n"));
    if (settings->get("parser_enable_php_lint") == CHECKED_YES) ui->phplintCheckbox->setChecked(true);
    if (settings->get("parser_enable_php_cs") == CHECKED_YES) ui->phpcsCheckbox->setChecked(true);
    ui->phpcsStandardLineEdit->setText(QString::fromStdString(settings->get("parser_phpcs_standard")));
    ui->phpcsErrorSpinBox->setValue(std::stoi(settings->get("parser_phpcs_error_severity")));
    ui->phpcsWarningSpinBox->setValue(std::stoi(settings->get("parser_phpcs_warning_severity")));
    ui->editorParseIntervalSpinBox->setValue(std::stoi(settings->get("editor_parse_interval")) / 1000);
    if (settings->get("parser_enable_git") == CHECKED_YES) ui->gitCheckbox->setChecked(true);
    if (settings->get("parser_enable_servers") == CHECKED_YES) ui->serversCheckbox->setChecked(true);
    if (settings->get("spellchecker_enabled") == CHECKED_YES) ui->spellCheckerCheckbox->setChecked(true);
    if (!Helper::isPluginExists(SPELLCHECKER_PLUGIN_NAME, QString::fromStdString(settings->get("plugins_path")))) ui->spellCheckerCheckbox->setEnabled(false);
    ui->phpPathLineEdit->setText(QString::fromStdString(settings->get("parser_php_path")));
    ui->phpcsPathLineEdit->setText(QString::fromStdString(settings->get("parser_phpcs_path")));
    ui->gitPathLineEdit->setText(QString::fromStdString(settings->get("parser_git_path")));
    ui->bashPathLineEdit->setText(QString::fromStdString(settings->get("parser_bash_path")));
    ui->sasscPathLineEdit->setText(QString::fromStdString(settings->get("parser_sassc_path")));
    ui->phpmanualLineEdit->setText(QString::fromStdString(settings->get("php_manual_path")));
    ui->pluginsFolderLineEdit->setText(QString::fromStdString(settings->get("plugins_path")));

    QString customThemesPath = QString::fromStdString(settings->get("custom_themes_path"));
    ui->customThemesFolderLineEdit->setText(customThemesPath);

    if (customThemesPath.size() == 0) {
        QDir customThemesPathDir = QDir("./"+CUSTOM_THEMES_FALLBACK_FOLDER);
        customThemesPath = customThemesPathDir.absolutePath();
        if (!Helper::folderExists(customThemesPath)) customThemesPath = "";
    }

    QStringList customThemesList;
    if (customThemesPath.size() > 0 && Helper::folderExists(customThemesPath)) {
        QDirIterator it(customThemesPath, QDir::Dirs | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            QString path = it.next();
            QFileInfo fInfo(path);
            if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isDir()) continue;
            if (!Helper::fileExists(path + "/" + CUSTOM_THEME_CSS_FILE)) continue;
            if (!Helper::fileExists(path + "/" + CUSTOM_THEME_SCHEME_FILE)) continue;
            if (!Helper::fileExists(path + "/" + CUSTOM_THEME_COLORS_FILE)) continue;
            if (fInfo.fileName() == THEME_SYSTEM || fInfo.fileName() == THEME_LIGHT || fInfo.fileName() == THEME_DARK) continue;
            if (fInfo.fileName() == COLOR_SCHEME_LIGHT || fInfo.fileName() == COLOR_SCHEME_DARK) continue;
            customThemesList.append(fInfo.fileName());
        }
    }
    for (auto customTheme : customThemesList) {
        ui->generalThemeCombobox->addItem(customTheme);
        ui->generalColorSchemeCombobox->addItem(customTheme);
    }

    QString theme = QString::fromStdString(settings->get("theme"));
    QString themeStr = theme;
    if (themeStr.size() > 0) themeStr = themeStr.at(0).toUpper() + themeStr.mid(1);
    if (theme == THEME_SYSTEM || theme == THEME_LIGHT || theme == THEME_DARK) {
        ui->generalThemeCombobox->setCurrentText(themeStr);
    } else {
        ui->generalThemeCombobox->setCurrentText(theme);
    }
    QString colorScheme = QString::fromStdString(settings->get("color_scheme"));
    QString colorSchemeStr = colorScheme;
    if (colorSchemeStr.size() > 0) colorSchemeStr = colorSchemeStr.at(0).toUpper() + colorSchemeStr.mid(1);
    if (colorScheme == COLOR_SCHEME_LIGHT || colorScheme == COLOR_SCHEME_DARK) {
        ui->generalColorSchemeCombobox->setCurrentText(colorSchemeStr);
    } else {
        ui->generalColorSchemeCombobox->setCurrentText(colorScheme);
    }

    connect(ui->projectsHomeButton, SIGNAL(pressed()), this, SLOT(projectHomeButtonPressed()));
    connect(ui->editorTabTypeTabsRadio, SIGNAL(toggled(bool)), this, SLOT(editorTabTypeTabsToggled(bool)));
    connect(ui->editorTabTypeSpacesRadio, SIGNAL(toggled(bool)), this, SLOT(editorTabTypeSpacesToggled(bool)));
    connect(ui->filesNewLineLFRadio, SIGNAL(toggled(bool)), this, SLOT(editorNewLineLFToggled(bool)));
    connect(ui->filesNewLineCRRadio, SIGNAL(toggled(bool)), this, SLOT(editorNewLineCRToggled(bool)));
    connect(ui->filesNewLineCRLFRadio, SIGNAL(toggled(bool)), this, SLOT(editorNewLineCRLFToggled(bool)));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(pressed()), this, SLOT(resetButtonPressed()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

std::unordered_map<std::string, std::string> SettingsDialog::getData()
{
    dataMap.clear();
    QString projectHomeStr = ui->projectsHomeLineEdit->text();
    if (projectHomeStr.size() > 1 && projectHomeStr.at(projectHomeStr.size()-1) == "/") projectHomeStr = projectHomeStr.mid(0, projectHomeStr.size()-1);
    if (projectHomeStr.size() > 0 && Helper::folderExists(projectHomeStr)) {
        dataMap["file_browser_home"] = projectHomeStr.toStdString();
        dataMap["file_dialog_path"] = projectHomeStr.toStdString();
    }

    if (ui->experimentalModeCheckbox->isChecked()) dataMap["experimental_mode_enabled"] = CHECKED_YES;
    else dataMap["experimental_mode_enabled"] = CHECKED_NO;

    if (ui->editorFontComboBox->currentFont().family().size() > 0 && ui->editorFontSpinBox->value() > 0) {
        dataMap["editor_font_family"] = ui->editorFontComboBox->currentFont().family().toStdString();
        dataMap["editor_font_size"] = Helper::intToStr(ui->editorFontSpinBox->value()).toStdString();
        if (ui->editorFontSpinBox->value() > 1) {
            dataMap["editor_popup_font_size"] = Helper::intToStr(ui->editorFontSpinBox->value()-1).toStdString();
        }
        if (ui->editorFontSpinBox->value() > 2) {
            dataMap["editor_tooltip_font_size"] = Helper::intToStr(ui->editorFontSpinBox->value()-2).toStdString();
            dataMap["editor_breadcrumbs_font_size"] = Helper::intToStr(ui->editorFontSpinBox->value()-2).toStdString();
        }
    }

    if (tabsType == TABS_TYPE_TABS || tabsType == TABS_TYPE_SPACES) dataMap["editor_tab_type"] = tabsType;

    if (ui->editorTabWidthSpinBox->value() > 0) dataMap["editor_tab_width"] = Helper::intToStr(ui->editorTabWidthSpinBox->value()).toStdString();

    if (ui->editorDetectTabTypeCheckbox->isChecked()) dataMap["editor_tab_type_detect"] = CHECKED_YES;
    else dataMap["editor_tab_type_detect"] = CHECKED_NO;

    if (ui->filesEncodingLineEdit->text().size() > 0) {
        std::string encoding = ui->filesEncodingLineEdit->text().toStdString();
        if (QTextCodec::codecForName(encoding.c_str()) != nullptr) dataMap["editor_encoding"] = encoding;
    }
    if (ui->filesFallbackEncodingLineEdit->text().size() > 0) {
        std::string encoding = ui->filesFallbackEncodingLineEdit->text().toStdString();
        if (QTextCodec::codecForName(encoding.c_str()) != nullptr) dataMap["editor_fallback_encoding"] = encoding;
    }

    if (newLineMode == NEW_LINE_LF || newLineMode == NEW_LINE_CR || newLineMode == NEW_LINE_CRLF) dataMap["editor_new_line_mode"] = newLineMode;

    if (ui->filesCleanForSaveCheckbox->isChecked()) dataMap["editor_clean_before_save"] = CHECKED_YES;
    else dataMap["editor_clean_before_save"] = CHECKED_NO;

    if (ui->breadcrumbsCheckbox->isChecked()) dataMap["editor_breadcrumbs_enabled"] = CHECKED_YES;
    else dataMap["editor_breadcrumbs_enabled"] = CHECKED_NO;

    if (ui->highlightSpacesCheckbox->isChecked()) dataMap["highlight_spaces"] = CHECKED_YES;
    else dataMap["highlight_spaces"] = CHECKED_NO;

    if (ui->editorAnnotationsCheckBox->isChecked()) dataMap["editor_show_annotations"] = CHECKED_YES;
    else dataMap["editor_show_annotations"] = CHECKED_NO;

    QString projectTypes = "";
    QString phpTypes = "";
    QStringList phpTypesList = ui->phpTypesEdit->toPlainText().split("\n");
    for (int i=0; i<phpTypesList.size(); i++) {
        QString t = phpTypesList.at(i);
        t= t.trimmed();
        if (t.size() == 0) continue;
        if (t.indexOf(".")==0) t = t.mid(1);
        if (t.size() == 0) continue;
        if (phpTypes.size() > 0) phpTypes += ", ";
        phpTypes += t;
    }
    if (phpTypes.size() > 0) {
        dataMap["highlight_php_extensions"] = phpTypes.toStdString();
        if (projectTypes.size() > 0) projectTypes += ", ";
        projectTypes += phpTypes;
    }

    QString jsTypes = "";
    QStringList jsTypesList = ui->jsTypesEdit->toPlainText().split("\n");
    for (int i=0; i<jsTypesList.size(); i++) {
        QString t = jsTypesList.at(i);
        t= t.trimmed();
        if (t.size() == 0) continue;
        if (t.indexOf(".")==0) t = t.mid(1);
        if (t.size() == 0) continue;
        if (jsTypes.size() > 0) jsTypes += ", ";
        jsTypes += t;
    }
    if (jsTypes.size() > 0) {
        dataMap["highlight_js_extensions"] = jsTypes.toStdString();
        if (projectTypes.size() > 0) projectTypes += ", ";
        projectTypes += jsTypes;
    }

    QString cssTypes = "";
    QStringList cssTypesList = ui->cssTypesEdit->toPlainText().split("\n");
    for (int i=0; i<cssTypesList.size(); i++) {
        QString t = cssTypesList.at(i);
        t= t.trimmed();
        if (t.size() == 0) continue;
        if (t.indexOf(".")==0) t = t.mid(1);
        if (t.size() == 0) continue;
        if (cssTypes.size() > 0) cssTypes += ", ";
        cssTypes += t;
    }
    if (cssTypes.size() > 0) {
        dataMap["highlight_css_extensions"] = cssTypes.toStdString();
        if (projectTypes.size() > 0) projectTypes += ", ";
        projectTypes += cssTypes;
    }

    QString htmlTypes = "";
    QStringList htmlTypesList = ui->htmlTypesEdit->toPlainText().split("\n");
    for (int i=0; i<htmlTypesList.size(); i++) {
        QString t = htmlTypesList.at(i);
        t= t.trimmed();
        if (t.size() == 0) continue;
        if (t.indexOf(".")==0) t = t.mid(1);
        if (t.size() == 0) continue;
        if (htmlTypes.size() > 0) htmlTypes += ", ";
        htmlTypes += t;
    }
    if (htmlTypes.size() > 0) {
        dataMap["highlight_html_extensions"] = htmlTypes.toStdString();
        if (projectTypes.size() > 0) projectTypes += ", ";
        projectTypes += htmlTypes;
    }

    QString dialogFilter = "";
    bool txtExists = false, htaccessExists = false;
    if (projectTypes.size() > 0) {
        QStringList projectTypesList = projectTypes.split(",");
        for (int i=0; i<projectTypesList.size(); i++) {
            QString t = projectTypesList.at(i);
            t = t.trimmed();
            if (t.size() == 0) continue;
            if (t == "txt") txtExists = true;
            if (t == "htaccess") htaccessExists = true;
            if (dialogFilter.size() > 0) dialogFilter += " ";
            dialogFilter += "*."+t;
        }
    }
    if (!txtExists) {
        if (dialogFilter.size() > 0) dialogFilter += " ";
        dialogFilter += "*.txt";
    }
    if (!htaccessExists) {
        if (dialogFilter.size() > 0) dialogFilter += " ";
        dialogFilter += "*.htaccess";
    }
    QString dialogFilterStr = "Project files (" + dialogFilter + ");;All files (*)";
    dataMap["file_dialog_filter"] = dialogFilterStr.toStdString();

    if (ui->phplintCheckbox->isChecked()) dataMap["parser_enable_php_lint"] = CHECKED_YES;
    else dataMap["parser_enable_php_lint"] = CHECKED_NO;

    if (ui->phpcsCheckbox->isChecked()) dataMap["parser_enable_php_cs"] = CHECKED_YES;
    else dataMap["parser_enable_php_cs"] = CHECKED_NO;

    if (ui->phpcsStandardLineEdit->text().size() > 0) {
        dataMap["parser_phpcs_standard"] = ui->phpcsStandardLineEdit->text().toStdString();
    }

    if (ui->phpcsErrorSpinBox->value() >= 0 && ui->phpcsErrorSpinBox->value() <= 9) {
        dataMap["parser_phpcs_error_severity"] = Helper::intToStr(ui->phpcsErrorSpinBox->value()).toStdString();
    }

    if (ui->phpcsWarningSpinBox->value() >= 0 && ui->phpcsWarningSpinBox->value() <= 9) {
        dataMap["parser_phpcs_warning_severity"] = Helper::intToStr(ui->phpcsWarningSpinBox->value()).toStdString();
    }

    if (ui->editorParseIntervalSpinBox->value() >= 1 && ui->editorParseIntervalSpinBox->value() <= 9) {
        dataMap["editor_parse_interval"] = Helper::intToStr(ui->editorParseIntervalSpinBox->value() * 1000).toStdString();
    }

    if (ui->gitCheckbox->isChecked()) dataMap["parser_enable_git"] = CHECKED_YES;
    else dataMap["parser_enable_git"] = CHECKED_NO;

    if (ui->serversCheckbox->isChecked()) dataMap["parser_enable_servers"] = CHECKED_YES;
    else dataMap["parser_enable_servers"] = CHECKED_NO;

    if (ui->spellCheckerCheckbox->isChecked()) dataMap["spellchecker_enabled"] = CHECKED_YES;
    else dataMap["spellchecker_enabled"] = CHECKED_NO;

    QString phpPathStr = ui->phpPathLineEdit->text();
    QString phpcsPathStr = ui->phpcsPathLineEdit->text();
    QString gitPathStr = ui->gitPathLineEdit->text();
    QString bashPathStr = ui->bashPathLineEdit->text();
    QString sasscPathStr = ui->sasscPathLineEdit->text();
    QString phpmanualPathStr = ui->phpmanualLineEdit->text();
    QString pluginsPathStr = ui->pluginsFolderLineEdit->text();
    QString customThemesPathStr = ui->customThemesFolderLineEdit->text();
    QString customThemesPath("");
    if (phpPathStr.size() > 1 && phpPathStr.at(phpPathStr.size()-1) == "/") phpPathStr = phpPathStr.mid(0, phpPathStr.size()-1);
    if (phpcsPathStr.size() > 1 && phpcsPathStr.at(phpcsPathStr.size()-1) == "/") phpcsPathStr = phpcsPathStr.mid(0, phpcsPathStr.size()-1);
    if (gitPathStr.size() > 1 && gitPathStr.at(gitPathStr.size()-1) == "/") gitPathStr = gitPathStr.mid(0, gitPathStr.size()-1);
    if (bashPathStr.size() > 1 && bashPathStr.at(bashPathStr.size()-1) == "/") bashPathStr = bashPathStr.mid(0, bashPathStr.size()-1);
    if (sasscPathStr.size() > 1 && sasscPathStr.at(sasscPathStr.size()-1) == "/") sasscPathStr = sasscPathStr.mid(0, sasscPathStr.size()-1);
    if (phpmanualPathStr.size() > 1 && phpmanualPathStr.at(phpmanualPathStr.size()-1) == "/") phpmanualPathStr = phpmanualPathStr.mid(0, phpmanualPathStr.size()-1);
    if (pluginsPathStr.size() > 1 && pluginsPathStr.at(pluginsPathStr.size()-1) == "/") pluginsPathStr = pluginsPathStr.mid(0, pluginsPathStr.size()-1);
    if (customThemesPathStr.size() > 1 && customThemesPathStr.at(customThemesPathStr.size()-1) == "/") customThemesPathStr = customThemesPathStr.mid(0, customThemesPathStr.size()-1);
    if (Helper::fileOrFolderExists(phpPathStr) || phpPathStr.size() == 0) dataMap["parser_php_path"] = phpPathStr.toStdString();
    if (Helper::fileOrFolderExists(phpcsPathStr) || phpcsPathStr.size() == 0) dataMap["parser_phpcs_path"] = phpcsPathStr.toStdString();
    if (Helper::fileOrFolderExists(gitPathStr) || gitPathStr.size() == 0) dataMap["parser_git_path"] = gitPathStr.toStdString();
    if (Helper::fileOrFolderExists(bashPathStr) || bashPathStr.size() == 0) dataMap["parser_bash_path"] = bashPathStr.toStdString();
    if (Helper::fileOrFolderExists(sasscPathStr) || sasscPathStr.size() == 0) dataMap["parser_sassc_path"] = sasscPathStr.toStdString();
    if (Helper::folderExists(phpmanualPathStr) || phpmanualPathStr.size() == 0) dataMap["php_manual_path"] = phpmanualPathStr.toStdString();
    if (Helper::folderExists(pluginsPathStr) || pluginsPathStr.size() == 0) dataMap["plugins_path"] = pluginsPathStr.toStdString();
    if (Helper::folderExists(customThemesPathStr) || customThemesPathStr.size() == 0) {
        dataMap["custom_themes_path"] = customThemesPathStr.toStdString();
        customThemesPath = customThemesPathStr;
    }

    if (customThemesPath.size() == 0) {
        QDir customThemesPathDir = QDir("./"+CUSTOM_THEMES_FALLBACK_FOLDER);
        customThemesPath = customThemesPathDir.absolutePath();
        if (!Helper::folderExists(customThemesPath)) customThemesPath = "";
    }

    QString themeStr = ui->generalThemeCombobox->currentText();
    QString themeStrL = themeStr;
    if (themeStr.size() > 0) themeStrL = themeStr.at(0).toLower() + themeStr.mid(1);
    if (themeStrL == THEME_SYSTEM || themeStrL == THEME_LIGHT || themeStrL == THEME_DARK) {
        dataMap["theme"] = themeStrL.toStdString();
    } else if (customThemesPath.size() > 0 && Helper::fileExists(customThemesPath + "/" + themeStr + "/" + CUSTOM_THEME_CSS_FILE)) {
        dataMap["theme"] = themeStr.toStdString();
    }
    QString colorSchemeStr = ui->generalColorSchemeCombobox->currentText();
    QString colorSchemeStrL = colorSchemeStr;
    if (colorSchemeStr.size() > 0) colorSchemeStrL = colorSchemeStr.at(0).toLower() + colorSchemeStr.mid(1);
    if (colorSchemeStrL == COLOR_SCHEME_LIGHT || colorSchemeStrL == COLOR_SCHEME_DARK) {
        dataMap["color_scheme"] = colorSchemeStrL.toStdString();
    } else if (customThemesPath.size() > 0 && Helper::fileExists(customThemesPath + "/" + colorSchemeStr + "/" + CUSTOM_THEME_SCHEME_FILE) && Helper::fileExists(customThemesPath + "/" + colorSchemeStr + "/" + CUSTOM_THEME_COLORS_FILE)) {
        dataMap["color_scheme"] = colorSchemeStr.toStdString();
    }

    return dataMap;
}

void SettingsDialog::editorTabTypeTabsToggled(bool checked)
{
    if (!checked) return;
    tabsType = TABS_TYPE_TABS;
}

void SettingsDialog::editorTabTypeSpacesToggled(bool checked)
{
    if (!checked) return;
    tabsType = TABS_TYPE_SPACES;
}

void SettingsDialog::editorNewLineLFToggled(bool checked)
{
    if (!checked) return;
    newLineMode = NEW_LINE_LF;
}

void SettingsDialog::editorNewLineCRToggled(bool checked)
{
    if (!checked) return;
    newLineMode = NEW_LINE_CR;
}

void SettingsDialog::editorNewLineCRLFToggled(bool checked)
{
    if (!checked) return;
    newLineMode = NEW_LINE_CRLF;
}

void SettingsDialog::projectHomeButtonPressed()
{
    QString home = ui->projectsHomeLineEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose projects home directory"), home, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.size() > 0) {
        ui->projectsHomeLineEdit->setText(dir);
    }
}


void SettingsDialog::resetButtonPressed()
{
    settings->reset();
    done(QDialog::Rejected);
    emit settings->restartApp();
}
