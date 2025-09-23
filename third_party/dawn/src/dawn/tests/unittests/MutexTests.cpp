// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/common/Mutex.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

#if defined(DAWN_ENABLE_ASSERTS)
constexpr bool kAssertEnabled = true;
#else
constexpr bool kAssertEnabled = false;
#endif

class MutexTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // IsLockedByCurrentThread() requires DAWN_ENABLE_ASSERTS flag enabled.
        if (!kAssertEnabled) {
            GTEST_SKIP() << "DAWN_ENABLE_ASSERTS is not enabled";
        }
    }

    Mutex mMutex;
};

// Test AutoLock automatically locks the mutex and unlocks it when out of scope.
TEST_F(MutexTest, AutoLock) {
    {
        Mutex::AutoLock autoLock(&mMutex);
        EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    }
    EXPECT_FALSE(mMutex.IsLockedByCurrentThread());
}

// Test AutoLockAndHoldRef will keep the mutex alive
TEST_F(MutexTest, AutoLockAndHoldRef) {
    auto* mutex = new Mutex();
    EXPECT_EQ(mutex->GetRefCountForTesting(), 1u);
    {
        Mutex::AutoLockAndHoldRef autoLock(mutex);
        EXPECT_TRUE(mutex->IsLockedByCurrentThread());
        EXPECT_EQ(mutex->GetRefCountForTesting(), 2u);

        mutex->Release();
        EXPECT_EQ(mutex->GetRefCountForTesting(), 1u);
    }
}

using MutexDeathTest = MutexTest;

// Double AutoLock calls should be cause assertion failure.
TEST_F(MutexDeathTest, DoubleLockCalls) {
    Mutex::AutoLock autoLock1(&mMutex);
    EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    ASSERT_DEATH_IF_SUPPORTED({ Mutex::AutoLock autoLock2(&mMutex); }, "");
}

class RecursiveMutexTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // IsLockedByCurrentThread() requires DAWN_ENABLE_ASSERTS flag enabled.
        if (!kAssertEnabled) {
            GTEST_SKIP() << "DAWN_ENABLE_ASSERTS is not enabled";
        }
    }

    RecursiveMutex mMutex;
};

// Test AutoLock automatically locks the mutex and unlocks it when out of scope.
TEST_F(RecursiveMutexTest, AutoLock) {
    {
        RecursiveMutex::AutoLock autoLock(&mMutex);
        EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    }
    EXPECT_FALSE(mMutex.IsLockedByCurrentThread());
}

// Test that recursive AutoLock in same thread is valid.
TEST_F(RecursiveMutexTest, RecursiveAutoLock) {
    {
        RecursiveMutex::AutoLock autoLock1(&mMutex);
        EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
        RecursiveMutex::AutoLock autoLock2(&mMutex);
        EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    }
    EXPECT_FALSE(mMutex.IsLockedByCurrentThread());
}

}  // anonymous namespace
}  // namespace dawn
