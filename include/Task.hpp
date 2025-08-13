#ifndef TASK_HPP
#define TASK_HPP

#include <logger/LOGGER_MACROS.hpp>

#include <future>
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
                LOG_ENTRY_DBG();
                static std::atomic<int> taskCnt(0);
                return taskCnt.fetch_add(1);
                LOG_EXIT_DBG();
            }
            Task() = default;

            inline int getTaskId() const { return m_taskId; }
            inline std::future<std::any> getTaskFuture() { return std::move(m_future); }
            inline std::function<void()> toFunction()
            {
                return [this]() mutable { this->runAndForget(); };
            }

            template <typename F, typename ...Args>
            void submit(F&& f, Args&& ...)
            {
                using Result = std::invoke_result_t<F, Args...>;
                auto boundFunc = std::bind(std::forward<F>(f), std::forward<Args>()...);
                std::packaged_task<std::any(Args...)> packagedTask(
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
                if (m_task.valid())
                {
                    m_task();
                    auto result = m_future.get();
                    return result;
                }
                LOG_EXIT_DBG();
                return std::any{};
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

        protected:
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