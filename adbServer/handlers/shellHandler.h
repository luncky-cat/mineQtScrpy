#ifndef SHELLHANDLER_H
#define SHELLHANDLER_H

#include <qDebug>
#include <string>

#include "interfaces/ICommandHandler.h"
#include "interfaces/ITransPort.h"

class shellHandler:public ICommandHandler
{
public:
    shellHandler();
    ~shellHandler() = default;
    bool CommandHandler(ITransPort& transport,DeviceContext& ctx);
private:
    bool execShell(ITransPort &transport, const int local_id, const int remote_id, std::string cmd, AdbMessage &out);
    bool openShell(ITransPort &transport, const int local_id, int &remote_id, AdbMessage &out);
    struct Registrar {
        Registrar() {
            qDebug()<<"注册shell";
            registerHandler(CmdType::Shell,new shellHandler());
        }
    };
    static Registrar registrar;
    std::string extractShellResult(const std::vector<std::string> &payloads);
};

#endif // SHELLHANDLER_H
