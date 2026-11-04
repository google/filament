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

#include <private/utils/InternalDebugRegistry.h>

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <thread>

#include <stddef.h>

using namespace utils;

// hasProperty() and the const accessors only read, so they must not hit the "callback is set"
// precondition of the non-const getPropertyAddress(). They are noexcept: a precondition panic
// there would terminate the process.

TEST(InternalDebugRegistryTest, HasPropertyWithCallback) {
    InternalDebugRegistry registry;
    bool b = false;
    std::atomic<bool> ab{ false };
    registry.registerProperty("d.test.bool", &b, [] {});
    registry.registerProperty("d.test.atomic_bool", &ab, [] {});

    InternalDebugRegistry const& cregistry = registry;
    EXPECT_TRUE(cregistry.hasProperty("d.test.bool"));
    EXPECT_TRUE(cregistry.hasProperty("d.test.atomic_bool"));
    EXPECT_FALSE(cregistry.hasProperty("d.test.missing"));
}

TEST(InternalDebugRegistryTest, ConstGetPropertyAddressWithCallback) {
    InternalDebugRegistry registry;
    int value = 42;
    registry.registerProperty("d.test.int", &value, [] {});

    InternalDebugRegistry const& cregistry = registry;
    EXPECT_EQ(cregistry.getPropertyAddress("d.test.int"), &value);
    EXPECT_EQ(cregistry.getPropertyAddress<int>("d.test.int"), &value);

    int const* p = nullptr;
    EXPECT_TRUE(cregistry.getPropertyAddress("d.test.int", &p));
    EXPECT_EQ(p, &value);

    EXPECT_EQ(cregistry.getPropertyAddress("d.test.missing"), nullptr);
}

TEST(InternalDebugRegistryTest, ConstGetPropertyWithCallback) {
    InternalDebugRegistry registry;
    bool b = true;
    int i = 7;
    float f = 0.5f;
    std::array<float, 3> f3 = { 1.0f, 2.0f, 3.0f };
    registry.registerProperty("d.test.bool", &b, [] {});
    registry.registerProperty("d.test.int", &i, [] {});
    registry.registerProperty("d.test.float", &f, [] {});
    registry.registerProperty("d.test.float3", &f3, [] {});

    InternalDebugRegistry const& cregistry = registry;

    bool bv = false;
    EXPECT_TRUE(cregistry.getProperty("d.test.bool", &bv));
    EXPECT_TRUE(bv);

    int iv = 0;
    EXPECT_TRUE(cregistry.getProperty("d.test.int", &iv));
    EXPECT_EQ(iv, 7);

    float fv = 0.0f;
    EXPECT_TRUE(cregistry.getProperty("d.test.float", &fv));
    EXPECT_EQ(fv, 0.5f);

    std::array<float, 3> f3v{};
    EXPECT_TRUE(cregistry.getProperty("d.test.float3", &f3v));
    EXPECT_EQ(f3v, f3);
}

TEST(InternalDebugRegistryTest, GetPropertyAddressWithoutCallbackIsWritable) {
    InternalDebugRegistry registry;
    int value = 1;
    registry.registerProperty("d.test.int", &value);

    int* const p = registry.getPropertyAddress<int>("d.test.int");
    ASSERT_EQ(p, &value);
    *p = 2;
    EXPECT_EQ(value, 2);
}

TEST(InternalDebugRegistryTest, DataSourceCreatorIsInvokedOnce) {
    InternalDebugRegistry registry;
    int data[4] = {};
    int calls = 0;
    bool const registered = registry.registerDataSource("d.test.source",
            [&]() -> InternalDebugRegistry::DataSource {
                calls++;
                return { data, 4 };
            });
    EXPECT_TRUE(registered);

    InternalDebugRegistry const& cregistry = registry;
    EXPECT_EQ(cregistry.getDataSource("d.test.missing").data, nullptr);
    EXPECT_EQ(calls, 0);

    for (int i = 0; i < 2; i++) {
        auto const ds = cregistry.getDataSource("d.test.source");
        EXPECT_EQ(ds.data, data);
        EXPECT_EQ(ds.count, 4u);
    }
    EXPECT_EQ(calls, 1);

    registry.unregisterDataSource("d.test.source");
    EXPECT_EQ(cregistry.getDataSource("d.test.source").data, nullptr);
    EXPECT_EQ(calls, 1);
}

TEST(InternalDebugRegistryTest, CreatedDataSourceIsKeyedByRegisteredName) {
    InternalDebugRegistry registry;
    int data = 0;
    registry.registerDataSource("d.test.source",
            [&]() -> InternalDebugRegistry::DataSource { return { &data, 1 }; });

    // The first query publishes the created DataSource. It must not be keyed by the caller's
    // string, which may not outlive the call.
    char name[] = "d.test.source";
    EXPECT_EQ(registry.getDataSource(name).data, &data);
    name[0] = 'x';
    EXPECT_EQ(registry.getDataSource("d.test.source").data, &data);
}

// The creator runs with the lock held, so unregisterDataSource() waits for it. Otherwise, the name
// could be unregistered and registered again while the creator runs, and its stale result would
// then replace the new registration.
TEST(InternalDebugRegistryTest, UnregisterWaitsForRunningCreator) {
    using namespace std::chrono_literals;
    InternalDebugRegistry registry;
    int data1 = 1;
    int data2 = 2;
    std::atomic<bool> creator1Started{ false };
    std::atomic<bool> releaseCreator1{ false };
    std::atomic<bool> creator2Started{ false };
    std::atomic<bool> releaseCreator2{ false };
    std::atomic<bool> unregistered{ false };

    auto const spinUntil = [](std::atomic<bool> const& flag) {
        while (!flag.load()) {
            std::this_thread::yield();
        }
    };

    registry.registerDataSource("d.test.source", [&]() -> InternalDebugRegistry::DataSource {
        creator1Started = true;
        spinUntil(releaseCreator1);
        return { &data1, 1 };
    });

    InternalDebugRegistry::DataSource ds1{};
    std::thread t1([&] { ds1 = registry.getDataSource("d.test.source"); });
    spinUntil(creator1Started);

    // While the first creator runs, replace the registration and query the new one.
    InternalDebugRegistry::DataSource ds2{};
    std::thread t2([&] {
        registry.unregisterDataSource("d.test.source");
        unregistered = true;
        registry.registerDataSource("d.test.source", [&]() -> InternalDebugRegistry::DataSource {
            creator2Started = true;
            spinUntil(releaseCreator2);
            return { &data2, 1 };
        });
        ds2 = registry.getDataSource("d.test.source");
    });

    // Give t2 a chance to get ahead of the first creator; it must not.
    auto const deadline = std::chrono::steady_clock::now() + 50ms;
    while (!creator2Started.load() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::yield();
    }
    EXPECT_FALSE(unregistered.load());

    // Finish the first creator before the second one. That's when its stale result would be
    // published if it didn't hold the lock.
    releaseCreator1 = true;
    t1.join();
    releaseCreator2 = true;
    t2.join();

    EXPECT_EQ(ds1.data, &data1);
    EXPECT_EQ(ds2.data, &data2);
    EXPECT_EQ(registry.getDataSource("d.test.source").data, &data2);
}

TEST(InternalDebugRegistryTest, ConcurrentFirstQueriesInvokeCreatorOnce) {
    InternalDebugRegistry registry;
    int data = 0;
    std::atomic<int> calls{ 0 };
    registry.registerDataSource("d.test.source", [&]() -> InternalDebugRegistry::DataSource {
        calls++;
        return { &data, 1 };
    });

    std::atomic<bool> start{ false };
    std::array<void const*, 8> results{};
    std::array<std::thread, 8> threads;
    for (size_t i = 0; i < threads.size(); i++) {
        threads[i] = std::thread([&, i] {
            while (!start.load()) {
                std::this_thread::yield();
            }
            results[i] = registry.getDataSource("d.test.source").data;
        });
    }
    start = true;
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(calls.load(), 1);
    for (void const* const result : results) {
        EXPECT_EQ(result, &data);
    }
}
