#include "AdbStreamContext.h"

AdbStreamContext::AdbStreamContext(uint32_t localId):localId_(localId), remoteId_(0){}

uint32_t AdbStreamContext::getLocalId() const {
    return localId_;
}
uint32_t AdbStreamContext::getRemoteId() const {
    return remoteId_;
}
void AdbStreamContext::setRemoteId(uint32_t remoteId) {
    remoteId_ = remoteId;
}

void AdbStreamContext::enqueueMessage(const AdbMessage& msg) {
    //std::lock_guard<std::mutex> lock(mutex_);
    messageQueue_.push(msg);
    //commandCompleted_ = true;  // 简单示例，收到消息视为完成
    //cv_.notify_all();
}

std::optional<AdbMessage> AdbStreamContext::tryDequeueMessage() {
    // std::lock_guard<std::mutex> lock(mutex_);
    if (messageQueue_.empty())
        return std::nullopt;
    AdbMessage msg = messageQueue_.front();
    messageQueue_.pop();
    return msg;
}
