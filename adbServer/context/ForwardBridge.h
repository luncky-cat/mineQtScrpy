#ifndef FORWARDBRIDGE_H
#define FORWARDBRIDGE_H

#include<asio/ip/tcp.hpp>
#include<asio/write.hpp>

class ForwardBridge {
public:
    ForwardBridge(asio::ip::tcp::socket socket, std::function<void(const std::vector<uint8_t>&)> onRecv);
    void sendToClient(const std::vector<uint8_t>& data);

private:
    void doRead();

private:
    asio::ip::tcp::socket clientSocket_;
    std::array<char, 1024> buffer_{};
    std::function<void(const std::vector<uint8_t>&)> onReceive_;
};

#endif // FORWARDBRIDGE_H
