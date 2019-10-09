/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "helpwords.h"

#include <QFile>
#include <QTextStream>
#include <QTimer>

const int LOAD_DELAY = 250;

HelpWords::HelpWords()
{
    QTimer::singleShot(LOAD_DELAY, this, SLOT(load()));
}

void HelpWords::load()
{
    loadPHPWords();
}

void HelpWords::reload()
{
    reset();
    load();
}

void HelpWords::reset()
{
    phpFunctionDescs.clear();
    phpClassMethodDescs.clear();
    phpFiles.clear();
}

void HelpWords::loadPHPWords()
{
    QString k, func, desc;

    // php function descriptions
    QFile pf(":/help/php_function_descs");
    pf.open(QIODevice::ReadOnly);
    QTextStream pin(&pf);
    while (!pin.atEnd()) {
        k = pin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        phpFunctionDescs[func.toStdString()] = desc.toStdString();
    }
    pf.close();

    // php class method descriptions
    QFile cf(":/help/php_class_method_descs");
    cf.open(QIODevice::ReadOnly);
    QTextStream cin(&cf);
    while (!cin.atEnd()) {
        k = cin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        phpClassMethodDescs[func.toStdString()] = desc.toStdString();
    }
    cf.close();

    // php manual files
    QFile ff(":/help/php_manual_files");
    ff.open(QIODevice::ReadOnly);
    QTextStream fin(&ff);
    while (!fin.atEnd()) {
        k = fin.readLine();
        if (k == "" || k.indexOf(" ") < 0) continue;
        func = k.mid(0, k.indexOf(" "));
        desc = k.mid(k.indexOf(" ")+1);
        phpFiles[func.toStdString()] = desc.toStdString();
    }
    ff.close();
}

QString HelpWords::findHelpFile(QString name)
{
    QString file = "";
    phpFilesIterator = phpFiles.find(name.toStdString());
    if (phpFilesIterator != phpFiles.end()) {
        file = QString::fromStdString(phpFilesIterator->second);
    }
    if (file.size() == 0 && name.indexOf("\\") >= 0) {
        name = name.mid(name.lastIndexOf("\\")+1);
        phpFilesIterator = phpFiles.find(name.toStdString());
        if (phpFilesIterator != phpFiles.end()) {
            file = QString::fromStdString(phpFilesIterator->second);
        }
    }
    return file;
}
