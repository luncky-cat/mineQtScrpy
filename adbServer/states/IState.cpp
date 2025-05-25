#include "interfaces/IState.h"

#include "context//DeviceContext.h"

void IState::setState(DeviceContext &context, std::unique_ptr<IState> newState)
{
    context.deviceState = std::move(newState);
    context.deviceState->handle(context);
}

