#ifndef TASK_DIRECTED_ASSOCIATED_GRAPH
#define TASK_DIRECTED_ASSOCIATED_GRAPH

#include "Task.hpp"

#include <unordered_set>

namespace t_pool
{
    using TASK_MAP = std::unordered_map<uint32_t, std::pair<std::shared_ptr<Task>, size_t>>;
    using TASK_GRAPH = std::unordered_map<uint32_t, std::vector<uint32_t>>;
    using TASK_QUEUE = std::queue<std::weak_ptr<Task>>;

    typedef class Task_As_DAG
    {
        public:
            Task_As_DAG& addTask(Task&& task);
            Task_As_DAG& addDependency(Task&& task);
            Task_As_DAG& removeDependency(Task&& task);
        private:
            bool removeDependencyRecurs(const uint32_t taskId);
            std::shared_ptr<Task> m_task;
            std::future<std::any> m_taskFuture;
            TASK_GRAPH m_taskGraph;
            TASK_MAP m_taskIdMap;
            TASK_QUEUE m_tasksSorted;
    } TDAG;
};

#endif //TASK_DIRECTED_ASSOCIATED_GRAPH