#ifndef WAIT_COMMAND_ASYNC_H
#define WAIT_COMMAND_ASYNC_H

#include <cstdint>
#include <memory>
#include <system_error>
#include <vector>

#include "DeviceSession.h"
#include "context/WaitCommandAwaitable.h"
#include "protocol/AdbMessage.h"


template<typename CompletionToken>
auto async_wait_command(
    std::shared_ptr<DeviceSession> session,
    std::vector<uint32_t> expectedCmds,
    int timeoutMs,
    uint32_t streamId,
    CompletionToken&& token
    ) {
    return asio::async_initiate<CompletionToken, void(std::error_code, AdbMessage)>(
        [=](auto&& handler) mutable {
            WaitCommandAwaitable awaitable(session, std::move(expectedCmds), timeoutMs, streamId);
            awaitable(std::forward<decltype(handler)>(handler));
        },
        token
        );
}

#endif // WAIT_COMMAND_ASYNC_H
