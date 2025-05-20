#include "WifiServer.h"
#include"Device/ConnectInfo.h"

class IServerFactory {
public:
    static IServer* create(ConnectType type){
        switch (type) {
        case ConnectType::WiFi:
            return &WifiServer::instance();
        default:
            return nullptr;
        }
    }
};

// DeviceServer.cpp

