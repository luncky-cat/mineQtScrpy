#ifndef ISERVER_H
#define ISERVER_H

struct DeviceContext;

class IServer       //基础服务类
{
public:
    ~IServer()=default;
    virtual bool connect(DeviceContext &ctx) = 0;
    virtual bool auth(DeviceContext& ctx) = 0;
    virtual bool execute(DeviceContext& ctx) = 0;
    virtual bool close(DeviceContext& ctx) = 0;
};

#endif // ISERVER_H
