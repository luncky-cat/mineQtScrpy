#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QNetworkInterface>


class netWorkUtils : public QObject      //网络监听类
{
    Q_OBJECT
public:
    explicit netWorkUtils(QObject *parent = nullptr);
    struct WifiInfo {
        QString ip;
        QString netmask;
    };
public:
    static bool getIpRange(const QString &ip, const QString &netmask, QString &startIp, QString &endIp);
    static WifiInfo activeWifiInterface();
signals:

};

#endif // NETWORKUTILS_H
