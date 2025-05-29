#include"ConnectInfo.h"

ConnectInfo::ConnectInfo(const std::string &ip, uint16_t port)
    :ip(ip),port(port),
    id(ip + ":" + std::to_string(port)),type(ConnectType::WiFi){}

ConnectInfo::ConnectInfo(const std::string&serial)
    :serial(serial),id(serial),type(ConnectType::USB){}

ConnectInfo::ConnectInfo()
    :port(0),id("Unknown"),type(ConnectType::Unknown) {}

std::string ConnectInfo::getId()
{
    return id;
}

ConnectType ConnectInfo::getType()
{
    return type;
}
