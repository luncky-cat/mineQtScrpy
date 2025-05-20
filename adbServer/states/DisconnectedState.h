#ifndef DISCONNECTEDSTATE_H
#define DISCONNECTEDSTATE_H

#include "interfaces/IState.h"

class DisconnectedState : public IState {
public:
    DisconnectedState()=default;
    void handle(DeviceContext& context) override;
    ~DisconnectedState()=default;
};

#endif // DISCONNECTEDSTATE_H
