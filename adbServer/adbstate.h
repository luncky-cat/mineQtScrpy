#ifndef ADBSTATE_H
#define ADBSTATE_H

enum class DeviceStatus {
    Disconnected,
    Connecting,
    Authenticating,
    Connected
};

struct DeviceContext;

class AdbServer;

class IAdbState {
public:
    virtual ~IAdbState() = default;
    virtual DeviceStatus getStatus()=0;
    virtual void handle(AdbServer& server,DeviceContext& context) = 0;
};

// 连接状态
class ConnectingState : public IAdbState {
public:
    ConnectingState()=default;
    void handle(AdbServer& server,DeviceContext& context) override;
    DeviceStatus getStatus() override;
    ~ConnectingState()=default;
};

// 认证状态
class AuthenticatingState : public IAdbState {
public:
    AuthenticatingState()=default;
    void handle(AdbServer& server,DeviceContext& context) override;
    DeviceStatus getStatus() override;
    ~AuthenticatingState()=default;
};

// 执行状态
class ConnectedState : public IAdbState {
public:
    ConnectedState()=default;
    void handle(AdbServer& server,DeviceContext& context) override;
    DeviceStatus getStatus() override;
    ~ConnectedState()=default;
};

// 断开状态
class DisconnectedState : public IAdbState {
public:
    DisconnectedState()=default;
    void handle(AdbServer& server,DeviceContext& context) override;
    DeviceStatus getStatus() override;
    ~DisconnectedState()=default;
};
#endif // ADBSTATE_H
