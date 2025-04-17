

#include "networkutils.h"

netWorkUtils::netWorkUtils(QObject *parent)
    : QObject{parent}
{}


netWorkUtils::WifiInfo netWorkUtils::activeWifiInterface() {
    // 获取所有网络接口
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    // 遍历所有接口
    for (const QNetworkInterface &interface : interfaces) {
        // 打印接口名称
        qDebug() << "Interface name:" << interface.humanReadableName();
        if (interface.flags() & QNetworkInterface::IsUp &&interface.flags() & QNetworkInterface::IsRunning) {
            if (interface.humanReadableName().contains("WLAN", Qt::CaseInsensitive)) {
                qDebug() << "Found active Wi-Fi interface:" << interface.humanReadableName();
                // 打印该接口的 IP 地址和子网掩码
                QList<QNetworkAddressEntry> entries = interface.addressEntries();
                for (const QNetworkAddressEntry &entry : entries) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        qDebug() << "IP Address:" << entry.ip().toString();
                        qDebug() << "Netmask:" << entry.netmask().toString();
                        //activeWifi=WifiInfo{entry.ip().toString(),entry.netmask().toString()};
                        return WifiInfo{entry.ip().toString(), entry.netmask().toString()};
                    }
                }
            }
        }
    }
}

// 用于获得网段范围的函数（将 IP 地址和子网掩码组合成网段）
bool netWorkUtils::getIpRange(const QString &ip, const QString &netmask, QString &startIp, QString &endIp) {
    QHostAddress ipAddr(ip);
    QHostAddress netmaskAddr(netmask);

    if (ipAddr.protocol() != QAbstractSocket::IPv4Protocol ||
        netmaskAddr.protocol() != QAbstractSocket::IPv4Protocol)
        return false;

    quint32 ipInt = ipAddr.toIPv4Address();
    quint32 netmaskInt = netmaskAddr.toIPv4Address();

    // 只允许合法子网掩码
    if (netmaskInt == 0 || netmaskInt == 0xFFFFFFFF)
        return false;

    quint32 networkAddrInt = ipInt & netmaskInt;
    quint32 broadcastAddrInt = networkAddrInt | (~netmaskInt);

    // 若范围不足 2 个主机地址，则无意义
    if (broadcastAddrInt - networkAddrInt <= 1)
        return false;

    quint32 startIpInt = networkAddrInt + 1;
    quint32 endIpInt = broadcastAddrInt - 1;

    startIp = QHostAddress(startIpInt).toString();
    endIp = QHostAddress(endIpInt).toString();
    qDebug() << "Start IP:" << startIp;
    qDebug() << "End IP:" << endIp;
    return true;
}
