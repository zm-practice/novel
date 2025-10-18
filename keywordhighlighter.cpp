#include "KeyWordHighlighter.h"
#include <QTextCursor>

KeyWordHighlighter::KeyWordHighlighter(QTextBrowser *browser, QObject *parent)
    : QObject(parent), m_browser(browser)
{
}

// 在keywordhighlighter.cpp中,修改highlight()以避免覆盖书签
void KeyWordHighlighter::highlight()
{
    setRules(KeywordManager::instance().rules());

    for (const KeywordRule& rule : rules) {
        QRegularExpression re(rule.useRegex ?
                                  QRegularExpression(rule.word) :
                                  QRegularExpression(QRegularExpression::escape(rule.word)));
        QRegularExpressionMatchIterator it = re.globalMatch(m_browser->toPlainText());

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int pos = match.capturedStart();
            int len = match.capturedLength();

            QTextCursor cursor(m_browser->document());
            cursor.setPosition(pos);
            cursor.setPosition(pos + len, QTextCursor::KeepAnchor);

            // 检查是否是书签区域(UserProperty被占用) ✅
            QTextCharFormat existingFmt = cursor.charFormat();
            if (existingFmt.property(QTextFormat::UserProperty).isNull()) {
                // 不是书签区域,可以高亮
                QTextCharFormat fmt;
                fmt.setForeground(rule.color);
                fmt.setFontWeight(QFont::Bold);
                cursor.mergeCharFormat(fmt);
            }
        }
    }
}
