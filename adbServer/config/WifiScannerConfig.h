#ifndef WIFISCANNERCONFIG_H
#define WIFISCANNERCONFIG_H

#include"interfaces/IConfig.h"

#include <cstdint>


class WifiScannerConfig:public IConfig {
public:
    ~WifiScannerConfig()=default;
    WifiScannerConfig();
public:
    uint16_t startPort;   //起始端口
    uint16_t endPort;//结束端口
    int timeoutMs;   //超时
    int intervalMms;  //间隔
    bool periodicEnabled;//支持定时器
};

#endif // WIFISCANNERCONFIG_H
