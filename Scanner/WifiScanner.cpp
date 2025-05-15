#include "WifiScanner.h"
#include "WifiScannerTask.h"

#include <QtConcurrent>
#include<QHostAddress>


WifiScanner::WifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort)
    :startIp(startIp), endIp(endIp), startPort(startPort),
    endPort(endPort),threadPool(new QThreadPool())
{
    futures.clear();
}

WifiScanner::WifiScanner():threadPool(new QThreadPool()) {
    futures.clear();
}

WifiScanner::~WifiScanner()
{

}

void WifiScanner::startScanning() {

    currentDevices.clear();

    threadPool->setMaxThreadCount(6);

    quint32 start = QHostAddress(startIp).toIPv4Address();
    quint32 end = QHostAddress(endIp).toIPv4Address();

    for (quint32 ip = start; ip <= end; ++ip) {
        QString ipStr = QHostAddress(ip).toString();
        if (!scannedAddresses.contains(ipStr)) {
            scannedAddresses.insert(ipStr);

            QFuture<void> future=QtConcurrent::run([this,ipStr]() {

                WifiScannerTask *task = new WifiScannerTask(ipStr);  // 使用堆对象(ipStr);  //默认都是5555 QueuedConnection
                connect(task,&WifiScannerTask::deviceFound,this,&WifiScanner::handleDeviceFound,Qt::DirectConnection);
                task->run();
                delete task;
            });
            futures.append(future);  // 将每个任务的QFuture添加到列表中
        }
    }

    for (auto& future : futures) {
        future.waitForFinished();  // 阻塞直到任务完成
    }

    qDebug()<<"本次任务完成";
    emit scanningFinished(currentDevices);
}


void WifiScanner::setWifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort)
{
    this->startIp = startIp;
    this->endIp = endIp;
    this->startPort = startPort;
    this->endPort = endPort;
}

void WifiScanner::handleDeviceFound(const QString& ip, quint16 port) {

    qDebug() << "scanner组织结构:" << ip << ", 端口:" << port;
    ConnectInfo info;
    info.deviceId=ip;
    info.ConnectType=ConnectType::WiFi;
    info.port=port;
    info.ipAddress=ip;
    currentDevices.insert(info);
}


