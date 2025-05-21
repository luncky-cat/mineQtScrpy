#ifndef SOCKETTRANSPORT_H
#define SOCKETTRANSPORT_H

#include"interfaces/ITransPort.h"
#include "protocol/AdbMessage.h"

#include <set>
#include <vector>

class SocketTransPort : public ITransPort {
public:
    explicit SocketTransPort();
    void setSocket(int sock) override;
    bool sendMsg(const std::vector<uint8_t>& data) override;
    bool recvMsg(AdbMessage& outMsg) override;
    ~SocketTransPort()=default;
   // bool waitForCommand(uint32_t expectCmd, AdbMessage &outMsg) override;
    bool waitForRecv(AdbMessage &outMsg, int maxAttempts=50, int intervalMs=100) override;
    std::vector<uint8_t> recvBuffer_;
    bool waitForCommands(const std::set<uint32_t> &expectedCmds, AdbMessage &inputMsg) override;
private:
    int socket_;

    //AdbMessage AdbMessage_;  // 作为发送/接收缓冲区

};

#endif // SOCKETTRANSPORT_H
