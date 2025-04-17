#ifndef LISTENWIFI_H
#define LISTENWIFI_H

class WifiScanner;
//struct DeviceInfo;

#include "listenDevice.h"

class ListenWifi : public ListenDevice {
    Q_OBJECT
private:
    ListenWifi();
    ~ListenWifi();
    void init();
public slots:
    void handleDeviceFound(const QString& ip, int port);
public:
    void startListening() override;
    void stopListening() override;
public slots:
    void scanDevices() override;
private:
    WifiScanner *scanner;   //扫描器
    QTimer *timer;
    qint32 interval;
    static ListenWifi instance;  // 静态实例，确保在程序启动时创建
};

#endif // LISTENWIFI_H


/*
动态调整线程数：可以根据系统的负载情况动态调整线程池的最大线程数。例如，在低负载时增加并发数，而在高负载时减少并发数。

结果缓存：如果某些 IP 地址的扫描结果已经缓存，可以避免重复扫描，减少不必要的开销。

停止扫描：在用户停止监听时，需要实现对线程池的停止控制，避免继续占用资源。

*/
