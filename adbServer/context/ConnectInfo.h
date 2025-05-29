#ifndef CONNECTINFO_H
#define CONNECTINFO_H

#include <string>

#include"ConnectType.h"

class ConnectInfo{      //所需基本信息
public:
    ConnectInfo();
    ConnectInfo(const std::string& ip, uint16_t port);
    ConnectInfo(const std::string &serial);
    std::string getId();
    ConnectType getType();
public:
    std::string ip;    // 设备IP地址
    uint16_t port;    // 设备IP地址
    std::string serial; // 设备序列号（仅对USB设备有效）
    std::string id;
    ConnectType type;   //类型
};

#endif // CONNECTINFO_H
