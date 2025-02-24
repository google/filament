//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalMutex_unittest:
//   Tests of the Scoped<*>GlobalMutexLock classes
//

#include <gtest/gtest.h>

#include "libANGLE/GlobalMutex.h"

namespace
{
template <class ScopedGlobalLockT, class... Args>
void runBasicGlobalMutexTest(bool expectToPass, Args &&...args)
{
    constexpr size_t kThreadCount    = 16;
    constexpr size_t kIterationCount = 50'000;

    std::array<std::thread, kThreadCount> threads;

    std::mutex mutex;
    std::condition_variable condVar;
    size_t readyCount = 0;

    std::atomic<size_t> testVar;

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i] = std::thread([&]() {
            {
                std::unique_lock<std::mutex> lock(mutex);
                ++readyCount;
                if (readyCount < kThreadCount)
                {
                    condVar.wait(lock, [&]() { return readyCount == kThreadCount; });
                }
                else
                {
                    condVar.notify_all();
                }
            }
            for (size_t j = 0; j < kIterationCount; ++j)
            {
                ScopedGlobalLockT lock(std::forward<Args>(args)...);
                const int local    = testVar.load(std::memory_order_relaxed);
                const int newValue = local + 1;
                testVar.store(newValue, std::memory_order_relaxed);
            }
        });
    }

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i].join();
    }

    if (expectToPass)
    {
        EXPECT_EQ(testVar.load(), kThreadCount * kIterationCount);
    }
    else
    {
        EXPECT_LE(testVar.load(), kThreadCount * kIterationCount);
    }
}

// Tests basic usage of ScopedGlobalEGLMutexLock.
TEST(GlobalMutexTest, ScopedGlobalEGLMutexLock)
{
    runBasicGlobalMutexTest<egl::ScopedGlobalEGLMutexLock>(true);
}

// Tests basic usage of ScopedOptionalGlobalMutexLock (Enabled).
TEST(GlobalMutexTest, ScopedOptionalGlobalMutexLockEnabled)
{
    runBasicGlobalMutexTest<egl::ScopedOptionalGlobalMutexLock>(true, true);
}

// Tests basic usage of ScopedOptionalGlobalMutexLock (Disabled).
TEST(GlobalMutexTest, ScopedOptionalGlobalMutexLockDisabled)
{
    runBasicGlobalMutexTest<egl::ScopedOptionalGlobalMutexLock>(false, false);
}

#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)
// Tests that ScopedGlobalEGLMutexLock can be recursively locked.
TEST(GlobalMutexTest, RecursiveScopedGlobalEGLMutexLock)
{
    egl::ScopedGlobalEGLMutexLock lock;
    egl::ScopedGlobalEGLMutexLock lock2;
}

// Tests that ScopedOptionalGlobalMutexLock can be recursively locked.
TEST(GlobalMutexTest, RecursiveScopedOptionalGlobalMutexLock)
{
    egl::ScopedOptionalGlobalMutexLock lock(true);
    egl::ScopedOptionalGlobalMutexLock lock2(true);
}
#endif

}  // anonymous namespace
