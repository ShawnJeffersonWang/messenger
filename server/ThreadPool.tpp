//
// Created by shawn on 23-8-8.
//

#ifndef CHATROOM_THREADPOOL_TPP
#define CHATROOM_THREADPOOL_TPP
//.tpp文件是一个惯例

/*将模板函数的声明和实现分别放在头文件和对应的实现文件中，并在头文件中包含实现文件
可以确保模板的定义在需要实例化时可见，从而避免链接错误*/
template<typename Func, typename...Args>
auto ThreadPool::addTask(Func &&func, Args &&...args) -> std::optional<std::future<decltype(func(args...))>> {
//        if (shutdown) {
//            return;
//        }
//        taskQ.addTask(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    if (shutdown) {
        //throw std::runtime_error("ThreadPool is shutting down");
        return std::nullopt;
    }
    //完美转发forward建议加上std::,不加有时会有警告
//        auto task = make_shared<packaged_task<decltype(func(args...))()>>(
//                std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
//        );
    using return_type = decltype(func(args...));
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            [func, args...]() mutable {
                return func(args...);
            }
    );

    std::future<return_type> res = task->get_future();
    {
        std::lock_guard<std::mutex> lock(pool_lock);
        tasks.emplace([task]() { (*task)(); });
    }
    not_empty.notify_one();
    return res;
}

#endif //CHATROOM_THREADPOOL_TPP
