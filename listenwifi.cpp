#include "listenwifi.h"
#include "networkutils.h"
#include "wifiscanner.h"     //wifi扫描线程
#include "DeviceInfo.h"

#include<QTimer>
#include<QProcess>

ListenWifi ListenWifi::instance;  // 定义静态成员

ListenWifi::ListenWifi() {
    init();
    ListenDevice::registerListener(this);
}

ListenWifi::~ListenWifi() {}

void ListenWifi::init()
{
    timer = new QTimer(this);
    scanner = new WifiScanner();
    connect(timer,&QTimer::timeout,this,&ListenWifi::scanDevices);
    connect(scanner, &WifiScanner::DeviceFound, this, &ListenWifi::handleDeviceFound);
    interval = 2000;
}

void ListenWifi::startListening()
{
    timer->start(interval);
}

void ListenWifi::stopListening()
{

}


void ListenWifi::handleDeviceFound(const QString& ip, int port) {
    QSet<DeviceInfo> currentDevices;
    QString fullAddress = QString("%1:%2").arg(ip).arg(port);
    QProcess* process = new QProcess();
    process->start("scrcpy/adb.exe", QStringList() << "-s" << fullAddress << "shell" << "getprop" << "ro.serialno");
    process->waitForFinished();

    QString serialOutput = process->readAllStandardOutput();
    qDebug() << "Device serial number:" << serialOutput;

    DeviceInfo info;
    info.deviceType = QString("WIFI");
    info.ipAddress = ip;
    info.port = port;
    info.serialNumber = serialOutput;
    currentDevices.insert(info);

    ListenDevice::updateChangeSet(currentDevices);   //立即执行
}


void ListenWifi::scanDevices()
{
    netWorkUtils::WifiInfo wifi = netWorkUtils::activeWifiInterface();
    QString startIp, endIp;
    netWorkUtils::getIpRange(wifi.ip, wifi.netmask, startIp, endIp);   //计算ip范围
    qDebug() << startIp << "" << endIp;
    scanner->setWifiScanner(startIp,endIp,5555,5555);  //设置默认5555端口
    scanner->startScanning();
}

