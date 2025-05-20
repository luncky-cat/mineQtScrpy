#ifndef SOCKETTRANSPORT_H
#define SOCKETTRANSPORT_H

#include"interfaces/ITransPort.h"

#include <vector>

class SocketTransPort : public ITransPort {
public:
    explicit SocketTransPort();
    void setSocket(int sock) override;
    bool sendMsg(const std::vector<uint8_t>& data) override;
    bool recvMsg(AdbMessage& outMsg) override;
    ~SocketTransPort()=default;
private:
    int socket_;
};

#endif // SOCKETTRANSPORT_H
