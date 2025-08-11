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
            static int nextTaskId() noexcept;
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
                std::packaged_task<std::any()> packagedTask = [boundFunc]() -> std::any
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
                };
                m_future = packagedTask.get_future();
                m_task = std::move(packagedTask);
                m_taskId = nextTaskId();
            }

            std::any run()
            {
                if (m_task.valid())
                {
                    m_task();
                    auto result = m_future.get();
                    return result;
                }
                return std::any{};
            }

            void runAndForget()
            {
                if (m_task.valid())
                {
                    m_task();
                }
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