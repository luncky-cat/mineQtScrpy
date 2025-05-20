#ifndef PUSHHANDLER_H
#define PUSHHANDLER_H

#include "interfaces/ICommandHandler.h"
#include "interfaces/ITransPort.h"

#include <string>
#include <winsock2.h>

class pushHandler:public ICommandHandler
{
public:
    pushHandler()=default;
    ~pushHandler()=default;
    bool CommandHandler(ITransPort& transport,DeviceContext& ctx) override;
private:
    bool openSyn(ITransPort &transport, const int local_id, int &remote_id, AdbMessage &out);
    bool pushFile(ITransPort &transport, int local_id, int remote_id, std::string &localFilePath, std::string remoteFilePath, AdbMessage &out);
};

#endif // PUSHHANDLER_H
