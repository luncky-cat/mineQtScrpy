#include "RemoteSession.h"

#include<asio/write.hpp>

RemoteCommandSession::RemoteCommandSession(asio::ip::tcp::socket socket,
                                           std::function<void(const std::string&, CommandCallback)> onCommandAsync)
    : socket_(std::move(socket)), onCommandAsync_(onCommandAsync) {}

void RemoteCommandSession::start() {
    doRead();
}

void RemoteCommandSession::doRead() {
    auto self = shared_from_this();
    socket_.async_read_some(asio::buffer(buffer_),
                            [this, self](std::error_code ec, std::size_t len) {
                                if (!ec) {
                                    std::string cmd(buffer_.data(), len);
                                    onCommandAsync_(cmd, [this, self](const std::string& result) {
                                        doWrite(result);
                                    });
                                }
                            });
}

void RemoteCommandSession::doWrite(const std::string& response) {
    auto self = shared_from_this();
    asio::async_write(socket_, asio::buffer(response),
                      [this, self](std::error_code ec, std::size_t /*len*/) {
                          if (ec) {
                              //std::cerr << "Write error: " << ec.message() << std::endl;
                              socket_.close(); // 出错时关闭连接
                          }else{
                              doRead();
                          }
                      }
                      );
}
