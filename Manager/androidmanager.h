#ifndef ANDROIDMANAGER_H
#define ANDROIDMANAGER_H

#include "devicemanager.h"
#include "../Device/device.h"
#include "../Device/DeviceInfo.h"

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QSet>



class AddDeviceWorker;    // 前向声明新增设备处理线程类
class RemoveDeviceWorker; // 前向声明移除设备处理线程类

class androidManager : public DeviceManager
{
    Q_OBJECT
public:
    androidManager();
    ~androidManager();

private:
    void signalConnect();

public slots:
    void onDevicesAdded(const QSet<ConnectInfo>& DeviceChangeSet);
    void onDevicesRemoved(const QSet<ConnectInfo>& DeviceChangeSet);
public:
    void getDevices();
    void closeDevices();
    void allDescription();
    void closeDevice();
    void description();
    void selectDevice();
    void listen();  // 监听设备变化
private:
    QList<Device*> devices;  // 当前设备列表
    QMap<QString,DeviceInfo>deviceInfos;   //设备信息详情列表
    QProcess* process;
};

#endif // ANDROIDMANAGER_H
