#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include <map>

#include "interfaces/IState.h"
#include "interfaces//IServer.h"
#include "interfaces/ITransPort.h"

#include "DeviceStatus.h"
#include "CommandInfo.h"
#include "ConnectInfo.h"
#include "protocol/AdbMessage.h"


class DeviceContext {
public:
    DeviceContext(IServer* strategyPtr = nullptr,ITransPort *transPortPtr=nullptr)
        :local_id(-1),
        remote_id(-1),
        status(DeviceStatus::Disconnected),isOpenShell(false),
        isOpenSync(false),strategy(strategyPtr),transPort(transPortPtr){}
    void setConnectInfo(ConnectInfo info){
        connectInfo=info;
    }
    AdbMessage msg;
    int local_id;
    int remote_id;
    ConnectInfo connectInfo;  //连接信息
    CommandInfo cmd; //命令类型
    DeviceStatus status;   //状态位
    std::map<std::string,std::string> deviceInfos;//认证成功返回的信息
    bool isOpenShell;
    bool isOpenSync;
    std::unique_ptr<IServer>strategy;
    std::unique_ptr<ITransPort> transPort;   //通信器
    std::unique_ptr<IState> currentState;
};


#endif // DEVICECONTEXT_H
