#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <asio/steady_timer.hpp>
#include <asio/io_context.hpp>
#include<asio/ip/tcp.hpp>
#include <unordered_set>

#include"interfaces/IScanner.h"


class NetworkScanner :public IScanner{
public:

    void start() override;

    void stop() override;

    void configure(const std::shared_ptr<IConfig>& config) override;


    using DeviceFoundCallback = std::function<void(const std::string& ip)>;
    NetworkScanner(uint16_t port=5555, int timeoutMs = 300);

    void scanRange(int startSuffix, int endSuffix);
    void setDeviceFoundCallback(DeviceFoundCallback cb);
    void startPeriodicScan(int intervalMs, int startSuffix, int endSuffix);
    void stopPeriodicScan();

private:
    void startScan(const std::string& ip);
    void handleConnect(const std::string& ip, std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code& ec);

    std::shared_ptr<asio::io_context> io_context;
    std::string base_ip;
    uint16_t port;
    int timeout_ms;
    DeviceFoundCallback device_callback;

    std::unordered_set<std::string> scanned_ips;

    std::unique_ptr<asio::steady_timer> periodic_timer;
    int scan_interval_ms = 0;
    int scan_range_start = 0;
    int scan_range_end = 0;
    bool periodic_enabled = false;
};

#endif // NETWORKSCANNER_H
