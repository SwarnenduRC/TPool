
//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter="TaskTests.*"
//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter=TaskTests.testSubmittingVariousLambdas

/**
* * MIT License
* 
* * Copyright (c) 2025 SwarnenduRC
* 
* * Permission is hereby granted, free of charge, to any person obtaining a copy
* * of this software and associated documentation files (the "Software"), to deal
* * in the Software without restriction, including without limitation the rights
* * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* * copies of the Software, and to permit persons to whom the Software is
* * furnished to do so, subject to the following conditions:
* 
* * The above copyright notice and this permission notice shall be included in all
* * copies or substantial portions of the Software.
* 
* * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* * SOFTWARE.
*/

/**
 * @file TaskTests.cpp
 * @brief Unit tests for the Task class in the ThreadPool library.
 *
 * This file contains a comprehensive suite of Google Test cases for the Task abstraction,
 * verifying correct submission, execution, and result retrieval for various callable types.
 * Test cases cover:
 *   - Submitting and running void functions and functors (with and without arguments)
 *   - Submitting and running non-void functions, including those returning pointers and shared_ptr
 *   - Submitting and running lambda expressions with different signatures
 *   - Running tasks with runAndForget and verifying future results
 *   - Converting tasks to std::function and executing them
 *
 * These tests ensure the Task class supports flexible callable submission and robust result handling.
 */

#include "Task.hpp"

#include <gtest/gtest.h>

using namespace t_pool;

class LocalTask : public Task
{
};

class TaskTests : public ::testing::Test
{
    public:
        static void voidFunc() noexcept
        {
            LOG_ENTRY_DBG("Calling a void function with no arguments");
            LOG_EXIT_DBG();
        }
        static void voidFuncWithArgs(const std::string& arg1, const std::string& arg2) noexcept
        {
            LOG_ENTRY_DBG("Calling a void function with arguments");
            LOG_DBG("First argument = {}", arg1);
            LOG_DBG("Second argument = {}", arg2);
            LOG_EXIT_DBG();
        }
        static int nonVoidFunc() noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc");
            auto retVal = 10;
            LOG_EXIT_DBG();
            return retVal;
        }
        // Unique pointer is not allowed as a return type
        // because it cannot be copied into a packaged_task.
        // So, we use shared_ptr instead.
        static std::shared_ptr<int> nonVoidFunc1() noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc1");
            auto retVal = 10;
            LOG_EXIT_DBG();
            return std::make_shared<int>(retVal);
        }
        static int* nonVoidFunc2() noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc2");
            static std::unique_ptr<int> retVal = std::make_unique<int>(10);
            LOG_EXIT_DBG();
            return retVal.get();
        }
        static int* nonVoidFunc3(const int val, const int* pVal) noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc3");
            static std::unique_ptr<int> retVal = std::make_unique<int>(val * *pVal);
            LOG_EXIT_DBG();
            return retVal.get();
        }
        static int* nonVoidFunc4(const int val, std::shared_ptr<int> pVal) noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc4");
            static std::unique_ptr<int> retVal = std::make_unique<int>(val * *pVal);
            LOG_EXIT_DBG();
            return retVal.get();
        }
        static int* nonVoidFunc5(const int val, const std::shared_ptr<int>& pVal) noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc5");
            static std::unique_ptr<int> retVal = std::make_unique<int>(val * *pVal);
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
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc3");
            auto val = 10;
            std::unique_ptr<int> pVal = std::make_unique<int>(10);
            task.submit(nonVoidFunc3, val, pVal.get());
            auto result = task.run();
            EXPECT_EQ(val * *pVal, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc3");
            std::function<int*(const int val, const int* pVal)> nonVoidFunctorWithZeroArgs = nonVoidFunc3;
            auto val = 10;
            std::unique_ptr<int> pVal = std::make_unique<int>(10);
            task.submit(nonVoidFunctorWithZeroArgs, val, pVal.get());
            auto result = task.run();
            EXPECT_EQ(val * *pVal, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc4");
            auto val = 10;
            std::shared_ptr<int> pVal = std::make_shared<int>(10);
            task.submit(nonVoidFunc4, val, pVal);
            auto result = task.run();
            EXPECT_EQ(val * *pVal, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc4");
            std::function<int*(const int val, std::shared_ptr<int> pVal)> nonVoidFunctorWithZeroArgs = nonVoidFunc4;
            auto val = 10;
            std::shared_ptr<int> pVal = std::make_shared<int>(10);
            task.submit(nonVoidFunctorWithZeroArgs, val, pVal);
            auto result = task.run();
            EXPECT_EQ(val * *pVal, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc5");
            std::shared_ptr<int> pVal = std::make_shared<int>(10);
            task.submit(nonVoidFunc5, *pVal, pVal);
            auto result = task.run();
            EXPECT_EQ(*pVal * *pVal, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc5");
            std::function<int*(const int val, const std::shared_ptr<int> pVal)> nonVoidFunctorWithZeroArgs = nonVoidFunc5;
            std::shared_ptr<int> pVal = std::make_shared<int>(10);
            task.submit(nonVoidFunctorWithZeroArgs, *pVal, pVal);
            auto result = task.run();
            EXPECT_EQ(*pVal * *pVal, *std::any_cast<int*>(result)) << *(std::any_cast<int*>(result));
        }
    }
}

TEST_F(TaskTests, testSubmittingVariousLambdas)
{
    {
        auto voidFuncZeroArgs = []()
        {
            LOG_ENTRY_DBG("Calling a lambda void func with no args");
            LOG_EXIT_DBG();
        };

        auto nonVoidFuncZeroArgs = []() -> std::any
        {
            LOG_ENTRY_DBG("Calling a lambda non void func with no args");
            LOG_EXIT_DBG();
            return 0;
        };

        auto nonVoidFuncWithArgs = [](const std::shared_ptr<int>& val1, int* val2) -> std::shared_ptr<int>
        {
            LOG_ENTRY_DBG("Calling a lambda non void func with two args");
            LOG_EXIT_DBG();
            return std::make_shared<int>(*val1.get() * *val2);
        };

        LocalTask task;
        task.submit(voidFuncZeroArgs);
        task.run();

        task.submit(nonVoidFuncZeroArgs);
        auto result = task.run();
        EXPECT_EQ(0, std::any_cast<int>(result));

        auto val1 = std::make_shared<int>(10);
        auto val2 = std::make_unique<int>(100);
        task.submit(nonVoidFuncWithArgs, val1, val2.get());
        result = task.run();
        EXPECT_EQ(*val1 * *val2, *std::any_cast<std::shared_ptr<int>>(result));
    }
}

TEST_F(TaskTests, testRunAndForget)
{
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc");
            task.submit(nonVoidFunc);
            task.runAndForget();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc");
            std::function<int()> nonVoidFunctorWithZeroArgs = nonVoidFunc;
            task.submit(nonVoidFunctorWithZeroArgs);
            task.runAndForget();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc2");
            task.submit(nonVoidFunc2);
            task.runAndForget();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *(std::any_cast<int*>(result.get()))) << *(std::any_cast<int*>(result.get()));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc2");
            std::function<int*()> nonVoidFunctorWithZeroArgs = nonVoidFunc2;
            task.submit(nonVoidFunctorWithZeroArgs);
            task.runAndForget();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *(std::any_cast<int*>(result.get()))) << *(std::any_cast<int*>(result.get()));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc1");
            task.submit(nonVoidFunc1);
            task.runAndForget();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *std::any_cast<std::shared_ptr<int>>(result.get())) << *(std::any_cast<std::shared_ptr<int>>(result.get()));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc1");
            std::function<std::shared_ptr<int>()> nonVoidFunctorWithZeroArgs = nonVoidFunc1;
            task.submit(nonVoidFunctorWithZeroArgs);
            task.runAndForget();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *std::any_cast<std::shared_ptr<int>>(result.get())) << *(std::any_cast<std::shared_ptr<int>>(result.get()));
        }
    }
}

TEST_F(TaskTests, testToFunction)
{
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc");
            task.submit(nonVoidFunc);
            auto function = task.toFunction();
            function();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc");
            std::function<int()> nonVoidFunctorWithZeroArgs = nonVoidFunc;
            task.submit(nonVoidFunctorWithZeroArgs);
            auto function = task.toFunction();
            function();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc2");
            task.submit(nonVoidFunc2);
            auto function = task.toFunction();
            function();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *(std::any_cast<int*>(result.get()))) << *(std::any_cast<int*>(result.get()));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc2");
            std::function<int*()> nonVoidFunctorWithZeroArgs = nonVoidFunc2;
            task.submit(nonVoidFunctorWithZeroArgs);
            auto function = task.toFunction();
            function();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *(std::any_cast<int*>(result.get()))) << *(std::any_cast<int*>(result.get()));
        }
    }
    {
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc1");
            task.submit(nonVoidFunc1);
            auto function = task.toFunction();
            function();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *std::any_cast<std::shared_ptr<int>>(result.get())) << *(std::any_cast<std::shared_ptr<int>>(result.get()));
        }
        {
            LocalTask task;
            task.setTaskName("nonVoidFunc1");
            std::function<std::shared_ptr<int>()> nonVoidFunctorWithZeroArgs = nonVoidFunc1;
            task.submit(nonVoidFunctorWithZeroArgs);
            auto function = task.toFunction();
            function();
            auto result = task.getTaskFuture();
            if (result.valid())
                EXPECT_EQ(10, *std::any_cast<std::shared_ptr<int>>(result.get())) << *(std::any_cast<std::shared_ptr<int>>(result.get()));
        }
    }
}
