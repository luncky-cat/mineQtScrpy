#include "DeviceSession.h"

#include<asio/write.hpp>

#include <QEventLoop>
#include <QTimer>
#include <qDebug>

#include "protocol/AdbProtocol.h"


DeviceSession::DeviceSession(std::shared_ptr<asio::ip::tcp::socket> socket)
    : socket_(std::move(socket)) {}

void DeviceSession::start() {
    qDebug()<<"开始会话";
    doRead();
}

void DeviceSession::doRead() {
    auto self(shared_from_this()); // 保证异步回调期间对象不会被释放
    socket_->async_read_some(asio::buffer(tempBuf_),
                             [this, self](std::error_code ec, std::size_t length) {
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
    std::lock_guard<std::mutex> lock(waiterMutex_);
    qDebug()<<"接受完整数据开始处理分发";
    dispatchToContext(msg); // 将消息交由上层处理（按 stream/ctx 分发）
    //唤醒等待
    //checkPendingWaiters();

}


bool DeviceSession::tryGetStreamMessage(uint32_t streamId, AdbMessage& outMsg) {  //遍历取专属流包
    auto it = streamContexts.find(streamId);
    if (it == streamContexts.end()) {
        return false;
    }
    auto msg = it->second->tryDequeueMessage();
    if (!msg.has_value()){
        return false;
    }
    outMsg = std::move(msg.value());
    return true;
}



QString DeviceSession::adbCommandToString(uint32_t cmd) {
    char buf[5] = {
        static_cast<char>(cmd & 0xFF),
        static_cast<char>((cmd >> 8) & 0xFF),
        static_cast<char>((cmd >> 16) & 0xFF),
        static_cast<char>((cmd >> 24) & 0xFF),
        '\0'
    };
    return QString::fromLatin1(buf);
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


bool DeviceSession::tryFindSessionCommand(const std::vector<uint32_t>& expectedCmds,AdbMessage &outMsg) { //遍历取无归属流包
    qDebug() << "查找匹配命令包";
    for (auto it = pendingMessages.begin(); it != pendingMessages.end(); ++it) {
        if(matchCommand(expectedCmds,*it)){
            outMsg = std::move(*it);
            pendingMessages.erase(it); // 原地删除，避免额外拷贝
            qDebug()<<"pendingMessages的大小"<<pendingMessages.size();
            qDebug() << "找到匹配的命令包";
            return true;
        }
    }

    qDebug() << "未找到匹配的命令包";
    return false;
}

bool DeviceSession::waitConnectCommand(const std::vector<uint32_t>& expectedCmds,int timeoutMs,std::function<void(const AdbMessage&)> cb){   //等待连接命令
    AdbMessage msg;
    if (tryFindSessionCommand(expectedCmds,msg)) {
        cb(msg);
        qDebug()<<"开始就找到了包，等待者"<<sessionWaits.size();
        return true;
    }
    sessionWaits.push_back({expectedCmds,std::chrono::steady_clock::now() + std::chrono::seconds(timeoutMs),cb});
    qDebug()<<"当前等待"<<sessionWaits.size();
    return false;
}


void DeviceSession::checkSessionWaiters() {
    qDebug()<<"唤醒";
    for (auto it = sessionWaits.begin(); it != sessionWaits.end(); ) {
        if (it->expireTime < std::chrono::steady_clock::now()) {    //超时移除
            it = sessionWaits.erase(it);
            continue;
        }

        AdbMessage msg;
        if (tryFindSessionCommand(it->expectedCmds, msg)) {
            auto cb = std::move(it->callback);  // callback 是 std::function<void(AdbMessage)>
            it = sessionWaits.erase(it);        // 先删除，避免迭代器失效
            cb(msg);
        } else {
            ++it;
        }
    }
}










// bool DeviceSession::waitStreamCommand(const std::vector<uint32_t>& expectedCmds, uint32_t streamId,int timeoutMs,std::function<void(const AdbMessage&)> cb){   //等待连接命令
//     AdbMessage msg;
//     if (tryGetStreamMessage(streamId,msg) && matchCommand(expectedCmds, msg)) {
//         cb(msg);
//         return true;
//     }
//    // streamWaits[streamId].emplace_back({expectedCmds,streamId,timeoutMs,cb});
//     return false;
// }



DeviceSession::~DeviceSession()
{

}



void DeviceSession::dispatchToContext(const AdbMessage& msg) {    //分发
    qDebug()<<"分发前打印msg的命令"<<msg.command;
    if (msg.arg1 == 0) {
        qDebug()<<"插入无归属流队列";
        pendingMessages.emplace_back(msg);
        qDebug()<<"pendingMessages size"<<pendingMessages.size();
        checkSessionWaiters();
    }else{
        // 流消息
        qDebug()<<"插入归属流队列";
        auto it = streamContexts.find(msg.arg1);
        if (it != streamContexts.end()) {
            it->second->enqueueMessage(msg);
        } else {
            // 可选：记录一个警告日志，表示非法/未知 stream 消息
            pendingMessages.emplace_back(msg);  // fallback 处理
        }
    }
    //checkPendingWaiters();
}




bool DeviceSession::isServiceOpened(const std::string& name) const {
    return openedServices.find(name) != openedServices.end();
}

void DeviceSession::send(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(sendMutex_);
    bool writing = !sendQueue_.empty(); // 如果队列为空，说明没有正在发送
    sendQueue_.push_back(data);
    if (!writing) {
        doWrite(); // 启动异步写
    }
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
