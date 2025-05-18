#include "AdbServer.h"

#include "ThreadPool.h"

#include <QNetworkProxy>
#include<qdebug.h>
#include <string>
#include <thread>
#include <sstream>

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


AdbServer &AdbServer::getInstance()
{
    static AdbServer instance;
    return instance;
}

AdbServer::~AdbServer()
{

}

void AdbServer::asyncConnectDevice(const std::string &deviceId)
{
    qDebug() << "adb连接设备异步操作";
    ThreadPool::getInstance().submit([this,deviceId](){
        this->doConnectDevice(deviceId);
    });
}

void AdbServer::doConnectDevice(const std::string &deviceId)
{
    qDebug() << "adb连接设备实际操作";
    auto ctx = getDeviceContext(deviceId);
    if (!ctx) return;
    setState(deviceId, std::make_unique<ConnectingState>());
    ctx->currentState->handle(*ctx);
}


DeviceContext* AdbServer::getDeviceContext(const std::string& deviceId) {
    auto it = deviceContextMap.find(deviceId);
    return it != deviceContextMap.end() ? &it->second : nullptr;
}

const DeviceContext *AdbServer::getDeviceContext(const std::string &deviceId) const
{
    auto it = deviceContextMap.find(deviceId);
    return it != deviceContextMap.end() ? &it->second : nullptr;
}

bool AdbServer::registerDevice(const ConnectInfo& info) {
    std::string deviceId=info.deviceId.toStdString();

    if(getDeviceContext(deviceId)){  //找到了
        return false;
    }

    qDebug()<<"注册"<<info.deviceId;

    DeviceContext context(fac->create(info.ConnectType));  //传入参数
    context.setConnectInfo(info);
    deviceContextMap[deviceId] = std::move(context);

    return true;
}

bool AdbServer::unregisterDevice(const std::string &deviceId)
{
    deviceContextMap.erase(deviceId);   //销毁ID对应资源
    return true;
}

std::vector<std::string> AdbServer::getRegisteredDevices() const
{
    std::vector<std::string> deviceIds;
    for (const auto& pair : deviceContextMap) {
        deviceIds.push_back(pair.first);

        qDebug() << QString::fromStdString(pair.first);  //
    }

    return deviceIds;
}

// bool AdbServer::connectDevice(const std::string& deviceId)
// {
//     auto ctx = getDeviceContext(deviceId);
//     if (!ctx) return false;
//     setState(deviceId, std::make_unique<ConnectingState>());
//     qDebug()<<"adb连接设备";

//     return ctx->currentState->handle(*ctx);  // 直接传递 ctx,返回结果
// }

// #include <functional> // For std::bind



bool AdbServer::disconnectDevice(const std::string &deviceId)
{
    auto ctx = getDeviceContext(deviceId);
    if (!ctx) return false;
    setState(deviceId, std::make_unique<DisconnectedState>());
    ctx->currentState->handle(*ctx);
    unregisterDevice(deviceId);
    return true;
}

// bool AdbServer::executeCommand(const std::string &deviceId, const std::string &command, std::function<void (const std::vector<uint8_t> &)> callback)
// {
//     auto ctx = getDeviceContext(deviceId);
//     if (!ctx) return false;
//     //将命令传入
//     setState(deviceId, std::make_unique<ConnectedState>());
//     return ctx->currentState->handle(*ctx);
// }

DeviceStatus AdbServer::getDeviceStatus(const std::string &deviceId) const
{
    auto context=getDeviceContext(deviceId);
    return context==nullptr?DeviceStatus::Disconnected:context->status;
}

bool AdbServer::isDeviceConnected(const std::string &deviceId) const
{
    auto context=getDeviceContext(deviceId);
    if(context->status==DeviceStatus::ExecutingState){
        return true;
    }
    return false;
}

AdbServer::AdbServer()
{
    fac=std::make_unique<DeviceServerFactory>();
    this->start();
    ThreadPool::getInstance().submit([this](){
        this->acceptSocket();  //监听
    });
}

// AdbServer::AdbServer()
// {

// }

bool AdbServer::setState(const std::string &deviceId, std::unique_ptr<IAdbState> newState)
{
    DeviceContext& context=deviceContextMap[deviceId];
    context.currentState=std::move(newState);
    return true;
}

void AdbServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ADB_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 的16进制

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        perror("listen failed");
        return;
    }

    qDebug()<< "ADB server listening on port " << ADB_PORT;

    // while (true) {
    //     int clientSocket = accept(serverSocket, nullptr, nullptr);
    //     if (clientSocket >= 0) {
    //         std::thread(&AdbServer::handleClient, this, clientSocket).detach();
    //     }
    // }
}


void AdbServer::acceptSocket(){
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket >= 0) {
            qDebug() << "New connection from" << clientSocket;
            std::thread(&AdbServer::handleClient, this, clientSocket).detach();
        }
    }
}


void AdbServer::handleClient(int clientSocket) {
    std::string jsonStr;
    while (readMessage1(clientSocket, jsonStr)) {
        qDebug()<< "收到命令: " << jsonStr;
        auto parsed = json::parse(jsonStr);
         std::string cmd = parsed["cmd"];
        std::string src = parsed["params"]["src"];
        std::string dst = parsed["params"]["dst"];
        QString qsrc = QString::fromStdString(src);
        qDebug() << "src:" << qsrc;

        // // 如果你不确定是否存在，可以加检查
        // if (parsed.contains("params") && parsed["params"].contains("src")) {
        //     std::string src = parsed["params"]["src"];  // 注意这一步
        //     QString qsrc = QString::fromStdString(src);
        //     qDebug() << "src:" << qsrc;
        // } else {
        //     qDebug() << "字段不存在";
        // }

        if(cmd=="push"){

        }


        //processCommand(clientSocket,cmd);
        // const std::vector<uint8_t> rawData(cmd.begin(),cmd.end());
        //WifiServer::instance().handleHostCommand(cmd,clientSocket);
    }
    closesocket(clientSocket);
}




// void parseJsonCommandQt(const QByteArray& jsonData) {
//     QJsonParseError parseError;
//     QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
//     if (parseError.error != QJsonParseError::NoError) {
//         qWarning() << "JSON parse error:" << parseError.errorString();
//         return;
//     }

//     if (!doc.isObject()) {
//         qWarning() << "JSON is not an object";
//         return;
//     }

//     QJsonObject obj = doc.object();

//     QString cmd = obj.value("cmd").toString();
//     qDebug() << "cmd:" << cmd;

//     QString target = obj.value("target").toString();
//     qDebug() << "target:" << target;

//     QJsonObject params = obj.value("params").toObject();
//     QString src = params.value("src").toString();
//     QString dst = params.value("dst").toString();

//     qDebug() << "src:" << src;
//     qDebug() << "dst:" << dst;
// }



bool AdbServer::readMessage1(int clientSocket, std::string& outMsg) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        outMsg.assign(buffer, bytesRead);
        qDebug()<<"接收成功";
        return true;
    } else if (bytesRead == 0) {
        // 连接关闭
        qDebug()<<"连接关闭";
        return false;
    } else {
        // 错误处理
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK || err == WSAEINTR) {
            // 非阻塞无数据或者被信号中断，可以选择重试
            qDebug()<<"中断";
            return true;
        }
        perror("recv failed");
        qDebug()<<"接收失败";
        return false;
    }
}



bool AdbServer::readMessage(int clientSocket, std::string& outCmd) {
    char lenBuf[5] = {0};
    int ret = recv(clientSocket, lenBuf, 4, MSG_WAITALL);
    if (ret != 4) return false;

    int len = std::stoi(std::string(lenBuf, 4), nullptr, 16);  // 16进制
    if (len <= 0 || len > BUFFER_SIZE) return false;

    std::vector<char> buf(len);
    ret = recv(clientSocket, buf.data(), len, MSG_WAITALL);
    if (ret != len) return false;

    outCmd.assign(buf.begin(), buf.end());
    return true;
}

void AdbServer::processCommand(int clientSocket, const std::string& cmd) {
    if (cmd == "host:version") {
        sendOkay(clientSocket);
        sendPayload(clientSocket, "0030");  // 表示 ADB 版本 0x0030
    } else if (cmd.find("host:transport:", 0) == 0) {
        std::string serial = cmd.substr(strlen("host:transport:"));
        // 模拟设备连接（这里只是示例逻辑）
        {
            // std::lock_guard<std::mutex> lock(mapMutex);
            // deviceTransportMap[serial] = clientSocket;
        }
        sendOkay(clientSocket);
    } else if (cmd.rfind("shell:", 0) == 0) {
        std::string shellCmd = cmd.substr(6);
        std::string output = "模拟返回: " + shellCmd + "\r\n$ ";
        sendOkay(clientSocket);
        sendPayload(clientSocket, output);
    } else if (cmd == "sync:") {
        sendOkay(clientSocket);
        // 你可以开始收推送协议包
    } else {
        sendFail(clientSocket, "unknown command");
    }
}

void AdbServer::sendOkay(int clientSocket) {
    send(clientSocket, "OKAY", 4, 0);
}

void AdbServer::sendFail(int clientSocket, const std::string& reason) {
    std::stringstream ss;
    ss << std::hex << reason.length();
    std::string lenHex = ss.str();
    lenHex.insert(lenHex.begin(), 4 - lenHex.length(), '0');

    send(clientSocket, "FAIL", 4, 0);
    send(clientSocket, lenHex.c_str(), 4, 0);
    send(clientSocket, reason.c_str(), reason.length(), 0);
}

void AdbServer::sendPayload(int clientSocket, const std::string& data) {
    std::stringstream ss;
    ss << std::hex << data.length();
    std::string lenHex = ss.str();
    lenHex.insert(lenHex.begin(), 4 - lenHex.length(), '0');

    send(clientSocket, lenHex.c_str(), 4, 0);
    send(clientSocket, data.c_str(), data.length(), 0);
}




















/*

 *改进扩展
 * #ifndef ADBSERVER_H
#define ADBSERVER_H

#include <boost/asio.hpp>
#include <string>
#include <memory>

class AdbServer {
public:
    explicit AdbServer(unsigned short port);
    void run();

private:
    void start_accept();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void read_handler(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                      std::shared_ptr<std::vector<char>> buffer,
                      const boost::system::error_code& ec, std::size_t bytes_transferred);

    void processCommand(int clientSocket, const std::string& cmd); // 保持原有接口
    bool readMessage(int clientSocket, std::string& cmd);          // 保持原有接口

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // ADBSERVER_H
#include "AdbServer.h"
#include <iostream>
#include <QDebug>

AdbServer::AdbServer(unsigned short port)
    : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    std::cout << "ADB server started on port " << port << std::endl;
}

void AdbServer::run() {
    start_accept();
    io_context_.run();  // 所有异步操作都在这个线程中运行
}

void AdbServer::start_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);

    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& ec) {
        if (!ec) {
            qDebug() << "New client connected";
            handle_client(socket);
        } else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
        }
        start_accept();  // 继续接受下一个连接
    });
}


void AdbServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024);

    socket->async_read_some(boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            read_handler(socket, buffer, ec, bytes_transferred);
        });
}

void AdbServer::read_handler(
    std::shared_ptr<boost::asio::ip::tcp::socket> socket,
    std::shared_ptr<std::vector<char>> buffer,
    const boost::system::error_code& ec,
    std::size_t bytes_transferred)
{
    if (ec) {
        std::cerr << "Connection closed: " << ec.message() << std::endl;
        return;
    }

    std::string cmd(buffer->data(), bytes_transferred);
    qDebug() << "收到命令：" << QString::fromStdString(cmd);

    // 注意：这里的 socket 没有 int fd，需要适配一下
    int dummy_fd = socket->native_handle();  // 在 Linux 上可用，在 Windows 上可能不同，需判断
    processCommand(dummy_fd, cmd);

    // 继续读取
    handle_client(socket);
}

#include "AdbServer.h"

int main() {
    try {
        AdbServer server(5037); // ADB 默认端口
        server.run();           // 启动事件循环
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

*/
