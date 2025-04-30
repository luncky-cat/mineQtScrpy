#ifndef CONNECTINFO_H
#define CONNECTINFO_H

#include<QString>
#include<QHash>

enum class DeviceType {
    USB,      // USB 设备
    WiFi      // Wi-Fi 设备
};

struct ConnectInfo{      //所需基本信息
    DeviceType deviceType;   //类型
    QString ipAddress;    // 设备IP地址
    quint16 port;    // 设备IP地址
    QString serialNumber; // 设备序列号（仅对USB设备有效）
    QString hashseed;

    bool operator==(const ConnectInfo& other) const {

        if (deviceType == DeviceType::WiFi) {
            return ipAddress == other.ipAddress && port == other.port;
        }

        if (deviceType == DeviceType::USB) {
            return serialNumber == other.serialNumber;
        }

        return false;  // 如果设备类型不同，则不相等
    }

    bool operator<(const ConnectInfo& other) const {

        if (deviceType == DeviceType::WiFi) {
            if (ipAddress == other.ipAddress) {
                return port < other.port;
            }
            return ipAddress < other.ipAddress;
        }

        // USB 设备按序列号排序
        if (deviceType == DeviceType::USB) {
            return serialNumber < other.serialNumber;
        }

        return false;
    }
};

inline uint qHash(const ConnectInfo& key, uint seed = 0) {
    uint h = seed;
    if (key.deviceType == DeviceType::WiFi) {
        // WiFi设备使用 IP 地址和端口来计算哈希值
        h = qHash(key.ipAddress, h);  // 对 QString 类型的 ipAddress 调用 qHash（已内建支持）
        h = qHash(key.port, h);       // 对 quint16 类型的端口调用 qHash（已内建支持）
    } else if (key.deviceType == DeviceType::USB) {
        // USB设备使用序列号来计算哈希值
        h = qHash(key.serialNumber, h);  // 对 QString 类型的 serialNumber 调用 qHash（已内建支持）
    }

    return h;
}

#endif // CONNECTINFO_H
