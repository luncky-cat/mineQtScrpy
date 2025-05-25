#include "ScannerManager.h"
#include "scanner/WifiScanner/NetworkScanner.h"

#include <qDebug>

ScannerManager &ScannerManager::getInstance()
{
    static ScannerManager instance;
    return instance;
}

void ScannerManager::registerAllScanners(){
    qDebug()<<"注册所有监听器";
    registerScanner("WIFI",std::make_shared<NetworkScanner>());
}

template<typename T>
std::shared_ptr<T> ScannerManager::getScanner(const std::string& name) {
    auto it = scanners.find(name);
    if (it != scanners.end())
        return  std::dynamic_pointer_cast<T>(it->second);
    return nullptr;
}

template<typename T>
void ScannerManager::registerScanner(const std::string& name, std::shared_ptr<T> scanner) {
    scanners[name] = scanner;
}

void ScannerManager::startAll() {
    for (auto& [name, scanner] : scanners) {
        scanner->start();
    }
}

void ScannerManager::stopAll() {
    for (auto& [name, scanner] : scanners) {
        scanner->stop();
    }
}

