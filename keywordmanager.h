#ifndef KEYWORDMANAGER_H
#define KEYWORDMANAGER_H

#include <QColor>
#include <QObject>
#include <QVector>

struct KeywordRule
{
    QString word;
    QColor  color;
    bool    useRegex = false;
};

class KeywordManager : public QObject
{
    Q_OBJECT
public:
    static KeywordManager& instance();
    QVector<KeywordRule>   rules() const { return m_rules; }
    void                   setRules(const QVector<KeywordRule>& r);  // 会发 rulesChanged + 持久化
    void                   load();
    void                   save();

public slots:
    void addRule(const KeywordRule& rule);
    void removeRule(int index);
    void replaceRule(int index, const KeywordRule& rule);

signals:
    void rulesChanged();  // 只要数据变就发

private:
    KeywordManager() = default;

    QVector<KeywordRule> m_rules;
};

#endif  // KEYWORDMANAGER_H
