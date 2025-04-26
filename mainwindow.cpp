#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "showlayout.h"
#include "Manager/androidmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    showLayout *show=new showLayout();
    QVBoxLayout * layout=new QVBoxLayout();
    layout->addWidget(show);
    ui->centralwidget->setLayout(layout);
    manger = new androidManager();
    manger->listen();
}

MainWindow::~MainWindow()
{
    delete ui;
}
