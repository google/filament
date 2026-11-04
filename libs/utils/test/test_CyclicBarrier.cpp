/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <utils/CyclicBarrier.h>

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

using namespace utils;

static inline bool isPrime(long number) {
    for (long i=2; i*i<=number; i++) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}

static inline long countPrimes(long max) {
    long r = 0;
    for (long i=0 ; i<max ; i++) {
        if (isPrime(i)) {
            r++;
        }
    }
    return r;
}

TEST(CyclicBarrierTest, Simple) {
    std::atomic_int phase1Arrivals{ 0 };
    std::atomic_int phase2Arrivals{ 0 };
    std::atomic<long> p0{ 0 }, p1{ 0 }, p2{ 0 };
    std::atomic<long> r0{ 0 }, r1{ 0 }, r2{ 0 };
    const long k0 = countPrimes(0x7FFFF);
    const long k1 = countPrimes(0x7FFF);
    const long k2 = countPrimes(0x7FF);
    CyclicBarrier barrier(4);
    EXPECT_EQ(4u, barrier.getThreadCount());

    std::thread a([&]() -> int {
        p0.store(countPrimes(0x7FFFF), std::memory_order_relaxed);
        phase1Arrivals.fetch_add(1, std::memory_order_release);
        barrier.await();
        EXPECT_EQ(4, phase1Arrivals.load(std::memory_order_acquire));
        r0.store(p0.load(std::memory_order_relaxed) + p1.load(std::memory_order_relaxed) +
                p2.load(std::memory_order_relaxed), std::memory_order_relaxed);
        phase2Arrivals.fetch_add(1, std::memory_order_release);
        barrier.await();
        EXPECT_EQ(4, phase2Arrivals.load(std::memory_order_acquire));
        return 0;
    });
    std::thread b([&]() -> int {
        p1.store(countPrimes(0x7FFF), std::memory_order_relaxed);
        phase1Arrivals.fetch_add(1, std::memory_order_release);
        barrier.await();
        EXPECT_EQ(4, phase1Arrivals.load(std::memory_order_acquire));
        r1.store(p0.load(std::memory_order_relaxed) + p1.load(std::memory_order_relaxed) +
                p2.load(std::memory_order_relaxed), std::memory_order_relaxed);
        phase2Arrivals.fetch_add(1, std::memory_order_release);
        barrier.await();
        EXPECT_EQ(4, phase2Arrivals.load(std::memory_order_acquire));
        return 0;
    });
    std::thread c([&]() -> int {
        p2.store(countPrimes(0x7FF), std::memory_order_relaxed);
        phase1Arrivals.fetch_add(1, std::memory_order_release);
        barrier.await();
        EXPECT_EQ(4, phase1Arrivals.load(std::memory_order_acquire));
        r2.store(p0.load(std::memory_order_relaxed) + p1.load(std::memory_order_relaxed) +
                p2.load(std::memory_order_relaxed), std::memory_order_relaxed);
        phase2Arrivals.fetch_add(1, std::memory_order_release);
        barrier.await();
        EXPECT_EQ(4, phase2Arrivals.load(std::memory_order_acquire));
        return 0;
    });

    phase1Arrivals.fetch_add(1, std::memory_order_release);
    barrier.await();
    EXPECT_EQ(4, phase1Arrivals.load(std::memory_order_acquire));

    EXPECT_EQ(k0, p0.load(std::memory_order_relaxed));
    EXPECT_EQ(k1, p1.load(std::memory_order_relaxed));
    EXPECT_EQ(k2, p2.load(std::memory_order_relaxed));

    phase2Arrivals.fetch_add(1, std::memory_order_release);
    barrier.await();
    EXPECT_EQ(4, phase2Arrivals.load(std::memory_order_acquire));

    EXPECT_EQ(k0 + k1 + k2, r0.load(std::memory_order_relaxed));
    EXPECT_EQ(k0 + k1 + k2, r1.load(std::memory_order_relaxed));
    EXPECT_EQ(k0 + k1 + k2, r2.load(std::memory_order_relaxed));

    a.join();
    b.join();
    c.join();
}
