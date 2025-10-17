#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    static ConfigManager& instance();

    /* 读取 */
    double     lastPage(const QString &book);
    double     totalPage(const QString &book);

    /* 写入 */
    void setLastPage(const QString &book, double page);
    void setTotalPage(const QString &book, double page);

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void load();

    mutable QMutex m_mutex;
    QSettings* settings; /*!< 设置类 */

    QString        m_iniPath;
    QString        m_lastFilePath;
    double         m_lastPage;
    double         m_totalPage;
};

#endif // CONFIGMANAGER_H
