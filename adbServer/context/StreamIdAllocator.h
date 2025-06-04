#ifndef STREAMIDALLOCATOR_H
#define STREAMIDALLOCATOR_H

#include <atomic>

class StreamIdAllocator {
public:
    // 分配一个唯一的 stream id（线程安全）
    static uint32_t allocate();

    // 可选：重置 ID 起始值
    static void reset(uint32_t startFrom = 0);

private:
    static std::atomic<uint32_t> idCounter;
};


#endif // STREAMIDALLOCATOR_H
