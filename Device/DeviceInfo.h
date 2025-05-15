#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "ConnectInfo.h"
#include "StreamStatus.h"

// 设备信息结构体
struct DeviceInfo {
    ConnectInfo connectInfo;
    StreamStatus streamStatus;
    QString network;      // 设备所属网络
    QString name;               // 可选的设备名称，便于在UI中展示
    QString model;              // 设备型号（可选，可通过 adb 获取）
    QString connectionHint;     // 可用于调试/显示连接信息，如 USB, Wi-Fi@192.168.1.2
    bool isOnline = false;      // 当前设备是否在线
};

#endif // DEVICEINFO_H
