#ifndef TASK_PARALLEL_LIBRARY_H
#define TASK_PARALLEL_LIBRARY_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "concurrentqueue.h"



namespace TPL {
    class TaskPool {
    public:
        TaskPool(size_t);
        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args)
            ->std::future<typename std::result_of<F(Args...)>::type>;
        ~TaskPool();
        size_t threadCount() { return m_threadCount;}
    private:
        // need to keep track of threads so we can join them
        std::vector< std::thread > workers;
        // the task queue
        moodycamel::ConcurrentQueue< std::function<void()> > tasks;

        // synchronization
        //std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
        size_t m_threadCount;
    };

    // the constructor just launches some amount of workers
    inline TaskPool::TaskPool(size_t threads)
        : stop(false), m_threadCount(threads)
    {
        for (size_t i = 0;i < threads;++i) {
            workers.emplace_back(
                [this]
                {
                    std::mutex mutex;
                    for (;;)
                    {
                        std::function<void()> task = nullptr;

                        std::unique_lock<std::mutex> lock(mutex);
                        this->condition.wait(lock,
                            [this] { return this->stop || this->tasks.size_approx() > 0; });
                        if (this->stop && this->tasks.size_approx() == 0)
                            return;

                        if (this->tasks.try_dequeue(task) && task) {
                            task();
                            condition.notify_all();
                        }
                        else {
                            std::this_thread::yield();
                            if (this->tasks.try_dequeue(task) && task) {
                                task();
                                condition.notify_all();
                            }
                        }
                    }
                }
                );
        }
    }

    // add new work item to the pool
    template<class F, class... Args>
    auto TaskPool::enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        std::future<return_type> res = task->get_future();
        {
            //std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.enqueue([task]() { (*task)(); });
        }
        condition.notify_all();
        return res;
    }

    // the destructor joins all threads
    inline TaskPool::~TaskPool()
    {
        {
            //std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }


}


#endif