#ifndef ADBSTATE_H
#define ADBSTATE_H

#include <memory>


struct DeviceContext;

class IAdbState {
public:
    virtual ~IAdbState() = default;
    virtual bool handle(DeviceContext& context) = 0;
protected:
    void setState(DeviceContext& context, std::unique_ptr<IAdbState> newState);
};

// 连接状态
class ConnectingState : public IAdbState {
public:
    ConnectingState()=default;
    bool handle(DeviceContext& context) override;
    ~ConnectingState()=default;
};

// 认证状态
class AuthenticatingState : public IAdbState {
public:
    AuthenticatingState()=default;
    bool handle(DeviceContext& context) override;
    ~AuthenticatingState()=default;
};

// 执行状态
class ExecutingState : public IAdbState {
public:
    ExecutingState()=default;
    bool handle(DeviceContext& context) override;
    ~ExecutingState()=default;
};

// 断开状态
class DisconnectedState : public IAdbState {
public:
    DisconnectedState()=default;
    bool handle(DeviceContext& context) override;
    ~DisconnectedState()=default;
};
#endif // ADBSTATE_H
