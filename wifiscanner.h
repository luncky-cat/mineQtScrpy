#ifndef WIFISCANNER_H
#define WIFISCANNER_H

#include <QObject>
#include <QThreadPool>

class WifiScanner:public QObject {
    Q_OBJECT
public:
    WifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort);
    WifiScanner();
    void startScanning();
    void setWifiScanner(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort);
    bool isValidIp(const QString& ip);
    bool isValidPort(quint16 port);
    bool validateAndStartScan(const QString& startIp, const QString& endIp, quint16 startPort, quint16 endPort);
public slots:
    void handleDeviceFound(const QString& ip, quint16 port);
signals:
    void DeviceFound(const QString& ip, quint16 port);  // 转发信号
private:
    QString startIp;
    QString endIp;
    quint16 startPort;
    quint16 endPort;
    QThreadPool* threadPool;
};

#endif // WIFISCANNER_H
