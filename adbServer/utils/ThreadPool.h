#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    ~ThreadPool();
    void submit(std::function<void()> task);
    void shutdown();
    static ThreadPool& getInstance();
private:
    void workerLoop();

    int maxThreads;
    std::atomic<int> currentThreads;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex mutex;
    std::condition_variable condVar;
    std::atomic<bool> stopFlag;

    ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

};


#endif
