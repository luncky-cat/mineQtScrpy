#include "AdbState.h"
#include "Device/DeviceContext.h"

#include<QDebug>

bool ConnectingState::handle(DeviceContext& context)
{
    qDebug()<<"进入连接态";
    bool successful=context.strategy->connect(context);
    if(successful){
        qDebug()<<"连接成功";
        setState(context,std::make_unique<AuthenticatingState>());
    }else{
        qDebug()<<"连接失败，保持断开";
    }
    return successful;
}

//认证
bool AuthenticatingState::handle(DeviceContext& context) {
    qDebug()<<"进入认证态";
    bool successful=context.strategy->auth(context);
    if(successful){
        qDebug()<<"认证成功";
        setState(context,std::make_unique<ExecutingState>());
    }else{
        qDebug()<<"认证失败,返回连接态重试";
        setState(context,std::make_unique<ConnectingState>());
    }
    return successful;
}

//执行
bool ExecutingState::handle(DeviceContext& context) {
    qDebug()<<"进入执行态";
    bool successful=context.strategy->execute(context);
    if(!successful){  //执行失败
        qDebug()<<"执行失败";
        setState(context,std::make_unique<DisconnectedState>());
    }
    return successful;
}


bool DisconnectedState::handle(DeviceContext& context) {
    bool successful=context.strategy->close(context);
    return successful;
}

void IAdbState::setState(DeviceContext &context, std::unique_ptr<IAdbState> newState)
{
    context.currentState = std::move(newState);
    context.currentState->handle(context);    //自动调用默认行为
}
