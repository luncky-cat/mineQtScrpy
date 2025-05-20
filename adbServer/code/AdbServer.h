#ifndef ADBSERVER_H
#define ADBSERVER_H

#include <string>
#include <vector>
#include <unordered_map>

class DeviceContext;

#include "server/DeviceServerFactory.hpp"
#include "interfaces/IState.h"

class AdbServer {
public:
    static AdbServer& getInstance();
    ~AdbServer();

    void asyncConnectDevice(const std::string& deviceId);
    void doConnectDevice(const std::string& deviceId);

    // 设备管理接口
    bool registerDevice(const ConnectInfo& info);  // 注册设备信息
    bool unregisterDevice(const std::string& deviceId);  // 注销设备
    std::vector<std::string> getRegisteredDevices() const;  // 获取已注册设备列表

    // 设备操作接口
    bool disconnectDevice(const std::string& deviceId);  // 断开设备

    // 设备状态查询接口
   // DeviceStatus getDeviceStatus(const std::string& deviceId) const;  // 获取设备状态
    bool isDeviceConnected(const std::string& deviceId) const;  // 检查设备是否连接



    void start();
    void handleClient(int clientSocket);
    bool readMessage(int clientSocket, std::string& outCmd);
    void processCommand(int clientSocket, const std::string& cmd);
    void sendOkay(int clientSocket);
    void sendFail(int clientSocket, const std::string& reason);
    void sendPayload(int clientSocket, const std::string& data);
    void acceptSocket();
    bool readMessage1(int clientSocket, std::string &outMsg);
private:
         // AdbServer()=default;
    AdbServer();
    AdbServer(const AdbServer&) = delete;
    AdbServer& operator=(const AdbServer&) = delete;

    // 状态管理
    bool setState(const std::string& deviceId, std::unique_ptr<IState> newState);

    // 设备上下文管理
    DeviceContext* getDeviceContext(const std::string& deviceId);
    const DeviceContext* getDeviceContext(const std::string& deviceId) const;

    int ADB_PORT = 8091;
    int BUFFER_SIZE = 4096;
    int serverSocket;
private:
    std::unordered_map<std::string, DeviceContext> deviceContextMap;
    std::unique_ptr<IServerFactory>fac;  //用于得到需要的服务
};

#endif // ADBSERVER_H
