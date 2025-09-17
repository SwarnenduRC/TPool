#include "TaskDAG.hpp"

using namespace t_pool;

TDAG& TDAG::addTask(Task&& task)
{
    LOG_ENTRY_DBG();
    if (!m_taskIdMap.count(task.getTaskId()))
    {
        m_task = std::shared_ptr<Task>(new Task(std::move(task)));
        m_taskIdMap[m_task->getTaskId()] = std::make_pair(m_task, 0);
        m_taskGraph[m_task->getTaskId()] = std::vector<uint32_t>{};
        m_taskFuture = m_task->getTaskFuture();
    }
    else
    {
        LOG_INFO("Task {:d} is already added earlier", task.getTaskId());
    }
    LOG_EXIT_DBG();
    return *this;
}

TDAG& TDAG::addDependency(Task&& task)
{
    LOG_ENTRY_DBG();
    auto itr = m_taskIdMap.find(m_task->getTaskId());
    if (itr != m_taskIdMap.end())
    {
        auto& inDegree = itr->second.second;
        auto graphItr = m_taskGraph.find(itr->first);
        auto& dependencyVec = graphItr->second;
        auto depItr = std::find(dependencyVec.cbegin(), 
                                dependencyVec.cend(), 
                                task.getTaskId());

        if (dependencyVec.cend() == depItr)
        {
            auto pTask = std::shared_ptr<Task>(new Task(std::move(task)));
            m_taskIdMap[pTask->getTaskId()] = std::make_pair(pTask, 0);
            dependencyVec.emplace_back(pTask->getTaskId());
            ++inDegree;
        }
        else
        {
            LOG_INFO("Dependency {:d} has already been added", task.getTaskId());
        }
    }
    else
    {
        LOG_ERR("The parent task is not been added, so the dependency can't be added either");
    }
    LOG_EXIT_DBG();
    return *this;
}

TDAG& TDAG::removeDependency(Task&& task)
{
    LOG_ENTRY_DBG();
    auto itr = m_taskIdMap.find(m_task->getTaskId());
    if (itr != m_taskIdMap.end())
    {
        auto graphItr = m_taskGraph.find(itr->first);
        auto& dependencyVec = graphItr->second;
        auto depItr = std::find(dependencyVec.cbegin(), 
                                dependencyVec.cend(), 
                                task.getTaskId());
        
        if (dependencyVec.cend() == depItr)
        {
            LOG_ERR("Dependency {:d} for task {:d} can't be found. Not removing anything", 
                    task.getTaskId(), m_task->getTaskId());
        }
        else
        {
            if (!removeDependencyRecurs(task.getTaskId()))
            {
                throw std::logic_error("Dependency removal unsuccessful");
            }
        }
    }
    else
    {
        LOG_ERR("Dependency for task {:d} can't be removed as the task itself is not added");
    }
    LOG_EXIT_DBG();
    return *this;
}

bool TDAG::removeDependencyRecurs(const uint32_t taskId)
{
    LOG_ENTRY_DBG();
    auto retVal = false;
    auto itr = m_taskIdMap.find(taskId);
    if (itr == m_taskIdMap.end())
    {
        LOG_ERR("Task {:d} can't be found to remove", taskId);
        return retVal;
    }
    else
    {
        auto graphItr = m_taskGraph.find(taskId);
        auto& dependencyVec = graphItr->second;
        if (dependencyVec.empty())
        {
            LOG_DBG("For task {:d} there is no dependency. Removing it straightaway", taskId);
            m_taskGraph.erase(taskId);
            auto parentTask = m_taskIdMap.find(graphItr->first);
            auto& parentTaskIndegree = parentTask->second.second;
            --parentTaskIndegree;
            m_taskIdMap.erase(taskId);
        }
        else
        {
            LOG_DBG("For task {:d} there are {:d} dependent tasks. "
                    "Removing them one by one recursively",
                    taskId, dependencyVec.size());
            for (const auto& depTaskId : dependencyVec)
                retVal = removeDependencyRecurs(depTaskId);
        }
    }
    LOG_EXIT_DBG();
    return retVal;
}
