// dictionaryservice.cpp
#include "dictionaryservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QDebug>

DictionaryService::DictionaryService(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &DictionaryService::onReplyFinished);

    // 调用在 .h 文件中声明的函数
    loadChineseDictionary();
}

void DictionaryService::loadChineseDictionary()
{
    QFile file(":/pinyin/data/pinyin.txt"); // 确保资源路径正确
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "错误：无法加载本地中文字典文件！";
        return;
    }
    m_chineseDict.clear();
    QTextStream in(&file);

    // 2. ★★★ 按照新的格式逐行解析 ★★★
    while (!in.atEnd()) {
        QString line = in.readLine();

        // a. 忽略注释行和空行
        if (line.startsWith('#') || line.trimmed().isEmpty()) {
            continue;
        }

        // b. 找到 '#' 分隔符，分割汉字和其他部分
        int commentPos = line.indexOf('#');
        if (commentPos == -1) {
            continue; // 格式不符，跳过
        }

        // 提取汉字 (在 '#' 之后，并去除首尾空格)
        QString hanzi = line.mid(commentPos + 1).trimmed();

        // 提取汉字之前的部分
        QString dataPart = line.left(commentPos).trimmed();

        // c. 找到 ':' 分隔符，分割 Unicode 码点和拼音
        int colonPos = dataPart.indexOf(':');
        if (colonPos == -1) {
            continue; // 格式不符，跳过
        }

        // 提取拼音部分 (在 ':' 之后，并去除首尾空格)
        QString pinyins = dataPart.mid(colonPos + 1).trimmed();

        // d. 一个汉字可能有多个读音，用逗号分隔。我们只取第一个。
        QString firstPinyin = pinyins.split(',').first();

        // e. 存入哈希表。因为这个数据源没有释义，我们只存拼音。
        //    Key: 汉字, Value: 拼音
        if (!hanzi.isEmpty() && !firstPinyin.isEmpty()) {
            m_chineseDict[hanzi] = firstPinyin;
        }
    }
    file.close();
    qDebug() << "本地中文字典加载完成，共" << m_chineseDict.size() << "个条目。";
}


void DictionaryService::query(const QString &text)
{
    QString trimmedText = text.trimmed();
    qDebug() << "DictionaryService::query 被调用，查询内容:" << trimmedText;

    if (trimmedText.isEmpty()) {
        qDebug() << "查询内容为空";
        return;
    }

    // ★★★ 改进：更准确的中文判断 ★★★
    QChar firstChar = trimmedText.at(0);
    bool isChinese = (firstChar.unicode() >= 0x4E00 && firstChar.unicode() <= 0x9FFF);

    qDebug() << "第一个字符:" << firstChar
             << "Unicode:" << QString::number(firstChar.unicode(), 16)
             << "是否为中文:" << isChinese;

    if (isChinese)
    {
        qDebug() << "使用本地词典查询中文";
        qDebug() << "字典大小:" << m_chineseDict.size();

        WordDefinition result;
        if (queryChineseLocally(trimmedText, result)) {
            qDebug() << "本地查询成功:" << result.displayWord << result.phonetic;
            emit querySuccess(result);
        } else {
            qDebug() << "本地词典中未找到:" << trimmedText;

            // ★★★ 改进：提供更详细的错误信息 ★★★
            QString errorMsg = QString("在本地词典中找不到'%1'").arg(trimmedText);
            if (m_chineseDict.isEmpty()) {
                errorMsg += "\n(词典未加载或为空)";
            }
            emit queryError(errorMsg);
        }
    }
    else
    {
        qDebug() << "使用在线API查询英文";
        QUrl url("https://api.dictionaryapi.dev/api/v2/entries/en/" + trimmedText);
        QNetworkRequest request(url);
        m_networkManager->get(request);
    }
}


// ★★★ 同时，我们需要修改中文查询函数，因为它现在没有释义了 ★★★
bool DictionaryService::queryChineseLocally(const QString& character, WordDefinition &result)
{
    if (m_chineseDict.contains(character)) {
        result.query = character;
        result.displayWord = character;
        result.phonetic = m_chineseDict[character]; // 直接获取拼音
        result.definition = "（暂无释义）";         // 提供一个占位符
        return true;
    }
    return false;
}

void DictionaryService::onReplyFinished(QNetworkReply *reply)
{
    // ... (onReplyFinished 的完整实现，和之前提供的一样)
    // a. 首先，检查网络层面是否有错误 (如DNS解析失败, 404 Not Found等)
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network Error:" << reply->errorString();
        emit queryError("网络错误: " + reply->errorString());
        reply->deleteLater(); // 必须调用 deleteLater() 来释放 reply 对象
        return;
    }

    // b. 读取服务器返回的所有数据
    QByteArray responseData = reply->readAll();

    // c. 将数据解析为 JSON 文档
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    // d. --- 核心：根据 dictionaryapi.dev 的格式解析 JSON ---

    // API 返回的数据最外层是一个 JSON 数组
    if (!jsonDoc.isArray()) {
        // 有时候查不到单词，API会返回一个对象而不是数组，我们需要处理这种情况
        if (jsonDoc.isObject()) {
            qDebug() << "Word not found, API returned an object:" << jsonDoc.object();
            emit queryError("未找到该单词的释义");
        } else {
            qDebug() << "JSON Parse Error: Response is not an array.";
            emit queryError("API返回格式错误");
        }
        reply->deleteLater();
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    if (jsonArray.isEmpty()) {
        qDebug() << "JSON Parse Error: Response array is empty.";
        emit queryError("API返回内容为空");
        reply->deleteLater();
        return;
    }

    // 通常我们只需要第一组释义，所以取数组的第一个元素
    QJsonObject jsonObj = jsonArray.first().toObject();

    WordDefinition result;
    result.query = jsonObj["word"].toString();
    result.displayWord = result.query;

    // --- 提取音标 ---
    // "phonetics" 是一个数组，里面可能包含英式、美式等多种音标
    QJsonArray phoneticsArray = jsonObj["phonetics"].toArray();
    for (const QJsonValue &phoneticValue : phoneticsArray) {
        QJsonObject phoneticObj = phoneticValue.toObject();
        // 我们优先寻找带有 "text" 字段的音标
        if (phoneticObj.contains("text") && !phoneticObj["text"].toString().isEmpty()) {
            result.phonetic = phoneticObj["text"].toString();
            break; // 找到一个就停止
        }
    }
    // 如果上面的循环没找到，就用顶层的 "phonetic" 字段作为备用
    if (result.phonetic.isEmpty()) {
        result.phonetic = jsonObj["phonetic"].toString();
    }


    // --- 提取释义 ---
    // "meanings" 是一个数组，包含名词、动词等不同词性的释义
    QJsonArray meaningsArray = jsonObj["meanings"].toArray();
    QStringList definitions;
    for (const QJsonValue &meaningValue : meaningsArray) {
        QJsonObject meaningObj = meaningValue.toObject();
        QString partOfSpeech = meaningObj["partOfSpeech"].toString(); // 词性 (noun, verb, etc.)

        // "definitions" 是该词性下的释义数组
        QJsonArray definitionsArray = meaningObj["definitions"].toArray();
        if (!definitionsArray.isEmpty()) {
            // 我们只取每种词性的第一条释义，并附上词性，让结果更清晰
            QString firstDefinition = definitionsArray.first().toObject()["definition"].toString();
            definitions.append(QString("[%1] %2").arg(partOfSpeech).arg(firstDefinition));
        }
    }

    if (definitions.isEmpty()) {
        emit queryError("未找到有效的释义");
    } else {
        // 将所有词性的释义用换行符合并成一个字符串
        result.definition = definitions.join("\n\n");
        emit querySuccess(result); // 发射成功信号
    }

    // e. 最后，清理 reply 对象
    reply->deleteLater();
}

