#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bookmanager.h"
#include "sidenavigationview.h" // 必须包含自定义控件头文件

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_workBookManager = new BookManager(this);
    m_leisureBookManager = new BookManager(this);

    m_workBookManager->setSettingsName("WorkBooks");
    m_leisureBookManager->setSettingsName("LeisureBooks");

    ui->workSpaceView->setBookManager(m_workBookManager);
    ui->leisureSpaceView->setBookManager(m_leisureBookManager);

    m_isWorkSpace = true;
    ui->mainstackedWidget->setCurrentWidget(ui->workSpaceWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionToggleSpace_triggered()
{
    if (m_isWorkSpace)
    {
        ui->mainstackedWidget->setCurrentWidget(ui->leisureSpaceWidget);
        ui->actionToggleSpace->setText("切换到工作空间(&S)");
        m_isWorkSpace = false;
    }
    else
    {
        ui->mainstackedWidget->setCurrentWidget(ui->workSpaceWidget);
        ui->actionToggleSpace->setText("切换到休闲空间(&S)");
        m_isWorkSpace = true;
    }
}
