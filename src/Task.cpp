#include "Task.hpp"

using namespace t_pool;

/*static*/ int Task::nextTaskId() noexcept
{
    LOG_ENTRY_DBG();
    static std::atomic<int> taskCnt(0);
    return taskCnt.fetch_add(1);
    LOG_EXIT_DBG();
}