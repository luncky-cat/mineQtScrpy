#include "pushHandler.h"

#include<fstream>
#include<chrono>
#include<thread>

#include <qDebug>

#include "interfaces/ITransPort.h"
#include "context//DeviceContext.h"
#include "protocol/AdbSyncProtocol.h"
#include "protocol/AdbProtocol.h"

pushHandler::Registrar pushHandler::registrar;

bool pushHandler::pushFile(ITransPort &transport,int local_id,int remote_id,std::string& localFilePath,std::string remoteFilePath,AdbMessage &out){
    std::string remotePathWithPerm = remoteFilePath + ",33206"; // 33206 是权限  rw-rw-rw-
    auto sendPayload = AdbSyncProtocol::generateSEND(remotePathWithPerm);
    auto sendMsg = AdbProtocol::generateWrite(local_id, remote_id, sendPayload);

    qDebug() << "发送 SEND 命令，远程路径: " << QString::fromStdString(remotePathWithPerm);
    transport.sendMsg(sendMsg);

    if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
        qDebug() << "未收到 SEND 命令的 OKAY 响应";
        return false;
    }

    // 2. 读取文件并分块发送 DATA
    std::ifstream file(localFilePath, std::ios::binary);
    if (!file.is_open()) {
        qDebug() << "打开本地文件失败：" << QString::fromStdString(localFilePath);
        return false;
    }

    const size_t blockSize=1024;
    std::vector<uint8_t> buffer(blockSize);

    while (file.read(reinterpret_cast<char*>(buffer.data()), blockSize) || file.gcount() > 0) {
        size_t readCount = file.gcount();
        qDebug() << "读取文件块大小：" << readCount;

        auto writeMsg = AdbSyncProtocol::generateDATA(local_id, remote_id,buffer,readCount);
       // auto writeMsg = AdbProtocol::generateWrite(local_id, remote_id, dataPayload);
        // 分开发送 WRTE 和数据（可选调试用）
        transport.sendMsg(writeMsg);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
            qDebug() << "未收到 DATA 块的 OKAY 响应";
            return false;
        }
    }


    file.close();

    // 3. 发送 DONE 命令，通知文件传输结束
    uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
    qDebug() << "发送 DONE 命令，时间戳：" << mtime;

    auto donePayload = AdbSyncProtocol::generateDONE(mtime);
    auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
    transport.sendMsg(doneMsg);

    if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
        qDebug() << "未收到 DONE 命令的 OKAY 响应";
        return false;
    }

    // 4. 关闭该传输流
    auto closeMsg = AdbProtocol::generateClose(local_id, remote_id);
    transport.sendMsg(closeMsg);

    qDebug() << "文件推送完成，已关闭流";


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
