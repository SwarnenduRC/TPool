#include "ThreadPool.hpp"

using namespace t_pool;

void ThreadPool::reset(const ui32 newPoolSize)
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

