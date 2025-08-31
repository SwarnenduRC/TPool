#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "Task.hpp"

namespace t_pool
{
    class ThreadPool
    {
        using ui32 = std::uint_fast32_t;
        using ui64 = std::uint_fast64_t;

        private:
            /**
             * @brief An uinque pointer to manage threads in the pool
             */
            std::unique_ptr<std::thread[]> m_pThreads;
            /**
             * @brief The size of the thread pool.
             * By default it is max allowed on the system
             * denoted by std::thread::hardware_concurrency()
             */
            ui32 m_poolSize = std::thread::hardware_concurrency();
            /**
             * @brief The no. of tasks still unfinished
             * The no. of tasks either in the queue or
             * still running
             */
            std::atomic<ui64> m_taskCntTotal;
            /**
             * @brief A mutex to protect the task queue.
             */
            std::mutex m_taskQueueMtx = {};
            /**
             * @brief A queue of tasks.
             * The queue holds the tasks and its respective id
             * to be executed by the threads in the pool.
             */
            std::queue<std::pair<std::function<void()>, uint32_t>> m_taskQueue;
            /**
             * @brief An atomic variable to indicate if the worker
             * threads should continue running/picking up the tasks
             * from the queue. Once set to FALSE the workers STOPPED.
             */
            std::atomic_bool m_taskRunning;
            /**
             * @brief An atomic bool variable to tell the workers
             * to pause for while from popping up the tasks from
             * the queue (if set to TRUE). The tasks already been
             * popped up will be continued to be worked upon.
             */
            std::atomic_bool m_pause;
            /**
             * @brief The duration for which a thread should take a nap.
             * Initially set to ZERO by default so no NAP by default.
             */
            ui32 m_sleepDuration = 0;
    }; 
} // namespace t_pool

#endif  // THREAD_POOL_HPP

