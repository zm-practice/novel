#ifndef KEYWORDHIGHLIGHTER_H
#define KEYWORDHIGHLIGHTER_H

#include <QObject>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "keywordmanager.h"

class KeyWordHighlighter : public QObject
{
    Q_OBJECT
public:
    explicit KeyWordHighlighter(QTextBrowser *browser, QObject *parent = nullptr);

    /* 外部调用：加载规则 */
    void setRules(const QVector<KeywordRule>& r)
    {
        rules = r;
        //        rehighlight();
    }

public slots:
    void highlight();

private:
    QVector<KeywordRule> rules;
    QTextBrowser *m_browser;
};

#endif // KEYWORDHIGHLIGHTER_H
