#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QObject>
#include <QString>
#include <QFont>
#include <QColor>


class SettingsManager : public QObject
{
    Q_OBJECT
public:
    // 定义背景模式的枚举
    enum BackgroundMode {
        Default,      // 默认（根据日夜模式决定）
        PaperYellow,  // 纸张黄
        GreenBean,    // 豆沙绿
        CustomImage   // 自定义图片
    };
    Q_ENUM(BackgroundMode) // 注册枚举，以便在信号槽中使用
    // 获取全局唯一实例的静态方法
    static SettingsManager& instance();

    // --- 公共接口 ---
    QString fontFamily() const;
    void setFontFamily(const QString& family);

    int fontSize() const;
    void setFontSize(int size);

    bool isNightMode() const;
    void setNightMode(bool isNight);

    BackgroundMode backgroundMode() const;
    void setBackgroundMode(BackgroundMode mode);

    QString customImagePath() const;
    void setCustomImagePath(const QString& path);

    // 根据日夜模式返回对应的颜色
    QColor backgroundColor() const;
    QColor textColor() const;

    // ★★★ 在这里添加新的接口声明 ★★★
    QString bookmarkIconPath() const;
    void setBookmarkIconPath(const QString& path);
//自动阅读！！！
    // ★★★ 新增接口 ★★★
    int autoScrollSpeed() const;
    void setAutoScrollSpeed(int speed);


signals:
    // 当任何设置发生改变时，发射此信号
    void settingsChanged();

private:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();
    // 禁止拷贝和赋值
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    void loadSettings();
    void saveSettings();

    // 私有成员变量，存储当前的设置
    QString m_fontFamily;
    int m_fontSize;
    bool m_isNightMode;

    // ★★★ 新增成员变量 ★★★，设置背景
    BackgroundMode m_backgroundMode;
    QString m_customImagePath;
    // ★★★ 在这里添加新的成员变量声明 ★★★
    QString m_bookmarkIconPath;

 //自动阅读！！！！
    // ★★★ 新增成员变量 ★★★
    int m_autoScrollSpeed; // 速度值，数值越小滚得越快
};

#endif // SETTINGSMANAGER_H
