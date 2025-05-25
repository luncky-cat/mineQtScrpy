#ifndef WIFISERVER_H
#define WIFISERVER_H

#include <memory>

#include"interfaces/IServer.h"

class WifiServer :public IServer,public std::enable_shared_from_this<WifiServer>  //通用的wifi服务接口
{
public:
    bool connect(DeviceContext& ctx) override;
    bool auth(DeviceContext& ctx) override;
    bool execute(DeviceContext& ctx)override;
    bool close(DeviceContext& ctx)override;
    static std::shared_ptr<WifiServer> instance();
    ~WifiServer()=default;
    WifiServer()=default;
private:
    void testPushThenList(DeviceContext &ctx);
};

#endif // WIFISERVER_H


