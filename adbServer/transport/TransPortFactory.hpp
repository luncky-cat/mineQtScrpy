#ifndef TRANSPORTFACTORY_H
#define TRANSPORTFACTORY_H

#include"socketTransPort.h"

#include "ConnectInfo.h"

class ITransPortFactory
{
public:
    static std::unique_ptr<ITransPort> create(ConnectType type){
        switch (type) {
        case ConnectType::WiFi:
            return std::make_unique<SocketTransPort>();
        default:
            return nullptr;
        }
    }
};

#endif // TRANSPORTFACTORY_H
