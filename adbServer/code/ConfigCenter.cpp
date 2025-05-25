#include "ConfigCenter.h"
#include"config/WifiScannerConfig.h"

#include <qDebug>

ConfigCenter& ConfigCenter::getInstance() {
    static ConfigCenter instance;
    return instance;
}

void ConfigCenter::registerAllConfigs()
{
    qDebug()<<"注册所有配置";
    registerConfig("WIFI",std::make_shared<WifiScannerConfig>());
}

template<typename T>
void ConfigCenter::registerConfig(const std::string& name, std::shared_ptr<T> config) {
    configs[name] = config;
}

template<typename T>
std::shared_ptr<T> ConfigCenter::getConfig(const std::string& name) {
    auto it = configs.find(name);
    if (it != configs.end()) {
        return std::dynamic_pointer_cast<T>(it->second);
    }
    return nullptr;
}
