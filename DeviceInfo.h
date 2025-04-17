#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include <QHash>

// 设备信息结构体
struct DeviceInfo {
    QString serialNumber; // 设备序列号
    bool isOnline = false;      // 当前设备是否在线
    bool isStreamable = false;  // 当前是否允许推流
    bool isStreaming = false;   // 当前是否正在推流
    QString name;               // 可选的设备名称，便于在UI中展示
    QString model;              // 设备型号（可选，可通过 adb 获取）
    QString connectionHint;     // 可用于调试/显示连接信息，如 USB, Wi-Fi@192.168.1.2
    QString ipAddress;    // 设备IP地址
    QString network;      // 设备所属网络
    QString deviceType;   // 设备类型（如 USB 或 Wi-Fi）
    quint16 port;         // 端口号（如 5555）
    bool operator==(const DeviceInfo& other) const {
        return serialNumber == other.serialNumber;  // 只比较 id 字段
    }
};

// 自定义哈希函数，基于设备的 id 字段
inline uint qHash(const DeviceInfo& device, uint seed = 0) {
    return qHash(device.serialNumber, seed);  // 使用 id 字段的哈希值
}

#endif // DEVICEINFO_H
