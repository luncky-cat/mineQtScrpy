#ifndef EXECUTINGSTATE_H
#define EXECUTINGSTATE_H

#include "interfaces/IState.h"

class ExecutingState : public IState {
public:
    ExecutingState()=default;
    void handle(DeviceContext& context) override;
    ~ExecutingState()=default;
};

#endif // EXECUTINGSTATE_H
