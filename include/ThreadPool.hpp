#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "Task.hpp"

namespace t_pool
{
    using ui32 = std::uint_fast32_t;
    using ui64 = std::uint_fast64_t;
    class ThreadPool
    {
        public:
            ThreadPool()
                : m_poolSize(std::thread::hardware_concurrency())
                , m_pThreads(std::make_unique<std::thread[]>(m_poolSize))
                , m_taskCntTotal(0)
                , m_taskRunning(true)
                , m_pause(false)
            {
                createThreads();
            }

            ThreadPool(const ui32 poolSize)
                : m_poolSize(poolSize)
                , m_pThreads(std::make_unique<std::thread[]>(m_poolSize))
                , m_taskCntTotal(0)
                , m_taskRunning(true)
                , m_pause(false)
            {
                createThreads();
            }

            ~ThreadPool()
            {
                waitForTaskCompletion();
                m_taskRunning = false;
                destroyThreads();
            }

            void reset(const ui32 newPoolSize)
            {
                waitForTaskCompletion();
                auto pauseStatus = m_pause.load();
                m_pause = true;
                m_taskRunning = false;
                destroyThreads();
                m_poolSize = newPoolSize;
                LOG_ASSERT(m_poolSize > 0);
                createThreads();
                m_pause = pauseStatus;
                m_taskRunning = true;
            }

            inline ui32 getTaskRunningCnt() noexcept 
                { return static_cast<ui32>(m_taskCntTotal - getTaskQueued()); }

            inline ui64 getTotalTaskCnt() const noexcept { return m_taskCntTotal; }

            ui64 getTaskQueued() noexcept
            {
                std::lock_guard<std::mutex> queueLock(m_taskQueueMtx);
                return m_taskQueue.size();
            }

            template<typename F, typename ...A>
            std::future<std::any> submit(F&& func, A&& ...args)
            {
                auto pTask = std::make_shared<Task>();
                pTask->submit(std::forward<F>(func), std::forward<A>(args)...);
                m_taskQueue.emplace(pTask);
                ++m_taskCntTotal;
                return pTask->getTaskFuture();
            }

        private:
            void worker()
            {
                while (m_taskRunning)
                {
                    std::shared_ptr<Task> pTask;
                    if (!m_pause && popTask(pTask))
                    {
                        if (pTask.get())
                        {
                            auto taskFunc = pTask->toFunction();
#if defined (DEBUG) || (__DEBUG__)
                            auto taskId = pTask->getTaskId();
                            std::ostringstream oss;
                            oss << std::this_thread::get_id();
                            LOG_DBG("Task(ID) {:d} is now going to be executed by the thread {}",
                                taskId, oss.str());
#endif

                            taskFunc();
                            --m_taskCntTotal;
#if defined (DEBUG) || (__DEBUG__)
                            LOG_DBG("Task(ID) {:d} execution completed now by the thread {}",
                                taskId, oss.str());
#endif
                        }
                    }
                    else
                    {
                        sleepOrYield();
                    }
                }
            }
            bool popTask(std::shared_ptr<Task>& pTask)
            {
                if (!m_pause)
                {
                    std::lock_guard<std::mutex> lock(m_taskQueueMtx);
                    if (!m_pause && !m_taskQueue.empty())
                    {
#if defined (DEBUG) || (__DEBUG__)
                        std::ostringstream oss;
                        oss << std::this_thread::get_id();
                        LOG_DBG("Task with task ID {:d} popped up from the queue by the thread {}",
                            m_taskQueue.front()->getTaskId(), 
                            oss.str());
#endif
                        pTask = m_taskQueue.front();
                        m_taskQueue.pop();
                        return true;
                    }
                }
                return false;
            }
            void waitForTaskCompletion()
            {
                while (true)
                {
                    if (!m_pause)
                    {
                        if (getTotalTaskCnt() == 0)
                            break;
                    }
                    else
                    {
                        if (getTaskRunningCnt() == 0)
                            break;
                    }
                    sleepOrYield();
                }
            }
            void createThreads()
            {
                if (m_poolSize)
                {
                    m_pThreads = std::make_unique<std::thread[]>(m_poolSize);
                    for (ui32 idx = 0; idx < m_poolSize; ++idx)
                        m_pThreads[idx] = std::thread(&ThreadPool::worker, this);
                }
                else
                {
                    LOG_ASSERT_MSG(m_poolSize > 0, "Thread pool size {:d} not defined", m_poolSize);
                }
            }
            void destroyThreads()
            {
                for (ui32 idx = 0; idx < m_poolSize; ++idx)
                {
                    if (m_pThreads[idx].joinable())
                        m_pThreads[idx].join();
                }
            }
            void sleepOrYield()
            {
                if (m_sleepDuration)
                    std::this_thread::sleep_for(std::chrono::microseconds(m_sleepDuration));
                else
                    std::this_thread::yield();
            }
            /**
             * @brief The size of the thread pool.
             * By default it is max allowed on the system
             * denoted by std::thread::hardware_concurrency()
             */
            ui32 m_poolSize = std::thread::hardware_concurrency();
            /**
             * @brief An uinque pointer to manage threads in the pool
             */
            std::unique_ptr<std::thread[]> m_pThreads;
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
            std::queue<std::shared_ptr<Task>> m_taskQueue;
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

