#ifndef DEVICESERVER_H
#define DEVICESERVER_H

struct DeviceContext;

class DeviceServer       //基础服务类
{
public:
    ~DeviceServer()=default;
    virtual bool connect(DeviceContext &ctx) = 0;
    virtual bool auth(DeviceContext& ctx) = 0;
    virtual bool execute(DeviceContext& ctx) = 0;
    virtual bool close(DeviceContext& ctx) = 0;
};


// class DeviceServerFactory {
// public:
//     static DeviceServer* create(DeviceType type);
// };

// DeviceServer* DeviceServer::create(DeviceType type) {
//     switch (type) {
//     case DeviceType::Wifi:
//         return &WifiServer::instance();
//     // case DeviceType::Usb:
//     //     return &UsbServer::instance();
//     default:
//         return nullptr;
//     }
// }

#endif // DEVICESERVER_H
