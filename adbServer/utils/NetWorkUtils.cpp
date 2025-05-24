#include "NetWorkUtils.h"

NetWorkUtils::NetWorkUtils(QObject *parent)
    : QObject{parent}
{}


bool NetWorkUtils::getActiveWifiInfo(std::string&ip_,std::string &netmask_) {
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& interface : interfaces) {
        if (!(interface.flags() & QNetworkInterface::IsUp) || !(interface.flags() & QNetworkInterface::IsRunning))
            continue;

        if (!interface.humanReadableName().contains("WLAN", Qt::CaseInsensitive))
            continue;

        for (const QNetworkAddressEntry& entry : interface.addressEntries()) {
            const QHostAddress& ip = entry.ip();
            if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isNull()) {
                ip_=ip.toString().toStdString();
                netmask_=entry.netmask().toString().toStdString();
                return true;
            }
        }
    }
    return false;
}

bool NetWorkUtils::getIpRange(const std::string& ip, const std::string& netmask, std::string& startIp,std::string& endIp) {
    QHostAddress ipAddr(QString::fromStdString(ip));
    QHostAddress netmaskAddr(QString::fromStdString(netmask));  //后期内部维护一个

    // 只处理 IPv4 地址
    if (ipAddr.protocol() != QAbstractSocket::IPv4Protocol ||
        netmaskAddr.protocol() != QAbstractSocket::IPv4Protocol) {
        return false;
    }

    quint32 ipInt = ipAddr.toIPv4Address();
    quint32 maskInt = netmaskAddr.toIPv4Address();

    // 掩码必须有效，不能是全 0 或全 1（广播）
    if (maskInt == 0 || maskInt == 0xFFFFFFFF) {
        return false;
    }

    quint32 network = ipInt & maskInt;
    quint32 broadcast = network | (~maskInt);

    // 至少要有 2 个主机地址
    if ((broadcast - network) <= 1) {
        return false;
    }

    quint32 start = network + 1;
    quint32 end = broadcast - 1;

    startIp = QHostAddress(start).toString().toStdString();
    endIp = QHostAddress(end).toString().toStdString();

    return true;
}


