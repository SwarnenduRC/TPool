#include "ThreadPool.hpp"

#include <gtest/gtest.h>

using namespace t_pool;

class ThreadPoolTests : public ::testing::Test
{
    public:
        static void voidFunc() noexcept
        {
            LOG_ENTRY_DBG("Calling a void function with no arguments");
            sleepFor();
            LOG_EXIT_DBG();
        }
        static void voidFuncWithArgs(const std::string& arg1, const std::string& arg2) noexcept
        {
            LOG_ENTRY_DBG("Calling a void function with arguments");
            LOG_DBG("First argument = {}", arg1);
            LOG_DBG("Second argument = {}", arg2);
            sleepFor();
            LOG_EXIT_DBG();
        }
        static int nonVoidFunc() noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc");
            auto retVal = 10;
            sleepFor();
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
            sleepFor();
            LOG_EXIT_DBG();
            return std::make_shared<int>(retVal);
        }
        static int* nonVoidFunc2() noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc2");
            static std::unique_ptr<int> retVal = std::make_unique<int>(10);
            sleepFor();
            LOG_EXIT_DBG();
            return retVal.get();
        }
        static int* nonVoidFunc3(const int val, const int* pVal) noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc3");
            static std::unique_ptr<int> retVal = std::make_unique<int>(val * *pVal);
            sleepFor();
            LOG_EXIT_DBG();
            return retVal.get();
        }
        static int* nonVoidFunc4(const int val, std::shared_ptr<int> pVal) noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc4");
            static std::unique_ptr<int> retVal = std::make_unique<int>(val * *pVal);
            sleepFor();
            LOG_EXIT_DBG();
            return retVal.get();
        }
        static int* nonVoidFunc5(const int val, const std::shared_ptr<int>& pVal) noexcept
        {
            LOG_ENTRY_DBG("Calling nonVoidFunc5");
            static std::unique_ptr<int> retVal = std::make_unique<int>(val * *pVal);
            sleepFor();
            LOG_EXIT_DBG();
            return retVal.get();
        }

        inline ui32 getCurrPoolSize() const { return m_poolSize; }
        inline ThreadPool& getPoolObject() { return m_tpool; }
        void reset(const ui32 poolsize)
        {
            m_poolSize = poolsize;
            m_tpool.reset(m_poolSize);
        }
    protected:
        static void sleepFor(const size_t duration)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(duration));
        }
        static void sleepFor()
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    private:
        ui32 m_poolSize = 5;
        ThreadPool m_tpool;
};

TEST_F(ThreadPoolTests, testSubmittingFuncs)
{
    getPoolObject().submit(voidFunc);

    auto result = getPoolObject().submit(nonVoidFunc);
    if (result.valid())
        EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
    else
        EXPECT_TRUE(false) << "result is not valid\n";

    getPoolObject().submit(voidFuncWithArgs, "Google", "Test");

    result = getPoolObject().submit(nonVoidFunc1);
    if (result.valid())
    {
        EXPECT_EQ(10, *(std::any_cast<std::shared_ptr<int>>(result.get())))
            << *(std::any_cast<std::shared_ptr<int>>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    result = getPoolObject().submit(nonVoidFunc2);
    if (result.valid())
    {
        EXPECT_EQ(10, *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    auto val1 = 100;
    auto val2 = std::make_shared<int>(10);
    result = getPoolObject().submit(nonVoidFunc3, val1, val2.get());
    if (result.valid())
    {
        EXPECT_EQ((val1 * *val2), *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    result = getPoolObject().submit(nonVoidFunc4, val1, val2);
    if (result.valid())
    {
        EXPECT_EQ((val1 * *val2), *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    result = getPoolObject().submit(nonVoidFunc5, val1, val2);
    if (result.valid())
    {
        EXPECT_EQ((val1 * *val2), *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }
}

TEST_F(ThreadPoolTests, testSubmittingFuncsThroughStdFuncObject)
{
    std::function<void()> voidFuncObj = voidFunc;
    getPoolObject().submit(voidFuncObj);

    std::function<int()> nonVoidFuncObj = nonVoidFunc;
    auto result = getPoolObject().submit(nonVoidFuncObj);
    if (result.valid())
        EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
    else
        EXPECT_TRUE(false) << "result is not valid\n";

    std::function<void(const std::string&, const std::string&)> voidFuncWithArgsFuncObj = voidFuncWithArgs;
    getPoolObject().submit(voidFuncWithArgsFuncObj, "Google", "Test");

    std::function<std::shared_ptr<int>()> nonVoidFunc1Obj = nonVoidFunc1;
    result = getPoolObject().submit(nonVoidFunc1Obj);
    if (result.valid())
    {
        EXPECT_EQ(10, *(std::any_cast<std::shared_ptr<int>>(result.get())))
            << *(std::any_cast<std::shared_ptr<int>>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    std::function<int*()> nonVoidFunc2Obj = nonVoidFunc2;
    result = getPoolObject().submit(nonVoidFunc2Obj);
    if (result.valid())
    {
        EXPECT_EQ(10, *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    auto val1 = 100;
    auto val2 = std::make_shared<int>(10);
    std::function<int*(int, const int*)> nonVoidFunc3Obj = nonVoidFunc3;
    result = getPoolObject().submit(nonVoidFunc3Obj, val1, val2.get());
    if (result.valid())
    {
        EXPECT_EQ((val1 * *val2), *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    std::function<int*(int, std::shared_ptr<int>)> nonVoidFunc4Obj = nonVoidFunc4;
    result = getPoolObject().submit(nonVoidFunc4Obj, val1, val2);
    if (result.valid())
    {
        EXPECT_EQ((val1 * *val2), *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }

    std::function<int*(int, const std::shared_ptr<int>&)> nonVoidFunc5Obj = nonVoidFunc5;
    result = getPoolObject().submit(nonVoidFunc5Obj, val1, val2);
    if (result.valid())
    {
        EXPECT_EQ((val1 * *val2), *(std::any_cast<int*>(result.get())))
            << *(std::any_cast<int*>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }
}

TEST_F(ThreadPoolTests, testSubmittingLambdas)
{
    auto voidLambda = []()
    {
        LOG_ENTRY_DBG("Entering void lambda");
        LOG_EXIT_DBG("Exiting void lambda");
    };
    getPoolObject().submit(voidLambda);

    auto nonVoidLambda = []()
    {
        LOG_ENTRY_DBG("Entering non void lambda");
        LOG_EXIT_DBG("Exiting non void lambda");
        return 10;
    };
    auto result = getPoolObject().submit(nonVoidLambda);
    if (result.valid())
        EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
    else
        EXPECT_TRUE(false) << "result is not valid\n";

    auto voidLambdaWithArgs = [](const std::string& arg1, const std::string& arg2)
    {
        LOG_ENTRY_DBG("Calling a void lambda with arguments");
        LOG_DBG("First argument = {}", arg1);
        LOG_DBG("Second argument = {}", arg2);
        LOG_EXIT_DBG();
    };
    getPoolObject().submit(voidLambdaWithArgs, "Google", "Test");

    auto nonVoidLambda1 = []()
    {
        LOG_ENTRY_DBG("Calling nonVoidLambda1");
        LOG_EXIT_DBG();
        sleepFor(10000);
        return std::make_shared<int>(10);
    };
    getPoolObject().reset(static_cast<ui32>(2));
    result = getPoolObject().submit(nonVoidLambda1);
    if (result.valid())
    {
        EXPECT_EQ(10, *(std::any_cast<std::shared_ptr<int>>(result.get())))
            << *(std::any_cast<std::shared_ptr<int>>(result.get()));
    }
    else
    {
        EXPECT_TRUE(false) << "result is not valid\n";
    }
}

//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter="ThreadPoolTests.*"
//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter=ThreadPoolTests.testSubmittingLambdas

