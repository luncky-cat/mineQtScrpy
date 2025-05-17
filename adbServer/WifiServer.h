#ifndef WIFISERVER_H
#define WIFISERVER_H

class executeCmd;

#include "DeviceServer.h"
#include "AdbMessage.h"

#include <vector>


#include <WinSock2.h>
#include <string>

class WifiServer : public DeviceServer   //通用的wifi服务接口
{
public:
    static WifiServer& instance();
    bool connect(DeviceContext& ctx);
    bool auth(DeviceContext& ctx);
    bool execute(DeviceContext& ctx);
    bool close(DeviceContext& ctx);
    bool executeShell();

    bool sendMsg(std::vector<uint8_t> &sendMsg, DeviceContext &ctx);
    bool recvMsg(DeviceContext &ctx);
    std::string extractShellResult(const std::vector<std::string> &payloads, const std::string &cmdEcho);
    int recvExact(int socket, void *buffer, int length);
    bool waitForRecv(DeviceContext &ctx, int maxAttempts = 50, int intervalMs = 100);
    bool waitForCommand(DeviceContext &ctx, uint32_t expectCmd);
    bool executeShell(std::string &cmd);
    bool executeShell(std::string &cmd, DeviceContext &ctx);
private:
    bool initNetwork();
    bool openShellChannel(DeviceContext &ctx);
    bool openSyncChannel(DeviceContext &ctx);
private:
    WifiServer();
    ~WifiServer()=default;

    bool networkInitialized;   //网络服务初始状态
    AdbMessage msg;
    std::vector<uint8_t> recvData;  // 初步开个缓冲区
    size_t bufferSize = 4096;
    int local_id;   //负责分配local_id
    executeCmd *cmd;
    std::vector<uint8_t> recvBuffer;


};

#endif // WIFISERVER_H
