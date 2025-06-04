#include "DeviceSession.h"

#include <QEventLoop>
#include <QTimer>
#include <qDebug>

#include <asio/awaitable.hpp>
#include<asio/write.hpp>

#include "asio/use_awaitable.hpp"
#include "context/wait_command_async.h"
#include "protocol/AdbProtocol.h"
#include"AdbStreamContext.h"
#include "WaitCommandAwaitable.h"

DeviceSession::DeviceSession(std::shared_ptr<asio::ip::tcp::socket> socket)
    : socket_(std::move(socket)) {}


DeviceSession::~DeviceSession(){}


void DeviceSession::start() {
    qDebug()<<"开始会话";
    doRead();
}


void DeviceSession::send(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(sendMutex_);
    bool writing = !sendQueue_.empty(); // 如果队列为空，说明没有正在发送
    sendQueue_.push_back(data);
    if (!writing) {
        doWrite(); // 启动异步写
    }
}


void DeviceSession::doRead() {
    auto self(shared_from_this()); // 保证异步回调期间对象不会被释放
    socket_->async_read_some(asio::buffer(tempBuf_),
                             [this, self](std::error_code ec, std::size_t length) {
                                 qDebug() << "[doRead]async_read_some 回调触发了，长度：" << length << " 错误：" << ec.message();
                                 if (!ec) {
                                     // 将读取数据追加到缓冲区
                                     recvBuffer_.insert(recvBuffer_.end(), tempBuf_.data(), tempBuf_.data() + length);
                                     size_t msgLen = 0;
                                     while (true) {
                                         // 尝试从缓冲中解析出完整的 ADB 消息
                                         auto result = AdbProtocol::parseAdbMessage(recvBuffer_, msgLen);
                                         if (!result.has_value()) break; // 数据不足，继续接收
                                         handleMessage(result.value());  //分发
                                         // 移除已解析的字节
                                         recvBuffer_.erase(recvBuffer_.begin(), recvBuffer_.begin() + msgLen);
                                     }
                                     doRead(); // 继续读取
                                 } else {
                                     //std::cerr << "Read error: " << ec.message() << std::endl;
                                     qDebug() << "Read error: " << ec.message();
                                 }
                             });
}


void DeviceSession::handleMessage(const AdbMessage& msg) {    //分发
    qDebug()<<"[handleMessage]接受完整数据开始处理分发";
    dispatchToContext(msg); // 将消息交由上层处理（按 stream/ctx 分发）
}


QString DeviceSession::adbCommandToString(uint32_t cmd) {
    char chars[5];  // 4字符 + \0
    chars[0] = static_cast<char>((cmd) & 0xFF);
    chars[1] = static_cast<char>((cmd >> 8) & 0xFF);
    chars[2] = static_cast<char>((cmd >> 16) & 0xFF);
    chars[3] = static_cast<char>((cmd >> 24) & 0xFF);
    chars[4] = '\0';
    return QString::fromLatin1(chars);
}

bool DeviceSession::matchCommand(const std::vector<uint32_t>& cmdsexpectedCmds, AdbMessage& inputMsg) {
    qDebug() << "[matchCommand] 当前消息命令:" << inputMsg.command
             << " (" << adbCommandToString(inputMsg.command) << ")";

    for (auto c : cmdsexpectedCmds) {
        qDebug() << "[matchCommand] 期待命令:" << c << " (" << adbCommandToString(c) << ")";
        if (c == inputMsg.command) {
            qDebug() << "[matchCommand] ✅ 找到了匹配命令:" << c;
            return true;
        }
    }
    qDebug() << "[matchCommand] ❌ 未匹配任何命令";
    return false;
}

std::deque<AdbMessage>& DeviceSession::findMessageDeque(uint32_t streamId){   //找寻消息队列
    qDebug()<<"[findMessageDeque]找寻消息队列";
    auto it = streamContexts.find(streamId);
    if (it == streamContexts.end()) {
         qDebug()<<"[findMessageDeque]默认消息队列";
        return pendingMessages;
    }
     qDebug()<<"[findMessageDeque]特定消息队列";
    return it->second->getStreamDeque();
}

std::list<commandWaiter>& DeviceSession::findWaiters(uint32_t streamId){   //找寻等待者
    auto it = streamWaiters.find(streamId);
    if (it == streamWaiters.end()) {
        throw std::runtime_error("Stream ID not found");
    }
    return it->second;
}

bool DeviceSession::findCommandByDeque(const std::vector<uint32_t>& expectedCmds,std::deque<AdbMessage>&messages,AdbMessage &outMsg) { //队列中找包
    qDebug() << "[findCommandByDeque]查找匹配命令包";
    for (auto it = messages.begin(); it != messages.end();) {
        if(matchCommand(expectedCmds, *it)) {
            outMsg = std::move(*it);
            it = messages.erase(it);  // erase返回下一个有效迭代器
            qDebug() << "[findCommandByDeque]找到匹配的命令包";
            return true;
        } else {
            ++it;
        }
    }
    qDebug() << "[findCommandByDeque]未找到匹配的命令包";
    return false;
}

asio::any_io_executor DeviceSession::getExecutor() const {
    return socket_->get_executor();
}

asio::awaitable<AdbMessage> DeviceSession::coWaitCommand(
    std::vector<uint32_t> expectedCmds, int timeoutMs, uint32_t streamId
    ) {
    co_return co_await async_wait_command(shared_from_this(), std::move(expectedCmds), timeoutMs, streamId, asio::use_awaitable);
}


void DeviceSession::addWaiter(uint32_t streamId,commandWaiter& waiter){
    qDebug()<<"[addWaiter]添加等待者:流id"<<streamId;
    streamWaiters[streamId].emplace_back(std::move(waiter));
}

std::shared_ptr<AdbStreamContext> DeviceSession::createStreamContext(uint32_t localId) {   //创建流的接口
    qDebug()<<"创建流对象:"<<localId;
    auto ctx = std::make_shared<AdbStreamContext>(localId);
    streamContexts[localId] = std::move(ctx);
    return ctx;
}

void DeviceSession::removeStreamContext(uint32_t localId){   //销毁流
    auto it=streamContexts.find(localId);
    if(it!=streamContexts.end()){
        streamContexts.erase(it);
    }
}

void DeviceSession::bindRemoteStreamId(uint32_t localId, uint32_t remoteId) {
    auto it = streamContexts.find(localId);
    if (it != streamContexts.end()) {
        it->second->setRemoteId(remoteId);
    } else {
        qWarning() << "绑定远程 ID 失败，localId 未找到:" << localId;
    }
}


void DeviceSession::dispatchToContext(const AdbMessage& msg) {
    qDebug()<<"[dispatchToContext]分发前打印msg的命令"<<adbCommandToString(msg.command);

    if (msg.command == AdbProtocol::CMD_CNXN) {
        qDebug()<<"[dispatchToContext]CNXN消息，插入无归属流队列";
        pendingMessages.emplace_back(msg);
        qDebug()<<"[dispatchToContext]pendingMessages size"<<pendingMessages.size();
        checkWaiters(streamWaiters[0],pendingMessages);  //默认流等待者
    } else if (msg.arg1 == 0) {
        qDebug()<<"[dispatchToContext]默认流消息，插入无归属流队列";
        pendingMessages.emplace_back(msg);
        qDebug()<<"[dispatchToContext]pendingMessages size"<<pendingMessages.size();
        checkWaiters(streamWaiters[0],pendingMessages);
    } else {
        qDebug()<<"[dispatchToContext]插入归属流队列";
        auto it = streamContexts.find(msg.arg1);
        qDebug()<<"[dispatchToContext]流队列配对:"<<"local_id"<<msg.arg1<<msg.arg0;
        if (it != streamContexts.end()) {
            qDebug()<<"[dispatchToContext]插入成功";
            it->second->enqueueMessage(msg);
            auto & waiters=findWaiters(msg.arg1);
            auto &deque=findMessageDeque(msg.arg1);
            checkWaiters(waiters,deque);
        } else {
            qDebug()<<"[dispatchToContext]未知流，插入无归属";
            pendingMessages.emplace_back(msg);
        }
    }
}




void DeviceSession::markServiceOpened(const std::string& name) {
    openedServices.insert(name);
}

bool DeviceSession::isServiceOpened(const std::string& name) const {
    return openedServices.find(name) != openedServices.end();
}


void DeviceSession::doWrite() {
    auto self(shared_from_this()); // 保持会话对象在异步过程中有效
    asio::async_write(*socket_, asio::buffer(sendQueue_.front()),
                      [this, self](std::error_code ec, std::size_t /*length*/) {
                          if (!ec) {
                              std::lock_guard<std::mutex> lock(sendMutex_);
                              sendQueue_.pop_front();
                              if (!sendQueue_.empty()) {
                                  doWrite(); // 继续发送下一条
                              }
                          } else {
                              //std::cerr << "Write error: " << ec.message() << std::endl;
                              qDebug()<< "Write error: " << ec.message();
                          }
                      });
}

// void DeviceSession::setupForwardBridge(uint32_t localId, const std::string& localPort) {
//     asio::ip::tcp::acceptor forwardAcceptor(socket_.get_executor());
//     forwardAcceptor.open(asio::ip::tcp::v4());
//     forwardAcceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
//     forwardAcceptor.bind({asio::ip::tcp::v4(), static_cast<unsigned short>(std::stoi(localPort))});
//     forwardAcceptor.listen();

//     auto self = shared_from_this();
//     auto newSocket = std::make_shared<asio::ip::tcp::socket>(socket_.get_executor());
//     forwardAcceptor.async_accept(*newSocket, [this, self, newSocket, localId](std::error_code ec) {
//         if (!ec) {
//             // std::cout << "scrcpy 客户端连接成功，建立 ForwardBridge" << std::endl;
//             qDebug()<<"scrcpy 客户端连接成功，建立 ForwardBridge";
//             streamToClient_[localId] = std::make_shared<ForwardBridge>(std::move(*newSocket), [this, localId](const std::vector<uint8_t>& data) {
//                 auto msg = AdbProtocol::generateWrite(localId, data);
//                 send(msg);
//             });
//         }
//     });
// }
