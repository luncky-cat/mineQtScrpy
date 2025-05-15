#ifndef ADBSERVER_H
#define ADBSERVER_H

#include "Device/DeviceContext.h"

#include <winsock2.h>
#include <string>
#include<vector>
#include <unordered_map>

class AdbServer {

public:

    AdbServer() = default;

    ~AdbServer() = default;

    bool connect(const std::string& deviceId);   //   连接某个设备

    //更新某个数据源
   // void updateDeviceInfo(const std::string& deviceId,const ConnectInfo& info);

    void setDeviceInfos(const std::vector<ConnectInfo>& infos);  //设置信息源

   // void executeCommand(const std::string& deviceId);

private:

    int connect(const ConnectInfo& info);   //返回认证后得设备信息

    bool initNetwork();

    SOCKET connectWiFi(const ConnectInfo& info);   //连接

    SOCKET connectDevice(const char *ip, int port);

    bool authWiFi(const SOCKET& sock);  //认证

  //  bool connectUSB(const ConnectInfo& info);

private:

   // ConnectInfo findInfoById(const std::string& deviceId);

    bool setState(const std::string& deviceId, std::unique_ptr<IAdbState> newState);   //后续可以用枚举值

private:

    friend class ConnectingState;  // 允许其访问私有成员
    friend class AuthenticatingState;
    friend class ConnectedState;
    friend class DisconnectedState;

    std::unordered_map<std::string, DeviceContext> deviceContextMap;
};


#endif // ADBSERVER_H
