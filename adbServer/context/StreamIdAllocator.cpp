#include "StreamIdAllocator.h"


// 初始化 ID 计数器，通常从 0 开始（也可从 1 开始，0 保留）
std::atomic<uint32_t> StreamIdAllocator::idCounter{0};

uint32_t StreamIdAllocator::allocate() {
    return ++idCounter;
}

void StreamIdAllocator::reset(uint32_t startFrom) {
    idCounter.store(startFrom);
}

