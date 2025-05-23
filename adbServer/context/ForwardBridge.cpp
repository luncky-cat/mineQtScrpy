#include "ForwardBridge.h"


ForwardBridge::ForwardBridge(asio::ip::tcp::socket socket, std::function<void(const std::vector<uint8_t>&)> onRecv)
    : clientSocket_(std::move(socket)), onReceive_(onRecv) {
    doRead();
}

void ForwardBridge::sendToClient(const std::vector<uint8_t>& data) {
    asio::async_write(clientSocket_, asio::buffer(data), [](std::error_code, std::size_t) {});
}

void ForwardBridge::doRead() {
    clientSocket_.async_read_some(asio::buffer(buffer_),
                                  [this](std::error_code ec, std::size_t len) {
                                      if (!ec) {
                                          std::vector<uint8_t> data(buffer_.data(), buffer_.data() + len);
                                          onReceive_(data);
                                          doRead();
                                      }
                                  });
}
