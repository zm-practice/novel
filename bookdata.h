#ifndef BOOKDATA_H
#define BOOKDATA_H

#include <QString>
#include <QColor>
#include <QTextCharFormat>
//书籍信息
struct BookInfo
{
    QString id;
    QString title;
    QString filePath;
    QString coverPath;
};
//书签
struct BookmarkInfo
{
    enum Type { Bookmark, Note };
    Type type;
    int chapterIndex;
    int startPos;
    int endPos;
    QString iconPath;
    QColor backgroundColor;
    int underlineStyle;
    QColor underlineColor;
    QString content;
};

#endif // BOOKDATA_H
