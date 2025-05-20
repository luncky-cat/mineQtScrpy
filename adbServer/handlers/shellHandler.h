#ifndef SHELLHANDLER_H
#define SHELLHANDLER_H

#include "interfaces/ICommandHandler.h"

class shellHandler:public ICommandHandler
{
public:
    shellHandler();
    ~shellHandler() = default;
    bool CommandHandler(ITransPort& transport,DeviceContext& ctx);
};

#endif // SHELLHANDLER_H
