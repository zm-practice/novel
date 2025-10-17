#ifndef KEYWORDDIALOG_H
#define KEYWORDDIALOG_H

#include <QDialog>
#include <QTableWidget>

class KeywordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KeywordDialog(QWidget* parent = nullptr);

private slots:
    void onAdd();
    void onApply();
    void refreshTable();

private:
    QTableWidget* m_table;
};

#endif  // KEYWORDDIALOG_H
