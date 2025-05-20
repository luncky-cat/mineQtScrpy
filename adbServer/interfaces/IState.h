#ifndef ISTATE_H
#define ISTATE_H

class DeviceContext;

#include <memory>

class IState {
public:
    virtual ~IState() = default;
    virtual void handle(DeviceContext& context) = 0;
protected:
    void setState(DeviceContext& context, std::unique_ptr<IState> newState);
};

#endif // ISTATE_H
