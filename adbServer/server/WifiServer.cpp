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

bool WifiServer::connect(DeviceContext &ctx)
{

    auto &info = ctx.connectInfo;
    std::string ip = info.ipAddress.toStdString();
    unsigned short port = static_cast<unsigned short>(info.port);

    // asio::ip::tcp::socket socket(io_context);  // 使用成员 io_context

    // try {
    //     asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip), port);
    //     socket.connect(endpoint);
    // }
    // catch (std::system_error& e) {
    //     printf("Connect failed: %s\n", e.what());
    //     return false;
    // }

    // ctx.setSocket(std::move(socket));  // 传入 asio socket，移动语义

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


bool WifiServer::auth(DeviceContext& ctx) {
    AdbMessage Msg;
    std::vector<uint8_t> connectMessage = AdbProtocol::generateConnect();
    ctx.sessionCtx->send(connectMessage);

    // 等待 AUTH 请求（type = TOKEN）
    if (!ctx.sessionCtx->tryFindNextCommand({AdbProtocol::CMD_AUTH}, Msg)) {
        qDebug() << "未收到 AUTH TOKEN";
        return false;
    }

    // 签名 TOKEN 并发送 AUTH(type=signature)
    std::vector<uint8_t> sigload = CryptoUtils::getInstance().signAdbTokenPayload(Msg.payload);
    std::vector<uint8_t> sigMsg = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_SIGNATURE, sigload);
    ctx.sessionCtx->send(sigMsg);

    // 再等待回应，可能是 CNXN 或再次 AUTH（type=public key）
    // 这里用两个命令重载，支持两个命令判断
    if (!ctx.sessionCtx->tryFindNextCommand({AdbProtocol::CMD_AUTH, AdbProtocol::CMD_CNXN}, Msg)) {
        qDebug() << "签名后无回应";
        return false;
    }

    if (Msg.command == AdbProtocol::CMD_AUTH) {
        // 签名不被信任，发送 PUBLIC KEY
        std::vector<uint8_t> pubKey = CryptoUtils::getInstance().getPublicKeyBytes();
        std::vector<uint8_t> authPubKeyMsg = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_RSA_PUBLIC, pubKey);
        ctx.sessionCtx->send(authPubKeyMsg);
        // 最后等 CNXN
        if (!ctx.sessionCtx->tryFindNextCommand({AdbProtocol::CMD_CNXN}, Msg)) {
            qDebug() << "发送公钥后未连接成功";
            return false;
        }
    } else if (Msg.command != AdbProtocol::CMD_CNXN) {
        qDebug() << "签名后收到意外命令";
        return false;
    }

    // 解析设备信息
    ctx.authInfos = parseDeviceInfo(Msg.payload);
    qDebug() << "设备信息:";
    for (const auto& i : ctx.authInfos) {
        qDebug() << i.first << " = " << i.second;
    }

    return true;
}


bool WifiServer::execute(DeviceContext &ctx)    //后续解耦
{
    testPushThenList(ctx);
    return true;
}

void WifiServer::testPushThenList(DeviceContext &ctx)
{
    // // ===============================
    // // Step 1. Push 文件到设备
    // // ===============================

    // //"D:\\Documents\\mineQtScrcpy\\CMakeLists.txt",
    // //"/data/local/tmp/CMakeLists.txt"
    // ctx.cmd.params.clear();
    // ctx.cmd.type = CmdType::Push;

    // ctx.cmd.params = {
    //     "D:/Documents/mineQtScrcpy/scrcpy/scrcpy-server",
    //     "/data/local/tmp/scrcpy-server.jar"
    // };
    // // ctx.cmd.params = {
    // //     "D:\\Documents\\mineQtScrcpy\\CMakeLists.txt",
    // //     "/data/local/tmp/CMakeLists.jar"
    // // };

    // qDebug() << "开始推送文件到设备";
    // if (auto handler = ICommandHandler::getHandler(ctx.cmd.type)) {
    //     if (handler->CommandHandler(*ctx.transPort,ctx)) {
    //         qDebug() << "文件推送成功";
    //     } else {
    //         qDebug() << "文件推送失败";
    //         return;
    //     }
    // } else {
    //     qDebug() << "未找到 Push 命令处理器";
    //     return;
    // }

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



bool WifiServer::close(DeviceContext &ctx)
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


