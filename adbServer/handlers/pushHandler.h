#ifndef PUSHHANDLER_H
#define PUSHHANDLER_H

#include "interfaces/ICommandHandler.h"
#include "interfaces/ITransPort.h"

#include <winsock2.h>

class pushHandler:public ICommandHandler
{
public:
    pushHandler()=default;
    ~pushHandler()=default;
    bool CommandHandler(ITransPort& transport,DeviceContext& ctx) override;
private:
    // bool openSyn(SOCKET s, const int local_id, int &remote_id);
};

#endif // PUSHHANDLER_H
