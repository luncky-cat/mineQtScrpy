#ifndef CONNECTINFO_H
#define CONNECTINFO_H

#include<QString>
#include<QHash>

enum class ConnectType {
    USB,      // USB 设备
    WiFi,      // Wi-Fi 设备
};

struct ConnectInfo{      //所需基本信息
    QString deviceId;
    ConnectType ConnectType;   //类型
    QString ipAddress;    // 设备IP地址
    quint16 port;    // 设备IP地址
    QString serialNumber; // 设备序列号（仅对USB设备有效）

    bool operator==(const ConnectInfo& other) const {
        if(deviceId==other.deviceId){
            return true;
        }
        return false;
    }
};

inline uint qHash(const ConnectInfo& key, uint seed = 0) {
    uint h = seed;
    h = qHash(key.deviceId, h);
    return h;
}

#endif // CONNECTINFO_H
