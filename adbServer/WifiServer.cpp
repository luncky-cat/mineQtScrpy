#include "wifiserver.h"

#include "AdbProtocol.h"
#include "AdbCryptoUtils.h"
#include "DeviceContext.h"
#include "qdebug.h"

#include <QThread>
#include <WinSock2.h>
#include <ws2tcpip.h>


WifiServer::WifiServer():networkInitialized(false) {
    networkInitialized=initNetwork();
    recvData.resize(4096);
    local_id=0;
}

WifiServer::~WifiServer()
{

}

WifiServer &WifiServer::instance()
{
    static WifiServer instance;
    return instance;
}

bool WifiServer::connect(DeviceContext &ctx)
{
    if (!networkInitialized) {
        return false;
    }

    auto &info=ctx.connectInfo;
    std::string ip = info.ipAddress.toStdString();
    unsigned short port = static_cast<unsigned short>(info.port);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        printf("Connect failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return false;
    }
    ctx.socket=sock;
    return true;
}


bool WifiServer::auth(DeviceContext &ctx)
{
    //连接请求cnxm
    SOCKET sock=ctx.socket;   //获得socket
    this->s=sock;

    std::vector<uint8_t> connectMessage = AdbProtocol::generateConnect();

    sendMsg(connectMessage,ctx);   //发送连接请求

    recvMsg(recvData,ctx,msg);  //接收结果

    if(msg.command!=0x48545541){
        return false;
    }

    //签名数据
    std::vector<uint8_t> sigload=AdbCryptoUtils::getInstance().signAdbTokenPayload(msg.payload);
    std::vector<uint8_t> sigMsg= AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_SIGNATURE, sigload);
    sendMsg(sigMsg,ctx);
    recvMsg(recvData,ctx,msg);

    //首次连接传输公钥
    std::vector<uint8_t> pubKey =AdbCryptoUtils::getInstance().getPublicKeyBytes();
    pubKey.push_back('\0');
    std::vector<uint8_t> authPubKey = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_RSA_PUBLIC,pubKey);
    sendMsg(authPubKey,ctx);
    recvMsg(recvData,ctx,msg);
    std::string readableStr(reinterpret_cast<const char*>(msg.payload.data()), msg.payload.size());
    qDebug() << QString::fromStdString(readableStr);
    //将得到的数据提取，存入context中

    // struct DeviceInfo {
    //     std::string productName;   // PD2338
    //     std::string model;         // V2338A
    //     std::string device;        // PD2338
    //     std::vector<std::string> features;  // shell_v2, cmd, ...
    // };
    return true;
}

bool WifiServer::execute(DeviceContext &ctx)
{

    if(!ctx.isOpenShell){    //执行前打开shell
        openShellChannel(ctx);

    }

    QThread::msleep(1000);  // 可适当 sleep 确保 shell 完全 ready

    std::string cmd = "id\n";  // ✅ 肯定有输出
    std::vector<uint8_t> cmdPayload(cmd.begin(), cmd.end());

    auto write = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, cmdPayload);
    sendMsg(write, ctx);

    // 接收回显和结果
    while (true) {
        recvMsg(recvData, ctx, msg);
        if (msg.command ==0x45545257) {
            std::string output(reinterpret_cast<const char*>(msg.payload.data()), msg.payload.size());
            qDebug()<< "[WRTE Payload] " << output;

            auto s=AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
            sendMsg(s, ctx);

            if (output.find("$ ") != std::string::npos || output.find("# ") != std::string::npos)
                break;
        }
    }

    //获得结果   存回ctx中也许，外部通过查询ctx来获得信息
    return true;
}

bool WifiServer::openShellChannel(DeviceContext& ctx) {

    auto openShell = AdbProtocol::generateOpen(++local_id, "shell:");
    ctx.local_id = local_id;
    sendMsg(openShell, ctx);
    while (true) {
        recvMsg(recvData, ctx, msg);
        if (msg.command ==AdbProtocol::CMD_OKAY) {

            ctx.remote_id = msg.arg0;
            ctx.remote_id=msg.arg0;
            qDebug()<< "ctx remote-id"<<ctx.remote_id;
        }

        if (msg.command == AdbProtocol::CMD_WRTE) {
            std::vector<uint8_t>okay=AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
            msg=AdbProtocol::parseAdbMessage(okay);
            AdbProtocol::printAdbMessage(msg);
            sendMsg(okay, ctx);
            return true;
        }
    }

    // 等待 shell 的 WRTE 输出
   // recvMsg(recvData, ctx, msg);
    // if (msg.command == AdbProtocol::CMD_WRTE) {

    //     std::vector<uint8_t>okay=AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
    //     msg=AdbProtocol::parseAdbMessage(okay);
    //     AdbProtocol::printAdbMessage(msg);
    //     sendMsg(okay, ctx);
    //     return true;
    // }

    return false;
}


bool WifiServer::close(DeviceContext &ctx)
{
    if (networkInitialized) {
        WSACleanup();
        networkInitialized = false;
    }

    auto sendMsg=AdbProtocol::generateClose(local_id,remote_id);
    int sendLen=send(s, (const char*)sendMsg.data(),sendMsg.size(),0);
    if(sendLen>0){
        QThread::msleep(1000);
    }
    //像设备发送

    return true;
}


bool WifiServer::initNetwork() {   //初始化网络资源
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);  // 初始化 Winsock 2.2
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return false;
    }
    return true;
}

bool WifiServer::sendMsg(std::vector<uint8_t>&sendMsg,DeviceContext& ctx)
{
    int sendLen=send(ctx.socket, (const char*)sendMsg.data(),sendMsg.size(),0);
    return sendLen>=0?true:false;
}

bool WifiServer::recvMsg(std::vector<uint8_t>&recvMsg,DeviceContext& ctx,AdbMessage& msg){
    recvMsg.assign(recvMsg.size(), 0);  // 清零，但保持缓冲区大小
    int recvLen = recv(ctx.socket, (char*)recvMsg.data(), recvMsg.size(), 0);
    msg =AdbProtocol::parseAdbMessage(recvData);
    AdbProtocol::printAdbMessage(msg);
    return recvLen <= 0?false:true;
}

// std::map<std::string, std::string> parseAdbDeviceInfo(const std::string& rawInfo) {
//     std::string info = rawInfo;

//     // 去掉前缀 device::
//     if (info.rfind("device::", 0) == 0) {
//         info = info.substr(8);
//     }

//     std::map<std::string, std::string> result;
//     std::stringstream ss(info);
//     std::string item;

//     while (std::getline(ss, item, ';')) {
//         size_t pos = item.find('=');
//         if (pos != std::string::npos) {
//             std::string key = item.substr(0, pos);
//             std::string value = item.substr(pos + 1);
//             result[key] = value;
//         }
//     }

//     return result;
// }


// std::string execShellCommand(DeviceContext& ctx, const std::string& cmd) {
//     // 发送命令，需包含 \n 以确保 shell 执行
//     std::string command = cmd;
//     if (!command.empty() && command.back() != '\n') {
//         command += '\n';
//     }

//     std::vector<uint8_t> cmdPayload(command.begin(), command.end());
//     auto write = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, cmdPayload);
//     sendMsg(write, ctx);

//     std::string result;

//     // 接收 WRTE 数据（命令回显、输出、提示符）
//     while (true) {
//         AdbMessage msg;
//         std::vector<uint8_t> recvData;
//         recvMsg(recvData, ctx, msg);

//         if (msg.command == AdbProtocol::CMD_WRTE) {
//             std::string output(reinterpret_cast<const char*>(msg.payload.data()), msg.payload.size());
//             result += output;

//             // 回复 OKAY
//             auto okay = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
//             sendMsg(okay, ctx);

//             // 判断是否结束（看到 shell 提示符 "$ " 或 root "# "）
//             if (output.find("$ ") != std::string::npos || output.find("# ") != std::string::npos) {
//                 break;
//             }
//         }
//     }

//     return result;
// }
