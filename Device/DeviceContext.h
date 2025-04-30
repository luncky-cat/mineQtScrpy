#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include "ConnectInfo.h"
#include "adbServer/adbstate.h"
#include <memory>

struct ConnectInfo;

#include<string>

struct DeviceContext {
    std::string deviceId;
    ConnectInfo connectInfo;   //连接信息
    int socket = -1;  // 连接的句柄
    std::unique_ptr<IAdbState> currentState;  //状态指针
    // std::optional<CommandInfo> pendingCommand;
    //DeviceContext() = default;  // ✅ 加上这个就行
};

#endif // DEVICECONTEXT_H
