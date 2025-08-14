//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter="TaskTests.*"

#include "Task.hpp"

#include <gtest/gtest.h>

using namespace t_pool;

class LocalTask : public Task
{
};

class TaskTests : public ::testing::Test
{
    public:
        static void voidFunc()
        {
            LOG_ENTRY_DBG("Calling a void function with no arguments");
            LOG_EXIT_DBG();
        }
};

TEST_F(TaskTests, testSubmittingVoidFuncWithNoArgs)
{
    LocalTask task;
    task.setTaskName("TestVoidTask");
    task.submit(voidFunc);
    task.run();
}

TEST_F(TaskTests, testSubmittingVoidFunctorWithNoArgs)
{
    LocalTask task;
    task.setTaskName("TestVoidFunctorTask");
    std::function<void()> voidFunctor = voidFunc;
    task.submit(voidFunctor);
    task.run();
}
