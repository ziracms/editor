/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef HELPWORDS_H
#define HELPWORDS_H

#include <QObject>
#include <QString>
#include <unordered_map>

class HelpWords : public QObject
{
    Q_OBJECT
public:
    static HelpWords& instance();
    static  void loadDelayed();
    static void reload();
    static void reset();
    static QString findHelpFile(QString name);
    std::unordered_map<std::string, std::string> phpFunctionDescs;
    std::unordered_map<std::string, std::string>::iterator phpFunctionDescsIterator;
    std::unordered_map<std::string, std::string> phpClassMethodDescs;
    std::unordered_map<std::string, std::string>::iterator phpClassMethodDescsIterator;
    std::unordered_map<std::string, std::string> phpFiles;
    std::unordered_map<std::string, std::string>::iterator phpFilesIterator;
protected:
    void loadPHPWords();
    void _loadDelayed();
    void _reload();
    void _reset();
    QString _findHelpFile(QString name);
    void _load();
private:
    HelpWords();
public slots:
    static void load();
};

#endif // HELPWORDS_H
