
#include "interfaces/ITransPort.h"

#include "protocol/AdbMessage.h"

bool ITransPort::waitForRecv(AdbMessage& outMsg,int maxAttempts, int intervalMs) {
    for (int i = 0; i < maxAttempts; ++i) {
        if (recvMsg(outMsg)) {
            return true;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
}

bool ITransPort::waitForCommand(uint32_t expectCmd,AdbMessage& inputMsg) {
    if (!waitForRecv(inputMsg)) {
        return false;
    }
    return inputMsg.command == expectCmd;
}
