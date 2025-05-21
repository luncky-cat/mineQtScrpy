#ifndef ICOMMANDHANDLER_H
#define ICOMMANDHANDLER_H

#include <map>

#include "context/CommandInfo.h"


class DeviceContext;
class ITransPort;

class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    virtual bool CommandHandler(ITransPort& transport,DeviceContext& ctx)=0;


    static std::map<CmdType, ICommandHandler*>& handlers() {
        static std::map<CmdType, ICommandHandler*> instance;
        return instance;
    }

    static ICommandHandler* getHandler(CmdType type) {
        auto it = handlers().find(type);
        return (it != handlers().end()) ? it->second : nullptr;
    }

    static void registerHandler(CmdType type, ICommandHandler* handler) {
        handlers()[type] = handler;
    }

};

#endif // ICOMMANDHANDLER_H




// static PushHandler& instance() {
//     static PushHandler instance;
//     return instance;
// }

// bool CommandHandler(ITransPort& transport, DeviceContext& ctx) override {
//     // 处理 push 命令逻辑
//     return true;
// }

// private:
// PushHandler() {
//     ICommandHandler::registerHandler(CmdType::Push, this);
// }
