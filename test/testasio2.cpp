/*
 * 一个 I/O Service 与多个线程，特点：
 * 先分配一个全局 io_context，然后开启多个线程，每个线程都调用这个 io_context的run()方法。这样，当某个异步事件完成时，io_context 就会将相应的 event handler 交给任意一个线程去执行。
 *
 * 然而这种方案在实际使用中，需要注意一些问题：
 * 在 event handler 中允许执行阻塞的操作 (例如数据库查询操作)。
 * 线程数可以大于 CPU 核心数，譬如说，如果需要在 event handler 中执行阻塞的操作，为了提高程序的响应速度，这时就需要提高线程的数目。
 * 由于多个线程同时运行事件循环(event loop)，所以会导致一个问题：即一个 socket 描述符可能会在多个线程之间共享，容易出现竞态条件 (race condition)。譬如说，如果某个 socket 的可读事件很快发生了两次，那么就会出现两个线程同时读同一个 socket 的问题 (可以使用strand解决这个问题)。
 *
 * g++ testasio2.cpp -ggdb -o testasio2 -lpthread -lwsock32
 */

#include <iostream>
#include <asio.hpp>
#include <vector>

class AsioThreadPool
{
public:
    //返回当前系统支持的并发线程数
    AsioThreadPool(std::size_t size = std::thread::hardware_concurrency()) :
        work_(new asio::io_context::work(io_context_))
    {
        printf("AsioThreadPool size = %d\n", size);
        for (std::size_t i = 0; i < size; ++i) {
            threads_.emplace_back([this]() {
                printf("io_context_ %p run, ThreadID: %d\n", this, GetCurrentThreadId());
                io_context_.run();
            });
        }
    }

    AsioThreadPool(const AsioThreadPool&) = delete;
    AsioThreadPool &operator=(const AsioThreadPool&) = delete;

    asio::io_context& getIOContext()
    {
        return io_context_;
    }

    void stop()
    {
        work_.reset();

        for (auto &t : threads_) {
            t.join();
        }
    }

private:
    asio::io_context io_context_;
    std::unique_ptr<asio::io_context::work> work_;
    std::vector<std::thread> threads_;
};

int main()
{
    AsioThreadPool pool(4);    // 开启 4 个线程
    asio::steady_timer timer1{pool.getIOContext(), std::chrono::seconds{1}};
    asio::steady_timer timer2{pool.getIOContext(), std::chrono::seconds{1}};
    int value = 0;
    asio::io_context::strand strand{pool.getIOContext()};

    timer1.async_wait(strand.wrap([&value] (const asio::error_code &ec) {
        printf("timer1.async_wait ThreadID: %d\n", GetCurrentThreadId());
        std::cout << "Hello, World! " << value++ << std::endl;
    }));
    timer2.async_wait(strand.wrap([&value] (const asio::error_code &ec) {
        printf("timer2.async_wait ThreadID: %d\n", GetCurrentThreadId());
        std::cout << "Hello, World! " << value++ << std::endl;
    }));

    pool.stop();
    return 0;
}
