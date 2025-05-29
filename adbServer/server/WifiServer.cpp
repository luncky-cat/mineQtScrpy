#include "wifiserver.h"

#include<qDebug>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Qvector>
#include<vector>
#include<string>
#include <QThread>
#include <sstream>

#include "protocol/AdbProtocol.h"
#include "utils/CryptoUtils.h"
#include "context/DeviceContext.h"
#include "interfaces/ICommandHandler.h"


std::shared_ptr<WifiServer> WifiServer::instance() {
    static std::shared_ptr<WifiServer> inst = std::make_shared<WifiServer>();
    return inst;
}

WifiServer::WifiServer()
{
    qDebug()<<"wifiservr构造";
}

bool WifiServer::connect(std::shared_ptr<DeviceContext>ctx)
{
    return true;
}

//提取auth后的字符
std::map<std::string, std::string> parseDeviceInfo(const std::vector<uint8_t>& payload) {

    std::map<std::string, std::string> infoMap;

    std::string readableStr(reinterpret_cast<const char*>(payload.data()), payload.size());

    std::stringstream ss(readableStr);
    std::string token;

    // 按照 ; 分割每个键值对
    while (std::getline(ss, token, ';')) {
        auto pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            infoMap[key] = value;
        }
    }

    return infoMap;
}

// 连接请求
void WifiServer::startConnect(std::shared_ptr<DeviceContext>Ctx) {
    Ctx->sessionCtx->send(AdbProtocol::generateConnect());
    Ctx->sessionCtx->waitConnectCommand({AdbProtocol::CMD_AUTH},100,
                                        [this,Ctx](const AdbMessage& tokenMsg) {
                                            handleAuthToken(Ctx,tokenMsg);
                                        });
}

// 第二步：收到 AUTH TOKEN，签名并发送
void WifiServer::handleAuthToken(std::shared_ptr<DeviceContext>Ctx, const AdbMessage& tokenMsg) {
    qDebug() << "[handleAuthToken] payload size = " << tokenMsg.payload.size();
    auto sigload = CryptoUtils::getInstance().signAdbTokenPayload(tokenMsg.payload);
    qDebug() << "[handleAuthToken] 签名长度: " << sigload.size();
    auto sigMsg = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_SIGNATURE, sigload);
    qDebug() << "[handleAuthToken] 准备发送签名";
    Ctx->sessionCtx->send(sigMsg);
    qDebug() << "[handleAuthToken] 发送完毕";
    Ctx->sessionCtx->waitConnectCommand({AdbProtocol::CMD_AUTH, AdbProtocol::CMD_CNXN},100,
                                        [this, Ctx](const AdbMessage& msg) {
                                            if (msg.command == AdbProtocol::CMD_AUTH) {
                                                qDebug() << "拒绝密钥";
                                                handleAuthRejected(Ctx);
                                            } else {
                                                qDebug() << "接受密钥";
                                                handleAuthAccepted(Ctx, msg);
                                            }
                                        });
}

// 第三步：签名失败，发送公钥
void WifiServer::handleAuthRejected(std::shared_ptr<DeviceContext>Ctx) {
    auto pubKey = CryptoUtils::getInstance().getPublicKeyBytes();
    auto pubKeyMsg = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_RSA_PUBLIC, pubKey);
    Ctx->sessionCtx->send(pubKeyMsg);
    qDebug()<<"发送公钥";
    Ctx->sessionCtx->waitConnectCommand({AdbProtocol::CMD_CNXN},100,
                                        [this,Ctx](const AdbMessage& cnxnMsg) {
                                            qDebug()<<"连接成功";
                                            handleAuthAccepted(Ctx, cnxnMsg);
                                        });
}

// 第四步：认证成功，解析设备信息
void WifiServer::handleAuthAccepted(std::shared_ptr<DeviceContext>Ctx, const AdbMessage& cnxnMsg) {
    Ctx->authInfos=parseDeviceInfo(cnxnMsg.payload);
    qDebug() << "设备信息:";
    for (const auto& kv : Ctx->authInfos){
        qDebug() << kv.first << "=" << kv.second;
    }
}


bool WifiServer::auth(std::shared_ptr<DeviceContext>ctx) {
    startConnect(ctx);
    return true;
}


bool WifiServer::execute(std::shared_ptr<DeviceContext>ctx)    //后续解耦
{
    testPushThenList(ctx);
    return true;
}

void WifiServer::testPushThenList(std::shared_ptr<DeviceContext>ctx)
{
    // // ===============================
    // // Step 1. Push 文件到设备
    // // ===============================

    //"D:\\Documents\\mineQtScrcpy\\CMakeLists.txt",
    //"/data/local/tmp/CMakeLists.txt"
    // ctx.cmd.params.clear();
    // ctx.cmd.type = CmdType::Push;

    // ctx.cmd.params = {
    //     "D:/Documents/mineQtScrcpy/scrcpy/scrcpy-server",
    //     "/data/local/tmp/scrcpy-server.jar"
    // };
    ctx->cmd.params = {
        "D:\\Documents\\mineQtScrcpy\\CMakeLists.txt",
        "/data/local/tmp/CMakeLists.jar"
    };

    qDebug() << "开始推送文件到设备";
    if (auto handler = ICommandHandler::getHandler(ctx.cmd.type)) {
        if (handler->CommandHandler(*ctx.transPort,ctx)) {
            qDebug() << "文件推送成功";
        } else {
            qDebug() << "文件推送失败";
            return;
        }
    } else {
        qDebug() << "未找到 Push 命令处理器";
        return;
    }

    // // ===============================
    // // Step 2. 执行 shell 查看目标目录
    // // ===============================
    // ctx.transPort->clearBuffer();
    // ctx.cmd.params.clear();
    // ctx.cmd.type = CmdType::Shell;
    // ctx.cmd.params = {
    //     "ls /data/local/tmp/\n"
    // };

    // qDebug() << "开始执行 shell 命令查看目录";
    // if (auto handler = ICommandHandler::getHandler(ctx.cmd.type)) {
    //     if (handler->CommandHandler(*ctx.transPort,ctx)) {
    //         qDebug() << "Shell 命令执行成功";
    //     } else {
    //         qDebug() << "Shell 命令执行失败";
    //     }
    // } else {
    //     qDebug() << "未找到 Shell 命令处理器";
    // }

}



bool WifiServer::close(std::shared_ptr<DeviceContext>ctx)
{
    // if (networkInitialized) {
    //     WSACleanup();
    //     networkInitialized = false;
    // }

    // auto sendMsg=AdbProtocol::generateClose(local_id,remote_id);
    // int sendLen=send(s, (const char*)sendMsg.data(),sendMsg.size(),0);
    //
    return true;
}


