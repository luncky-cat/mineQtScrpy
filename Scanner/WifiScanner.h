#ifndef WIFISCANNER_H
#define WIFISCANNER_H

#include <QObject>
#include <QThreadPool>
#include<QSet>
#include<QFuture>

#include <Device/DeviceInfo.h>


class WifiScanner:public QObject {
    Q_OBJECT

public:
    WifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort);
    WifiScanner();
    ~WifiScanner();
public:
    void startScanning();
    void setWifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort);

public slots:
    void handleDeviceFound(const QString& ip, quint16 port);
    //void handleDevicesList(QSet<ConnectInfo>& currentDevices);
signals:
    void DeviceFound(const QString& ip, quint16 port);  // 转发信号
private:
    QString startIp;
    QString endIp;
    quint16 startPort;
    quint16 endPort;
    QThreadPool* threadPool;
    QSet<QString> scannedAddresses;
    QList<QFuture<void>> futures;
    QSet<ConnectInfo> currentDevices;
signals:
    void scanningFinished(QSet<ConnectInfo>& currentDevices);  // 扫描完成信号
};

#endif // WIFISCANNER_H
