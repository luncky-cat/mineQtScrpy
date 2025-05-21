#ifndef PUSHHANDLER_H
#define PUSHHANDLER_H

#include <qDebug>
#include <string>

#include "interfaces/ICommandHandler.h"
#include "interfaces/ITransPort.h"


class pushHandler:public ICommandHandler
{
public:
    pushHandler()=default;
    ~pushHandler()=default;
    bool CommandHandler(ITransPort& transport,DeviceContext& ctx) override;
private:
    bool openSyn(ITransPort &transport, const int local_id, int &remote_id, AdbMessage &out);
    bool pushFile(ITransPort &transport, int local_id, int remote_id, std::string &localFilePath, std::string remoteFilePath, AdbMessage &out);

    struct Registrar {
        Registrar() {
            qDebug()<<"注册pushHandler";
            registerHandler(CmdType::Push,new pushHandler());
        }
    };
    static Registrar registrar;
};

#endif // PUSHHANDLER_H
