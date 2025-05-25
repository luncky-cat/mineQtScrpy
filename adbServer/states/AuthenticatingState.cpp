#include "AuthenticatingState.h"

#include"context/DeviceContext.h"
#include "ExecutingState.h"
#include"ConnectingState.h"

#include <qDebug>

void AuthenticatingState::handle(DeviceContext& context) {
    qDebug()<<"进入认证态";
    bool successful=context.server->auth(context);
    if(successful){
        qDebug()<<"认证成功";
        setState(context,std::make_unique<ExecutingState>());
    }else{
        qDebug()<<"认证失败,返回连接态重试";
        setState(context,std::make_unique<ConnectingState>());
    }
}
