#ifndef ADBSTREAMCONTEXT_H
#define ADBSTREAMCONTEXT_H


#include <queue>
// #include <vector>
// #include <mutex>
// #include <condition_variable>
// #include <cstdint>
#include <optional>

#include "protocol/AdbMessage.h"

class AdbStreamContext {
public:
    // 阻塞等待命令完成（根据具体逻辑调整条件）
    // bool waitCommand(int timeoutMs) {
    //     std::unique_lock<std::mutex> lock(mutex_);
    //     return cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
    //                         [this] { return commandCompleted_; });
    // }

    // // 清空消息队列，重置状态
    // void reset() {
    //     std::lock_guard<std::mutex> lock(mutex_);
    //     while (!messageQueue_.empty()) messageQueue_.pop();
    //     commandCompleted_ = false;
    // }

    AdbStreamContext(uint32_t localId);
    uint32_t getLocalId() const;
    uint32_t getRemoteId() const;
    void setRemoteId(uint32_t remoteId);
    void enqueueMessage(const AdbMessage &msg);
    std::optional<AdbMessage> tryDequeueMessage();
private:
    uint32_t localId_;   // 流唯一标识
    uint32_t remoteId_;  // 设备分配的流ID
    std::queue<AdbMessage> messageQueue_;
    //bool commandCompleted_;
    //std::mutex mutex_;
    //std::condition_variable cv_;
};

#endif // ADBSTREAMCONTEXT_H
