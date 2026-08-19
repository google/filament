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

#include <private/utils/ThreadMap.h>

#include <utils/algorithm.h>

#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

using namespace utils;

TEST(ThreadMapTest, BasicOperations) {
    ThreadMap<int> map(4);
    EXPECT_EQ(4u, map.getCapacity());
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(0u, map.size());

    // Current thread should not be present initially
    EXPECT_FALSE(map.contains());
    EXPECT_EQ(0, map.get());
    EXPECT_EQ((decltype(map)::INVALID_INDEX), map.findIndex());

    // Register current thread at slot 0
    map.set(0, 42);
    EXPECT_FALSE(map.empty());
    EXPECT_EQ(1u, map.size());
    EXPECT_TRUE(map.contains());
    EXPECT_EQ(0u, map.findIndex());
    EXPECT_EQ(42, map.get());

    // Update value for the current thread at slot 0
    map.set(0, 100);
    EXPECT_EQ(100, map.get());

    // Erase current thread
    EXPECT_TRUE(map.erase());
    EXPECT_FALSE(map.contains());
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(0u, map.size());

    // Erasing non-existent thread returns false
    EXPECT_FALSE(map.erase());
}

TEST(ThreadMapTest, MultiThreadedStaticSlots) {
    constexpr uint32_t THREAD_COUNT = 8;
    constexpr size_t ITERATIONS = 5000;

    ThreadMap<size_t> map(THREAD_COUNT);
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (uint32_t i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&map, &start, i]() {
            // Assign dedicated slot for each thread (matching JobSystem worker pool)
            map.set(i, i * 42);

            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (size_t iter = 0; iter < ITERATIONS; ++iter) {
                EXPECT_TRUE(map.contains());
                EXPECT_EQ(i * 42, map.get());
                EXPECT_EQ(i, map.findIndex());
            }

            map.eraseAt(i);
        });
    }

    start.store(true, std::memory_order_release);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(map.empty());
}

TEST(ThreadMapTest, SlotReassignment) {
    ThreadMap<int> map(1);
    std::thread t1([] {});
    std::thread t2([] {});
    auto const tid1 = t1.get_id();
    auto const tid2 = t2.get_id();
    t1.join();
    t2.join();

    // First thread uses slot 0
    map.set(0, 100, tid1);
    EXPECT_EQ(100, map.get(tid1));
    EXPECT_EQ(0, map.get(tid2));

    // First thread emancipates slot 0
    EXPECT_TRUE(map.eraseAt(0));
    EXPECT_EQ(0, map.get(tid1));
    EXPECT_EQ(0, map.get(tid2));

    // Second thread claims slot 0
    map.set(0, 200, tid2);
    EXPECT_EQ(0, map.get(tid1));
    EXPECT_EQ(200, map.get(tid2));

    EXPECT_TRUE(map.eraseAt(0));
    EXPECT_TRUE(map.empty());
}

TEST(ThreadMapTest, ConcurrentReaderWhileOtherThreadActive) {
    ThreadMap<int> map(2);
    std::atomic<bool> done{false};
    std::atomic<uint32_t> errors{0};

    std::thread t1([] {});
    std::thread t2([] {});
    auto const tid1 = t1.get_id();
    auto const tid2 = t2.get_id();
    t1.join();
    t2.join();

    std::thread reader([&]() {
        while (!done.load(std::memory_order_relaxed)) {
            // tid1 is never registered; get(tid1) must always return 0
            if (map.get(tid1) != 0) {
                errors.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    std::thread worker([&]() {
        for (int i = 0; i < 200000; ++i) {
            map.set(0, 42, tid2);
            map.eraseAt(0);
        }
        done.store(true, std::memory_order_release);
    });

    worker.join();
    reader.join();

    EXPECT_EQ(0u, errors.load()) << "Reader querying unregistered tid observed another thread's value!";
}

TEST(ThreadMapTest, ConcurrentSetAndFindIndexDataRace) {
    ThreadMap<int> map(2);
    std::atomic<bool> done{false};
    std::atomic<uint32_t> invalidFinds{0};

    std::thread t1([] {});
    std::thread t2([] {});
    auto const tid1 = t1.get_id();
    auto const tid2 = t2.get_id();
    t1.join();
    t2.join();

    std::thread reader([&]() {
        while (!done.load(std::memory_order_relaxed)) {
            uint32_t const idx = map.findIndex(tid2);
            if (idx == 0) {
                invalidFinds.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    std::thread writer([&]() {
        for (size_t i = 0; i < 50000; ++i) {
            map.set(0, 42, tid1);
            map.eraseAt(0);
        }
        done.store(true, std::memory_order_release);
    });

    writer.join();
    reader.join();

    EXPECT_EQ(0u, invalidFinds.load()) << "findIndex(tid2) erroneously returned slot 0 during concurrent set/erase!";
}

TEST(ThreadMapTest, ConcurrentSetReaderDataRace) {
    ThreadMap<uint64_t> map(2);
    std::atomic<bool> done{false};
    std::atomic<uint32_t> validNonZeroReads{0};

    std::thread t1([] {});
    auto const tid1 = t1.get_id();
    t1.join();

    std::thread reader([&]() {
        while (!done.load(std::memory_order_relaxed)) {
            uint64_t const val = map.get(tid1);
            if (val != 0) {
                validNonZeroReads.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    std::thread writer([&]() {
        for (size_t i = 1; i <= 500000; ++i) {
            map.set(0, i, tid1);
        }
        done.store(true, std::memory_order_release);
    });

    writer.join();
    reader.join();

    EXPECT_GT(validNonZeroReads.load(), 50u) << "Reader should have observed valid non-zero payloads during set()!";
}

TEST(ThreadMapTest, JobSystemAdoptEmancipatePattern) {
    constexpr uint32_t CAPACITY = 16;
    constexpr uint32_t THREAD_COUNT = 8;
    constexpr size_t ITERATIONS = 2000;

    ThreadMap<uint64_t> map(CAPACITY);
    std::atomic<uint32_t> slotMask{(1u << CAPACITY) - 1u};
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (uint32_t t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&map, &slotMask, &start, t]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (size_t iter = 0; iter < ITERATIONS; ++iter) {
                // 1. Claim a slot from bitmask (simulating JobSystem::adopt)
                uint32_t mask = slotMask.load(std::memory_order_relaxed);
                uint32_t slot = 0;
                while (true) {
                    if (mask == 0) {
                        std::this_thread::yield();
                        mask = slotMask.load(std::memory_order_relaxed);
                        continue;
                    }
                    slot = static_cast<uint32_t>(utils::ctz(mask));
                    if (slotMask.compare_exchange_weak(mask, mask & ~(1u << slot),
                            std::memory_order_acquire, std::memory_order_relaxed)) {
                        break;
                    }
                }

                // 2. Set exclusive slot
                uint64_t const payload = (static_cast<uint64_t>(t) << 32) | iter;
                map.set(slot, payload);

                // 3. Verify self lookup
                EXPECT_TRUE(map.contains());
                EXPECT_EQ(payload, map.get());
                EXPECT_EQ(slot, map.findIndex());

                // 4. Erase slot (simulating JobSystem::emancipate)
                EXPECT_TRUE(map.eraseAt(slot));
                EXPECT_FALSE(map.contains());

                // 5. Release slot back to mask
                slotMask.fetch_or(1u << slot, std::memory_order_release);
            }
        });
    }

    start.store(true, std::memory_order_release);

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(map.empty());
}

TEST(ThreadMapTest, EraseAbaProtection) {
    ThreadMap<uint64_t> map(2);
    std::atomic<bool> done{false};
    std::atomic<uint32_t> stolenSlots{0};

    std::thread dummyA([] {});
    std::thread dummyB([] {});
    auto const tidA = dummyA.get_id();
    auto const tidB = dummyB.get_id();
    dummyA.join();
    dummyB.join();

    std::thread t1([&]() {
        for (int i = 0; i < 200000 && !done.load(std::memory_order_relaxed); ++i) {
            map.set(0, 111, tidA);
            map.erase(tidA);
            map.erase(tidA);
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 200000 && !done.load(std::memory_order_relaxed); ++i) {
            map.set(1, 222, tidB);
            if (!map.contains(tidB) || map.get(tidB) != 222) {
                stolenSlots.fetch_add(1, std::memory_order_relaxed);
            }
            map.erase(tidB);
        }
        done.store(true, std::memory_order_release);
    });

    t1.join();
    t2.join();

    EXPECT_EQ(0u, stolenSlots.load())
            << "erase(tidA) suffered from an race and unregistered tidB's slot!";
}



