#include "NetworkScanner.h"

#include "scanner/ConnectConfig/WifiConnectConfig.h"

#include "utils/NetWorkUtils.h"


NetworkScannerAutoRegister NetworkScanner::_autoRegisterScanner;

void NetworkScanner::start()
{
    startPeriodicScan();
}

void NetworkScanner::stop()
{
    stopPeriodicScan();
}

void NetworkScanner::configure(const std::shared_ptr<IConfig> &config)
{
    auto specificConfig = std::dynamic_pointer_cast<WifiConnectConfig>(config);
    if(specificConfig){
        config_=specificConfig;
    }
}


void NetworkScanner::setContext(std::shared_ptr<asio::io_context>& context)
{
    io_context=context;
}

void NetworkScanner::setDeviceFoundCallback(DeviceFoundCallback cb) {
    device_callback = std::move(cb);
}


void NetworkScanner::startScan(const std::string& ip,uint16_t port,int timeout_ms) {    //扫描单个ip
    auto target = std::make_pair(ip, port);
    if (scanned_targets.count(target)) return;
    scanned_targets.insert(target);

    auto socket = std::make_shared<asio::ip::tcp::socket>(io_context);
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip),port);

    auto timer = std::make_shared<asio::steady_timer>(io_context);
    timer->expires_after(std::chrono::milliseconds(timeout_ms));

    socket->async_connect(endpoint, [this, ip,port,socket, timer](const asio::error_code& ec) {
        timer->cancel();
        handleConnect(ip, port,socket, ec);
    });

    timer->async_wait([socket](const asio::error_code& ec) {
        if (!ec) {
            asio::error_code ignored_ec;
            socket->close(ignored_ec);
        }
    });
}

uint32_t NetworkScanner::ipToInt(const std::string& ip) {
    std::stringstream ss(ip);
    uint32_t a, b, c, d;
    char dot;
    ss >> a >> dot >> b >> dot >> c >> dot >> d;
    return (a << 24) | (b << 16) | (c << 8) | d;
}

std::string NetworkScanner::intToIp(uint32_t ip) {
    return std::to_string((ip >> 24) & 0xFF) + "." +
           std::to_string((ip >> 16) & 0xFF) + "." +
           std::to_string((ip >> 8) & 0xFF) + "." +
           std::to_string(ip & 0xFF);
}

// 自动模式：获取当前网络环境并推导扫描范围
bool NetworkScanner::updateNetworkInfo(){
    if(!NetWorkUtils::getActiveWifiInfo(ip,netmask)){
        return false;
    }

    if(!NetWorkUtils::getIpRange(ip,netmask,startIp,endIp)){
        return false;
    }

    return true;
}


void NetworkScanner::scanFromIpRangeAny(const std::string& startIp, const std::string& endIp,uint16_t startPort,uint16_t endPort, int timeoutMs) {
    uint32_t start = ipToInt(startIp);
    uint32_t end = ipToInt(endIp);
    if (start > end) std::swap(start, end);
    if (startPort > endPort) std::swap(startPort, endPort);

    for (uint32_t ip = start; ip <= end; ++ip) {
        std::string ipStr = intToIp(ip);  // 避免重复转换
        for (uint16_t port = startPort; port <= endPort; ++port) {
            startScan(ipStr, port, timeoutMs);
        }
    }
}


void NetworkScanner::handleConnect(const std::string& ip,uint16_t port,std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code& ec) {
    if (!ec) {
        if (device_callback) {
           device_callback(ip, port, socket);
        }
    }
}



void NetworkScanner::startPeriodicScan() {
    if (!updateNetworkInfo()) return;

    if (!periodic_timer) {
        periodic_timer = std::make_unique<asio::steady_timer>(*io_context);
    }

    periodic_timer->expires_after(std::chrono::milliseconds(config_->intervalMms));

    periodic_timer->async_wait([this](const asio::error_code& ec) {
        if (!ec &&config_->periodicEnabled) {
            // scanned_targets.clear();
            scanFromIpRangeAny(startIp, endIp, config_->startPort, config_->endPort, config_->timeoutMs);
            startPeriodicScan(); // 递归再次启动定时器
        }
    });
}

void NetworkScanner::stopPeriodicScan() {
    config_->periodicEnabled = false;
    if (periodic_timer) {
        asio::error_code ec;
        periodic_timer->cancel(ec);
    }
}


