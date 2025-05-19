#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include "AdbMessage.h"
#include "AdbState.h"
#include "CommandInfo.h"
#include "ConnectInfo.h"
#include "DeviceServer.h"

#include <map>

enum class DeviceStatus {
    Disconnected,
    Connecting,
    Authenticating,
    ExecutingState
};

class DeviceContext {
public:
    DeviceContext(DeviceServer* strategyPtr = nullptr)
        :strategy(strategyPtr),
        status(DeviceStatus::Disconnected),
        socket(-1),
        isOpenShell(false),isOpenSync(false),
        local_id(-1),remote_id(-1){}
    void setConnectInfo(ConnectInfo info){
        connectInfo=info;
    }
public:
    AdbMessage msg;
    int local_id;
    int remote_id;
    ConnectInfo connectInfo;  //连接信息
    CommandInfo cmd; //命令类型
    DeviceStatus status;   //状态位
    int socket;
    std::map<std::string,std::string> deviceInfos;//认证成功返回的信息
    bool isOpenShell;
    bool isOpenSync;
    DeviceServer* strategy;
    std::unique_ptr<IAdbState> currentState;
};


#endif // DEVICECONTEXT_H
