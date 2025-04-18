#ifndef LISTENDEVICE_H
#define LISTENDEVICE_H

struct DeviceInfo;

#include<QObject>
#include <QSet>

class ListenDevice : public QObject {
    Q_OBJECT
public:
    virtual ~ListenDevice() = default;
    virtual void startListening() = 0;  //监听
    virtual void stopListening() = 0;   //停止
    void updateChangeSet(const QSet<DeviceInfo>& newDevices);
    void registerListener(ListenDevice* listener);   //注册

    static QList<ListenDevice*>getListeners();  //获得     
    static void listen();

public slots:
    virtual void scanDevices()=0;    //实际操作
signals:
    void devicesAdd(const QSet<DeviceInfo>& newDevices);  // 新增信号
    void devicesRemove(const QSet<DeviceInfo>& removedDevices);  //移除信号 
private:
    static QList<ListenDevice*> listeners;  // 所有监听器 
    QSet<DeviceInfo>lastDevices; // 上一次扫描设备列表
};

#endif // LISTENDEVICE_H
