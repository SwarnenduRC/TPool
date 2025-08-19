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
        static int nonVoidFunc()
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc");
            auto retVal = 10;
            LOG_EXIT_DBG();
            return retVal;
        }
        // Unique pointer is not allowed as a return type
        // because it cannot be copied into a packaged_task.
        // So, we use shared_ptr instead.
        static std::shared_ptr<int> nonVoidFunc1()
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc1");
            auto retVal = 10;
            LOG_EXIT_DBG();
            return std::make_shared<int>(retVal);
        }
        static int* nonVoidFunc2()
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc2");
            static std::unique_ptr<int> retVal = std::make_unique<int>(10);
            LOG_EXIT_DBG();
            return retVal.get();
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

TEST_F(TaskTests, testSubmittingNonVoidFunc)
{
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc");
            task.submit(nonVoidFunc);
            auto result = task.run();
            EXPECT_EQ(10, std::any_cast<int>(result)) << (std::any_cast<int>(result));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc");
            std::function<int()> nonVoidFunctorWithZeroArgs = nonVoidFunc;
            task.submit(nonVoidFunctorWithZeroArgs);
            auto result = task.run();
            EXPECT_EQ(10, std::any_cast<int>(result)) << (std::any_cast<int>(result));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc2");
            task.submit(nonVoidFunc2);
            auto result = task.run();
            EXPECT_EQ(10, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc2");
            std::function<int*()> nonVoidFunctorWithZeroArgs = nonVoidFunc2;
            task.submit(nonVoidFunctorWithZeroArgs);
            auto result = task.run();
            EXPECT_EQ(10, *(std::any_cast<int*>(result))) << *(std::any_cast<int*>(result));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc1");
            task.submit(nonVoidFunc1);
            auto result = task.run();
            EXPECT_EQ(10, *std::any_cast<std::shared_ptr<int>>(result)) << *(std::any_cast<std::shared_ptr<int>>(result));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc1");
            std::function<std::shared_ptr<int>()> nonVoidFunctorWithZeroArgs = nonVoidFunc1;
            task.submit(nonVoidFunctorWithZeroArgs);
            auto result = task.run();
            EXPECT_EQ(10, *(std::any_cast<std::shared_ptr<int>>(result))) << *(std::any_cast<std::shared_ptr<int>>(result));
        }
    }
}
