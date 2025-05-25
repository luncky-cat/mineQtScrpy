#include "ConnectingState.h"

#include"context/DeviceContext.h"
#include"AuthenticatingState.h"

#include <qDebug>



void ConnectingState::handle(DeviceContext& context)
{
    qDebug()<<"进入连接态";
    bool successful=context.server->connect(context);
    if(successful){
        qDebug()<<"连接成功";
        setState(context,std::make_unique<AuthenticatingState>());
    }else{
        qDebug()<<"连接失败，保持断开";
    }
}

