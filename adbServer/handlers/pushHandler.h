#ifndef PUSHHANDLER_H
#define PUSHHANDLER_H

#include <qDebug>
#include <string>

#include "interfaces/ICommandHandler.h"

class pushHandler:public ICommandHandler
{
public:
    pushHandler()=default;
    ~pushHandler()=default;
    asio::awaitable<bool> CommandHandler(std::shared_ptr<DeviceSession> sessionCtx, CommandInfo cmds);
private:
    asio::awaitable<bool> openSyn(std::shared_ptr<DeviceSession> sessionCtx, const int local_id, int &remote_id);
    asio::awaitable<bool> pushFile(std::shared_ptr<DeviceSession> sessionCtx, int local_id, int remote_id, std::string localFilePath, std::string remoteFilePath);
    struct Registrar {
        Registrar() {
            qDebug()<<"[pushHandler]注册";
            registerHandler(CmdType::Push,new pushHandler());
        }
    };
    static Registrar registrar;
};

#endif // PUSHHANDLER_H
