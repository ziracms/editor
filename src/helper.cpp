/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "helper.h"
#include <QFile>
#include <QTextStream>
#include <sstream>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDir>

const QString APPLICATION_NAME = "Zira Editor";
const QString APPLICATION_VERSION = "1.2.7";
const QString ORGANIZATION_NAME = "Zira";
const QString AUTHOR_EMAIL_USERNAME = "ziracms";
const QString AUTHOR_EMAIL_DOMAIN = "gmail.com";
const QString AUTHOR_CARD_URL = "https://money.yandex.ru/to";
const QString AUTHOR_CARD_ID = "410014796567498";
const QString AUTHOR_CMS_URL = "https://github.com/ziracms/zira";

QString Helper::loadFile(QString path, std::string encoding, std::string fallbackEncoding, bool silent)
{
    QFile inputFile(path);
    if (!inputFile.open(QIODevice::ReadOnly)) return "";
    QByteArray byteArray = inputFile.readAll();
    inputFile.close();

    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName(encoding.c_str());
    QString txt = codec->toUnicode(byteArray.constData(), byteArray.size(), &state);
    if (state.invalidChars > 0) {
        if (!silent) {
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("File has a not valid byte sequence. Fallback encoding will be used."));
            msgBox.exec();
        }
        QTextCodec *fcodec = QTextCodec::codecForName(fallbackEncoding.c_str());
        txt = fcodec->toUnicode(byteArray.constData(), byteArray.size(), &state);
    }

    return txt;
}

QString Helper::loadTextFile(QString path, std::string encoding, std::string fallbackEncoding, bool silent)
{
    QFile inputFile(path);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) return "";
    QByteArray byteArray = inputFile.readAll();
    inputFile.close();

    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName(encoding.c_str());
    QString txt = codec->toUnicode(byteArray.constData(), byteArray.size(), &state);
    if (state.invalidChars > 0) {
        if (!silent) {
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("File has a not valid byte sequence. Fallback encoding will be used."));
            msgBox.exec();
        }
        QTextCodec *fcodec = QTextCodec::codecForName(fallbackEncoding.c_str());
        txt = fcodec->toUnicode(byteArray.constData(), byteArray.size(), &state);
    }

    return txt;
}

bool Helper::saveTextFile(QString path, const QString & text, std::string encoding)
{
    QFile outputFile(path);
    if (!outputFile.open(QIODevice::WriteOnly)) return false;
    QTextStream out(&outputFile);
    out.setGenerateByteOrderMark(false);
    out.setCodec(encoding.c_str());
    out << text;
    outputFile.close();
    return true;
}

bool Helper::createFile(QString path)
{
    QFile f(path);
    if (f.exists()) return false;
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.close();
    return true;
}

bool Helper::createDir(QString path)
{
    QDir dir;
    if (dir.exists(path)) return false;
    return dir.mkpath(path);
}

bool Helper::deleteFile(QString path)
{
    QFile f(path);
    if (!f.exists(path)) return false;
    return f.remove();
}

bool Helper::deleteFolder(QString path)
{
    QDir dir;
    if (!dir.exists(path)) return false;
    return dir.rmdir(path);
}

bool Helper::renameFile(QString path, QString newpath)
{
    QFile f(path);
    if (!f.exists()) return false;
    return f.rename(newpath);
}

bool Helper::renameDir(QString path, QString newpath)
{
    QDir dir;
    if (!dir.exists(path)) return false;
    return dir.rename(path, newpath);
}

bool Helper::renameFileOrFolder(QString path, QString newpath)
{
    QFileInfo fInfo(path);
    if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isWritable()) return false;
    if (fInfo.isDir()) return renameDir(path, newpath);
    else return renameFile(path, newpath);
}

bool Helper::copyFile(QString path, QString newpath)
{
    QFile f(path);
    if (!f.exists()) return false;
    return f.copy(newpath);
}

bool Helper::fileExists(QString path)
{
    QFileInfo check_file(path);
    return check_file.exists() && check_file.isFile() && check_file.isReadable();
}

bool Helper::folderExists(QString path)
{
    QFileInfo check_file(path);
    return check_file.exists() && check_file.isDir() && check_file.isReadable();
}

bool Helper::fileOrFolderExists(QString path)
{
    QFileInfo check_file(path);
    return check_file.exists();
}

QString Helper::intToStr(int n)
{
    std::stringstream ss;
    ss << n;
    return QString::fromStdString(ss.str());
}

QString Helper::doubleToStr(double n)
{
    std::stringstream ss;
    ss << n;
    return QString::fromStdString(ss.str());
}

QString Helper::stripScopedText(QString scopedText)
{
    // parenthesis
    int pars = 0, parsStart = -1;
    int p = scopedText.indexOf("(");
    if (p >= 0) {
        pars++;
        parsStart = p;
    }
    while(p >= 0) {
        int pO = scopedText.indexOf("(", p+1);
        int pC = scopedText.indexOf(")", p+1);
        int pM = -1;
        if (pO >= 0 && pC >= 0) pM = std::min(pO, pC);
        else if (pO >= 0 && pC < 0) pM = pO;
        else if (pO < 0 && pC >= 0) pM = pC;
        if (pM >= 0) {
            if (pM == pO) {
                if (pars == 0) parsStart = pM;
                pars++;
            }
            if (pM == pC) pars--;
            int start = parsStart + 1;
            int len = pM-parsStart-1;
            if (pars == 0 && len > 0) {
                scopedText.replace(start, len, QString(" ").repeated(len));
            }
        }
        p = pM;
    }
    // braces
    int scope = 0, scopeStart = -1;
    p = scopedText.indexOf("{");
    if (p >= 0) {
        scope++;
        scopeStart = p;
    }
    while(p >= 0) {
        int pO = scopedText.indexOf("{", p+1);
        int pC = scopedText.indexOf("}", p+1);
        int pM = -1;
        if (pO >= 0 && pC >= 0) pM = std::min(pO, pC);
        else if (pO >= 0 && pC < 0) pM = pO;
        else if (pO < 0 && pC >= 0) pM = pC;
        if (pM >= 0) {
            if (pM == pO) {
                if (scope == 0) scopeStart = pM;
                scope++;
            }
            if (pM == pC) scope--;
            int start = scopeStart + 1;
            int len = pM-scopeStart-1;
            if (scope == 0 && len > 0) {
                scopedText.replace(start, len, QString(" ").repeated(len));
            }
        }
        p = pM;
    }
    return scopedText.replace(QRegularExpression("[\\s]+"), " ");
}

void Helper::log(int n)
{
    QString str = intToStr(n);
    QTextStream out(stdout);
    out << str;
}

void Helper::log(QString str)
{
    QTextStream out(stdout);
    out << str;
}

void Helper::log(const char * str)
{
    QTextStream out(stdout);
    out << str;
}

void Helper::log(std::string str)
{
    const char * cstr = str.c_str();
    QTextStream out(stdout);
    out << cstr;
}
