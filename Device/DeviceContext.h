#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include "AdbState.h"
#include "ConnectInfo.h"
#include "DeviceServer.h"
#include <memory>
#include<QTcpSocket>


enum class DeviceStatus {
    Disconnected,
    Connecting,
    Authenticating,
    Connected
};

// struct DeviceContext {
// public:
//     ConnectInfo connectInfo;   //连接信息
//     DeviceStatus status; //状态
//     DeviceServer *strategy; // 抽象策略接口
//     std::unique_ptr<IAdbState> currentState;  //状态指针
//     //连接态的信息及状态位
//     bool networkInitialized;
//     int socket = -1;  // 连接的句柄


//     //执行态
//     bool isOpenShell;
//     QTcpSocket* test;

//     void init(){

//     }
// };


class DeviceContext {
public:
    DeviceContext(DeviceServer* strategyPtr = nullptr)
        : strategy(strategyPtr),
        status(DeviceStatus::Disconnected),
        networkInitialized(false),
        socket(-1),
        isOpenShell(false),
        test(nullptr),local_id(-1),remote_id(-1){}
    void setConnectInfo(ConnectInfo info){
        connectInfo=info;
    }
public:
    ConnectInfo connectInfo;
    DeviceStatus status;
    bool networkInitialized;
    int socket;
    bool isOpenShell;
    DeviceServer* strategy;
    QTcpSocket* test;
    std::unique_ptr<IAdbState> currentState;
    int local_id;
    int remote_id;

};


#endif // DEVICECONTEXT_H
