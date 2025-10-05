#include "tooltippopup.h"
#include "ui_tooltippopup.h"
#include <QFocusEvent>

TooltipPopup::TooltipPopup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TooltipPopup)
{
    ui->setupUi(this);
    // ★★★ 在代码中设置窗口标志 ★★★
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
}

TooltipPopup::~TooltipPopup()
{
    delete ui;
}

void TooltipPopup::showDefinition(const WordDefinition &def, const QPoint &globalPos)
{
    ui->wordLabel->setText(def.displayWord);
    ui->phoneticLabel->setText(def.phonetic);
    ui->definitionLabel->setText(def.definition);

    // 必须在 show() 之前调用 adjustSize()
    adjustSize();
    move(globalPos.x(), globalPos.y() + 15); // 移动到鼠标下方
    show();
    activateWindow(); // 确保能接收焦点事件
}

void TooltipPopup::focusOutEvent(QFocusEvent *event)
{
    QDialog::focusOutEvent(event);
    close(); // 当窗口失去焦点时，自动关闭
}
