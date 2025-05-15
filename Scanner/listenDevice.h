#ifndef LISTENDEVICE_H
#define LISTENDEVICE_H

#include "Device/ConnectInfo.h"

#include<QObject>
#include <QSet>

class ListenDevice : public QObject {
    Q_OBJECT
public:
    virtual ~ListenDevice() = default;

    virtual void startListening() = 0;  //监听

    virtual void stopListening() = 0;   //停止

    void updateChangeSet(const QSet<ConnectInfo>& newDevices);   //获得差异

    void registerListener(ListenDevice* listener);   //注册

    static QList<ListenDevice*>getListeners();  //获得

    static void listen();//统一开启监听

    static void close();

signals:

    void devicesAdd(const QSet<ConnectInfo>& newDevices);  // 新增信号

    void devicesRemove(const QSet<ConnectInfo>& removedDevices);  //移除信号

private:

    static QList<ListenDevice*> listeners;  // 所有监听器

    QSet<ConnectInfo>lastDevices; // 上一次扫描设备列表
};

#endif // LISTENDEVICE_H




/*
class ListenDevice : public QObject {
    Q_OBJECT
public:
    // 基本操作
    virtual bool startListening() = 0;    // 开始监听
    virtual void stopListening() = 0;     // 停止监听
    virtual bool isListening() const = 0; // 监听状态

    // 设备管理
    virtual QList<ConnectInfo> getDevices() const = 0;  // 获取设备列表
    virtual bool hasDevice(const QString& serial) const = 0;  // 检查设备存在

    // 状态通知
    virtual void updateChangeSet(const QSet<ConnectInfo>& newDevices) = 0;  // 更新设备集合

signals:
    void devicesAdd(const QList<ConnectInfo>& devices);    // 设备添加信号
    void devicesRemove(const QList<ConnectInfo>& devices); // 设备移除信号
    void listeningStateChanged(bool isListening);          // 监听状态变化信号
    void errorOccurred(const QString& error);              // 错误通知信号

protected:
    // 资源管理
    virtual void initialize() = 0;     // 初始化资源
    virtual void cleanup() = 0;        // 清理资源
    virtual void handleError(const QString& error) = 0;  // 错误处理

    // 设备操作
    virtual void addDevice(const ConnectInfo& device);    // 添加设备
    virtual void removeDevice(const QString& serial);     // 移除设备
    virtual void updateDevice(const ConnectInfo& device); // 更新设备

private:
    QSet<ConnectInfo> m_currentDevices;  // 当前设备集合
    bool m_isListening;                  // 监听状态
};
 */
