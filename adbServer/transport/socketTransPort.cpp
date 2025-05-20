#include "SocketTransPort.h"

#include "protocol/AdbProtocol.h"

#include<WinSock2.h>
#include <qDebug>

SocketTransPort::SocketTransPort(){}

void SocketTransPort::setSocket(int sock)
{
    socket_=sock;
}

bool SocketTransPort::sendMsg(const std::vector<uint8_t>& data) {   //根据套接字来发送
    int sendLen = send(socket_, reinterpret_cast<const char*>(data.data()), data.size(), 0);
    return sendLen >0;
}

bool SocketTransPort::recvMsg(AdbMessage& outMsg) {
    char tempBuf[512];
    std::vector<uint8_t>recvBuffer(512);
    int recvLen = recv(socket_, tempBuf, sizeof(tempBuf), 0);
    if (recvLen <= 0) {
        qDebug()<<"接收出错";
        return false;
    }

    // 粘包缓冲
    recvBuffer.insert(recvBuffer.end(), tempBuf, tempBuf + recvLen);

    while (true) {
        size_t msgLen = 0;
        auto result = AdbProtocol::parseAdbMessage(recvBuffer, msgLen);
        if (!result.has_value()) {
            qDebug()<<"数据不够，继续接收";
            break;
        }

        outMsg = result.value();  // 取出已完成包
        recvBuffer.erase(recvBuffer.begin(),recvBuffer.begin() + msgLen);  // 移除已消费
        return true;
    }

    return false;  // 当前数据不足，等待更多数据
}
