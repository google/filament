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

#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <string>
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
    EXPECT_EQ(nullptr, map.find());
    EXPECT_EQ(0, map.get());
    EXPECT_EQ((decltype(map)::INVALID_INDEX), map.findIndex());

    // Register current thread at slot 0
    map.set(0, 42);
    EXPECT_FALSE(map.empty());
    EXPECT_EQ(1u, map.size());
    EXPECT_TRUE(map.contains());
    EXPECT_EQ(0u, map.findIndex());
    EXPECT_EQ(42, map.get());

    int* val = map.find();
    ASSERT_NE(nullptr, val);
    EXPECT_EQ(42, *val);

    // Modify value via pointer
    *val = 100;
    EXPECT_EQ(100, map.get());

    // Erase current thread
    EXPECT_TRUE(map.erase());
    EXPECT_FALSE(map.contains());
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(0u, map.size());

    // Erasing non-existent thread returns false
    EXPECT_FALSE(map.erase());
}

TEST(ThreadMapTest, EmplaceAndCapacity) {
    ThreadMap<std::string> map(2);
    EXPECT_EQ(2u, map.getCapacity());

    // Create unique artificial thread IDs via threads
    std::thread t1([] {});
    std::thread t2([] {});
    std::thread t3([] {});
    const auto tid1 = t1.get_id();
    const auto tid2 = t2.get_id();
    const auto tid3 = t3.get_id();
    t1.join();
    t2.join();
    t3.join();

    // Emplace first thread
    uint32_t const idx1 = map.emplace("thread1", tid1);
    EXPECT_NE((decltype(map)::INVALID_INDEX), idx1);
    EXPECT_EQ(1u, map.size());
    EXPECT_EQ("thread1", map.get(tid1));

    // Emplace second thread
    uint32_t const idx2 = map.emplace("thread2", tid2);
    EXPECT_NE((decltype(map)::INVALID_INDEX), idx2);
    EXPECT_NE(idx1, idx2);
    EXPECT_EQ(2u, map.size());
    EXPECT_EQ("thread2", map.get(tid2));

    // Emplace third thread should fail (capacity exceeded)
    uint32_t const idx3 = map.emplace("thread3", tid3);
    EXPECT_EQ((decltype(map)::INVALID_INDEX), idx3);
    EXPECT_EQ(2u, map.size());

    // Re-emplacing an existing thread should update value and return same index
    uint32_t const idx1_re = map.emplace("thread1_updated", tid1);
    EXPECT_EQ(idx1, idx1_re);
    EXPECT_EQ("thread1_updated", map.get(tid1));

    // Erase at slot index
    map.eraseAt(idx1);
    EXPECT_FALSE(map.contains(tid1));
    EXPECT_EQ(1u, map.size());

    // Now third thread can be emplaced
    uint32_t const idx3_new = map.emplace("thread3", tid3);
    EXPECT_EQ(idx1, idx3_new);
    EXPECT_TRUE(map.contains(tid3));
    EXPECT_EQ("thread3", map.get(tid3));

    // Clear map
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_FALSE(map.contains(tid2));
    EXPECT_FALSE(map.contains(tid3));
}

TEST(ThreadMapTest, MultiThreadedConcurrentAccess) {
    constexpr uint32_t THREAD_COUNT = 8;
    constexpr size_t ITERATIONS = 1000;

    ThreadMap<size_t> map(THREAD_COUNT);
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (uint32_t i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&map, &start, i]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (size_t iter = 0; iter < ITERATIONS; ++iter) {
                // Register
                uint32_t const slot = map.emplace(i * 10000 + iter);
                EXPECT_NE((decltype(map)::INVALID_INDEX), slot);

                // Verify self lookup
                EXPECT_TRUE(map.contains());
                EXPECT_EQ(i * 10000 + iter, map.get());
                EXPECT_EQ(slot, map.findIndex());

                // Unregister
                EXPECT_TRUE(map.erase());
                EXPECT_FALSE(map.contains());
            }
        });
    }

    start.store(true, std::memory_order_release);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(map.empty());
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
            // Assign dedicated slot for each thread
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

struct Payload {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

TEST(ThreadMapTest, ConcurrentEmplaceReaderDataRace) {
    ThreadMap<Payload> map(2);
    std::atomic<bool> done{false};
    std::atomic<uint32_t> failures{0};
    std::atomic<std::thread::id> writerTid{};

    std::thread reader([&]() {
        while (!done.load(std::memory_order_relaxed)) {
            const auto tid = writerTid.load(std::memory_order_relaxed);
            if (tid != std::thread::id{}) {
                Payload const p = map.get(tid);
                if (p.id != 0 && !p.valid()) {
                    failures.fetch_add(1, std::memory_order_relaxed);
                }
            }
        }
    });

    std::thread writer([&]() {
        writerTid.store(std::this_thread::get_id(), std::memory_order_release);
        for (size_t i = 1; i <= 20000; ++i) {
            Payload p{i};
            map.emplace(p);
            map.erase();
        }
        done.store(true, std::memory_order_release);
    });

    writer.join();
    reader.join();

    EXPECT_EQ(0u, failures.load()) << "Reader observed uninitialized or partially written value in emplace()!";
}

TEST(ThreadMapTest, EraseDestroysResource) {
    struct Resource {
        std::shared_ptr<int> ref;
    };

    ThreadMap<Resource> map(4);
    auto val = std::make_shared<int>(42);
    EXPECT_EQ(1, val.use_count());

    map.emplace(Resource{val});
    EXPECT_EQ(2, val.use_count());

    map.erase();
    EXPECT_EQ(1, val.use_count()) << "erase() must destroy the stored resource payload!";
}

TEST(ThreadMapTest, ReEmplaceUpdatesValueSafely) {
    ThreadMap<int> map(4);
    auto const tid = std::this_thread::get_id();

    EXPECT_EQ(0u, map.emplace(100, tid));
    EXPECT_EQ(100, map.get(tid));

    // Re-emplace same tid with a new value
    EXPECT_EQ(0u, map.emplace(200, tid));
    EXPECT_EQ(200, map.get(tid));
    EXPECT_EQ(1u, map.size());

    map.erase(tid);
    EXPECT_TRUE(map.empty());
}

