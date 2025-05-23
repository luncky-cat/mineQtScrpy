#ifndef REMOTESESSION_H
#define REMOTESESSION_H

#include<asio/ip/tcp.hpp>
#include <memory>
#include <functional>
#include <array>


class RemoteCommandSession : public std::enable_shared_from_this<RemoteCommandSession> {
public:
    using CommandCallback = std::function<void(const std::string&)>;
    RemoteCommandSession(asio::ip::tcp::socket socket, std::function<void (const std::string &, CommandCallback)> onCommandAsync);
      void start();
private:
    void doRead();  // 异步读取客户端命令
    void doWrite(const std::string& response);  // 异步写入响应

    asio::ip::tcp::socket socket_;
    std::array<char, 512> buffer_{};
    std::function<void(const std::string&, CommandCallback)> onCommandAsync_;
};
#endif // REMOTESESSION_H
