#ifndef SPELLWORDS_H
#define SPELLWORDS_H

#include <QObject>
#include <unordered_map>

class SpellWords : public QObject
{
    Q_OBJECT
public:
    explicit SpellWords();
    void reload();
    void reset();
    std::unordered_map<std::string, std::string> words;
    std::unordered_map<std::string, std::string>::iterator wordsIterator;
    std::unordered_map<std::string, std::string> wordsCS;
    std::unordered_map<std::string, std::string>::iterator wordsCSIterator;
protected:
    void loadWords();
public slots:
    void load();
};

#endif // SPELLWORDS_H
