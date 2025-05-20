#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <vector>

struct AdbMessage;

class ITransPort {
public:
    virtual bool sendMsg(const std::vector<uint8_t>& data) = 0;
    virtual bool recvMsg(AdbMessage& outMsg) = 0;
    bool waitForRecv(AdbMessage &outMsg, int maxAttempts=50, int intervalMs=100);
    bool waitForCommand(uint32_t expectCmd, AdbMessage &inputMsg);
    virtual void setSocket(int s)=0;
    virtual ~ITransPort() = default;
};


#endif // transPort_H
