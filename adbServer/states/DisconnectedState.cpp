#include "DisconnectedState.h"

#include "context/DeviceContext.h"

void DisconnectedState::handle(DeviceContext& context) {
    context.strategy->close(context);
}

