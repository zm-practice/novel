#include "KeyWordHighlighter.h"
#include <QTextCursor>

KeyWordHighlighter::KeyWordHighlighter(QTextBrowser *browser, QObject *parent)
    : QObject(parent), m_browser(browser)
{
}

void KeyWordHighlighter::highlight()
{
    setRules(KeywordManager::instance().rules());
    // 1. 把光标重置到开头
    QTextCursor cursor(m_browser->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat clean;
    clean.setForeground(QBrush());  // 恢复默认
    clean.setFontWeight(QFont::Normal);
    cursor.mergeCharFormat(clean);

    // 2. 逐个关键词刷格式
    for (const KeywordRule& rule : rules) {
        QRegularExpression              re(rule.useRegex ? QRegularExpression(rule.word) :
                                  QRegularExpression(QRegularExpression::escape(rule.word)));
        QRegularExpressionMatchIterator it = re.globalMatch(m_browser->toPlainText());
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int                     pos   = match.capturedStart();
            int                     len   = match.capturedLength();
            cursor.setPosition(pos);
            cursor.setPosition(pos + len, QTextCursor::KeepAnchor);
            QTextCharFormat fmt;
            fmt.setForeground(rule.color);
            fmt.setFontWeight(QFont::Bold);
            cursor.mergeCharFormat(fmt);
        }
    }
}
