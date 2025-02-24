//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CircularBuffer_unittest:
//   Tests of the CircularBuffer class
//

#include <gtest/gtest.h>

#include "common/FixedQueue.h"
#include "common/system_utils.h"

#include <chrono>
#include <thread>

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FixedQueue, Constructors)
{
    FixedQueue<int> q(5);
    EXPECT_EQ(0u, q.size());
    EXPECT_EQ(true, q.empty());
}

// Make sure the destructor destroys all elements.
TEST(FixedQueue, Destructor)
{
    struct s
    {
        s() : counter(nullptr) {}
        s(int *c) : counter(c) {}
        ~s()
        {
            if (counter)
            {
                ++*counter;
            }
        }

        s(const s &)            = default;
        s &operator=(const s &) = default;

        int *counter;
    };

    int destructorCount = 0;

    {
        FixedQueue<s> q(11);
        q.push(s(&destructorCount));
        // Destructor called once for the temporary above.
        EXPECT_EQ(1, destructorCount);
    }

    // Destructor should be called one more time for the element we pushed.
    EXPECT_EQ(2, destructorCount);
}

// Make sure the pop destroys the element.
TEST(FixedQueue, Pop)
{
    struct s
    {
        s() : counter(nullptr) {}
        s(int *c) : counter(c) {}
        ~s()
        {
            if (counter)
            {
                ++*counter;
            }
        }

        s(const s &) = default;
        s &operator=(const s &s)
        {
            // increment if we are overwriting the custom initialized object
            if (counter)
            {
                ++*counter;
            }
            counter = s.counter;
            return *this;
        }

        int *counter;
    };

    int destructorCount = 0;

    FixedQueue<s> q(11);
    q.push(s(&destructorCount));
    // Destructor called once for the temporary above.
    EXPECT_EQ(1, destructorCount);
    q.pop();
    // Copy assignment should be called for the element we popped.
    EXPECT_EQ(2, destructorCount);
}

// Test circulating behavior.
TEST(FixedQueue, WrapAround)
{
    FixedQueue<int> q(7);

    for (int i = 0; i < 7; ++i)
    {
        q.push(i);
    }

    EXPECT_EQ(0, q.front());
    q.pop();
    // This should wrap around
    q.push(7);
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(i + 1, q.front());
        q.pop();
    }
}

// Test concurrent push and pop behavior.
TEST(FixedQueue, ConcurrentPushPop)
{
    FixedQueue<uint64_t> q(7);
    double timeOut    = 1.0;
    uint64_t kMaxLoop = 1000000ull;
    std::atomic<bool> enqueueThreadFinished(false);
    std::atomic<bool> dequeueThreadFinished(false);

    std::thread enqueueThread = std::thread([&]() {
        double t1      = angle::GetCurrentSystemTime();
        double elapsed = 0.0;
        uint64_t value = 0;
        do
        {
            while (q.full() && !dequeueThreadFinished)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }

            // No point pushing new values once deque thread is finished.
            if (dequeueThreadFinished)
            {
                break;
            }
            ASSERT(!q.full());

            // test push
            q.push(value);
            value++;

            elapsed = angle::GetCurrentSystemTime() - t1;
        } while (elapsed < timeOut && value < kMaxLoop);
        // Can exit if: timed out, all values are pushed, or dequeue thread is finished earlier.
        ASSERT(elapsed >= timeOut || value == kMaxLoop || dequeueThreadFinished);
        enqueueThreadFinished = true;
    });

    std::thread dequeueThread = std::thread([&]() {
        double t1              = angle::GetCurrentSystemTime();
        double elapsed         = 0.0;
        uint64_t expectedValue = 0;
        do
        {
            while (q.empty() && !enqueueThreadFinished)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }

            // Let's continue processing the queue even if enqueue thread is already finished.
            if (q.empty())
            {
                ASSERT(enqueueThreadFinished);
                break;
            }

            EXPECT_EQ(expectedValue, q.front());
            // test pop
            q.pop();

            ASSERT(expectedValue < kMaxLoop);
            expectedValue++;

            elapsed = angle::GetCurrentSystemTime() - t1;
        } while (elapsed < timeOut);
        // Can exit if: timed out, or queue is empty and will stay that way.
        ASSERT(elapsed >= timeOut || (q.empty() && enqueueThreadFinished));
        dequeueThreadFinished = true;
    });

    enqueueThread.join();
    dequeueThread.join();
}

// Test concurrent push and pop behavior. When queue is full, instead of wait, it will try to
// increase capacity. At dequeue thread, it will also try to shrink the queue capacity when size
// fall under half of the capacity.
TEST(FixedQueue, ConcurrentPushPopWithResize)
{
    static constexpr size_t kInitialQueueCapacity = 64;
    static constexpr size_t kMaxQueueCapacity     = 64 * 1024;
    FixedQueue<uint64_t> q(kInitialQueueCapacity);
    double timeOut    = 1.0;
    uint64_t kMaxLoop = 1000000ull;
    std::atomic<bool> enqueueThreadFinished(false);
    std::atomic<bool> dequeueThreadFinished(false);
    std::mutex enqueueMutex;
    std::mutex dequeueMutex;

    std::thread enqueueThread = std::thread([&]() {
        double t1      = angle::GetCurrentSystemTime();
        double elapsed = 0.0;
        uint64_t value = 0;
        do
        {
            std::unique_lock<std::mutex> enqueueLock(enqueueMutex);
            if (q.full())
            {
                // Take both lock to ensure no one will access while we try to double the
                // storage. Note that under a well balanced system, this should happen infrequently.
                std::unique_lock<std::mutex> dequeueLock(dequeueMutex);
                // Check again to see if queue is still full after taking the dequeueMutex.
                size_t newCapacity = q.capacity() * 2;
                if (q.full() && newCapacity <= kMaxQueueCapacity)
                {
                    // Double the storage size while we took the lock
                    q.updateCapacity(newCapacity);
                }
            }

            // If queue is still full, lets wait for dequeue thread to make some progress
            while (q.full() && !dequeueThreadFinished)
            {
                enqueueLock.unlock();
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                enqueueLock.lock();
            }

            // No point pushing new values once deque thread is finished.
            if (dequeueThreadFinished)
            {
                break;
            }
            ASSERT(!q.full());

            // test push
            q.push(value);
            value++;

            elapsed = angle::GetCurrentSystemTime() - t1;
        } while (elapsed < timeOut && value < kMaxLoop);
        // Can exit if: timed out, all values are pushed, or dequeue thread is finished earlier.
        ASSERT(elapsed >= timeOut || value == kMaxLoop || dequeueThreadFinished);
        enqueueThreadFinished = true;
    });

    std::thread dequeueThread = std::thread([&]() {
        double t1              = angle::GetCurrentSystemTime();
        double elapsed         = 0.0;
        uint64_t expectedValue = 0;
        do
        {
            std::unique_lock<std::mutex> dequeueLock(dequeueMutex);
            if (q.size() < q.capacity() / 10 && q.capacity() > kInitialQueueCapacity)
            {
                // Shrink the storage if we only used less than 10% of storage. We must take both
                // lock to ensure no one is accessing it when we update storage. And the lock must
                // take in the same order as other thread to avoid deadlock.
                dequeueLock.unlock();
                std::unique_lock<std::mutex> enqueueLock(enqueueMutex);
                dequeueLock.lock();
                // Figure out what the new capacity should be
                size_t newCapacity = q.capacity() / 2;
                while (q.size() < newCapacity)
                {
                    newCapacity /= 2;
                }
                newCapacity *= 2;
                newCapacity = std::max(newCapacity, kInitialQueueCapacity);

                if (newCapacity < q.capacity())
                {
                    q.updateCapacity(newCapacity);
                }
            }

            while (q.empty() && !enqueueThreadFinished)
            {
                dequeueLock.unlock();
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                dequeueLock.lock();
            }

            // Let's continue processing the queue even if enqueue thread is already finished.
            if (q.empty())
            {
                ASSERT(enqueueThreadFinished);
                break;
            }

            EXPECT_EQ(expectedValue, q.front());
            // test pop
            q.pop();

            ASSERT(expectedValue < kMaxLoop);
            expectedValue++;

            elapsed = angle::GetCurrentSystemTime() - t1;
        } while (elapsed < timeOut);
        // Can exit if: timed out, or queue is empty and will stay that way.
        ASSERT(elapsed >= timeOut || (q.empty() && enqueueThreadFinished));
        dequeueThreadFinished = true;
    });

    enqueueThread.join();
    dequeueThread.join();
}

// Test clearing the queue
TEST(FixedQueue, Clear)
{
    FixedQueue<int> q(5);
    for (int i = 0; i < 5; ++i)
    {
        q.push(i);
    }
    q.clear();
    EXPECT_EQ(0u, q.size());
    EXPECT_EQ(true, q.empty());
}
}  // namespace angle
