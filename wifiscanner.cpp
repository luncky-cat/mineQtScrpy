#include "WifiScanner.h"
#include "WifiScannerTask.h"

#include <QtConcurrent>
#include<QHostAddress>


WifiScanner::WifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort)
:startIp(startIp), endIp(endIp), startPort(startPort), endPort(endPort) {
    threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(4);  // 设置最大线程数为 4
}

WifiScanner::WifiScanner() {
    threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(4);  // 设置最大线程数为 4
}

void WifiScanner::startScanning() {

    if (!validateAndStartScan(startIp, endIp, startPort, endPort)) {
        qDebug() << "Scan aborted due to invalid parameters.";
        return;  // 参数无效，返回
    }

    quint32 start = QHostAddress(startIp).toIPv4Address();
    quint32 end = QHostAddress(endIp).toIPv4Address();
  //  QProcess* process = new QProcess();
    for (quint32 ip = start; ip <= end; ++ip) {
        QString ipStr = QHostAddress(ip).toString();
        QtConcurrent::run([this, ipStr]() -> QPair<QString, quint16> {
            WifiScannerTask task(ipStr, startPort, endPort);
            QPair<QString, quint16> result = task.run();
            return result;  // 返回扫描结果
            }).then([this](QPair<QString, quint16> result) {
                if (!result.first.isEmpty()) {
                    this->handleDeviceFound(result.first, result.second);
                }
                else {
                    //将不符合的ipStr,port加入列表，不在扫描
                }
                });
    }
}

void WifiScanner::setWifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort)
{
    this->startIp = startIp;
    this->endIp = endIp;
    this->startPort = startPort;
    this->endPort = endPort;
}

void WifiScanner::handleDeviceFound(const QString& ip, quint16 port) {
    qDebug() << "设备发现，IP:" << ip << ", 端口:" << port;
    emit DeviceFound(ip, port);  // 转发设备信息
}


bool WifiScanner::isValidIp(const QString& ip) {
    QHostAddress address(ip);
    return address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol;
}

bool WifiScanner::isValidPort(quint16 port) {
    return port >= 1 && port <= 65535;
}

bool WifiScanner::validateAndStartScan(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort) {
    // 检查 startIp 和 endIp 是否有效
    if (!isValidIp(startIp) || !isValidIp(endIp)) {
        qDebug() << "Invalid IP address.";
        return false;
    }

    // 检查端口范围是否有效
    if (!isValidPort(startPort) || !isValidPort(endPort)) {
        qDebug() << "Invalid port range.";
        return false;
    }

    // 如果所有条件都有效，执行扫描
    qDebug() << "Starting scan...";
    return true;
}
