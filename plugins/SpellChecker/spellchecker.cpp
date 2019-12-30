#include "spellchecker.h"
#include <QFile>
#include <QTextStream>

const QString SPELLCHECKER_DICTIONARIES_SUBDIR = "dicts";
const QString SPELLCHECKER_DEFAULT_DICTIONARY_AFF = "default.aff";
const QString SPELLCHECKER_DEFAULT_DICTIONARY_DIC = "default.dic";
const QString SPELLCHECKER_FALLBACK_DICTIONARY_AFF = "fallback.aff";
const QString SPELLCHECKER_FALLBACK_DICTIONARY_DIC = "fallback.dic";

SpellChecker::SpellChecker(QObject *parent) : SpellCheckerInterface (parent)
{
    hunspell = nullptr;
    hunspellFallback = nullptr;
}

SpellChecker::~SpellChecker()
{
    if (hunspell != nullptr) delete hunspell;
    if (hunspellFallback != nullptr) delete hunspellFallback;
}

void SpellChecker::initialize(QString path)
{
    QString pluginsDir = PLUGINS_DEFAULT_FOLDER_NAME;
    if (path.size() > 0) pluginsDir = path;
    QString pluginDir = pluginsDir + "/" + getDirName();
    QString defaultAff = pluginDir + "/" + SPELLCHECKER_DICTIONARIES_SUBDIR + "/" + SPELLCHECKER_DEFAULT_DICTIONARY_AFF;
    QString defaultDic = pluginDir + "/" + SPELLCHECKER_DICTIONARIES_SUBDIR + "/" + SPELLCHECKER_DEFAULT_DICTIONARY_DIC;
    QString fallbackAff = pluginDir + "/" + SPELLCHECKER_DICTIONARIES_SUBDIR + "/" + SPELLCHECKER_FALLBACK_DICTIONARY_AFF;
    QString fallbackDic = pluginDir + "/" + SPELLCHECKER_DICTIONARIES_SUBDIR + "/" + SPELLCHECKER_FALLBACK_DICTIONARY_DIC;

    QByteArray affPathDefaultBA = defaultAff.toLocal8Bit();
    QByteArray dicPathDefaultBA = defaultDic.toLocal8Bit();
    hunspell = new Hunspell(affPathDefaultBA.constData(), dicPathDefaultBA.constData());
    if (fallbackAff.size() > 0 && fallbackDic.size() > 0) {
        QByteArray affPathFallbackBA = fallbackAff.toLocal8Bit();
        QByteArray dicPathFallbackBA = fallbackDic.toLocal8Bit();
        hunspellFallback = new Hunspell(affPathFallbackBA.constData(), dicPathFallbackBA.constData());
    }

    // detect encoding analyzing the SET option in the affix file
    QRegExp encRegExp("^\\s*SET\\s+([A-Z0-9\\-]+)\\s*", Qt::CaseInsensitive);
    QString defaultEncoding = "ISO8859-1";
    QFile defaultAffFile(defaultAff);
    if (defaultAffFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&defaultAffFile);
        for(QString line = stream.readLine(); !line.isEmpty(); line = stream.readLine()) {
            if (encRegExp.indexIn(line) > -1) {
                defaultEncoding = encRegExp.cap(1);
                break;
            }
        }
        defaultAffFile.close();
    }
    defaultCodec = QTextCodec::codecForName(defaultEncoding.toLatin1().constData());
    QString fallbackEncoding = "ISO8859-1";
    QFile fallbackAffFile(fallbackAff);
    if (fallbackAffFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&fallbackAffFile);
        for(QString line = stream.readLine(); !line.isEmpty(); line = stream.readLine()) {
            if (encRegExp.indexIn(line) > -1) {
                fallbackEncoding = encRegExp.cap(1);
                break;
            }
        }
        fallbackAffFile.close();
    }
    fallbackCodec = QTextCodec::codecForName(fallbackEncoding.toLatin1().constData());
}

bool SpellChecker::check(QString & word)
{
    if (hunspell == nullptr) return true;
    if (hunspell != nullptr && !hunspell->spell(defaultCodec->fromUnicode(word).toStdString())) {
        if (hunspellFallback != nullptr) return hunspellFallback->spell(fallbackCodec->fromUnicode(word).toStdString());
        else return false;
    }
    return true;
}

QStringList SpellChecker::suggest(QString & word)
{
    if (hunspell == nullptr) return QStringList();
    std::vector<std::string> suggestedList = hunspell->suggest(defaultCodec->fromUnicode(word).toStdString());
    QStringList suggestions;
    for(std::string suggested : suggestedList) {
        suggestions << defaultCodec->toUnicode(suggested.c_str());
    }
    if (suggestions.size() == 0 && hunspellFallback != nullptr) {
        std::vector<std::string> suggestedList = hunspellFallback->suggest(fallbackCodec->fromUnicode(word).toStdString());
        for(std::string suggested : suggestedList) {
            suggestions << fallbackCodec->toUnicode(suggested.c_str());
        }
    }
    return suggestions;
}

QString SpellChecker::getDirName()
{
    return SPELLCHECKER_PLUGIN_NAME;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(SpellChecker, SpellChecker)
#endif // QT_VERSION < 0x050000
