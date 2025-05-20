#ifndef AUTHENTICATINGSTATE_H
#define AUTHENTICATINGSTATE_H

#include "interfaces/IState.h"

class AuthenticatingState : public IState {
public:
    AuthenticatingState()=default;
    void handle(DeviceContext& context) override;
    ~AuthenticatingState()=default;
};

#endif // AUTHENTICATINGSTATE_H
