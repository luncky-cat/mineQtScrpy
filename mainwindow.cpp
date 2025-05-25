#include "mainwindow.h"
#include "CustomWidgets/SidebarWidget.h"
#include "CustomWidgets/showlayout.h"
#include "ui_mainwindow.h"
#include "Manager/androidmanager.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建展示区和侧边栏
    showLayout *show = new showLayout();
    sidebar = new SidebarWidget(this);

    // 设置主布局为横向布局
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(sidebar);   // 左侧栏
    mainLayout->addWidget(show, 1);   // 中间展示区，占比更大

    // 应用到 centralWidget
    QWidget *central = ui->centralwidget;
    central->setLayout(mainLayout);

    // 可选：连接侧边栏隐藏按钮
    connect(sidebar, &SidebarWidget::toggleVisibility, this, [=]() {
        sidebar->setVisible(!sidebar->isVisible());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
