#include "listenwifi.h"

#include "networkutils.h"
#include "wifiscanner.h"     //wifi扫描线程
#include "../Device/DeviceInfo.h"

#include<QTimer>
#include<QProcess>
#include<QScopedPointer>

ListenWifi ListenWifi::instance;  // 定义静态成员

ListenWifi::ListenWifi():interval(5000),timer(new QTimer()),scanner(new WifiScanner()){
    initSignals();
    ListenDevice::registerListener(this);
}

void ListenWifi::initSignals()
{
    connect(timer.get(), &QTimer::timeout, this, &ListenWifi::scanDevices);
   // connect(scanner, &WifiScanner::DeviceFound, this, &ListenWifi::handleDeviceFound);
    connect(scanner, &WifiScanner::scanningFinished, this, &ListenWifi::onScanningFinished);
}

void ListenWifi::startListening()
{
    timer->start(interval);
}

void ListenWifi::stopListening()
{
    timer->stop();
}


void ListenWifi::handleDeviceFound(const QString& ip, int port) {

}

void ListenWifi::scanDevices()
{
    timer->stop();
    netWorkUtils::WifiInfo wifi = netWorkUtils::activeWifiInterface();
    QString startIp, endIp;
    netWorkUtils::getIpRange(wifi.ip, wifi.netmask, startIp, endIp);   //计算ip范围
    qDebug() << startIp << "" << endIp;
    scanner->setWifiScanner(startIp,"192.168.1.20",5555,5555);  //设置默认5555端口
    scanner->startScanning();
}

void ListenWifi::onScanningFinished(QSet<ConnectInfo>& currentDevices){

    for(auto i:currentDevices){
        qDebug()<<"ip+port:"<<i.ipAddress<<i.ipAddress;
    }
   ListenDevice::updateChangeSet(currentDevices);   //发送qdebug
    timer->start(interval);
}
