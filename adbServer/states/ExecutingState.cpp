#include "ExecutingState.h"

#include"context/DeviceContext.h"
#include"DisconnectedState.h"

#include <qDebug>

void ExecutingState::handle(DeviceContext& context) {
    qDebug()<<"进入执行态";
    bool successful=context.server->execute(context);
    if(!successful){  //执行失败
        qDebug()<<"执行失败,进入断开态";
        setState(context,std::make_unique<DisconnectedState>());
    }
}

