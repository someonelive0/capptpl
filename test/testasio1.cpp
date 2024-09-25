/*
 * 每个线程一个 I/O Context[2], 特点：
 * 在多核的机器上，这种方案可以充分利用多个 CPU 核心。
 * 某个 socket 描述符并不会在多个线程之间共享，所以不需要引入同步机制。
 * 在 event handler 中不能执行阻塞的操作，否则将会阻塞掉 io_context 所在的线程。
 *
 * g++ testasio1.cpp -ggdb -o testasio1 -lpthread -lwsock32
 */

#include <iostream>
#include <asio.hpp>
#include <vector>

class AsioIOContextPool
{
public:
    using IOContext = asio::io_context;
    using Work = asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;

    //返回当前系统支持的并发线程数
    AsioIOContextPool(std::size_t size = std::thread::hardware_concurrency()) :
        ioContexts_(size),
        works_(size),
        nextIOContext_(0)
    {
        printf("AsioIOContextPool size = %d\n", size);
        for (std::size_t i = 0; i < size; ++i) {
            works_[i] = std::unique_ptr<Work>(new Work(ioContexts_[i]));
        }

        for (std::size_t i = 0; i < ioContexts_.size(); ++i) {
            threads_.emplace_back([this, i]() {
                printf("ioContexts_ %d run, ThreadID: %d\n", i, GetCurrentThreadId());
                ioContexts_[i].run();
            });
        }
    }

    AsioIOContextPool(const AsioIOContextPool&) = delete;
    AsioIOContextPool &operator=(const AsioIOContextPool&) = delete;

    asio::io_context& getIOContext()
    {
        auto &context = ioContexts_[nextIOContext_++];
        if (nextIOContext_ == ioContexts_.size()) {
            nextIOContext_ = 0;
        }
        return context;
    }

    void stop()
    {
        for (auto &work : works_) {
            work.reset();
        }

        for (auto &t : threads_) {
            t.join();
        }
    }

private:
    std::vector<IOContext> ioContexts_;
    std::vector<WorkPtr> works_;
    std::vector<std::thread> threads_;
    std::size_t nextIOContext_;
};

int main()
{
    std::mutex mtx;
    AsioIOContextPool pool;
    asio::steady_timer timer{pool.getIOContext(), std::chrono::seconds{2}};

    timer.async_wait([&mtx](const asio::error_code &ec) {
        printf("timer.async_wait ThreadID: %d\n", GetCurrentThreadId());
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "timer.async_wait: Hello, World!" << std::endl;
    });

    pool.stop();
    return 0;
}
