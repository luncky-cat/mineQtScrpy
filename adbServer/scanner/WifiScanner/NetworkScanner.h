#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <asio/steady_timer.hpp>
#include <asio/io_context.hpp>
#include<asio/ip/tcp.hpp>
#include <set>

#include"interfaces/IScanner.h"


struct NetworkScannerAutoRegister;
class WifiConnectConfig;

class NetworkScanner :public IScanner{
public:
    void start() override;

    void stop() override;

    void configure(const std::shared_ptr<IConfig>& config) override;

    void setContext(std::shared_ptr<asio::io_context>& context);


private:
    NetworkScanner()=default;
    NetworkScanner(const NetworkScanner&)=delete;
    NetworkScanner& operator=(const NetworkScanner&) = delete;

private:
    using DeviceFoundCallback = std::function<void(const std::string& ip, uint16_t port, std::shared_ptr<asio::ip::tcp::socket> socket)>;
    DeviceFoundCallback device_callback;
    void setDeviceFoundCallback(DeviceFoundCallback cb);

    std::string intToIp(uint32_t ip);
    uint32_t ipToInt(const std::string &ip);
    void startScan(const std::string &ip, uint16_t port, int timeout_ms);
    void scanFromIpRangeAny(const std::string &startIp, const std::string &endIp, uint16_t startPort, uint16_t endPort, int timeoutMs);
    bool updateNetworkInfo();   //更新网络信息
    void startPeriodicScan();  //开启定时任务
    void stopPeriodicScan();
    void handleConnect(const std::string &ip, uint16_t port, std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code &ec);
private:
    std::shared_ptr<asio::io_context> io_context;
    std::shared_ptr<WifiConnectConfig> config_;
    std::set<std::pair<std::string, uint16_t>> scanned_targets;
    std::unique_ptr<asio::steady_timer> periodic_timer;
    static NetworkScannerAutoRegister _autoRegisterScanner;

    std::string ip;
    std::string netmask;
    std::string startIp;
    std::string endIp;


};

struct NetworkScannerAutoRegister {
    NetworkScannerAutoRegister() {
        auto scanner = std::make_shared<NetworkScanner>();
        IScanner::registerScanner(scanner);
    }
};

#endif // NETWORKSCANNER_H
