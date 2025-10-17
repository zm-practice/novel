#include "KeywordManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

KeywordManager& KeywordManager::instance()
{
    static KeywordManager inst;
    return inst;
}
void KeywordManager::setRules(const QVector<KeywordRule>& r)
{
    m_rules = r;
    save();
    emit rulesChanged();
}
void KeywordManager::addRule(const KeywordRule& rule)
{
    m_rules.append(rule);
    save();
    emit rulesChanged();
}
void KeywordManager::removeRule(int idx)
{
    if (idx < 0 || idx >= m_rules.size())
        return;
    m_rules.removeAt(idx);
    save();
    emit rulesChanged();
}
void KeywordManager::replaceRule(int idx, const KeywordRule& rule)
{
    if (idx < 0 || idx >= m_rules.size())
        return;
    m_rules[idx] = rule;
    save();
    emit rulesChanged();
}
void KeywordManager::load()
{
    QString programDir = QCoreApplication::applicationDirPath();

    QString path = programDir + "/keywords.json";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
    m_rules.clear();
    for (const QJsonValue& v : arr) {
        QJsonObject o = v.toObject();
        KeywordRule r;
        r.word     = o["word"].toString();
        r.color    = QColor(o["color"].toString());
        r.useRegex = o["regex"].toBool(false);
        if (!r.word.isEmpty() && r.color.isValid())
            m_rules.append(r);
    }
    file.close();
}
void KeywordManager::save()
{
    QJsonArray arr;
    for (const auto& r : qAsConst(m_rules)) {
        QJsonObject o;
        o["word"]  = r.word;
        o["color"] = r.color.name(QColor::HexArgb);
        o["regex"] = r.useRegex;
        arr.append(o);
    }

    // 用法
    QString programDir = QCoreApplication::applicationDirPath();

    QString path = programDir + "/keywords.json";

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
        file.close();
    } else {
        qDebug() << "write fall" << file.error();
    }
}
