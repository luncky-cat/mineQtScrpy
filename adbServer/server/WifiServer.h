#ifndef WIFISERVER_H
#define WIFISERVER_H

#include <memory>

#include "context/DeviceSession.h"
#include"interfaces/IServer.h"

class WifiServer :public IServer,public std::enable_shared_from_this<WifiServer>  //通用的wifi服务接口
{
public:
    bool connect(std::shared_ptr<DeviceContext>ctx) override;
    bool auth(std::shared_ptr<DeviceContext>ctx) override;
    bool execute(std::shared_ptr<DeviceContext>ctx)override;
    bool close(std::shared_ptr<DeviceContext>ctx)override;
    static std::shared_ptr<WifiServer> instance();
    ~WifiServer()=default;
    WifiServer();
private:
     void handleAuthToken(std::shared_ptr<DeviceContext> Ctx, const AdbMessage &tokenMsg);
    void handleAuthRejected(std::shared_ptr<DeviceContext> ctx);
    void handleAuthAccepted(std::shared_ptr<DeviceContext> ctx, const AdbMessage &cnxnMsg);
    void startConnect(std::shared_ptr<DeviceContext> ctx);
private:
    void testPushThenList(std::shared_ptr<DeviceContext>ctx);
};

#endif // WIFISERVER_H


