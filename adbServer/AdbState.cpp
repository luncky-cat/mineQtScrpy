#include "adbstate.h"
#include "adbserver.h"

//连接：获得数据-调用连接函数
void ConnectingState::handle(AdbServer &server,DeviceContext &context)
{
    ConnectInfo& info=context.connectInfo;
    context.socket=server.connect(info);   //获得socket
    if( context.socket!=-1){   //转化为认证状态
        server.setState(context.deviceId,std::make_unique<AuthenticatingState>());  //转化为认证状态
    }else{
        server.setState(context.deviceId,std::make_unique<DisconnectedState>());  //转化为认证状态
    }
}

//认证
void AuthenticatingState::handle(AdbServer& server,DeviceContext &context) {    //后续扩展成usb认证
    // authWiFi
    SOCKET sock=context.socket;
    bool result=server.authWiFi(sock);
    if(result){   //执行态
        server.setState(context.deviceId,std::make_unique<ConnectedState>());  //转化为执行态
    }else{    //断开态
        server.setState(context.deviceId,std::make_unique<DisconnectedState>());  //转化为认证状态
    }
}

//执行
void ConnectedState::handle(AdbServer& server,DeviceContext&context) {
    // 设备断开连接或需要关闭连接时的操作

    //执行

    //接收

}

void DisconnectedState::handle(AdbServer& server, DeviceContext &context) {



}


DeviceStatus ConnectingState::getStatus()
{
    return DeviceStatus::Connecting;
}

DeviceStatus AuthenticatingState::getStatus()
{

    return DeviceStatus::Authenticating;
}

DeviceStatus ConnectedState::getStatus()
{
    return DeviceStatus::Connected;
}

DeviceStatus DisconnectedState::getStatus()
{
    return DeviceStatus::Disconnected;
}

