#include "WifiServer.h"
#include"Device/ConnectInfo.h"

class DeviceServerFactory {
public:
    static DeviceServer* create(ConnectType type){
        switch (type) {
        case ConnectType::WiFi:
            return &WifiServer::instance();
        // case DeviceType::Usb:
        //     return &UsbServer::instance();
        default:
            return nullptr;
        }
    }
};

// DeviceServer.cpp

