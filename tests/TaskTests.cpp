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
        static void voidFuncWithArgs(const std::string& arg1, const std::string& arg2)
        {
            LOG_ENTRY_DBG("Calling a void function with arguments");
            LOG_DBG("First argument = {}", arg1);
            LOG_DBG("Second argument = {}", arg2);
            LOG_EXIT_DBG();
        }

};

TEST_F(TaskTests, testSubmittingVoidFuncWithNoArgs)
{
    {
        LocalTask task;
        task.setTaskName("TestVoidTask");
        task.submit(voidFunc);
        task.run();
    }
    {
        LocalTask task;
        task.setTaskName("TestVoidFunctorTask");
        std::function<void()> voidFunctor = voidFunc;
        task.submit(voidFunctor);
        task.run();
    }
}

TEST_F(TaskTests, testSubmittingVoidFunctorWithArgs)
{
    {
        LocalTask task;
        task.setTaskName("TestVoidTaskWithArgs");
        task.submit(voidFuncWithArgs, "Google", "Test");
        task.run();
    }
    {
        LocalTask task;
        task.setTaskName("TestVoidFunctorWithArgsTask");
        std::function<void(const std::string& arg1, const std::string& arg2)> voidFunctorWithArgs = voidFuncWithArgs;
        task.submit(voidFunctorWithArgs, "Google", "Test");
        task.run();
    }
}
