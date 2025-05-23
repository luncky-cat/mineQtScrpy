#include "context/DeviceContext.h"

// #include <limits>
// #include <qdebug>

// // int DeviceContext::allocShellId = 1;
// // int DeviceContext::allocSyncId = std::numeric_limits<int>::max();

#include "server/IServerFactory.hpp"

DeviceContext::DeviceContext(ConnectInfo info)
    :connectInfo(info),status(DeviceStatus::Disconnected){
    cmdServer=IServerFactory::create(info.ConnectType);//

}

void DeviceContext::setSocket(asio::ip::tcp::socket socket) {
    sessionCtx = std::make_unique<DeviceSession>(std::move(socket));
}
