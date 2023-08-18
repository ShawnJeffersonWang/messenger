//
// Created by shawn on 23-8-7.
//
#include "ThreadPool.hpp"
#include <iostream>
/*
 * 所有的非模板代码都可以放在一个.cc文件中，然后在.hpp文件中#include "ThreadPool.tpp"来提供模板函数的实现
 * 这样可以将模板函数的定义与其他代码分开*/
using namespace std;

ThreadPool::ThreadPool(size_t minNum, size_t maxNum) noexcept
        : min_num(minNum), max_num(maxNum), busy_num(0), alive_num(minNum),
          shutdown(false), manager_stopped(false) {
    for (size_t i = 0; i < minNum; i++) {
        //pthread_create(&threads[i], nullptr, worker, this);
        //threads[i] = std::thread(&ThreadPool::worker, this);
        threads.emplace_back(&ThreadPool::worker, this);
    }
    //pthread_create(&managerID, nullptr, manager, this);
    //threads[minNum] = std::thread(&ThreadPool::manager, this);
    //threads.emplace_back(&ThreadPool::manager, this);
    manager_thread = thread(&ThreadPool::manager, this);
}

ThreadPool::~ThreadPool() noexcept {
    manager_stopped = true;
    if (manager_thread.joinable()) {
        manager_thread.join();
    }

    shutdown = true;
    not_empty.notify_all();
    for (auto &t: threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadPool::worker() {
    std::function<void()> task;
    while (true) {
        {
            unique_lock<mutex> lock(pool_lock);
//                if (shutdown && taskQ.size() == 0) {
//                    return;
//                }
            //cout << "thread waiting..." << endl;
            not_empty.wait(lock, [this] { return shutdown || !tasks.empty(); });

            if (shutdown && tasks.empty()) {
                //之前没有加下面这句
                alive_num--;
                return;
            }
            //task = taskQ.takeTask();
            task = std::move(tasks.front());
            tasks.pop();
            busy_num++;
        }
        //cout << "thread start working..." << endl;
        task();
        //cout << "thread end working..." << endl;
        busy_num--;
    }
}

void ThreadPool::manager() {
    while (!manager_stopped) {
        //std::this_thread::_for(std::chrono::seconds(3));
        unique_lock<mutex> lock(pool_lock);
//        cout << "tasks.size(): " << tasks.size() << endl;
//        cout << "alive_num: " << alive_num << endl;
//        cout << "busy_num: " << busy_num << endl;
        not_empty.wait_for(lock, chrono::seconds(3), [this] {
            return shutdown || manager_stopped || (tasks.size() > alive_num && alive_num < max_num) ||
                   (busy_num * 2 < alive_num && alive_num > min_num);
        });
//            while (tasks.size() > alive_num && alive_num < max_num) {
//                threads.emplace_back(&ThreadPool::worker, this);
//                alive_num++;
//            }
//            // 销毁
//            while (busy_num * 2 < alive_num && alive_num > min_num) {
//                threads.back().detach();
//                threads.pop_back();
//                alive_num--;
//            }
        if (!shutdown && !manager_stopped) {
            while (tasks.size() > alive_num && alive_num < max_num) {
                cout << "扩容一个线程.." << endl;
                threads.emplace_back(&ThreadPool::worker, this);
                alive_num++;
            }
            while (busy_num * 2 < alive_num && alive_num > min_num) {
                cout << "销毁一个线程..." << endl;
//        threads.back().detach();
//        threads.pop_back();
//        alive_num--;
                for (auto it = threads.begin(); it != threads.end(); it++) {
                    if (!it->joinable()) {
                        it = threads.erase(it);
                        alive_num--;
                        break;
                    }
                }
            }
        }
    }
}

//void ThreadPool::addWorkers() {
//    while (tasks.size() > alive_num && alive_num < max_num) {
//        cout << "扩容一个线程.." << endl;
//        threads.emplace_back(&ThreadPool::worker, this);
//        alive_num++;
//    }
//}
//
//void ThreadPool::removeIdleWorkers() {
//    while (busy_num * 2 < alive_num && alive_num > min_num) {
//        cout << "销毁一个线程..." << endl;
////        threads.back().detach();
////        threads.pop_back();
////        alive_num--;
//        for (auto it = threads.begin(); it != threads.end(); it++) {
//            if (!it->joinable()) {
//                it = threads.erase(it);
//                alive_num--;
//                break;
//            }
//        }
//    }
//}