#include "snippets.h"
#include "helper.h"
#include "highlight.h"
#include <QRegularExpression>

const QString SNIPPETS_REGEXP = "[[][{]([a-zA-Z]+)[:][@]([a-zA-Z0-9]{2,})[}][]][\\s]*[=][\\s]*[[][[]([^]]+)[]][]]";

const QString SNIPPET_CURSOR_TAG = "{$cursor}";
const QString SNIPPET_SELECT_START_TAG = "{$selectStart}";
const QString SNIPPET_SELECT_END_TAG = "{$selectEnd}";
const QString SNIPPET_MULTI_SELECT_START_TAG = "{$multiSelectStart}";
const QString SNIPPET_MULTI_SELECT_END_TAG = "{$multiSelectEnd}";

Snippets::Snippets(){}

Snippets& Snippets::instance()
{
    static Snippets _instance;
    return _instance;
}

void Snippets::load()
{
    instance()._load();
}

void Snippets::_load()
{
    QString snippets = QString::fromStdString(Settings::get("snippets")).trimmed();
    QString customSnippetsFile = QString::fromStdString(Settings::get("custom_snippets_file"));
    if (Helper::fileExists(customSnippetsFile)) {
        std::string encoding = Settings::get("editor_encoding");
        std::string encodingFallback = Settings::get("editor_fallback_encoding");
        QString customSnippets = Helper::loadTextFile(customSnippetsFile, encoding, encodingFallback, true).trimmed();
        if (customSnippets.size() > 0) snippets += "\n"+customSnippets;
    }
    if (snippets.size() > 0) {
        QRegularExpression snippetsExpr(SNIPPETS_REGEXP);
        QRegularExpressionMatch snippetMatch;
        QStringList snippetsList = snippets.split("\n");
        for (QString snippet : snippetsList) {
            if (snippet.indexOf("//")==0) continue;
            snippetMatch = snippetsExpr.match(snippet);
            if (snippetMatch.capturedStart() >= 0) {
                std::string mode = snippetMatch.captured(1).toLower().toStdString();
                QString snippetCode = snippetMatch.captured(2).trimmed();
                QString snippetText = snippetMatch.captured(3).trimmed();
                if (snippetCode.size() == 0 || snippetText.size() == 0) continue;
                if (mode == MODE_PHP) {
                    phpSnippets[snippetCode] = snippetText;
                } else if (mode == MODE_JS) {
                    jsSnippets[snippetCode] = snippetText;
                } else if (mode == MODE_CSS) {
                    cssSnippets[snippetCode] = snippetText;
                } else if (mode == MODE_HTML) {
                    htmlSnippets[snippetCode] = snippetText;
                }
            }
        }
    }
}

QString Snippets::parse(QString data, QString prefix, QString indent, int & moveCursorBack, int & setSelectStartFromEnd, int & setSelectLength, int & setMultiSelectStartFromEnd, int & setMultiSelectLength)
{
    QString text = data.replace("\\n","\n"+prefix).replace("\\t", indent);
    int cp = text.indexOf(SNIPPET_CURSOR_TAG);
    int sp1 = text.indexOf(SNIPPET_SELECT_START_TAG);
    int sp2 = text.indexOf(SNIPPET_SELECT_END_TAG);
    int mp1 = text.indexOf(SNIPPET_MULTI_SELECT_START_TAG);
    int mp2 = text.indexOf(SNIPPET_MULTI_SELECT_END_TAG);

    if (cp >= 0 && sp1 < 0 && sp2 < 0 && mp1 < 0 && mp2 < 0 && text.count(SNIPPET_CURSOR_TAG) == 1) {
        text.replace(SNIPPET_CURSOR_TAG,"");
        moveCursorBack = text.size() - cp;
    }

    if (cp < 0 && mp1 < 0 && mp2 < 0 && sp1 >= 0 && sp2 >= 0 && sp2 > sp1 && text.count(SNIPPET_SELECT_START_TAG) == 1 && text.count(SNIPPET_SELECT_END_TAG) == 1) {
        text.replace(SNIPPET_SELECT_START_TAG,"");
        text.replace(SNIPPET_SELECT_END_TAG,"");
        setSelectStartFromEnd = text.size() - sp1;
        setSelectLength = sp2 - sp1 - SNIPPET_SELECT_START_TAG.length();
    }

    if (cp < 0 && sp1 < 0 && sp2 < 0 && mp1 >= 0 && mp2 >= 0 && mp2 > mp1 && text.count(SNIPPET_MULTI_SELECT_START_TAG) == 1 && text.count(SNIPPET_MULTI_SELECT_END_TAG) == 1) {
        text.replace(SNIPPET_MULTI_SELECT_START_TAG,"");
        text.replace(SNIPPET_MULTI_SELECT_END_TAG,"");
        setMultiSelectStartFromEnd = text.size() - mp1;
        setMultiSelectLength = mp2 - mp1 - SNIPPET_MULTI_SELECT_START_TAG.length();
    }

    return text;
}
