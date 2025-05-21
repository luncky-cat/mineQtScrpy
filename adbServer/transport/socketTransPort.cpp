#include "SocketTransPort.h"

#include "protocol/AdbProtocol.h"

#include<chrono>
#include <qDebug>
#include<thread>
#include<WinSock2.h>
#include <qDebug>
#include <set>

SocketTransPort::SocketTransPort(){}

void SocketTransPort::setSocket(int sock)
{
    socket_=sock;
}

bool SocketTransPort::sendMsg(const std::vector<uint8_t>& data) {   //根据套接字来发送
    qDebug()<<socket_<<"发送数据";
    int sendLen = send(socket_, reinterpret_cast<const char*>(data.data()), data.size(), 0);
    return sendLen >0;
}

bool SocketTransPort::recvMsg(AdbMessage& outMsg) {
    qDebug()<<socket_<<"接收数据";
    char tempBuf[512];
    int recvLen = recv(socket_, tempBuf, sizeof(tempBuf), 0);
    if (recvLen <= 0) {
        qDebug()<<"接收出错";
        return false;
    }

    // 粘包缓冲
    recvBuffer_.insert(recvBuffer_.end(), tempBuf, tempBuf + recvLen);

    while (true) {
        size_t msgLen = 0;
        auto result = AdbProtocol::parseAdbMessage(recvBuffer_, msgLen);
        if (!result.has_value()) {
            qDebug()<<"数据不够，继续接收";
            break;
        }

        outMsg = result.value();  // 取出已完成包
        qDebug()<<"有一个数据包去出"<<outMsg.command;
        recvBuffer_.erase(recvBuffer_.begin(),recvBuffer_.begin() + msgLen);  // 移除已消费
        return true;
    }

    return false;  // 当前数据不足，等待更多数据
}

bool SocketTransPort::waitForRecv(AdbMessage& outMsg,int maxAttempts, int intervalMs) {
    for (int i = 0; i < maxAttempts; ++i) {
        if (recvMsg(outMsg)) {
            return true;
        }
        qDebug()<<"等待100ms";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
}

bool SocketTransPort::waitForCommands(const std::set<uint32_t>& expectedCmds, AdbMessage& inputMsg) {
    if (!waitForRecv(inputMsg)) {
        return false; // 接收失败
    }
    return expectedCmds.count(inputMsg.command) > 0;
}

