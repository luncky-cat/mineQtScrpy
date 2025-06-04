#ifndef DEVICESESSION_H
#define DEVICESESSION_H

#include <QObject>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include<qdebug.h>

#include<asio/ip/tcp.hpp>

#include <asio/awaitable.hpp>

#include "ForwardBridge.h"
#include "protocol/AdbMessage.h"

class AdbStreamContext;
struct AdbMessage;
class WaitCommandAwaitable;

struct commandWaiter {
    std::vector<uint32_t> expectedCmds;
    std::chrono::steady_clock::time_point expireTime;
    std::function<void(const AdbMessage&)> notify;
};

class DeviceSession :public std::enable_shared_from_this<DeviceSession> {
public:
    DeviceSession(std::shared_ptr<asio::ip::tcp::socket> socket);
    ~DeviceSession();
public:
    //工具，启动
    void start(); // 启动读循环
    void send(const std::vector<uint8_t>& data); // 发送数据
    QString adbCommandToString(uint32_t cmd);
    bool matchCommand(const std::vector<uint32_t> &cmdsexpectedCmds, AdbMessage &inputMsg);

    //服务
    bool isServiceOpened(const std::string &name) const;
    void markServiceOpened(const std::string &name);

    //流管理
    std::shared_ptr<AdbStreamContext> createStreamContext(uint32_t localId);
    void removeStreamContext(uint32_t localId);
    void bindRemoteStreamId(uint32_t localId, uint32_t remoteId);

    bool findCommandByDeque(const std::vector<uint32_t> &expectedCmds, std::deque<AdbMessage> &messages, AdbMessage &outMsg); //队列里找命令
    std::deque<AdbMessage> &findMessageDeque(uint32_t streamId=0);   //默认无归属
    void addWaiter(uint32_t streamId, commandWaiter& waiter);

private:
    void doRead(); // 异步读取
    void doWrite(); // 异步写入
    void handleMessage(const AdbMessage& msg); // 处理解析后的 ADB 消息
    void dispatchToContext(const AdbMessage& msg); // 分发消息到上下文
    void setupForwardBridge(uint32_t localId, const std::string& localPort);

public:
    //接收数据
    std::unordered_map<uint32_t, std::list<commandWaiter>> streamWaiters;
    std::deque<AdbMessage> pendingMessages; //未归属队列
    //WaitCommandAwaitable coWaitCommand(std::vector<uint32_t> expectedCmds, int timeoutMs, uint32_t streamId);

    asio::awaitable<AdbMessage> coWaitCommand(std::vector<uint32_t> expectedCmds, int timeoutMs, uint32_t streamId);
    asio::any_io_executor getExecutor() const;
private:
    std::shared_ptr<asio::ip::tcp::socket> socket_; // 与设备通信的 socket
    std::vector<uint8_t> recvBuffer_; // 接收缓冲，支持粘包处理
    std::array<char, 512> tempBuf_; // 临时缓冲区用于读 socket
    std::mutex sendMutex_; // 发送队列线程锁
    std::deque<std::vector<uint8_t>> sendQueue_; // 发送队列
    std::map<uint32_t, std::shared_ptr<ForwardBridge>> streamToClient_; // localId 到客户端socket映射
    std::map<uint32_t,std::shared_ptr<AdbStreamContext>>streamContexts; //流对象
    std::set<std::string> openedServices;   //维护流服务的类型

    template<typename T>   //唤醒等待者队列，需要查找的队列
    void checkWaiters(std::list<T> &Waiters,std::deque<AdbMessage>&deque);
    std::list<commandWaiter> &findWaiters(uint32_t streamId);

};

//唤醒分两种：事件驱动（有数据时）+ 定时驱动（超时） 后期添加
template<typename T>
void DeviceSession::checkWaiters(std::list<T>& Waiters,std::deque<AdbMessage>&deque) {   //单个归属的唤醒者队列
    qDebug()<<"[checkWaiters]唤醒等待者,当前等待者数量"<<Waiters.size();
    for (auto it = Waiters.begin(); it != Waiters.end(); ) {
        if (it->expireTime < std::chrono::steady_clock::now()) {    //超时移除
            it = Waiters.erase(it);
            continue;
        }

        AdbMessage msg;
        if (findCommandByDeque(it->expectedCmds,deque,msg)) {   //在合适的队列寻找
            auto cb = std::move(it->notify);  // callback 是 std::function<void(AdbMessage)>
            it = Waiters.erase(it);        // 先删除，避免迭代器失效
            cb(msg);
        } else {
            ++it;
        }
    }
    qDebug()<<"[checkWaiters]队列现有的包数量:"<<deque.size();
}



#endif // DEVICESESSION_H
