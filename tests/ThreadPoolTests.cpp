#include "ThreadPool.hpp"

#include <gtest/gtest.h>

using namespace t_pool;

class ThreadPoolTests : public ::testing::Test
{
    public:
        static void voidFunc() noexcept
        {
            //LOG_ENTRY_DBG("Calling a void function with no arguments");
            sleepFor();
            //LOG_EXIT_DBG();
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
        ui32 m_poolSize = 10;
        ThreadPool m_tpool;
};

TEST_F(ThreadPoolTests, testSubmittingFuncs)
{
    getPoolObject().submit(voidFunc);
    auto result = getPoolObject().submit(nonVoidFunc);
    if (result.valid())
        EXPECT_EQ(10, std::any_cast<int>(result.get())) << (std::any_cast<int>(result.get()));
}

//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter="ThreadPoolTests.*"
//leaks --atExit --list -- ./bin/TestThreadPool_d --gtest_shuffle --gtest_repeat=3 --gtest_filter=ThreadPoolTests.testSubmittingFuncs

