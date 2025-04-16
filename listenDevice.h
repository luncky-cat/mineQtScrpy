#ifndef LISTENDEVICE_H
#define LISTENDEVICE_H

#include "DeviceInfo.h"

#include<QWidget>

class ListenDevice : public QObject {
    Q_OBJECT
public:
    virtual ~ListenDevice() = default;
    virtual void startListening() = 0;  //监听
    virtual void stopListening() = 0;   //停止

    static  void registerListener(ListenDevice* listener);   //注册
    static QList<ListenDevice*>getListeners();  //获得     
    void updateChangeSet(const QSet<DeviceInfo>& newDevices);   //保留 新增和移除
public slots:
    virtual void scanDevices()=0;    //实际操作
signals:
    void devicesChanged(QPair<QSet<DeviceInfo>,QSet<DeviceInfo>>&);
private:
    static QList<ListenDevice*> listeners;  // 存储所有的监听器 
    QSet<DeviceInfo>lastDevices; // 上一次扫描的设备列表
};

#endif // LISTENDEVICE_H
