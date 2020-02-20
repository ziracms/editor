/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "ui_search.h"
#include <QDialog>

class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    SearchDialog(QWidget * parent);
    ~SearchDialog() override;
    void setDirectory(QString path);
    void setText(QString text);
    void setExtensions(QString text);
    QString getDirectory();
    QString getText();
    QString getExtensions();
    void setCaseOption(bool checked);
    bool getCaseOption();
    void setWordOption(bool checked);
    bool getWordOption();
    void setRegexpOption(bool checked);
    bool getRegexpOption();
    void focusDirectory();
    void focusText();
    void setExcludeDirs(QStringList dirs);
    void addExcludeDir(QString dir);
    void removeExcludeDir(QString dir);
    void clearExcludeDirs();
    QStringList getExcludeDirs();
    void addExcludeDirButton(QString dir);
protected:
    void checkPath(QString directory, QString text);
private:
    Ui::SearchDialog * ui;
    QStringList excludeDirs;
private slots:
    void directoryChanged(QString path);
    void textChanged(QString text);
    void chooseButtonPressed(void);
    void excludeButtonPressed(void);
    void removeExcludeDirButton();
};

#endif // SEARCHDIALOG_H
