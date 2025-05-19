#include "pushHandler.h"

#include "AdbProtocol.h"

#include "WifiServer.h"

#include <qDebug>


bool pushHandler::operator()(DeviceContext &ctx)
{
    if(openSyn()){
        return false;
    }
    pushFile();
}


bool pushHandler::pushFile(SOCKET sock,int local_id,int remote_id,std::string& localFilePath,std::string remoteFilePath){
    //SEND命令
    std::string remotePath = remoteFilePath+",33206"; // 第二个参数是权限
    std::vector<uint8_t> sendPayload =AdbSyncProtocol::generateSEND(remotePath);
    auto sMsg = AdbProtocol::generateWrite(local_id,remote_id,sendPayload);
    sendMsg(sock,sMsg);
    if (!waitForCommand(sock, AdbProtocol::CMD_OKAY)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }

    //打开文件，读取发送
    std::ifstream file(localFilePath,std::ios::binary);
    const size_t blockSize = 64 * 1024;
    std::vector<uint8_t> buffer(blockSize);  // 这一行是定义 buffer 的地方
    while (file.read((char*)buffer.data(), blockSize) || file.gcount() > 0) {
        auto dataPayload = AdbSyncProtocol::generateDATA(buffer, file.gcount());
        auto writeMsg = AdbProtocol::generateWrite(local_id,remote_id, dataPayload);
        sendMsg(sock,writeMsg);
        waitForCommand(sock, AdbProtocol::CMD_OKAY);
    }

    uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
    auto donePayload = AdbSyncProtocol::generateDONE(mtime);
    auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
    sendMsg(sock,doneMsg);
    waitForCommand(sock, AdbProtocol::CMD_OKAY);
    auto closeMsg = AdbProtocol::generateClose(local_id,remote_id);
    sendMsg(sock,closeMsg);
}

bool pushHandler::openSyn(SOCKET s,const int local_id,int& remote_id){
    auto openSync = AdbProtocol::generateOpen(local_id,"sync:");
    sendMsg(s,openSync);
    if (!waitForCommand(s,AdbProtocol::CMD_OKAY)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }
    remote_id = msg.arg0;
}


