#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QNetworkInterface>


class NetWorkUtils : public QObject      //网络监听类
{
    Q_OBJECT
public:
    explicit NetWorkUtils(QObject *parent = nullptr);
public:
    static bool getActiveWifiInfo(std::string &ip_, std::string &netmask_);
    static bool getIpRange(const std::string &ip, const std::string &netmask, std::string &startIp, std::string &endIp);
};

#endif // NetWorkUtils_H
