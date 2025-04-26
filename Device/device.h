#ifndef DEVICE_H
#define DEVICE_H

#include "DeviceInfo.h"

#include<QObject>
#include <QString>
#include <QVariant>

#include <QDebug>

class Device : public QObject {
    Q_OBJECT

public:
    virtual ~Device() {}

    // 获取设备的基本信息
    virtual QString getDeviceName() const = 0;
    virtual QString getDeviceModel() const = 0;
    virtual QString getConnectionStatus() const = 0;

    // 设备连接/断开操作
    virtual bool connectDevice() = 0;  // 连接设备
    virtual void disconnectDevice() = 0;  // 断开设备连接

    // 设备推流操作
    virtual bool startStreaming() = 0;  // 启动推流
    virtual bool stopStreaming() = 0;   // 停止推流

    // 执行命令到设备
    virtual QVariant executeCommand(const QString &command) = 0;  // 执行命令，获取返回值

    // 判断设备是否在线
    virtual bool isDeviceOnline() const = 0;

    // 设备重启/复位操作（可选）
    virtual bool rebootDevice() = 0;

signals:
    void deviceConnected();     // 设备连接信号
    void deviceDisconnected();  // 设备断开信号
    void streamingStarted();    // 推流开始信号
    void streamingStopped();    // 推流停止信号
    void deviceRebooted();      // 设备重启信号
private:
    DeviceInfo info;
};


#endif // DEVICE_H
