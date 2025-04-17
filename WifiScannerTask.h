#ifndef WIFISCANNERTASK_H
#define WIFISCANNERTASK_H

#include <QString>
#include<QProcess>

class WifiScannerTask {
public:
    // 构造函数声明
    explicit WifiScannerTask(const QString& ip, quint16 startPort, quint16 endPort);
    explicit WifiScannerTask(const QString& ip);  // 默认扫描构造函数

    QPair<QString, quint16> run();  // 执行扫描
    bool isPortOpen(const QString& ip, quint16 port, int timeoutMs);
private:
    bool isAdbService(const QString& ip, quint16 port);  // 判断是否为 ADB
    QString ip;
    quint16 startPort;
    quint16 endPort;
    QProcess* process;  // 将 QProcess 放在成员中
};

#endif // WIFISCANNERTASK_H
