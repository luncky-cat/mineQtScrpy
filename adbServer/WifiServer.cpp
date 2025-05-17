#include "wifiserver.h"

#include "AdbProtocol.h"
#include "AdbCryptoUtils.h"
#include "DeviceContext.h"

#include<qDebug>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <fstream>
#include <Qvector>
#include<vector>
#include<string>
#include <QThread>


namespace AdbSyncProtocol {

constexpr uint32_t SYNC_SEND = ('S') | ('E' << 8) | ('N' << 16) | ('D' << 24);
constexpr uint32_t SYNC_DATA = ('D') | ('A' << 8) | ('T' << 16) | ('A' << 24);
constexpr uint32_t SYNC_DONE = ('D') | ('O' << 8) | ('N' << 16) | ('E' << 24);

/// 构造 SEND 命令 payload
std::vector<uint8_t> generateSEND(const std::string& remotePathWithMode) {
    std::vector<uint8_t> payload;
    uint32_t pathLen = static_cast<uint32_t>(remotePathWithMode.size());

    // 构造 SEND header (command + path length)
    payload.push_back('S'); payload.push_back('E');
    payload.push_back('N'); payload.push_back('D');

    payload.push_back(pathLen & 0xFF);
    payload.push_back((pathLen >> 8) & 0xFF);
    payload.push_back((pathLen >> 16) & 0xFF);
    payload.push_back((pathLen >> 24) & 0xFF);

    // 添加路径和权限（如 "/data/local/tmp/test.txt,33206"）
    payload.insert(payload.end(), remotePathWithMode.begin(), remotePathWithMode.end());

    return payload;
}

/// 构造 DATA 块 payload（每块最大 64K）
std::vector<uint8_t> generateDATA(const std::vector<uint8_t>& buf, size_t len) {
    std::vector<uint8_t> payload;

    // 写入 "DATA" 标识
    payload.push_back('D'); payload.push_back('A');
    payload.push_back('T'); payload.push_back('A');

    // 写入数据长度
    payload.push_back(len & 0xFF);
    payload.push_back((len >> 8) & 0xFF);
    payload.push_back((len >> 16) & 0xFF);
    payload.push_back((len >> 24) & 0xFF);

    // 附加实际文件内容
    payload.insert(payload.end(), buf.begin(), buf.begin() + len);

    return payload;
}

/// 构造 DONE payload（mtime 为最后修改时间）
std::vector<uint8_t> generateDONE(uint32_t mtime) {
    std::vector<uint8_t> payload;

    payload.push_back('D'); payload.push_back('O');
    payload.push_back('N'); payload.push_back('E');

    payload.push_back(mtime & 0xFF);
    payload.push_back((mtime >> 8) & 0xFF);
    payload.push_back((mtime >> 16) & 0xFF);
    payload.push_back((mtime >> 24) & 0xFF);

    return payload;
}

}



bool WifiServer::sendMsg(std::vector<uint8_t>& sendMsg, DeviceContext& ctx) {
    int sendLen = send(ctx.socket, reinterpret_cast<const char*>(sendMsg.data()), sendMsg.size(), 0);
    return sendLen >= 0;
}

bool WifiServer::recvMsg(DeviceContext& ctx) {
    char tempBuf[4096];
    int recvLen = recv(ctx.socket, tempBuf, sizeof(tempBuf), 0);
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

        msg = result.value();  // 取出已完成包
        recvBuffer.erase(recvBuffer.begin(),recvBuffer.begin() + msgLen);  // 移除已消费
        return true;
    }

    return false;  // 当前数据不足，等待更多数据
}


WifiServer::WifiServer():networkInitialized(false),recvData(4096),local_id(0){
    networkInitialized=initNetwork();
}

WifiServer &WifiServer::instance()
{
    static WifiServer instance;
    return instance;
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


// bool WifiServer::auth(DeviceContext &ctx)
// {
//     //连接请求cnxm
//     std::vector<uint8_t> connectMessage = AdbProtocol::generateConnect();
//     sendMsg(connectMessage,ctx);   //发送连接请求
//     while(!recvMsg(ctx)){
//         break;
//     }

//     if(msg.command!=AdbProtocol::CMD_WRTE){
//         return false;
//     }

//     //签名数据
//     std::vector<uint8_t> sigload=AdbCryptoUtils::getInstance().signAdbTokenPayload(msg.payload);
//     std::vector<uint8_t> sigMsg= AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_SIGNATURE, sigload);
//     sendMsg(sigMsg,ctx);   //发送连接请求
//     while(!recvMsg(ctx)){
//         break;
//     }
//     qDebug()<<"发送签名数据";


//     //首次连接传输公钥
//     std::vector<uint8_t> pubKey =AdbCryptoUtils::getInstance().getPublicKeyBytes();
//     // pubKey.push_back('\0');
//     std::vector<uint8_t> authPubKey = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_RSA_PUBLIC,pubKey);
//     sendMsg(authPubKey,ctx);   //发送连接请求
//     while(!recvMsg(ctx)){
//         break;
//     }
//     ctx.deviceInfos=parseDeviceInfo(msg.payload);  //解析信息放到上下文中
//     qDebug()<<"公钥签名完成";
//     for(auto i:ctx.deviceInfos){
//         qDebug()<<i.first<<" "<<i.second;
//     }

//     return true;
// }


bool WifiServer::auth(DeviceContext& ctx) {
    //发送 CONNECT 请求
    std::vector<uint8_t> connectMessage = AdbProtocol::generateConnect();
    sendMsg(connectMessage, ctx);

    // 等待 AUTH 请求（type = TOKEN）
    if (!waitForCommand(ctx, AdbProtocol::CMD_AUTH)) {
        qDebug() << "未收到 AUTH TOKEN";
        return false;
    }

    //  签名 TOKEN 并发送 AUTH(type=signature)
    std::vector<uint8_t> sigload = AdbCryptoUtils::getInstance().signAdbTokenPayload(msg.payload);
    std::vector<uint8_t> sigMsg = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_SIGNATURE, sigload);
    sendMsg(sigMsg, ctx);

    //  再等待回应，可能是 CNXN 或再次 AUTH（type=public key）
    if (!waitForRecv(ctx)) {
        qDebug() << "签名后无回应";
        return false;
    }

    if (msg.command == AdbProtocol::CMD_AUTH) {
        // 签名不被信任，发送 PUBLIC KEY
        std::vector<uint8_t> pubKey = AdbCryptoUtils::getInstance().getPublicKeyBytes();
        std::vector<uint8_t> authPubKey = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_RSA_PUBLIC, pubKey);
        sendMsg(authPubKey, ctx);

        // 最后等 CNXN
        if (!waitForCommand(ctx, AdbProtocol::CMD_CNXN)) {
            qDebug() << "发送公钥后未连接成功";
            return false;
        }
    } else if (msg.command != AdbProtocol::CMD_CNXN) {
        qDebug() << "签名后收到意外命令";
        return false;
    }

    //  解析设备信息
    ctx.deviceInfos = parseDeviceInfo(msg.payload);
    qDebug() << "设备信息:";
    for (const auto& i : ctx.deviceInfos) {
        qDebug() << i.first << " = " << i.second;
    }

    return true;
}

bool WifiServer::waitForRecv(DeviceContext& ctx, int maxAttempts, int intervalMs) {
    for (int i = 0; i < maxAttempts; ++i) {
        if (recvMsg(ctx)) {
            return true;
        }
        QThread::msleep(intervalMs);  // 可用 std::this_thread::sleep_for
    }
    return false;
}

bool WifiServer::waitForCommand(DeviceContext& ctx, uint32_t expectCmd) {
    if (!waitForRecv(ctx)) {
        return false;
    }
    return msg.command == expectCmd;
}



bool WifiServer::openShellChannel(DeviceContext& ctx) {

    auto openShell = AdbProtocol::generateOpen(++local_id, "shell:");
    ctx.local_id = local_id;
    sendMsg(openShell,ctx);   //发送连接请求

    if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {
        qDebug() << "未收到 ok回复";
        return false;
    }

    ctx.remote_id = msg.arg0;
    qDebug()<< "提取remote-id"<<ctx.remote_id;


    if (!waitForCommand(ctx, AdbProtocol::CMD_WRTE)) {    //等待写
        qDebug() << "未收到回写";
        return false;
    }

    auto okay = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
    sendMsg(okay, ctx);
    std::string resultStr(msg.payload.begin(), msg.payload.end());
    if(resultStr.find("/ $")!=std::string::npos){
        qDebug()<<"找到了PD2338"<<resultStr;
        return true;
    }

    return false;
}

bool WifiServer::openSyncChannel(DeviceContext &ctx)   //打开流服务，将文件推送到客户端
{
    // auto openSync = AdbProtocol::generateOpen(++local_id, "sync:");
    // ctx.local_id = local_id;
    // sendMsg(openSync, ctx);

    // // 等待设备返回 OKAY，表示 sync 服务已打开
    // while (true) {
    //     recvMsg(recvData, ctx, msg);
    //     if (msg.command == AdbProtocol::CMD_OKAY) {
    //         qDebug()<<"打开流";
    //         ctx.remote_id = msg.arg0;  // 必须用设备返回的 remote_id
    //         break;
    //     }
    // }

    // Sleep(1000);

    // std::string remotePath = "/sdcard/test.txt";
    // std::string sendCmd = "SEND" + remotePath + "," + std::to_string(33206);

    // // 构建 payload
    // std::vector<uint8_t> sendPayload(sendCmd.begin(), sendCmd.end());
    // auto wrteSend = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, sendPayload);
    // sendMsg(wrteSend, ctx);
    // while (true) {
    //     recvMsg(recvData, ctx, msg);
    //     if (msg.command == AdbProtocol::CMD_OKAY) {
    //         break;
    //     }
    // }

    // const size_t CHUNK_SIZE = 4096;
    // std::ifstream file("test.txt", std::ios::binary);

    // while (file) {
    //     std::vector<uint8_t> chunk(CHUNK_SIZE);
    //     file.read(reinterpret_cast<char*>(chunk.data()), CHUNK_SIZE);
    //     size_t bytesRead = file.gcount();

    //     if (bytesRead == 0) break;

    //     // 构造 "DATA" + len + payload
    //     std::vector<uint8_t> dataPayload;
    //     dataPayload.insert(dataPayload.end(), {'D','A','T','A'});

    //     uint32_t len = static_cast<uint32_t>(bytesRead);
    //     dataPayload.push_back(len & 0xff);
    //     dataPayload.push_back((len >> 8) & 0xff);
    //     dataPayload.push_back((len >> 16) & 0xff);
    //     dataPayload.push_back((len >> 24) & 0xff);

    //     dataPayload.insert(dataPayload.end(), chunk.begin(), chunk.begin() + bytesRead);

    //     auto wrteData = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
    //     sendMsg(wrteData, ctx);
    //     while (true) {
    //         recvMsg(recvData, ctx, msg);
    //         if (msg.command == AdbProtocol::CMD_OKAY) {
    //             break;
    //         }
    //     }
    // }

    // uint32_t timestamp = static_cast<uint32_t>(std::time(nullptr)); // 当前时间

    // std::vector<uint8_t> donePayload = {'D','O','N','E'};
    // donePayload.push_back(timestamp & 0xff);
    // donePayload.push_back((timestamp >> 8) & 0xff);
    // donePayload.push_back((timestamp >> 16) & 0xff);
    // donePayload.push_back((timestamp >> 24) & 0xff);

    // auto wrteDone = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, donePayload);
    // sendMsg(wrteDone, ctx);
    // while (true) {
    //     recvMsg(recvData, ctx, msg);
    //     if (msg.command == AdbProtocol::CMD_OKAY) {
    //         break;
    //     }
    // }


    // std::vector<uint8_t> quitPayload = {'Q','U','I','T'};
    // auto wrteQuit = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, quitPayload);
    // sendMsg(wrteQuit, ctx);
    // while (true) {
    //     recvMsg(recvData, ctx, msg);
    //     if (msg.command == AdbProtocol::CMD_OKAY) {
    //         break;
    //     }
    // }

    // // 发送 CLSE 关闭
    // auto close = AdbProtocol::generateClose(ctx.local_id, ctx.remote_id);
    // sendMsg(close, ctx);



    return true;
}


// void startScrcpy(const QString &serial)
// {
//     QString program = "scrcpy";
//     QStringList arguments;
//     arguments << "-s" << serial; // 指定序列号

//     // 可选参数（可根据需要添加）：
//     // arguments << "--no-control";  // 禁止控制，只推流
//     // arguments << "--always-on-top"; // 窗口置顶
//     // arguments << "--window-title" << "MyDevice";

//     QProcess *process = new QProcess;
//     process->start(program, arguments);

//     // 你也可以连接槽函数监控 process 的状态
//     QObject::connect(process, &QProcess::started, []() {
//         qDebug() << "scrcpy started.";
//     });

//     QObject::connect(process, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
//         qDebug() << "Failed to start scrcpy. Error:" << error;
//     });
// }

bool WifiServer::executeShell(std::string &cmd,DeviceContext &ctx){

    if(!openShellChannel(ctx)){
        qDebug()<<"打开流失败";
        return false;
    }

    std::vector<std::string> wrtePayloads;
    std::string shellPromptSuffix = "/ $ ";  // 更通用的 shell 提示符

    // std::string cmd = "getprop ro.serialno\n";  // 必须带换行符
    std::vector<uint8_t> dataPayload(cmd.begin(), cmd.end());  // 转换为字节数组
    auto wrteData = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
    sendMsg(wrteData, ctx); // 发送 WRTE（命令）

    if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }

    while (true) {     //循环接收写

        if (!waitForCommand(ctx, AdbProtocol::CMD_WRTE)) {    //等待写
            qDebug() << "未收到回写";
            return false;
        }

        auto readyMsg = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
        sendMsg(readyMsg, ctx);
        qDebug() << "收到回写 发送 OKAY";
        if(msg.payload.empty()){
            qDebug()<<"payload为空,跳过后续处理";
            continue;
        }
        // 提取数据
        std::string result(msg.payload.begin(), msg.payload.end());
        wrtePayloads.push_back(result);
        qDebug() << "payload接收: " << QString::fromStdString(result);

        // 判断接收是否结束
        if (result.find(shellPromptSuffix) != std::string::npos) {
            qDebug() << "检测到提示符，命令执行结束";
            break;
        }
    }


    std::string serialno = extractShellResult(wrtePayloads,cmd);
    if (!serialno.empty()) {
        qDebug() << "提取执行结果: " << QString::fromStdString(serialno);
    } else {
        qDebug() << "未能提取到执行结果";
    }

    QString str=QString::fromStdString(serialno);
}


bool WifiServer::execute(DeviceContext &ctx)    //后续解耦
{
    // if(!openShellChannel(ctx)){
    //     qDebug()<<"打开流失败";
    //     return false;
    // }

    std::string cmd = "getprop ro.serialno\n";
    // executeShell(cmd,ctx);

    cmd = "ls /data/local/tmp\n";
    executeShell(cmd,ctx);

    // cmd = "ls\n";
    // executeShell(cmd,ctx);

    // cmd = "date\n";
    // executeShell(cmd,ctx);


    // cmd = "whoami\n";
    // executeShell(cmd,ctx);

    auto openSync = AdbProtocol::generateOpen(++local_id, "sync:");
    ctx.local_id = local_id;
    sendMsg(openSync, ctx);
    if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }
    ctx.remote_id = msg.arg0;

    //SEND

    std::string remotePath = "/data/local/tmp/CMakeLists.txt,33206"; // 第二个参数是权限
    std::vector<uint8_t> sendPayload =AdbSyncProtocol::generateSEND(remotePath);
    auto sMsg = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id,sendPayload);
    sendMsg(sMsg,ctx);
    if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }

    std::ifstream file("D:/Documents/mineQtScrcpy/CMakeLists.txt", std::ios::binary);
    const size_t blockSize = 64 * 1024;
    std::vector<uint8_t> buffer(blockSize);  // 这一行是定义 buffer 的地方
    while (file.read((char*)buffer.data(), blockSize) || file.gcount() > 0) {
        auto dataPayload = AdbSyncProtocol::generateDATA(buffer, file.gcount());
        auto writeMsg = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
        sendMsg(writeMsg, ctx);
        waitForCommand(ctx, AdbProtocol::CMD_OKAY);
    }

    uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
    auto donePayload = AdbSyncProtocol::generateDONE(mtime);
    auto doneMsg = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, donePayload);
    sendMsg(doneMsg, ctx);
    waitForCommand(ctx, AdbProtocol::CMD_OKAY);





    // auto closeMsg = AdbProtocol::generateClose(ctx.local_id, ctx.remote_id);
    // sendMsg(closeMsg, ctx);


    // std::vector<std::string> wrtePayloads;
    // std::string shellPromptSuffix = "/ $ ";  // 更通用的 shell 提示符

    // std::string cmd = "getprop ro.serialno\n";  // 必须带换行符
    // std::vector<uint8_t> dataPayload(cmd.begin(), cmd.end());  // 转换为字节数组
    // auto wrteData = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
    // sendMsg(wrteData, ctx); // 发送 WRTE（命令）

    // if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
    //     qDebug() << "未收到ok";
    //     return false;
    // }

    // while (true) {     //循环接收写

    //     if (!waitForCommand(ctx, AdbProtocol::CMD_WRTE)) {    //等待写
    //         qDebug() << "未收到回写";
    //         return false;
    //     }

    //     auto readyMsg = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
    //     sendMsg(readyMsg, ctx);
    //     qDebug() << "收到回写 发送 OKAY";
    //     if(msg.payload.empty()){
    //         qDebug()<<"payload为空,跳过后续处理";
    //         continue;
    //     }
    //     // 提取数据
    //     std::string result(msg.payload.begin(), msg.payload.end());
    //     wrtePayloads.push_back(result);
    //     qDebug() << "payload接收: " << QString::fromStdString(result);

    //     // 判断接收是否结束
    //     if (result.find(shellPromptSuffix) != std::string::npos) {
    //         qDebug() << "检测到提示符，命令执行结束";
    //         break;
    //     }
    // }


    // std::string serialno = extractShellResult(wrtePayloads, "getprop ro.serialno");
    // if (!serialno.empty()) {
    //     qDebug() << "设备序列号为: " << QString::fromStdString(serialno);
    // } else {
    //     qDebug() << "未能提取到序列号";
    // }

    // QString str=QString::fromStdString(serialno);

    return true;
}

// bool WifiServer::executeShell(int local_id,int remote_id,SOCKET sock,std::string cmd){
//     std::string cmd = "getprop ro.serialno\n";  // 必须带换行符
//     std::vector<uint8_t> dataPayload(cmd.begin(), cmd.end());  // 转换为字节数组

//     auto wrteData = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
//     sendMsg(wrteData, ctx);
//     while (true) {
//         std::vector<uint8_t> recvData;
//         AdbMessage msg;
//         recvMsg(recvData, ctx, msg);

//         if (msg.command == AdbProtocol::CMD_WRTE) {
//             std::string result(recvData.begin(), recvData.end());
//             qDebug() << "设备返回: " << result;

//             // 发送 READY 表示我已接收 WRTE 数据
//             auto readyMsg = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
//             sendMsg(readyMsg, ctx);
//             break;
//         }
//         else if (msg.command == AdbProtocol::CMD_CLSE) {
//             // 通道关闭了，结束通信
//             break;
//         }
//     }
// }


// std::string execShellCommand(DeviceContext& ctx, const std::string& command) {
//     // 1. 构造 shell 命令数据
//     std::string cmdWithNewline = command + "\n";  // 必须带换行符
//     std::vector<uint8_t> dataPayload(cmdWithNewline.begin(), cmdWithNewline.end());

//     // 2. 发送 WRTE 消息
//     auto wrteData = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
//     sendMsg(wrteData, ctx);

//     std::string output;

//     // 3. 等待 WRTE 或 CLSE
//     while (true) {
//         std::vector<uint8_t> recvData;
//         AdbMessage msg;
//         if (!recvMsg(recvData, ctx, msg)) {
//             break;  // 读取失败
//         }

//         if (msg.command == AdbProtocol::CMD_WRTE) {
//             std::string part(recvData.begin(), recvData.end());
//             output += part;  // 追加输出结果

//             // 必须回一个 READY 表示我已经处理 WRTE
//             auto readyMsg = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
//             sendMsg(readyMsg, ctx);
//         }
//         else if (msg.command == AdbProtocol::CMD_CLSE) {
//             break;  // 通道关闭
//         }
//     }

//     return output;
// }


std::string WifiServer::extractShellResult(const std::vector<std::string>& payloads, const std::string& cmdEcho) {
    qDebug()<<"提取";
    for (const auto& line : payloads) {
        // 跳过命令回显
        if (line.find(cmdEcho) != std::string::npos)
            continue;

        // 跳过提示符
        if (line.find("/ $") != std::string::npos || line.find(":~$") != std::string::npos)
            continue;

        // 去除结尾 \r \n
        std::string cleaned = line;
        cleaned.erase(cleaned.find_last_not_of("\r\n") + 1);

        if (!cleaned.empty())
            return cleaned;
    }
    qDebug()<<"提取失败";
    return {};
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

// 伪代码，依赖你已有的 sendMsg, recvMsg 和 AdbProtocol 相关方法

// bool pushFile(DeviceContext& ctx, const std::string& localFilePath, const std::string& devicePath) {
//     // 1. 读取本地文件内容
//     std::ifstream file(localFilePath, std::ios::binary);
//     if (!file.is_open()) {
//         qDebug() << "打开文件失败:" << QString::fromStdString(localFilePath);
//         return false;
//     }

//     // 2. 发送 SEND 请求：路径 + 权限
//     // 权限一般用 0644，路径后用逗号分隔权限
//     std::string sendPath = devicePath + ",0644";
//     auto sendPayload = std::vector<uint8_t>(sendPath.begin(), sendPath.end());

//     auto sendMsg = AdbProtocol::generateSyncSend(ctx.local_id, ctx.remote_id, sendPayload);
//     if (!sendMsg(sendMsg, ctx)) {
//         qDebug() << "发送 SEND 失败";
//         return false;
//     }

//     // 3. 读取文件分块发送 DATA
//     const size_t bufferSize = 64 * 1024; // 64KB
//     std::vector<uint8_t> buffer(bufferSize);

//     while (!file.eof()) {
//         file.read(reinterpret_cast<char*>(buffer.data()), bufferSize);
//         std::streamsize bytesRead = file.gcount();

//         if (bytesRead <= 0) break;

//         std::vector<uint8_t> dataPayload(buffer.begin(), buffer.begin() + bytesRead);
//         auto dataMsg = AdbProtocol::generateSyncData(ctx.local_id, ctx.remote_id, dataPayload);

//         if (!sendMsg(dataMsg, ctx)) {
//             qDebug() << "发送 DATA 失败";
//             return false;
//         }
//     }

//     // 4. 发送 DONE，时间戳一般用0或当前时间（单位秒）
//     uint32_t mtime = static_cast<uint32_t>(time(nullptr));
//     auto doneMsg = AdbProtocol::generateSyncDone(ctx.local_id, ctx.remote_id, mtime);

//     if (!sendMsg(doneMsg, ctx)) {
//         qDebug() << "发送 DONE 失败";
//         return false;
//     }

//     // 5. 等待服务器回复 OKAY 或 FAIL
//     if (!recvMsg(ctx)) {
//         qDebug() << "接收响应失败";
//         return false;
//     }

//     auto result = AdbProtocol::parseAdbMessage(recvData);
//     if (!result.has_value()) {
//         qDebug() << "解析响应失败";
//         return false;
//     }

//     AdbMessage msg = result.value();

//     if (msg.command == AdbProtocol::CMD_OKAY) {
//         qDebug() << "文件推送成功";
//         return true;
//     } else if (msg.command == AdbProtocol::CMD_FAIL) {
//         std::string failReason(msg.payload.begin(), msg.payload.end());
//         qDebug() << "文件推送失败：" << QString::fromStdString(failReason);
//         return false;
//     }

//     return false;
// }
