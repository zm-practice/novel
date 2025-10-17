#include "ConfigManager.h"
#include <QCoreApplication>
#include <QDir>

ConfigManager& ConfigManager::instance()
{
    static ConfigManager ins;
    return ins;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent), m_lastPage(0)
{
    m_iniPath = QCoreApplication::applicationDirPath() + "/configmanager.ini";
    settings = new QSettings(m_iniPath, QSettings::IniFormat);
}

ConfigManager::~ConfigManager()
{
}

double ConfigManager::lastPage(const QString &book)
{
    QMutexLocker locker(&m_mutex);
    settings->beginGroup(book);
    m_lastPage =  settings->value("LastPage").toDouble();
    settings->endGroup();
    return m_lastPage;

}

double ConfigManager::totalPage(const QString &book)
{
    QMutexLocker locker(&m_mutex);
    settings->beginGroup(book);
    m_totalPage =  settings->value("TotalPage").toDouble();
    settings->endGroup();
    return m_totalPage;
}


void ConfigManager::setLastPage(const QString &book, double page)
{
    QMutexLocker locker(&m_mutex);
    settings->setValue(book + "/LastPage",     page);
    settings->sync();
}

void ConfigManager::setTotalPage(const QString &book, double page)
{
    QMutexLocker locker(&m_mutex);
    settings->setValue(book + "/TotalPage",     page);
    settings->sync();
}
