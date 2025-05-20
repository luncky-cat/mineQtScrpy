#ifndef WIFISERVER_H
#define WIFISERVER_H

#include <map>

class ICommandHandler;

#include"interfaces/IServer.h"
#include "context/CommandInfo.h"

class WifiServer : public IServer   //通用的wifi服务接口
{
public:
    static WifiServer& instance();
    bool connect(DeviceContext& ctx) override;
    bool auth(DeviceContext& ctx) override;
    bool execute(DeviceContext& ctx)override;
    bool close(DeviceContext& ctx)override;

protected:
    std::map<CmdType,ICommandHandler*>commandHandlers;

private:

    WifiServer();
    ~WifiServer()=default;
    bool initNetwork();

private:
    bool networkInitialized;   //网络服务初始状态
};

#endif // WIFISERVER_H


// bool executeShell();
// std::string extractShellResult(const std::vector<std::string> &payloads, const std::string &cmdEcho);
// int recvExact(int socket, void *buffer, int length);
// bool executeShell(std::string &cmd);
// bool executeShell(std::string &cmd, DeviceContext &ctx);
// bool handleScrcpyHostCommand(const std::string &cmd, int clientSocket);
// std::vector<uint8_t> create_adb_packet(const std::string &payload);
// std::string adb_version_response(int version = 31);
// void handleHostCommand(std::string& outCmd, int clientSocket);
// std::string buildAdbBinaryResponse(const std::string &responsePayload);
// std::string buildResponse(const std::string &payload);
// std::string buildAdbStringResponse(const std::string &payloadStr);
