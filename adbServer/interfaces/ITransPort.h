#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <vector>

struct AdbMessage;

class ITransPort {
public:
    virtual bool sendMsg(const std::vector<uint8_t>& data) = 0;
    virtual bool recvMsg(AdbMessage& outMsg) = 0;
    virtual bool waitForRecv(AdbMessage &outMsg, int maxAttempts=50, int intervalMs=100)=0;
    virtual bool waitForCommand(uint32_t expectCmd, AdbMessage &outMsg)=0;
    virtual void setSocket(int s)=0;
    virtual ~ITransPort() = default;
};


#endif // transPort_H
