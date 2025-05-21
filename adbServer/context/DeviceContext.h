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
    DeviceContext(IServer *strategyPtr = nullptr, ITransPort *transPortPtr = nullptr);
    void setConnectInfo(ConnectInfo info);

    //流信息相关
    static int allocShellId;
    static int allocSyncId;
    bool isOpenShell;
    bool isOpenSync;
    int shellLocalId;
    int syncLocalId;
    int shellRemoteId;
    int synRemoteId;

    AdbMessage msg;

    ConnectInfo connectInfo;  //连接信息
    CommandInfo cmd; //命令类型
    DeviceStatus status;   //状态位
    std::map<std::string,std::string> deviceInfos;//认证成功返回的信息

    std::unique_ptr<IServer>strategy;   //执行策略
    std::unique_ptr<ITransPort> transPort;   //通信器
    std::unique_ptr<IState> currentState;//当前状态
};




#endif // DEVICECONTEXT_H
