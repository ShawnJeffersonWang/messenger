//
// Created by shawn on 23-8-7.
//

#ifndef CHATROOM_THREADPOOL_HPP
#define CHATROOM_THREADPOOL_HPP

#include <queue>
#include <future>
#include <optional>

class ThreadPool {
public:
    ThreadPool(size_t minNum, size_t maxNum) noexcept;

    ~ThreadPool() noexcept;

    //将模板的声明和定义放在同一个头文件中，不然会有链接错误，确保编译器在实例化模板时能够正确找到模板的定义。
    template<typename Func, typename...Args>
    auto addTask(Func &&func, Args &&...args) -> std::optional<std::future<decltype(func(args...))>>;

private:
    void worker();

    void manager();

    void addWorkers();

    void removeIdleWorkers();

private:
    std::mutex pool_lock;
    std::condition_variable not_empty;
    //std::condition_variable all_workers_idle;
    std::vector<std::thread> threads;
    std::thread manager_thread;
    //TaskQueue taskQ;
    std::queue<std::function<void()>> tasks;
    const size_t min_num;
    const size_t max_num;
//    size_t busy_num;
//    size_t alive_num;
    std::atomic<size_t> busy_num;
    std::atomic<size_t> alive_num;
    //bool shutdown;
    std::atomic<bool> shutdown;
    std::atomic<bool> manager_stopped;
};
//在C++中，模板函数或模板类的定义需要在声明的地方，也就是头文件中。这是因为C++的模板实例化是在编译时完成的
#include "ThreadPool.tpp"

#endif //CHATROOM_THREADPOOL_HPP
