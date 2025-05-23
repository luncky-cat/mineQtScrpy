#ifndef DEVICESESSION_H
#define DEVICESESSION_H

#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <set>

#include<asio/ip/tcp.hpp>

#include "AdbStreamContext.h"
#include "ForwardBridge.h"

struct AdbMessage;

class DeviceSession : public std::enable_shared_from_this<DeviceSession> {
public:
    DeviceSession(asio::ip::tcp::socket socket);
    void start(); // 启动读循环
    void send(const std::vector<uint8_t>& data); // 发送数据
private:
    void doRead(); // 异步读取
    void doWrite(); // 异步写入
    void handleMessage(const AdbMessage& msg); // 处理解析后的 ADB 消息
    void dispatchToContext(const AdbMessage& msg); // 分发消息到上下文

    // bool isServiceOpened(const std::string& name) const {
    //     return openedServices.find(name) != openedServices.end();
    // }
    void setupForwardBridge(uint32_t localId, const std::string& localPort);

    asio::ip::tcp::socket socket_; // 与设备通信的 socket
    std::vector<uint8_t> recvBuffer_; // 接收缓冲，支持粘包处理
    std::deque<std::vector<uint8_t>> sendQueue_; // 发送队列
    std::mutex sendMutex_; // 发送队列线程锁
    std::array<char, 512> tempBuf_; // 临时缓冲区用于读 socket
    std::map<uint32_t, std::shared_ptr<ForwardBridge>> streamToClient_; // localId 到客户端socket映射
    std::map<uint32_t,std::shared_ptr<AdbStreamContext>>streamContexts; //流对象
    std::set<std::string> openedServices;   //维护流服务的类型
};

#endif // DEVICESESSION_H
