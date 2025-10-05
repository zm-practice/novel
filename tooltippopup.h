#ifndef TOOLTIPPOPUP_H
#define TOOLTIPPOPUP_H

#include <QDialog>
#include "dictionaryservice.h" // 包含 WordDefinition 结构体

namespace Ui {
class TooltipPopup;
}

class TooltipPopup : public QDialog
{
    Q_OBJECT

public:
    explicit TooltipPopup(QWidget *parent = nullptr);
    ~TooltipPopup();

    // 公共接口，用于显示释义
    void showDefinition(const WordDefinition& def, const QPoint& globalPos);

protected:
    // 重写事件，实现点击窗口外自动关闭
    void focusOutEvent(QFocusEvent *event) override;

private:
    Ui::TooltipPopup *ui;
};

#endif // TOOLTIPPOPUP_H
