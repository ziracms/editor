#ifndef SPELLWORDS_H
#define SPELLWORDS_H

#include <QObject>
#include <unordered_map>

class SpellWords : public QObject
{
    Q_OBJECT
public:
    static SpellWords& instance();
    static void loadDelayed();
    static void reload();
    static void reset();
    std::unordered_map<std::string, std::string> words;
    std::unordered_map<std::string, std::string>::iterator wordsIterator;
    std::unordered_map<std::string, std::string> wordsCS;
    std::unordered_map<std::string, std::string>::iterator wordsCSIterator;
protected:
    void loadWords();
    void _loadDelayed();
    void _reload();
    void _reset();
    void _load();
private:
    explicit SpellWords();
public slots:
    static void load();
};

#endif // SPELLWORDS_H
