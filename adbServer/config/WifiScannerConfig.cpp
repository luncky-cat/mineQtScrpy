#include "WifiScannerConfig.h"

#include <qDebug>

WifiScannerConfig::WifiScannerConfig():startPort(5555),endPort(5555),
    timeoutMs(300),intervalMms(5000),periodicEnabled(false){
    qDebug()<<"WifiScannerConfig构造";
}
