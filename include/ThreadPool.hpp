/**
 * @file ThreadPool.hpp
 * @brief ThreadPool class header for managing a pool of worker threads and task scheduling.
 *
 * This file defines the t_pool::ThreadPool class, which provides a thread pool implementation
 * for concurrent execution of tasks. The thread pool manages a fixed number of worker threads,
 * allows submitting tasks, and handles synchronization and task completion.
 *
 * Purpose:
 * - Efficiently execute multiple tasks concurrently using a pool of threads.
 * - Manage task queueing, execution, and thread lifecycle.
 * - Provide thread-safe interfaces for submitting tasks and controlling the pool.
 *
 * Copyright (c) 2025 Swarnendu RC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
            /**
             * @brief Construct a new Thread Pool object
             * Default constructor that initializes the thread pool
             * with the number of threads equal to the hardware concurrency
             * supported by the system.
             */
            ThreadPool()
                : m_poolSize(std::thread::hardware_concurrency())
                , m_pThreads(std::make_unique<std::thread[]>(m_poolSize))
                , m_taskCntTotal(0)
                , m_taskRunning(true)
                , m_pause(false)
            {
                createThreads();
            }

            /**
             * @brief Constructs a ThreadPool with a specified number of threads.
             * 
             * If no size is provided, the pool size defaults to the number of hardware
             * concurrency supported by the system.
             * 
             * @param [in] poolSize The number of threads in the pool. Defaults to hardware concurrency.
             */
            ThreadPool(const ui32 poolSize)
                : m_poolSize(poolSize)
                , m_pThreads(std::make_unique<std::thread[]>(m_poolSize))
                , m_taskCntTotal(0)
                , m_taskRunning(true)
                , m_pause(false)
            {
                createThreads();
            }

            /**
             * @brief Destroy the Thread Pool object
             * Destructor that stops all worker threads and cleans up resources.
             * It waits for all tasks to complete before shutting down the threads.
             */
            ~ThreadPool()
            {
                waitForTaskCompletion();
                m_taskRunning = false;
                destroyThreads();
            }

            /**
             * @brief Reset the thread pool with a new size.
             * This method stops all current worker threads, waits for any running tasks to complete,
             * and then recreates the thread pool with the specified new size.
             * It is thread-safe and can be called at any time.
             * 
             * @note If there are pending tasks in the queue, this method will wait for them to complete
             * 
             * @param [in] newPoolSize The new size for the thread pool.
             * @note The new pool size must be greater than zero.
             */
            void reset(const ui32 newPoolSize)
            {
                waitForTaskCompletion();
                auto pauseStatus = m_pause.load();  // save current pause status
                m_pause = true;
                m_taskRunning = false;  // signal threads to stop
                destroyThreads();
                m_poolSize = newPoolSize;
                LOG_ASSERT(m_poolSize > 0); // pool size must be > 0
                createThreads();
                m_pause = pauseStatus;  // restore previous pause status
                m_taskRunning = true;
            }

            /**
             * @brief Get the Task Running Cnt
             * It returns the number of tasks currently being executed by the worker threads.
             * This is calculated as the total number of tasks minus the number of tasks still queued.
             * 
             * @return ui32 The number of tasks currently running.
             * @note This value is approximate and may change as tasks complete or new tasks are added
             */
            inline ui32 getTaskRunningCnt() noexcept 
                { return static_cast<ui32>(m_taskCntTotal - getTaskQueued()); }

            /**
             * @brief Get the Total Task Cnt object
             * Returns the total number of tasks that have been submitted to the thread pool
             * 
             * @return ui64 The total number of tasks submitted to the pool.
             * @note This includes both tasks that are currently running and those that are still queued.
             */
            inline ui64 getTotalTaskCnt() const noexcept { return m_taskCntTotal; }

            /**
             * @brief Get the Task Queued object
             * Returns the number of tasks currently queued in the thread pool.
             * These are tasks that have been submitted but not yet picked up by any worker thread for execution.
             * 
             * @return ui64 The number of tasks currently in the queue.
             * @note This value is approximate and may change as tasks are picked up by worker threads.
             */
            ui64 getTaskQueued() noexcept
            {
                std::lock_guard<std::mutex> queueLock(m_taskQueueMtx);
                return m_taskQueue.size();
            }

            /**
             * @brief Submits a task to the thread pool for execution.
             * This method accepts a callable (function, lambda, functor) and its arguments,
             * creates a Task object to encapsulate the callable, and adds it to the task queue.
             * A future associated with the task's result is returned to the caller.
             * 
             * @tparam F The type of the callable (function, lambda, functor).
             * @tparam A The types of the arguments to pass to the callable.
             * @param [in] func The callable to be executed.
             * @param [in] args The arguments to pass to the callable.
             * @return std::future<std::any> The future associated with the task's result.
             */
            template<typename F, typename ...A>
            std::future<std::any> submit(F&& func, A&& ...args)
            {
                // A shared pointer to manage task memory.
                // The task will be automatically cleaned up when no longer needed
                // but will be kept alive as long as there are references to it.
                auto pTask = std::make_shared<Task>();
                pTask->submit(std::forward<F>(func), std::forward<A>(args)...);
                m_taskQueue.emplace(pTask);
                ++m_taskCntTotal;
                return pTask->getTaskFuture();
            }

        private:
            /**
             * @brief The worker function executed by each thread in the pool.
             * Each worker thread runs this function in a loop, continuously checking for new tasks
             * to execute from the task queue. If a task is available, it is executed; otherwise,
             * the thread sleeps or yields to avoid busy-waiting.
             */
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

            /**
             * @brief Pops a task from the task queue in a thread-safe manner.
             * If the pool is not paused and there are tasks in the queue, the front task
             * is removed from the queue and returned via the pTask parameter.
             * 
             * @param [out] pTask A shared pointer to hold the popped task.
             * @return true if a task was successfully popped; false otherwise.
             */
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

            /**
             * @brief Waits for all tasks in the pool to complete.
             * This method blocks until there are no remaining tasks in the pool.
             * It checks both the queued tasks and the currently running tasks,
             * ensuring that all tasks have finished before returning.
             */
            void waitForTaskCompletion()
            {
                while (true)
                {
                    if (!m_pause) // if not paused, check both queued and running tasks
                    {
                        if (getTotalTaskCnt() == 0)
                            break;
                    }
                    else    // if paused, only check running tasks
                    {
                        if (getTaskRunningCnt() == 0)
                            break;
                    }
                    sleepOrYield();
                }
            }

            /**
             * @brief Creates the worker threads for the thread pool.
             * This method initializes the array of threads and starts each thread,
             * assigning them to execute the worker function. If the pool size is zero,
             * an assertion failure occurs.
             */
            void createThreads()
            {
                if (m_poolSize)
                {
                    m_pThreads = std::make_unique<std::thread[]>(m_poolSize);
                    // Start each thread, assigning it to the worker function
                    // which will continuously look for and execute tasks.
                    for (ui32 idx = 0; idx < m_poolSize; ++idx)
                        m_pThreads[idx] = std::thread(&ThreadPool::worker, this);
                }
                else
                {
                    LOG_ASSERT_MSG(m_poolSize > 0, "Thread pool size {:d} not defined", m_poolSize);
                }
            }

            /**
             * @brief Joins and cleans up all worker threads in the pool.
             * This method ensures that all threads are properly joined before
             * the thread pool is destroyed or reset. It checks if each thread
             * is joinable before calling join to avoid any exceptions.
             */
            void destroyThreads()
            {
                for (ui32 idx = 0; idx < m_poolSize; ++idx)
                {
                    if (m_pThreads[idx].joinable())
                        m_pThreads[idx].join();
                }
            }

            /**
             * @brief Sleeps or yields the current thread to avoid busy-waiting.
             * If a sleep duration is configured, the thread sleeps for that duration.
             * Otherwise, it yields its execution to allow other threads to run.
             */
            inline void sleepOrYield()
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

