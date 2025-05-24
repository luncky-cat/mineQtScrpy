#include "NetworkScanner.h"

#include <iostream>

void NetworkScanner::start()
{

}

void NetworkScanner::stop()
{

}

void NetworkScanner::configure(const std::shared_ptr<IConfig> &config)
{

}

NetworkScanner::NetworkScanner(uint16_t port, int timeoutMs)
    : port(port), timeout_ms(timeoutMs) {}

void NetworkScanner::setDeviceFoundCallback(DeviceFoundCallback cb) {
    device_callback = std::move(cb);
}

void NetworkScanner::scanRange(int startSuffix, int endSuffix) {
    for (int i = startSuffix; i <= endSuffix; ++i) {
        std::string ip = base_ip + std::to_string(i);
        startScan(ip);
    }
}

void NetworkScanner::startScan(const std::string& ip) {
    if (scanned_ips.count(ip)) return;
    scanned_ips.insert(ip);

    auto socket = std::make_shared<asio::ip::tcp::socket>(io_context);
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip), port);

    auto timer = std::make_shared<asio::steady_timer>(io_context);
    timer->expires_after(std::chrono::milliseconds(timeout_ms));

    socket->async_connect(endpoint, [this, ip, socket, timer](const asio::error_code& ec) {
        timer->cancel();
        handleConnect(ip, socket, ec);
    });

    timer->async_wait([socket](const asio::error_code& ec) {
        if (!ec) {
            asio::error_code ignored_ec;
            socket->close(ignored_ec);
        }
    });
}

void NetworkScanner::handleConnect(const std::string& ip, std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code& ec) {
    if (!ec) {
        if (device_callback) {
            device_callback(ip);
        }
    }
}

void NetworkScanner::startPeriodicScan(int intervalMs, int startSuffix, int endSuffix) {
    scan_interval_ms = intervalMs;
    scan_range_start = startSuffix;
    scan_range_end = endSuffix;
    periodic_enabled = true;

    if (!periodic_timer) {
        periodic_timer = std::make_unique<asio::steady_timer>(io_context);
    }

    periodic_timer->expires_after(std::chrono::milliseconds(scan_interval_ms));
    periodic_timer->async_wait([this](const asio::error_code& ec) {
        if (!ec && periodic_enabled) {
            scanned_ips.clear();
            scanRange(scan_range_start, scan_range_end);
            startPeriodicScan(scan_interval_ms, scan_range_start, scan_range_end);
        }
    });
}

void NetworkScanner::stopPeriodicScan() {
    periodic_enabled = false;
    if (periodic_timer) {
        asio::error_code ec;
        periodic_timer->cancel(ec);
    }
}

