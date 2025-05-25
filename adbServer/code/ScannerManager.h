#ifndef SCANNERMANAGER_H
#define SCANNERMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "interfaces/IScanner.h"

class ScannerManager
{
public:
    static ScannerManager &getInstance();
    void registerAllScanners();
    template<typename T>
    std::shared_ptr<T> getScanner(const std::string &name);
    void startAll();
    void stopAll();

private:
    ScannerManager()=default;
    template<typename T>
    void registerScanner(const std::string &name, std::shared_ptr<T> scanner);

private:
    std::unordered_map<std::string, std::shared_ptr<IScanner>> scanners;
};

#endif // SCANNERMANAGER_H
