// // bookdata.h
// #ifndef BOOKDATA_H
// #define BOOKDATA_H

// #include <QString>
// #include <QColor>
// #include <QTextCharFormat> // ★★★ 补上头文件

// struct BookInfo
// {
//     QString title;
//     QString filePath;
//     QString coverPath;
// };

// struct BookmarkInfo
// {
//     int chapterIndex;
//     int startPos;
//     int endPos;
//     QString iconPath;
//     QColor backgroundColor;
//     int underlineStyle;
//     QColor underlineColor;
// };

// #endif // BOOKDATA_H




#ifndef BOOKDATA_H
#define BOOKDATA_H

#include <QString>
#include <QColor>
#include <QTextCharFormat>

struct BookInfo
{
    QString title;
    QString filePath;
    QString coverPath;
};

struct BookmarkInfo
{
    int chapterIndex;
    int startPos;
    int endPos;
    QString iconPath;
    QColor backgroundColor;
    int underlineStyle;
    QColor underlineColor;
};

#endif // BOOKDATA_H
