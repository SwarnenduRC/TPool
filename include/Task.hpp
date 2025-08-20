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
    class Task
    {
        public:
            static int nextTaskId() noexcept
            {
                static std::atomic<int> taskCnt(0);
                return taskCnt.fetch_add(1);
            }
            Task() = default;

            inline int getTaskId() const { return m_taskId; }
            inline std::future<std::any> getTaskFuture() { return std::move(m_future); }
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
                using Result = std::invoke_result_t<F, Args...>;
                auto boundFunc = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
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
                m_future = packagedTask.get_future();
                m_task = std::move(packagedTask);
                m_taskId = nextTaskId();
            }

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

            void runAndForget()
            {
                LOG_ENTRY_DBG();
                if (m_task.valid())
                {
                    m_task();
                }
                LOG_EXIT_DBG();
            }
            inline void setTaskName(std::string_view taskName) noexcept { m_taskName = taskName; }
            inline std::string getTaskName() const noexcept { return m_taskName; }

        private:
            std::packaged_task<std::any()> m_task;
            std::future<std::any> m_future;
            int m_taskId = -1;
            std::string m_taskName;
    };
};   // namespace t_pool

#endif  // TASK_HPP

