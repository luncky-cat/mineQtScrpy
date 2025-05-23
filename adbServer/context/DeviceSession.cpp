#include "DeviceSession.h"

#include<asio/write.hpp>

#include "protocol/AdbProtocol.h"

#include <qDebug>

DeviceSession::DeviceSession(asio::ip::tcp::socket socket)
    : socket_(std::move(socket)) {}

void DeviceSession::start() {
    doRead(); // 启动读取数据
}

void DeviceSession::doRead() {
    auto self(shared_from_this()); // 保证异步回调期间对象不会被释放
    socket_.async_read_some(asio::buffer(tempBuf_),
                            [this, self](std::error_code ec, std::size_t length) {
                                if (!ec) {
                                    // 将读取数据追加到缓冲区
                                    recvBuffer_.insert(recvBuffer_.end(), tempBuf_.data(), tempBuf_.data() + length);
                                    size_t msgLen = 0;
                                    while (true) {
                                        // 尝试从缓冲中解析出完整的 ADB 消息
                                        auto result = AdbProtocol::parseAdbMessage(recvBuffer_, msgLen);
                                        if (!result.has_value()) break; // 数据不足，继续接收
                                        handleMessage(result.value());
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
    dispatchToContext(msg); // 将消息交由上层处理（按 stream/ctx 分发）
}

void DeviceSession::dispatchToContext(const AdbMessage& msg) {
    // std::cout << "Received message cmd: " << std::hex << msg.command << std::endl;

    // if (msg.command == A_CNXN) {
    //     // 设备连接建立时打印
    //     std::cout << "设备连接成功" << std::endl;
    // } else if (msg.command == A_WRTE) {
    //     auto it = streamToClient_.find(msg.arg0);
    //     if (it != streamToClient_.end()) {
    //         it->second->sendToClient(msg.payload);
    //     }
    // } else if (msg.command == A_OPEN) {
    //     std::string destination(reinterpret_cast<const char*>(msg.payload.data()), msg.payload.size());
    //     std::cout << "OPEN 请求: " << destination << std::endl;
    //     if (destination.find("scrcpy") != std::string::npos) {
    //         // 模拟 forward：监听本地端口，等待 scrcpy 客户端连接
    //         setupForwardBridge(msg.arg0, "27183");
    //     }
    // }

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
    asio::async_write(socket_, asio::buffer(sendQueue_.front()),
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

