#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "../connection/tcp_conn.h"
#include "../lock/locker.h"
#include "../utils.h"

using namespace std;

// class FutureThreadPool
// {
// public:
//     FutureThreadPool(size_t);
//     template <class F, class... Args>
//     auto Enqueue(F &&f, Args &&...args)
//         -> std::future<typename std::result_of<F(Args...)>::type>;
//     ~FutureThreadPool();

// private:
//     std::vector<std::thread> workers_;
//     std::queue<std::function<void()>> tasks_;
//     std::mutex queue_mutex_;
//     std::condition_variable condition_;
//     bool stop_;
// };

// inline FutureThreadPool::FutureThreadPool(size_t threads)
//     : stop_(false)
// {
//     for (size_t i = 0; i < threads; ++i)
//         workers_.emplace_back(
//             [this] {
//                 for (;;)
//                 {
//                     std::function<void()> task;

//                     {
//                         std::unique_lock<std::mutex> lock(this->queue_mutex_);
//                         this->condition_.wait(lock,
//                                              [this] { return this->stop_ || !this->tasks_.empty(); });
//                         if (this->stop_ && this->tasks_.empty())
//                             return;
//                         task = std::move(this->tasks_.front());
//                         this->tasks_.pop();
//                     }
//                     task();
//                 }
//             });
// }

// // Adds new work item to the pool
// template <class F, class... Args>
// auto FutureThreadPool::Enqueue(F &&f, Args &&...args)
//     -> std::future<typename std::result_of<F(Args...)>::type>
// {
//     using return_type = typename std::result_of<F(Args...)>::type;

//     auto task = std::make_shared<std::packaged_task<return_type()>>(
//         std::bind(std::forward<F>(f), std::forward<Args>(args)...));

//     std::future<return_type> res = task->get_future();
//     {
//         std::unique_lock<std::mutex> lock(queue_mutex_);

//         // don't allow enqueueing after stopping the pool
//         if (stop_)
//             throw std::runtime_error("enqueue on stopped FutureThreadPool");

//         tasks_.emplace([task]() { (*task)(); });
//     }
//     condition_.notify_one();
//     return res;
// }

// // The destructor joins all threads
// inline FutureThreadPool::~FutureThreadPool()
// {
//     {
//         std::unique_lock<std::mutex> lock(queue_mutex_);
//         stop_ = true;
//     }
//     condition_.notify_all();
//     for (std::thread &worker : workers_)
//         worker.join();
// }

// template <typename T>
// class WorkerThreadPool
// {
// public:
//     explicit WorkerThreadPool(int thread_number, uint64_t* pointer_chasing_mem_data);
//     ~WorkerThreadPool();
//     bool Append(T *request);

// private:
//     static void *Work(void *arg);
//     void Run();

// private:
//     pthread_t* threads_;
//     std::list<T*> request_queue_;
//     locker request_queue_lock_;
//     sem request_queue_sem_;
//     uint64_t* pointer_chasing_mem_data_;
//     uint64_t** mem_data_array_;
// };

// template <typename T>
// WorkerThreadPool<T>::WorkerThreadPool(int thread_number, uint64_t* pointer_chasing_mem_data) : pointer_chasing_mem_data_(pointer_chasing_mem_data), threads_(NULL)
// {
//     if (thread_number <= 0)
//         throw std::exception();

//     threads_ = new pthread_t[thread_number];
//     mem_data_array_ = new uint64_t*[thread_number];

//     if (!threads_)
//         throw std::exception();
//     for (int i = 0; i < thread_number; ++i)
//     {
//         mem_data_array_[i] = new uint64_t[256 * 1024 * 1024 / 8];
//         if (pthread_create(threads_ + i, NULL, Work, this) != 0)
//         {
//             delete[] threads_;
//             throw std::exception();
//         }
//         if (pthread_detach(threads_[i]))
//         {
//             delete[] threads_;
//             throw std::exception();
//         }
//     }
// }

// template <typename T>
// WorkerThreadPool<T>::~WorkerThreadPool()
// {
//     delete[] threads_;
//     for (int i = 0; i < thread_number_; ++i)
//         delete[] mem_data_array_[i];
//     delete[] mem_data_array_;
// }

// template <typename T>
// bool WorkerThreadPool<T>::Append(T* request)
// {
//     request_queue_lock_.lock();
//     request_queue_.push_back(request);
//     request_queue_lock_.unlock();
//     request_queue_sem_.post();
//     return true;
// }

// template <typename T>
// void *WorkerThreadPool<T>::Work(void *arg)
// {
//     WorkerThreadPool* worker_thread_pool = (WorkerThreadPool *)arg;
//     worker_thread_pool->Run();
//     return worker_thread_pool;
// }

// template <typename T>
// void WorkerThreadPool<T>::Run()
// {
//     uint64_t* curr_mem_addrs = new uint64_t[23];
//     uint64_t cache_init_size = 64;
//     uint64_t cache_max_size = 256 * 1024 * 1024;

//     uint64_t curr_idx = 0;
//     for (int i = 0; i < 23; i++) {
//         curr_mem_addrs[i] = (uint64_t)&mem_data_[curr_idx];
//         curr_idx = (cache_init_size << i) / 8;
//     }
//     uint64_t request_id = 0;

//     while (true)
//     {
//         request_queue_sem_.wait();
//         request_queue_lock_.lock();
//         if (request_queue_.empty())
//         {
//             request_queue_lock_.unlock();
//             continue;
//         }
//         T* request = request_queue_.front();
//         request_queue_.pop_front();
//         request_queue_lock_.unlock();
//         if (!request)
//             continue;
//         request->Process(curr_mem_addrs, request_id);
//     }
// }

template <typename T>
class NetworkThread
{
public:
    // WorkerThreadPool<T>* worker_thread_pool_;
    // FutureThreadPool* future_thread_pool_;
    T* conns_;
    int epoll_fd_ = -1;
    int* epoll_fds_;
    int thread_index_;
    bool dispatch_;
    uint64_t* pointer_chasing_mem_data_;

    NetworkThread(T* conns, int thread_index, int* epoll_fds, bool dispatch,  uint64_t* pointer_chasing_mem_data)
    {
        conns_ = conns;
        epoll_fds_ = epoll_fds;
        thread_index_ = thread_index;
        dispatch_ = dispatch;
        // worker_thread_pool_ = worker_thread_pool;
        // future_thread_pool_ = future_thread_pool;
        pointer_chasing_mem_data_ = pointer_chasing_mem_data;
    };

    static void *Work(void *args)
    {
        NetworkThread* network_thread = (NetworkThread*)args;
        network_thread->Run();
        return network_thread;
    }

    void Run()
    {
        epoll_event events[MAX_EVENT_NUMBER];
        epoll_fd_ = epoll_create(5);
        assert(epoll_fd_ != -1);
        epoll_fds_[thread_index_] = epoll_fd_;

        uint64_t curr_idx = 0;
        uint64_t cache_init_size = 64;
        uint64_t cache_max_size = 256*1024*1024;

        uint64_t* curr_mem_addrs = new uint64_t[23];
        uint64_t* curr_pointer_chasing_mem_addrs = new uint64_t[23];
        uint64_t* mem_data = new uint64_t[cache_max_size / 8];
        
        for (int i = 0; i < 23; i++) {
            curr_mem_addrs[i] = (uint64_t) &mem_data[curr_idx];
            curr_pointer_chasing_mem_addrs[i] = (uint64_t) &pointer_chasing_mem_data_[curr_idx];
            curr_idx = (cache_init_size << i) / 8; // 8 bytes per uint64_t
        }

        uint64_t request_id = 0;

        bool stop = false;
        while (!stop)
        {
            int number = epoll_wait(epoll_fd_, events, MAX_EVENT_NUMBER, -1);
            if (number < 0 && errno != EINTR)
            {
                printf("%s", "worker thread epoll failure");
                break;
            }

            for (int i = 0; i < number; i++)
            {
                int socket_fd = events[i].data.fd;
                if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
                {
                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_fd, 0);
                    close(socket_fd);
                }
                else if (events[i].events & EPOLLIN)
                {
                    T* conn = conns_ + socket_fd;
                    conn->ReadOnce();
                    if (!dispatch_)
                    {
                        conn->Process(mem_data, curr_mem_addrs, curr_pointer_chasing_mem_addrs, request_id);
                    }
                    else
                    {
                        // worker_thread_pool_->Append(conn);
                    }
                }
                else if (events[i].events & EPOLLOUT)
                {
                    (conns_ + socket_fd)->WriteOnce();
                }
            }
        }
    }
    ~NetworkThread(){};
};

template <typename T>
class NetworkThreadPool
{
public:
    pthread_t* threads_;
    vector<NetworkThread<T>> network_threads_;

    NetworkThreadPool(T* conns, int* epoll_fds, int thread_number, bool dispatch, uint64_t* pointer_chasing_mem_data)
    {
        if (thread_number <= 0)
            throw std::exception();
        threads_ = new pthread_t[thread_number];

        for (int i = 0; i < thread_number; ++i)
        {
            network_threads_.push_back(NetworkThread<T>(conns, i, epoll_fds, dispatch, pointer_chasing_mem_data));
        }

        if (!threads_)
            throw std::exception();
        for (int i = 0; i < thread_number; ++i)
        {
            pthread_create(threads_ + i, NULL, NetworkThread<T>::Work, &network_threads_[i]);
            pthread_detach(threads_[i]);
        }
    }
    ~NetworkThreadPool()
    {
        delete[] threads_;
        vector<NetworkThread<T>>().swap(network_threads_);
    }
};

#endif
