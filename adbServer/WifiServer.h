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
    friend class pushHandler;
public:
    static WifiServer& instance();
    bool connect(DeviceContext& ctx) override;
    bool auth(DeviceContext& ctx) override;
    bool execute(DeviceContext& ctx)override;
    bool close(DeviceContext& ctx)override;

    bool executeShell();
    std::string extractShellResult(const std::vector<std::string> &payloads, const std::string &cmdEcho);
    int recvExact(int socket, void *buffer, int length);
    bool executeShell(std::string &cmd);
    bool executeShell(std::string &cmd, DeviceContext &ctx);
    bool handleScrcpyHostCommand(const std::string &cmd, int clientSocket);
    std::vector<uint8_t> create_adb_packet(const std::string &payload);
    std::string adb_version_response(int version = 31);
    void handleHostCommand(std::string& outCmd, int clientSocket);
    std::string buildAdbBinaryResponse(const std::string &responsePayload);
    std::string buildResponse(const std::string &payload);
    std::string buildAdbStringResponse(const std::string &payloadStr);


protected:

    //接收发送数据相关处理
    bool waitForCommand(SOCKET s, uint32_t expectCmd);
    bool recvMsg(SOCKET sock,AdbMessage &outMsg);
    bool waitForCommand(SOCKET socket_, uint32_t expectCmd, AdbMessage &inputMsg);
    bool waitForRecv(SOCKET socket_, AdbMessage &outMsg, int maxAttempts=50, int intervalMs=100);
    bool sendMsg(SOCKET socket_, std::vector<uint8_t> &sendMsg);

private:

    WifiServer();
    ~WifiServer()=default;
    bool initNetwork();

private:
    bool networkInitialized;   //网络服务初始状态
};

#endif // WIFISERVER_H
