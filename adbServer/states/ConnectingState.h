#ifndef CONNECTINGSTATE_H
#define CONNECTINGSTATE_H

#include "interfaces/IState.h"
// 连接状态
class ConnectingState : public IState {
public:
    ConnectingState()=default;
    void handle(DeviceContext& context) override;
    ~ConnectingState()=default;
};

#endif // CONNECTINGSTATE_H
