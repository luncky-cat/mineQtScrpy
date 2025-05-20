#include "ThreadPool.h"


ThreadPool::ThreadPool(size_t threadCount) : currentThreads(0),stopFlag(false) {
    for (size_t i = 0; i < threadCount/2; ++i) {
        workers.emplace_back([this]() { this->workerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}


void ThreadPool::submit(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        tasks.push(std::move(task));
    }
    condVar.notify_one();
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        stopFlag = true;
    }
    condVar.notify_all();
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex);
            condVar.wait(lock, [this]() {
                return stopFlag || !tasks.empty();
            });

            if (stopFlag && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }
        task();  // 执行任务
    }
}

ThreadPool& ThreadPool::getInstance() {
    static ThreadPool instance; // 假设我们希望创建4个工作线程
    return instance;
}
