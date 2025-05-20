#ifndef TRANSPORTFACTORY_H
#define TRANSPORTFACTORY_H

#include"socketTransPort.h"

#include "ConnectInfo.h"

class ITransPortFactory
{
public:
    static ITransPort* create(ConnectType type){
        switch (type) {
        case ConnectType::WiFi:
            return new SocketTransPort;
        default:
            return nullptr;
        }
    }
};

#endif // TRANSPORTFACTORY_H
