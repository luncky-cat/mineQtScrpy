#include "context/DeviceContext.h"

#include <limits>
#include <qdebug>

int DeviceContext::allocShellId = 1;
int DeviceContext::allocSyncId = std::numeric_limits<int>::max();

DeviceContext::DeviceContext(IServer* strategyPtr,ITransPort *transPortPtr)
    :status(DeviceStatus::Disconnected),isOpenShell(false),isOpenSync(false),
    strategy(strategyPtr),transPort(transPortPtr){
    shellLocalId=allocShellId++;
    syncLocalId=allocSyncId--;
    qDebug()<<shellLocalId<<"synid"<<syncLocalId;
}
void DeviceContext::setConnectInfo(ConnectInfo info){
    connectInfo=info;
}
