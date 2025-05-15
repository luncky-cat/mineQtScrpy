#ifndef WIFISERVER_H
#define WIFISERVER_H

#include "AdbMessage.h"
#include "DeviceServer.h"

#include <vector>
#include <WinSock2.h>

class WifiServer : public DeviceServer   //通用的wifi服务接口
{
public:
    static WifiServer& instance();
    bool connect(DeviceContext& ctx);
    bool auth(DeviceContext& ctx);
    bool execute(DeviceContext& ctx);
    bool close(DeviceContext& ctx);

    bool sendMsg(std::vector<uint8_t> &sendMsg, DeviceContext& ctx);

    bool recvMsg(std::vector<uint8_t> &recvMsg, DeviceContext &ctx,AdbMessage& msg);
    bool openShellChannel(DeviceContext &ctx);
private:
    bool initNetwork();
private:
    bool networkInitialized;   //初始化标志
    AdbMessage msg;
    std::vector<uint8_t> recvData;  // 初步开个缓冲区
    WifiServer();
    ~WifiServer();
    SOCKET s;
    int local_id;
    int remote_id;


};

#endif // WIFISERVER_H
