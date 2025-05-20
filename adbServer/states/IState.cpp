#include "interfaces/IState.h"

#include "context//DeviceContext.h"

void IState::setState(DeviceContext &context, std::unique_ptr<IState> newState)
{
    context.currentState = std::move(newState);
    context.currentState->handle(context);
}

