#include "pushHandler.h"

#include<fstream>
#include<chrono>
#include<thread>

#include <qDebug>

#include "interfaces/ITransPort.h"
#include "context//DeviceContext.h"
#include "protocol/AdbSyncProtocol.h"
#include "protocol/AdbProtocol.h"
#include "context/StreamIdAllocator.h"
#include"context/AdbStreamContext.h"

pushHandler::Registrar pushHandler::registrar;

// bool pushHandler::pushFile(std::shared_ptr<DeviceSession> sessionCtx,int local_id,int remote_id,std::string& localFilePath,std::string remoteFilePath,AdbMessage &out){
//     std::string remotePathWithPerm = remoteFilePath + ",33206"; // 33206 是权限  rw-rw-rw-
//     auto sendPayload = AdbSyncProtocol::generateSEND(remotePathWithPerm);
//     auto sendMsg = AdbProtocol::generateWrite(local_id, remote_id, sendPayload);

//     qDebug() << "发送 SEND 命令，远程路径: " << QString::fromStdString(remotePathWithPerm);
//     sessionCtx->send(sendMsg);

//     if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
//         qDebug() << "未收到 SEND 命令的 OKAY 响应";
//         return false;
//     }

//     // 2. 读取文件并分块发送 DATA
//     std::ifstream file(localFilePath, std::ios::binary);
//     if (!file.is_open()) {
//         qDebug() << "打开本地文件失败：" << QString::fromStdString(localFilePath);
//         return false;
//     }

//     const size_t blockSize=1024;
//     std::vector<uint8_t> buffer(blockSize);

//     while (file.read(reinterpret_cast<char*>(buffer.data()), blockSize) || file.gcount() > 0) {
//         size_t readCount = file.gcount();
//         qDebug() << "读取文件块大小：" << readCount;

//         auto writeMsg = AdbSyncProtocol::generateDATA(local_id, remote_id,buffer,readCount);
//         // auto writeMsg = AdbProtocol::generateWrite(local_id, remote_id, dataPayload);
//         // 分开发送 WRTE 和数据（可选调试用）
//         transport.sendMsg(writeMsg);
//         std::this_thread::sleep_for(std::chrono::milliseconds(50));
//         if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
//             qDebug() << "未收到 DATA 块的 OKAY 响应";
//             return false;
//         }
//     }


//     file.close();

//     // 3. 发送 DONE 命令，通知文件传输结束
//     uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
//     qDebug() << "发送 DONE 命令，时间戳：" << mtime;

//     auto donePayload = AdbSyncProtocol::generateDONE(mtime);
//     auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
//     transport.sendMsg(doneMsg);

//     if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
//         qDebug() << "未收到 DONE 命令的 OKAY 响应";
//         return false;
//     }

//     // 4. 关闭该传输流
//     auto closeMsg = AdbProtocol::generateClose(local_id, remote_id);
//     transport.sendMsg(closeMsg);

//     qDebug() << "文件推送完成，已关闭流";


//     return true;
// }

// bool pushHandler::pushFile(std::shared_ptr<DeviceSession> sessionCtx, int local_id, int remote_id,
//                            std::string& localFilePath, std::string remoteFilePath) {
//     std::string remotePathWithPerm = remoteFilePath + ",33206"; // rw-rw-rw-
//     auto sendPayload = AdbSyncProtocol::generateSEND(remotePathWithPerm);
//     auto sendMsg = AdbProtocol::generateWrite(local_id, remote_id, sendPayload);

//     sessionCtx->send(sendMsg);
//     if (!sessionCtx->waitCommand({AdbProtocol::CMD_OKAY}, local_id, 30, [](const AdbMessage& msg) {
//             //out = msg;
//             return true;
//         })) {
//         qDebug() << "未收到 SEND 的 OKAY";
//         return false;
//     }

//     // 发送 DATA 块
//     std::ifstream file(localFilePath, std::ios::binary);
//     if (!file.is_open()) {
//         qDebug() << "文件打开失败: " << QString::fromStdString(localFilePath);
//         return false;
//     }

//     const size_t blockSize = 1024;
//     std::vector<uint8_t> buffer(blockSize);
//     while (file.read(reinterpret_cast<char*>(buffer.data()), blockSize) || file.gcount() > 0) {
//         size_t readCount = file.gcount();
//         auto dataMsg = AdbSyncProtocol::generateDATA(local_id, remote_id, buffer, readCount);
//         sessionCtx->send(dataMsg);
//         std::this_thread::sleep_for(std::chrono::milliseconds(50));

//         if (!sessionCtx->waitCommand({AdbProtocol::CMD_OKAY}, local_id, 30, [](const AdbMessage& msg) {
//                 //out = msg;
//                 return true;
//             })) {
//             qDebug() << "未收到 DATA 块的 OKAY 响应";
//             return false;
//         }
//     }

//     file.close();

//     // DONE
//     auto donePayload = AdbSyncProtocol::generateDONE(static_cast<uint32_t>(std::time(nullptr)));
//     auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
//     sessionCtx->send(doneMsg);

//     if (!sessionCtx->waitCommand({AdbProtocol::CMD_OKAY}, local_id, 30, [](const AdbMessage& msg) {
//             //out = msg;
//             return true;
//         })) {
//         qDebug() << "未收到 DONE 的 OKAY 响应";
//         return false;
//     }

//     // CLOSE
//     auto closeMsg = AdbProtocol::generateClose(local_id, remote_id);
//     sessionCtx->send(closeMsg);
//     return true;
// }

asio::awaitable<bool> pushHandler::pushFile(std::shared_ptr<DeviceSession> sessionCtx, int local_id, int remote_id,
                                            std::string localFilePath, std::string remoteFilePath) {
    std::string remotePathWithPerm = remoteFilePath + ",33206";  // rw-rw-rw-
    qDebug() << "[pushFile] 开始推送文件: " << QString::fromStdString(localFilePath);

    auto sendPayload = AdbSyncProtocol::generateSEND(remotePathWithPerm);
    auto sendMsg = AdbProtocol::generateWrite(local_id, remote_id, sendPayload);
    sessionCtx->send(sendMsg);

    {
        AdbMessage msg = co_await sessionCtx->coWaitCommand({AdbProtocol::CMD_OKAY}, 3000, local_id);
        if (!sessionCtx->matchCommand({AdbProtocol::CMD_OKAY}, msg)) {
            qDebug() << "[pushFile] 未收到 SEND 的 OKAY";
            co_return false;
        }
    }

    // 发送 DATA
    std::ifstream file(localFilePath, std::ios::binary);
    if (!file.is_open()) {
        qDebug() << "[pushFile] 文件打开失败: " << QString::fromStdString(localFilePath);
        co_return false;
    }

    const size_t blockSize = 1024;
    std::vector<uint8_t> buffer(blockSize);

    while (file.read(reinterpret_cast<char*>(buffer.data()), blockSize) || file.gcount() > 0) {
        size_t readCount = file.gcount();
        auto dataMsg = AdbSyncProtocol::generateDATA(local_id, remote_id, buffer, readCount);
        sessionCtx->send(dataMsg);

        // 等待 ACK
        AdbMessage msg = co_await sessionCtx->coWaitCommand({AdbProtocol::CMD_OKAY}, 3000, local_id);
        if (!sessionCtx->matchCommand({AdbProtocol::CMD_OKAY}, msg)) {
            qDebug() << "[pushFile] 未收到 DATA 块的 OKAY 响应";
            co_return false;
        }

        co_await asio::this_coro::executor;  // 控制一下节奏（可选，或加 delay）
    }

    file.close();

    // DONE
    auto donePayload = AdbSyncProtocol::generateDONE(static_cast<uint32_t>(std::time(nullptr)));
    auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
    sessionCtx->send(doneMsg);

    {
        AdbMessage msg = co_await sessionCtx->coWaitCommand({AdbProtocol::CMD_OKAY}, 3000, local_id);
        if (!sessionCtx->matchCommand({AdbProtocol::CMD_OKAY}, msg)) {
            qDebug() << "[pushFile] 未收到 DONE 的 OKAY 响应";
            co_return false;
        }
    }

    // CLOSE
    auto closeMsg = AdbProtocol::generateClose(local_id, remote_id);
    sessionCtx->send(closeMsg);

    qDebug() << "[pushFile] 文件推送完成";
    co_return true;
}


asio::awaitable<bool> pushHandler::openSyn(std::shared_ptr<DeviceSession> sessionCtx, const int local_id, int& remote_id) {
    qDebug() << "[openSyn] 发送 OPEN 命令, local_id:" << local_id << " service: sync:";

    auto openSync = AdbProtocol::generateOpen(local_id, "sync:");
    sessionCtx->send(openSync);

    qDebug() << "[openSyn] 等待 CMD_OKAY 响应...";
    AdbMessage msg = co_await sessionCtx->coWaitCommand({AdbProtocol::CMD_OKAY}, 1000, local_id);

    if (sessionCtx->matchCommand({AdbProtocol::CMD_OKAY}, msg)) {
        remote_id = msg.arg0; // CMD_OKAY 的 arg0 是 remote_id
        qDebug() << "[openSyn] 收到 CMD_OKAY, remote_id:" << remote_id;
        co_return true;
    } else {
        qDebug() << "[openSyn] 未收到期望响应，命令：" << msg.command;
        co_return false;
    }
}



asio::awaitable<bool> pushHandler::CommandHandler(std::shared_ptr<DeviceSession> sessionCtx, CommandInfo cmds) {
    qDebug() << "[CommandHandler] 开始执行 pushHandler";

    int local_id = StreamIdAllocator::allocate();
    int remote_id = -1;
    qDebug() << "[CommandHandler] 分配流 ID -> local:" << local_id << "remote:" << remote_id;

    sessionCtx->createStreamContext(local_id);
    qDebug() << "[CommandHandler] 创建本地 StreamContext，local_id:" << local_id;

    if (!sessionCtx->isServiceOpened("push")) {
        qDebug() << "[CommandHandler] 服务 push 尚未开启，尝试打开";

        bool ok = co_await openSyn(sessionCtx, local_id, remote_id);
        if (!ok) {
            qDebug() << "[CommandHandler] openSyn 打开失败，终止推送";
            // 可选清理逻辑：sessionCtx->removeStreamContext(local_id);
            sessionCtx->removeStreamContext(local_id);
            co_return false;
        }

        qDebug() << "[CommandHandler] openSyn 打开成功，remote_id:" << remote_id;
        sessionCtx->markServiceOpened("push");
    } else {
        qDebug() << "[CommandHandler] push 服务已开启，直接绑定 remote_id";
        //remote_id = sessionCtx->getBoundRemoteStreamId("push");
    }

    sessionCtx->bindRemoteStreamId(local_id, remote_id);
    qDebug() << "[CommandHandler] 绑定 remote_id 成功，local_id:" << local_id << "remote_id:" << remote_id;

    // 异步推送文件逻辑（预留）
    bool result = co_await pushFile(sessionCtx, local_id, remote_id, cmds.params[0], cmds.params[1]);
    if (result) {
        qDebug() << "[CommandHandler] 推送文件成功";
    } else {
        qDebug() << "[CommandHandler] 推送文件失败";
    }

    co_return result;
}


