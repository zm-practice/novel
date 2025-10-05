//这个类我们将实现为单例，意味着整个程序中只有一个它的实例。
#include "settingsmanager.h"
#include <QSettings>
#include <QVariant> // 用于在 QSettings 中存储枚举
SettingsManager& SettingsManager::instance()
{
    static SettingsManager self; // C++11 保证了静态局部变量的线程安全初始化
    return self;
}

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
    loadSettings();
}
SettingsManager::~SettingsManager() {}

void SettingsManager::loadSettings()
{
    QSettings settings("MyNovelReader", "Appearance");
    m_fontFamily = settings.value("fontFamily", "Microsoft YaHei").toString();
    m_fontSize = settings.value("fontSize", 18).toInt();
    m_isNightMode = settings.value("isNightMode", false).toBool();
    m_backgroundMode = static_cast<BackgroundMode>(settings.value("backgroundMode", Default).toInt());
    m_customImagePath = settings.value("customImagePath", "").toString();

    // ★★★ 添加对新设置的加载，并提供一个默认值 ★★★
    // 确保你的 .qrc 资源文件里确实有一个叫 feather.png 的文件
    m_bookmarkIconPath = settings.value("bookmarkIconPath", ":/icon1/icons/羽毛笔.png").toString();

//自动阅读！！！！
    // ★★★ 加载新设置，默认值设为 100 毫秒 ★★★
    m_autoScrollSpeed = settings.value("autoScrollSpeed", 100).toInt();

}

void SettingsManager::saveSettings()
{
    QSettings settings("MyNovelReader", "Appearance");
    settings.setValue("fontFamily", m_fontFamily);
    settings.setValue("fontSize", m_fontSize);
    settings.setValue("isNightMode", m_isNightMode);

    settings.setValue("backgroundMode", static_cast<int>(m_backgroundMode));
    settings.setValue("customImagePath", m_customImagePath);

    // ★★★ 添加对新设置的保存 ★★★
    settings.setValue("bookmarkIconPath", m_bookmarkIconPath);
//自动阅读！！！
    // ★★★ 保存新设置 ★★★
    settings.setValue("autoScrollSpeed", m_autoScrollSpeed);

}


//自动阅读！！！
// --- 实现新的 Getter 和 Setter ---
int SettingsManager::autoScrollSpeed() const { return m_autoScrollSpeed; }

void SettingsManager::setAutoScrollSpeed(int speed)
{
    if (m_autoScrollSpeed != speed) {
        m_autoScrollSpeed = speed;
        saveSettings();
        emit settingsChanged(); // 同样发射信号，让UI可以响应
    }
}


QString SettingsManager::fontFamily() const { return m_fontFamily; }
int SettingsManager::fontSize() const { return m_fontSize; }
bool SettingsManager::isNightMode() const { return m_isNightMode; }

// --- 核心逻辑：根据一个设置（夜间模式）派生出多个值（颜色） ---
QColor SettingsManager::backgroundColor() const
{
    // 如果是自定义图片模式，返回一个透明色，让图片显示出来
    if (m_backgroundMode == CustomImage && !m_customImagePath.isEmpty()) {
        return Qt::transparent;
    }

    switch(m_backgroundMode) {
    case PaperYellow: return QColor("#f5f5dc");
    case GreenBean:   return QColor("#c7edcc");
    case Default:
    default:
        return m_isNightMode ? QColor("#1e1e1e") : QColor("#f5f5dc"); // 深灰 vs 米黄
    }

}
QColor SettingsManager::textColor() const
{
    return m_isNightMode ? QColor("#dcdcdc") : QColor("#333333"); // 浅灰 vs 深黑
}

// --- Setters: 当设置改变时，保存并发出信号 ---
void SettingsManager::setFontFamily(const QString &family)
{
    if (m_fontFamily != family) {
        m_fontFamily = family;
        saveSettings();
        emit settingsChanged();
    }
}

void SettingsManager::setFontSize(int size)
{
    if (m_fontSize != size) {
        m_fontSize = size;
        saveSettings();
        emit settingsChanged();
    }
}

void SettingsManager::setNightMode(bool isNight)
{
    if (m_isNightMode != isNight) {
        m_isNightMode = isNight;
        saveSettings();
        emit settingsChanged();
    }
}
// --- 实现新的 Getter 和 Setter ---
SettingsManager::BackgroundMode SettingsManager::backgroundMode() const { return m_backgroundMode; }
QString SettingsManager::customImagePath() const { return m_customImagePath; }

void SettingsManager::setBackgroundMode(BackgroundMode mode)
{
    if (m_backgroundMode != mode) {
        m_backgroundMode = mode;
        saveSettings();
        emit settingsChanged();
    }
}

void SettingsManager::setCustomImagePath(const QString &path)
{
    if (m_customImagePath != path) {
        m_customImagePath = path;
        // 只有当模式已经是自定义图片时，才需要立即保存和更新
        if(m_backgroundMode == CustomImage) {
            saveSettings();
            emit settingsChanged();
        }
    }
}

// --- ★★★ 实现新的 Getter 和 Setter ★★★ ---

QString SettingsManager::bookmarkIconPath() const
{
    return m_bookmarkIconPath;
}

void SettingsManager::setBookmarkIconPath(const QString &path)
{
    if (m_bookmarkIconPath != path) {
        m_bookmarkIconPath = path;
        saveSettings();      // 保存更改
        emit settingsChanged(); // 发出信号，通知界面刷新
    }
}
