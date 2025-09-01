#include "ThreadPool.hpp"

using namespace t_pool;

void ThreadPool::createThreads()
{
    if (m_poolSize)
    {
        for (ui32 idx = 0; idx < m_poolSize; ++idx)
            m_pThreads[idx] = std::thread(&ThreadPool::worker, this);
    }
    else
    {
        LOG_ASSERT_MSG(m_poolSize > 0, "Thread pool size {:d} nor defined", m_poolSize);
    }
}

void ThreadPool::worker()
{
    while (m_taskRunning)
    {
        std::pair<std::function<void()>, uint32_t> task;
        if (!m_pause && popTask(task))
        {
            auto taskFunc = std::move(task.first);
            auto taskId = std::move(task.second);
            LOG_DBG("Task(ID) {:d} is now going to be executed by the thread {:#08x}",
                taskId, std::hash<std::thread::id>{}(std::this_thread::get_id()));
            taskFunc();
            --m_taskCntTotal;
            LOG_DBG("Task(ID) {:d} execution completed now by the thread",
                taskId, std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }
        else
        {
            sleepOrYield();
        }
    }
}

bool ThreadPool::popTask(std::pair<std::function<void()>, uint32_t>& task)
{
    if (!m_pause)
    {
        std::scoped_lock<std::mutex> lock(m_taskQueueMtx);
        if (!m_pause && !m_taskQueue.empty())
        {
            LOG_DBG("Task with task ID {:d} popped up from the queue by the thread {:#08x}",
                m_taskQueue.front().second,
                std::hash<std::thread::id>{}(std::this_thread::get_id()));
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
            return true;
        }
    }
    return false;
}

void ThreadPool::sleepOrYield()
{
    if (m_sleepDuration)
        std::this_thread::sleep_for(std::chrono::microseconds(m_sleepDuration));
    else
        std::this_thread::yield();
}

