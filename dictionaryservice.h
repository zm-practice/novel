// dictionaryservice.h
#ifndef DICTIONARYSERVICE_H
#define DICTIONARYSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager> // 包含网络管理类
#include <QHash>               // 包含哈希表

// ★★★ 确保 WordDefinition 结构体是最新版本 ★★★
struct WordDefinition
{
    QString query;      // 原始查询词
    QString displayWord; // 显示的词（可能是汉字或英文）
    QString phonetic;   // 音标或拼音
    QString definition; // 释义
};

class DictionaryService : public QObject
{
    Q_OBJECT
public:
    explicit DictionaryService(QObject *parent = nullptr);

    // ★★★ 确保公共接口是 query() ★★★
    void query(const QString& text);

signals:
    void querySuccess(const WordDefinition& result);
    void queryError(const QString& errorString);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    // ★★★ 确保这些私有函数都被声明了 ★★★
    // ★★★ 新增：加载本地中文词典 ★★★
    void loadChineseDictionary();
    // ★★★ 新增：在本地词典中查询 ★★★
    bool queryChineseLocally(const QString& character, WordDefinition& result);

    QNetworkAccessManager *m_networkManager;
    // ★★★ 新增：用于存储本地词典的哈希表 ★★★
    QHash<QString, QString> m_chineseDict; // 用于存储本地词典
};

#endif // DICTIONARYSERVICE_H
