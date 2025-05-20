#ifndef ICOMMANDHANDLER_H
#define ICOMMANDHANDLER_H

class DeviceContext;
class ITransPort;

class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    virtual bool CommandHandler(ITransPort& transport,DeviceContext& ctx)=0;
};

#endif // ICOMMANDHANDLER_H
