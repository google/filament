/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/debug/Mutex.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <new>
#include <thread>
#include <vector>

using namespace utils;

TEST(MutexTest, BasicLockUnlock) {
    Mutex mLock;
    mLock.lock();
    mLock.unlock();

    EXPECT_TRUE(mLock.try_lock());
    mLock.unlock();

    {
        LockGuard const guard(mLock);
        std::thread t([&mLock]() {
            EXPECT_FALSE(mLock.try_lock());
        });
        t.join();
    }

    {
        UniqueLock lock(mLock);
        lock.unlock();
        lock.lock();
    }
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(DebugMutexTest, SelfDeadlock) {
    EXPECT_DEATH({
        debug::Mutex mLock;
        mLock.lock();
        mLock.lock();
    }, "Self-Deadlock / Recursive Lock Detected");
}
#endif

TEST(DebugMutexTest, ValidOrderNoCycle) {
    debug::Mutex A;
    debug::Mutex B;
    debug::Mutex C;

    A.lock();
    B.lock();
    C.lock();

    C.unlock();
    B.unlock();
    A.unlock();
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(DebugMutexTest, DirectOrderInversion) {
    EXPECT_DEATH({
        debug::Mutex A;
        debug::Mutex B;

        // Thread 1 establishes A -> B order
        A.lock();
        B.lock();
        B.unlock();
        A.unlock();

        // Thread 2 attempts B -> A inversion
        std::thread t([&A, &B]() {
            B.lock();
            A.lock();
        });
        t.join();
    }, "Lock-Order Inversion / Potential Deadlock Detected");
}

TEST(DebugMutexTest, TransitiveOrderInversion) {
    EXPECT_DEATH({
        debug::Mutex A;
        debug::Mutex B;
        debug::Mutex C;

        // Thread 1 establishes A -> B
        A.lock();
        B.lock();
        B.unlock();
        A.unlock();

        // Thread 2 establishes B -> C
        std::thread t2([&B, &C]() {
            B.lock();
            C.lock();
            C.unlock();
            B.unlock();
        });
        t2.join();

        // Thread 3 attempts C -> A (closing the 3-node cycle A -> B -> C -> A)
        std::thread t3([&C, &A]() {
            C.lock();
            A.lock();
        });
        t3.join();
    }, "Lock-Order Inversion / Potential Deadlock Detected");
}
#endif

TEST(DebugMutexTest, DestructionPointerReuse) {
    debug::Mutex B;
    alignas(debug::Mutex) std::byte storage[sizeof(debug::Mutex)];

    auto* A = new (storage) debug::Mutex();
    void const* const addrA = A;
    A->lock();
    B.lock();
    B.unlock();
    A->unlock();
    A->~Mutex(); // A is destroyed here, clearing edge A -> B from the graph

    // Constructing A2 at the exact same address and establishing B -> A2 must not trigger a cycle.
    auto* A2 = new (storage) debug::Mutex();
    ASSERT_EQ(addrA, static_cast<void const*>(A2));
    B.lock();
    A2->lock();
    A2->unlock();
    B.unlock();
    A2->~Mutex();
}

TEST(DebugMutexTest, OutOfOrderUnlock) {
    debug::Mutex A;
    debug::Mutex B;

    A.lock();
    B.lock();
    // Unlock out of LIFO order
    A.unlock();
    B.unlock();
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(DebugMutexTest, TryLockOrderInversion) {
    EXPECT_DEATH({
        debug::Mutex A;
        debug::Mutex B;

        // Thread 1 establishes A -> B order
        A.lock();
        EXPECT_TRUE(B.try_lock());
        B.unlock();
        A.unlock();

        // Thread 2 attempts B -> A inversion via try_lock()
        std::thread backgroundThread([&B, &A]() {
            B.lock();
            (void)A.try_lock();
        });
        backgroundThread.join();
    }, "Lock-Order Inversion / Potential Deadlock Detected");
}
#endif
