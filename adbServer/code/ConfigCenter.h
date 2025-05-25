#ifndef CONFIGCENTER_H
#define CONFIGCENTER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "interfaces/IConfig.h"

class ConfigCenter {
public:
    static ConfigCenter &getInstance();
    void registerAllConfigs();
    template<typename T>
    std::shared_ptr<T> getConfig(const std::string &name);

private:
    ConfigCenter() = default;
    template<typename T>
    void registerConfig(const std::string &name, std::shared_ptr<T> config);

private:
    std::unordered_map<std::string, std::shared_ptr<IConfig>> configs;
};


#endif // CONFIGCENTER_H
