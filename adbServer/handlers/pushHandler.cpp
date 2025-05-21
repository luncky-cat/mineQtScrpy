#include "pushHandler.h"

#include<fstream>

#include <qDebug>

#include "interfaces/ITransPort.h"
#include "context//DeviceContext.h"
#include "protocol/AdbSyncProtocol.h"
#include "protocol/AdbProtocol.h"

pushHandler::Registrar pushHandler::registrar;

bool pushHandler::pushFile(ITransPort &transport,int local_id,int remote_id,std::string& localFilePath,std::string remoteFilePath,AdbMessage &out){
    //SEND命令
    std::string remotePath = remoteFilePath+",33206"; // 第二个参数是权限
    std::vector<uint8_t> sendPayload =AdbSyncProtocol::generateSEND(remotePath);
    auto sendMsg = AdbProtocol::generateWrite(local_id,remote_id,sendPayload);
    transport.sendMsg(sendMsg);
    if (!transport.waitForCommands({AdbProtocol::CMD_OKAY},out)) {    //接收ok
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
        transport.sendMsg(writeMsg);
        transport.waitForCommands({AdbProtocol::CMD_OKAY},out);
    }

    uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
    auto donePayload = AdbSyncProtocol::generateDONE(mtime);
    auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
    transport.sendMsg(doneMsg);
    transport.waitForCommands({AdbProtocol::CMD_OKAY},out);
    // auto closeMsg = AdbProtocol::generateClose(local_id,remote_id);
    // transport.sendMsg(closeMsg);
    return true;
}

bool pushHandler::openSyn(ITransPort &transport,const int local_id,int& remote_id,AdbMessage &out){
    auto openSync = AdbProtocol::generateOpen(local_id,"sync:");
    transport.sendMsg(openSync);
    if (!transport.waitForCommands({AdbProtocol::CMD_OKAY},out)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }
    remote_id = out.arg0;
    return true;
}

bool pushHandler::CommandHandler(ITransPort &transport, DeviceContext &ctx)
{
    qDebug()<<"执行pushHandler";
    qDebug()<<"分配的本地id"<<ctx.syncLocalId;
    if (!ctx.isOpenSync) {
        ctx.isOpenSync=openSyn(transport,ctx.syncLocalId,ctx.synRemoteId,ctx.msg);
        if(!ctx.isOpenSync){
            qDebug()<<"打开syn流失败";
            return false;
        }
    }
    bool result=pushFile(transport,ctx.syncLocalId,ctx.synRemoteId,ctx.cmd.params[0],ctx.cmd.params[1],ctx.msg);
    if(result){
        qDebug()<<"推送文件成功";
    }
    return true;
}
