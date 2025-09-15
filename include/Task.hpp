/**
 * @file Task.hpp
 * @brief Task abstraction for thread pool implementation.
 *
 * Copyright (c) 2025 SwarnenduRC
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

#ifndef TASK_HPP
#define TASK_HPP

#include <logger/LOGGER_MACROS.hpp>

#include <future>
#include <functional>
#include <type_traits>
#include <any>
#include <utility>
#include <atomic>

using namespace logger;

namespace t_pool
{
    /**
     * @class Task
     * @brief Represents a unit of work that can be executed asynchronously and tracked via a unique task ID.
     *
     * The Task class encapsulates a callable object (function, lambda, or functor) along with its arguments,
     * allowing it to be submitted for asynchronous execution. The result of the execution is stored as a std::any
     * and can be retrieved via a future. Each Task is assigned a unique ID for identification and tracking.
     *
     * Key features:
     * - Submission of any callable with arguments using the submit() method.
     * - Execution of the task and retrieval of its result using run() or runAndForget().
     * - Conversion to a std::function for flexible scheduling.
     * - Assignment and retrieval of a human-readable task name.
     * - Thread-safe generation of unique task IDs.
     *
     * @note The callable and its arguments must be copyable types, as they are stored in a std::packaged_task.
     *       Move semantics are not supported for return and argument types.
     */
    class Task
    {
        public:
            /** Default constructor and destructor */
            Task() = default;
            ~Task() = default;

            /**
             * @brief Move constructor to transfer ownership of a Task.
             * 
             * @param rhs The Task instance to move from.
             */
            Task(Task&& rhs)
            {
                m_task = std::move(rhs.m_task);
                m_future = std::move(rhs.m_future);
                m_taskName = std::move(rhs.m_taskName);
                m_taskId.store(rhs.m_taskId);
                rhs.m_taskId.store(0);
            }

            /**
             * @brief Generates a unique task ID in a thread-safe manner.
             * 
             * @return int The next unique task ID.
             */
            static uint32_t nextTaskId() noexcept
            {
                static std::atomic<uint32_t> taskCnt(0);
                return taskCnt.fetch_add(1);
            }

            /**
             * @brief Get the Task Id object
             * Gets the unique identifier for this task instance.
             * 
             * @return int The next unique task ID.
             */
            int getTaskId() noexcept { return m_taskId; }

            /**
             * @brief Get the Task Future object
             * Retrieves the future associated with the task's execution result.
             * 
             * @return std::future<std::any>  The future holding the task's result.
             * @note The future is moved out, so it can only be retrieved once.
             */
            inline std::future<std::any> getTaskFuture() { return std::move(m_future); }

            /**
             * @brief Converts the task into a callable std::function that executes the task asynchronously.
             * 
             * @return std::function<void()> A callable that, when invoked, runs the task and ignores its result.
             */
            inline std::function<void()> toFunction()
            {
                return [this]() mutable { this->runAndForget(); };
            }

            /**
             * @brief Submits a callable task with arguments to be executed asynchronously.
             *
             * This template method accepts any callable object (function, lambda, functor) and its arguments,
             * binds them together, and wraps the invocation in a std::packaged_task that returns a std::any.
             * The result type is deduced using std::invoke_result_t. If the callable returns void, an empty
             * std::any is returned; otherwise, the result is returned as std::any.
             *
             * The future associated with the packaged task is stored in m_future, and the packaged task itself
             * is stored in m_task for later execution. A unique task ID is assigned via nextTaskId().
             *
             * @tparam F Type of the callable object.
             * @tparam Args Types of the arguments to pass to the callable.
             * @param f The callable object to execute.
             * @param args Arguments to pass to the callable object.
             *
             * @note Please keep in mind that the return and argument types of the callable must be copyable,
             * as it will be stored in a packaged_task.
             * What it means is either you can have premitive types or types that have copy constructor defined.
             * Or, a derived class that implements the copy constructor. Move semantics are not supported here
             * for return and arguments types. So use std shared_ptr in place of unique_ptr or raw pointers.
             */
            template <typename F, typename ...Args>
            void submit(F&& f, Args&&... args)
            {
                LOG_ENTRY_DBG();
                using Result = std::invoke_result_t<F, Args...>;
                auto boundFunc = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
                // Wrap the bound function in a packaged_task that returns std::any
                // The lambda inside the packaged_task handles both void and non-void return types
                // by checking if Result is void at compile time. Use constructor as assignment or
                // copy initialization does not work with std::packaged_task.
                std::packaged_task<std::any()> packagedTask(
                [boundFunc]() -> std::any
                {
                    if constexpr (std::is_void_v<Result>)
                    {
                        boundFunc();
                        return std::any{};
                    }
                    else
                    {
                        return boundFunc();
                    }
                });
                m_taskId.store(nextTaskId());
                m_future = packagedTask.get_future();
                m_task = std::move(packagedTask);
                LOG_EXIT_DBG();
            }

            /**
             * @brief Executes the submitted task and retrieves its result.
             *
             * This method runs the task if it has been submitted and is valid. It waits for the task to complete
             * and retrieves the result from the associated future. The result is returned as a std::any.
             * If the task has not been submitted or is invalid, an empty std::any is returned.
             *
             * @return std::any The result of the task execution, or an empty std::any if the task is invalid.
             */
            std::any run()
            {
                LOG_ENTRY_DBG();
                std::any result;
                if (m_task.valid())
                {
                    m_task();
                    result = m_future.get();
                }
                LOG_EXIT_DBG();
                return result;
            }

            /**
             * @brief Executes the submitted task without waiting for its result.
             *
             * This method runs the task if it has been submitted and is valid. It does not wait for the task
             * to complete or retrieve its result. If the task has not been submitted or is invalid, this method
             * does nothing.
             */
            void runAndForget()
            {
                LOG_ENTRY_DBG();
                if (m_task.valid())
                {
                    m_task();
                }
                LOG_EXIT_DBG();
            }

            /**
             * @brief Sets the human-readable name for the task.
             * 
             * @param taskName The name to assign to the task.
             */
            inline void setTaskName(std::string_view taskName) noexcept { m_taskName = taskName; }

            /**
             * @brief Gets the human-readable name of the task.
             * 
             * @return std::string The name assigned to the task.
             */
            inline std::string getTaskName() const noexcept { return m_taskName; }

        private:
            std::packaged_task<std::any()> m_task;
            std::future<std::any> m_future;
            std::atomic<uint32_t> m_taskId = 0;
            std::string m_taskName;
    };

};   // namespace t_pool

#endif  // TASK_HPP

