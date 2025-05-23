#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include <map>

#include "interfaces/IState.h"
#include "interfaces//IServer.h"

#include "CommandInfo.h"
#include "ConnectInfo.h"
#include "DeviceStatus.h"
#include "context/DeviceSession.h"

class DeviceContext {
public:
    DeviceContext(ConnectInfo info);
    ConnectInfo connectInfo;  //连接信息
    CommandInfo cmd; //命令类型
    DeviceStatus status;   //状态位
    std::map<std::string, std::string> authInfos; // 认证后的设备信息
    std::unique_ptr<DeviceSession> sessionCtx;   // 会话上下文，含 socket 与数据流
    std::shared_ptr<IServer> cmdServer;          // 设备控制策略，用于执行命令
    std::unique_ptr<IState> deviceState;               // 当前设备状态（认证中/已连接/断开等）
    void setSocket(asio::ip::tcp::socket socket);
};


#endif // DEVICECONTEXT_H
