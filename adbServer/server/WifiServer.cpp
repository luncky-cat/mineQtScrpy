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


WifiServer::WifiServer():networkInitialized(false){
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

    ctx.transPort->setSocket(sock);

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
    AdbMessage &Msg=ctx.msg;
    auto& transPort=ctx.transPort;
    std::vector<uint8_t> connectMessage = AdbProtocol::generateConnect();
    transPort->sendMsg(connectMessage);

    // 等待 AUTH 请求（type = TOKEN）
    if (!transPort->waitForCommand(AdbProtocol::CMD_AUTH,Msg)) {
        qDebug() << "未收到 AUTH TOKEN";
        return false;
    }

    //  签名 TOKEN 并发送 AUTH(type=signature)
    std::vector<uint8_t> sigload = CryptoUtils::getInstance().signAdbTokenPayload(Msg.payload);
    std::vector<uint8_t> sigMsg = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_SIGNATURE, sigload);
    transPort->sendMsg(sigMsg);

    //  再等待回应，可能是 CNXN 或再次 AUTH（type=public key）
    if (!transPort->waitForRecv(Msg)) {
        qDebug() << "签名后无回应";
        return false;
    }

    if (Msg.command == AdbProtocol::CMD_AUTH) {
        // 签名不被信任，发送 PUBLIC KEY
        std::vector<uint8_t> pubKey = CryptoUtils::getInstance().getPublicKeyBytes();
        std::vector<uint8_t> authPubKey = AdbProtocol::generateAuth(AdbProtocol::AUTH_TYPE_RSA_PUBLIC, pubKey);
        transPort->sendMsg(authPubKey);

        // 最后等 CNXN
        if (!transPort->waitForCommand(AdbProtocol::CMD_CNXN,Msg)) {
            qDebug() << "发送公钥后未连接成功";
            return false;
        }
    } else if (Msg.command != AdbProtocol::CMD_CNXN) {
        qDebug() << "签名后收到意外命令";
        return false;
    }

    //  解析设备信息
    ctx.deviceInfos = parseDeviceInfo(Msg.payload);
    qDebug() << "设备信息:";
    for (const auto& i : ctx.deviceInfos) {
        qDebug() << i.first << " = " << i.second;
    }

    return true;
}


// bool WifiServer::openShellChannel(DeviceContext& ctx) {

//     SOCKET s=ctx.socket;
//     auto openShell = AdbProtocol::generateOpen(++local_id, "shell:");
//     ctx.local_id = local_id;
//     sendMsg(s,openShell);   //发送连接请求

//     if (!waitForCommand(s, AdbProtocol::CMD_OKAY)) {
//         qDebug() << "未收到 ok回复";
//         return false;
//     }

//     ctx.remote_id = msg.arg0;
//     qDebug()<< "提取remote-id"<<ctx.remote_id;


//     if (!waitForCommand(s, AdbProtocol::CMD_WRTE)) {    //等待写
//         qDebug() << "未收到回写";
//         return false;
//     }

//     auto okay = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
//     sendMsg(s,okay);
//     std::string resultStr(msg.payload.begin(), msg.payload.end());
//     if(resultStr.find("/ $")!=std::string::npos){
//         qDebug()<<"找到了PD2338"<<resultStr;
//         return true;
//     }

//     return false;
// }

// bool WifiServer::openSyncChannel(DeviceContext &ctx)   //打开流服务，将文件推送到客户端
// {
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



//     return true;
// }


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

// bool WifiServer::executeShell(std::string &cmd,DeviceContext &ctx){

//     if(!openShellChannel(ctx)){
//         qDebug()<<"打开流失败";
//         return false;
//     }

//     std::vector<std::string> wrtePayloads;
//     std::string shellPromptSuffix = "/ $ ";  // 更通用的 shell 提示符

//     // std::string cmd = "getprop ro.serialno\n";  // 必须带换行符
//     std::vector<uint8_t> dataPayload(cmd.begin(), cmd.end());  // 转换为字节数组
//     auto wrteData = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
//     sendMsg(ctx.socket,wrteData); // 发送 WRTE（命令）

//     if (!waitForCommand(ctx.socket, AdbProtocol::CMD_OKAY)) {    //接收ok
//         qDebug() << "未收到ok";
//         return false;
//     }

//     while (true) {     //循环接收写

//         if (!waitForCommand(ctx.socket, AdbProtocol::CMD_WRTE)) {    //等待写
//             qDebug() << "未收到回写";
//             return false;
//         }

//         auto readyMsg = AdbProtocol::generateReady(ctx.local_id, ctx.remote_id);
//         sendMsg(ctx.socket,readyMsg);
//         qDebug() << "收到回写 发送 OKAY";
//         if(msg.payload.empty()){
//             qDebug()<<"payload为空,跳过后续处理";
//             continue;
//         }
//         // 提取数据
//         std::string result(msg.payload.begin(), msg.payload.end());
//         wrtePayloads.push_back(result);
//         qDebug() << "payload接收: " << QString::fromStdString(result);

//         // 判断接收是否结束
//         if (result.find(shellPromptSuffix) != std::string::npos) {
//             qDebug() << "检测到提示符，命令执行结束";
//             break;
//         }
//     }


//     std::string serialno = extractShellResult(wrtePayloads,cmd);
//     if (!serialno.empty()) {
//         qDebug() << "提取执行结果: " << QString::fromStdString(serialno);
//     } else {
//         qDebug() << "未能提取到执行结果";
//     }

//     QString str=QString::fromStdString(serialno);
// }


// auto openSync = AdbProtocol::generateOpen(++local_id, "sync:");
// ctx.local_id = local_id;
// sendMsg(openSync, ctx);
// if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
//     qDebug() << "未收到ok";
//     return false;
// }
// ctx.remote_id = msg.arg0;





// bool WifiServer::pushFile(SOCKET sock,int local_id,int remote_id,std::string& localFilePath,std::string remoteFilePath){
//     //SEND命令
//     std::string remotePath = remoteFilePath+",33206"; // 第二个参数是权限
//     std::vector<uint8_t> sendPayload =AdbSyncProtocol::generateSEND(remotePath);
//     auto sMsg = AdbProtocol::generateWrite(local_id,remote_id,sendPayload);
//     sendMsg(sock,sMsg);
//     if (!waitForCommand(sock, AdbProtocol::CMD_OKAY)) {    //接收ok
//         qDebug() << "未收到ok";
//         return false;
//     }

//     //打开文件，读取发送
//     std::ifstream file(localFilePath,std::ios::binary);
//     const size_t blockSize = 64 * 1024;
//     std::vector<uint8_t> buffer(blockSize);  // 这一行是定义 buffer 的地方
//     while (file.read((char*)buffer.data(), blockSize) || file.gcount() > 0) {
//         auto dataPayload = AdbSyncProtocol::generateDATA(buffer, file.gcount());
//         auto writeMsg = AdbProtocol::generateWrite(local_id,remote_id, dataPayload);
//         sendMsg(sock,writeMsg);
//         waitForCommand(sock, AdbProtocol::CMD_OKAY);
//     }

//     uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
//     auto donePayload = AdbSyncProtocol::generateDONE(mtime);
//     auto doneMsg = AdbProtocol::generateWrite(local_id, remote_id, donePayload);
//     sendMsg(sock,doneMsg);
//     waitForCommand(sock, AdbProtocol::CMD_OKAY);
//     auto closeMsg = AdbProtocol::generateClose(local_id,remote_id);
//     sendMsg(sock,closeMsg);
// }





bool WifiServer::execute(DeviceContext &ctx)    //后续解耦
{
    //解析命令

    //执行

    // const auto& cmd = ctx.cmd;
    // auto it=CommandHandlers.find(cmd.type);
    // if (it != CommandHandlers.end()) {
    //     return it->second->CommandHandler(TransPort, ctx);
    // }
    return false;



    // if(!openShellChannel(ctx)){
    //     qDebug()<<"打开流失败";
    //     return false;
    // }

    // std::string cmd = "getprop ro.serialno\n";
    // // executeShell(cmd,ctx);

    // cmd = "ls /data/local/tmp\n";
    // executeShell(cmd,ctx);

    // // cmd = "ls\n";
    // // executeShell(cmd,ctx);

    // // cmd = "date\n";
    // // executeShell(cmd,ctx);


    // // cmd = "whoami\n";
    // // executeShell(cmd,ctx);

    // auto openSync = AdbProtocol::generateOpen(++local_id, "sync:");
    // ctx.local_id = local_id;
    // sendMsg(openSync, ctx);
    // if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
    //     qDebug() << "未收到ok";
    //     return false;
    // }
    // ctx.remote_id = msg.arg0;

    // //SEND

    // std::string remotePath = "/data/local/tmp/CMakeLists.txt,33206"; // 第二个参数是权限
    // std::vector<uint8_t> sendPayload =AdbSyncProtocol::generateSEND(remotePath);
    // auto sMsg = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id,sendPayload);
    // sendMsg(sMsg,ctx);
    // if (!waitForCommand(ctx, AdbProtocol::CMD_OKAY)) {    //接收ok
    //     qDebug() << "未收到ok";
    //     return false;
    // }

    // std::ifstream file("D:/Documents/mineQtScrcpy/CMakeLists.txt", std::ios::binary);
    // const size_t blockSize = 64 * 1024;
    // std::vector<uint8_t> buffer(blockSize);  // 这一行是定义 buffer 的地方
    // while (file.read((char*)buffer.data(), blockSize) || file.gcount() > 0) {
    //     auto dataPayload = AdbSyncProtocol::generateDATA(buffer, file.gcount());
    //     auto writeMsg = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, dataPayload);
    //     sendMsg(writeMsg, ctx);
    //     waitForCommand(ctx, AdbProtocol::CMD_OKAY);
    // }

    // uint32_t mtime = static_cast<uint32_t>(std::time(nullptr));
    // auto donePayload = AdbSyncProtocol::generateDONE(mtime);
    // auto doneMsg = AdbProtocol::generateWrite(ctx.local_id, ctx.remote_id, donePayload);
    // sendMsg(doneMsg, ctx);
    // waitForCommand(ctx, AdbProtocol::CMD_OKAY);





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


// std::string WifiServer::extractShellResult(const std::vector<std::string>& payloads, const std::string& cmdEcho) {
//     qDebug()<<"提取";
//     for (const auto& line : payloads) {
//         // 跳过命令回显
//         if (line.find(cmdEcho) != std::string::npos)
//             continue;

//         // 跳过提示符
//         if (line.find("/ $") != std::string::npos || line.find(":~$") != std::string::npos)
//             continue;

//         // 去除结尾 \r \n
//         std::string cleaned = line;
//         cleaned.erase(cleaned.find_last_not_of("\r\n") + 1);

//         if (!cleaned.empty())
//             return cleaned;
//     }
//     qDebug()<<"提取失败";
//     return {};
// }



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



// std::vector<uint8_t> WifiServer::create_adb_packet(const std::string& payload) {
//     std::vector<uint8_t> packet;
//     uint32_t length = static_cast<uint32_t>(payload.length());

//     // 添加长度字段（网络字节序）
//     packet.push_back((length >> 24) & 0xFF);
//     packet.push_back((length >> 16) & 0xFF);
//     packet.push_back((length >> 8) & 0xFF);
//     packet.push_back(length & 0xFF);

//     // 添加payload
//     for (char c : payload) {
//         packet.push_back(static_cast<uint8_t>(c));
//     }

//     return packet;
// }

// std::string WifiServer::adb_version_response(int version = 31) {
//     if (version < 0 || version >= 0x10000) {
//         throw std::invalid_argument("Version number must be in the range 0 to 65535.");
//     }
//     char buffer[8];
//     snprintf(buffer, sizeof(buffer), "OKAY%04x", version);
//     return std::string(buffer);
// }

// 处理 scrcpy 发来的 host: 系列命令

// std::string WifiServer::buildAdbStringResponse(const std::string& payloadStr) {
//     std::string response = "OKAY";

//     uint32_t len = static_cast<uint32_t>(payloadStr.size());
//     char lenBytes[4] = {
//         static_cast<char>((len >> 24) & 0xFF),
//         static_cast<char>((len >> 16) & 0xFF),
//         static_cast<char>((len >> 8) & 0xFF),
//         static_cast<char>((len) & 0xFF),
//     };
//     response.append(lenBytes, 4);
//     response.append(payloadStr);
//     return response;
// }




// void WifiServer::handleHostCommand(std::string& cmdStr, int clientSocket) {
//     // if (rawData.size() < 4) {
//     //     qDebug() << "数据不足，无法解析";
//     //     return;
//     // }

//     // // 第一步：解析命令长度
//     // std::string lenStr(rawData.begin(), rawData.begin() + 4);  // 比如 "000c"
//     // int payloadLen = std::stoi(lenStr, nullptr, 16);

//     // if (rawData.size() < 4 + payloadLen) {
//     //     qDebug() << "数据未接收完整";
//     //     return;
//     // }

//     // 第二步：提取命令字符串
//     // std::string cmdStr(rawData.begin() + 4, rawData.begin() + 4 + payloadLen);
//     qDebug() << "收到 host 命令:" << QString::fromStdString(cmdStr);

//     // 构造响应内容
//     std::string response;

//     if (cmdStr == "host:version") {
//         std::string payload = "0029";  // adb版本31 = 0x001f
//         response = "OKAY0004" + payload;
//         //response=buildAdbBinaryResponse("0029");
//         response=buildAdbStringResponse("0029");
//     } else if (cmdStr.find("host:devices")!=std::string::npos) {
//         std::string deviceList = "192.168.1.2:5555\tdevice\n";
//         response=buildAdbStringResponse(deviceList);
//     } /*else if (cmdStr.find("host:transport") != std::string::npos) {
//         // host:transport:序列号/设备ip
//         response = "OKAY";  // 表示切换 transport 成功
//         // 后续数据将以 wire protocol 处理（比如 OPEN/WRTE 等）
//         //isTransported = true;

//     } else {
//         std::string unknown = "unknown command\n";
//         std::ostringstream oss;
//         oss << "FAIL" << std::setw(4) << std::setfill('0') << std::hex << unknown.size();
//         oss << unknown;
//         response = oss.str();
//     }*/

//     // 第三步：发送回应
//     //send(clientSocket, response.data(), response.size(), 0);
//     qDebug()<<"response"<<response.data()<<"长度"<<response.size();
//     send(clientSocket, response.data(), response.size(), 0);
//     qDebug() << "已回复 host 指令:" << QString::fromStdString(cmdStr);
// }


// 发送 OKAY 包的辅助函数
// void WifiServer::sendOkay(uint32_t local, uint32_t remote, DeviceContext& ctx) {
//     auto msg = AdbProtocol::generateOkay(local, remote);
//     sendMsg(msg, ctx);
// }

// // 发送 WRTE + READY 包的辅助函数（用于回复数据）
// void WifiServer::sendWrite(uint32_t local, uint32_t remote, const std::string& data, DeviceContext& ctx) {
//     auto payload = std::vector<uint8_t>(data.begin(), data.end());
//     sendMsg(AdbProtocol::generateWrite(local, remote, payload), ctx);
//     sendMsg(AdbProtocol::generateReady(local, remote), ctx);
// }

// // 你主循环处理接收包时的简要示范（伪代码）
// void WifiServer::onReceiveAdbMessage(DeviceContext& ctx) {
//     if (msg.command == AdbProtocol::CMD_WRTE) {
//         std::string content(msg.payload.begin(), msg.payload.end());
//         if (content.rfind("host:", 0) == 0) {
//             // 处理 host: 系列命令
//             handleScrcpyHostCommand(content, ctx);
//         }
//         else if (content.rfind("shell:", 0) == 0) {
//             // 处理 shell 命令，如启动 scrcpy-server.jar
//             // 你已有 shell channel 代码，可在这里调度
//             openShellChannel(ctx);
//         }
//         else {
//             // 其他命令或数据，按你的设计处理
//         }
//     }
//     else if (msg.command == AdbProtocol::CMD_CLSE) {
//         // 连接关闭，清理资源
//     }
// }

// void WifiServer::handleClient(){

// }

