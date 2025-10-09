// #include "tooltippopup.h"
// #include "ui_tooltippopup.h"
// #include <QFocusEvent>

// TooltipPopup::TooltipPopup(QWidget *parent) :
//     QDialog(parent),
//     ui(new Ui::TooltipPopup)
// {
//     ui->setupUi(this);
//     // ★★★ 在代码中设置窗口标志 ★★★
//     setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
// }

// TooltipPopup::~TooltipPopup()
// {
//     delete ui;
// }

// void TooltipPopup::showDefinition(const WordDefinition &def, const QPoint &globalPos)
// {
//     ui->wordLabel->setText(def.displayWord);
//     ui->phoneticLabel->setText(def.phonetic);
//     ui->definitionLabel->setText(def.definition);

//     // 必须在 show() 之前调用 adjustSize()
//     adjustSize();
//     move(globalPos.x(), globalPos.y() + 15); // 移动到鼠标下方
//     show();
//     activateWindow(); // 确保能接收焦点事件
// }

// void TooltipPopup::focusOutEvent(QFocusEvent *event)
// {
//     QDialog::focusOutEvent(event);
//     close(); // 当窗口失去焦点时，自动关闭
// }


#include "tooltippopup.h"
#include "ui_tooltippopup.h"
#include <QFocusEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

TooltipPopup::TooltipPopup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TooltipPopup)
{
    ui->setupUi(this);

    // ★★★ 关键修复: 使用更合适的窗口标志 ★★★
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    // 设置窗口属性
    setAttribute(Qt::WA_ShowWithoutActivating, false); // 允许激活窗口
    setAttribute(Qt::WA_DeleteOnClose, false); // 不要在关闭时删除
}

TooltipPopup::~TooltipPopup()
{
    delete ui;
}

void TooltipPopup::showDefinition(const WordDefinition &def, const QPoint &globalPos)
{
    qDebug() << "显示释义:" << def.displayWord; // 调试信息

    // 更新内容
    ui->wordLabel->setText(def.displayWord);
    ui->phoneticLabel->setText(def.phonetic);
    ui->definitionLabel->setText(def.definition);

    // 调整窗口大小以适应内容
    adjustSize();

    // ★★★ 改进: 确保弹窗不会超出屏幕边界 ★★★
    QPoint finalPos = globalPos;
    finalPos.setY(finalPos.y() + 20); // 在鼠标下方20像素

    // 获取屏幕几何信息
    QScreen *screen = QGuiApplication::screenAt(globalPos);
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();

        // 如果弹窗会超出屏幕右边界,向左移动
        if (finalPos.x() + width() > screenGeometry.right()) {
            finalPos.setX(screenGeometry.right() - width() - 10);
        }

        // 如果弹窗会超出屏幕下边界,显示在鼠标上方
        if (finalPos.y() + height() > screenGeometry.bottom()) {
            finalPos.setY(globalPos.y() - height() - 10);
        }
    }

    move(finalPos);
    show();
    raise(); // 确保窗口在最前面
    activateWindow(); // 激活窗口以接收焦点

    qDebug() << "弹窗已显示在:" << finalPos;
}

void TooltipPopup::focusOutEvent(QFocusEvent *event)
{
    qDebug() << "弹窗失去焦点,准备关闭";
    QDialog::focusOutEvent(event);
    hide(); // 使用 hide() 而不是 close(),避免重复创建
}
