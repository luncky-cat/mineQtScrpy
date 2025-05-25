#include "mainwindow.h"

#include <QApplication>

#include"utils/ThreadPool.h"
#include "adbServer/code/AdbServer.h"

#include"code/ConfigCenter.h"
#include"code/ScannerManager.h"

int main(int argc, char *argv[])
{
    ConfigCenter::getInstance().registerAllConfigs();
    ScannerManager::getInstance().registerAllScanners();
    ThreadPool::getInstance().submit([] {
        AdbServer::getInstance().start();
    });

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
