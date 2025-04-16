#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include <QHash>

// 设备信息结构体
struct DeviceInfo {
    QString id;           // 唯一标识符（序列号或 IP 地址）
    QString serialNumber; // 设备序列号
    QString ipAddress;    // 设备IP地址
    QString network;      // 设备所属网络
    QString deviceType;   // 设备类型（如 USB 或 Wi-Fi）
    quint16 port;         // 端口号（如 5555）

    bool operator==(const DeviceInfo& other) const {
        return id == other.id;  // 只比较 id 字段
    }
};

// 自定义哈希函数，基于设备的 id 字段
inline uint qHash(const DeviceInfo& device, uint seed = 0) {
    return qHash(device.id, seed);  // 使用 id 字段的哈希值
}

#endif // DEVICEINFO_H
