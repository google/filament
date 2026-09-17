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

#include "CallbackManager.h"
#include "DriverBase.h"

#include "noop/NoopDriver.h"

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

using namespace filament::backend;

namespace {

class CallbackManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mDriver = static_cast<DriverBase*>(NoopDriver::create());
    }

    void TearDown() override {
        delete mDriver;
    }

    DriverBase* mDriver = nullptr;
};

TEST_F(CallbackManagerTest, ZeroConditions) {
    CallbackManager cm(*mDriver);
    int callCount = 0;

    auto callback = [](void* user) {
        auto* counter = static_cast<int*>(user);
        (*counter)++;
    };

    cm.setCallback(nullptr, callback, &callCount);
    mDriver->purge();

    EXPECT_EQ(callCount, 1);
}

TEST_F(CallbackManagerTest, SetCallbackBeforePut) {
    CallbackManager cm(*mDriver);
    int callCount = 0;

    auto callback = [](void* user) {
        auto* counter = static_cast<int*>(user);
        (*counter)++;
    };

    CallbackManager::Handle h = cm.get();
    cm.setCallback(nullptr, callback, &callCount);
    mDriver->purge();
    EXPECT_EQ(callCount, 0);

    cm.put(h);
    mDriver->purge();
    EXPECT_EQ(callCount, 1);
}

TEST_F(CallbackManagerTest, PutBeforeSetCallback) {
    CallbackManager cm(*mDriver);
    int callCount = 0;

    auto callback = [](void* user) {
        auto* counter = static_cast<int*>(user);
        (*counter)++;
    };

    CallbackManager::Handle h = cm.get();
    cm.put(h);
    mDriver->purge();
    EXPECT_EQ(callCount, 0);

    cm.setCallback(nullptr, callback, &callCount);
    mDriver->purge();
    EXPECT_EQ(callCount, 1);
}

TEST_F(CallbackManagerTest, MultipleConditions) {
    CallbackManager cm(*mDriver);
    int callCount = 0;

    auto callback = [](void* user) {
        auto* counter = static_cast<int*>(user);
        (*counter)++;
    };

    constexpr size_t N = 10;
    std::vector<CallbackManager::Handle> handles;
    for (size_t i = 0; i < N; ++i) {
        handles.push_back(cm.get());
    }

    cm.setCallback(nullptr, callback, &callCount);
    mDriver->purge();
    EXPECT_EQ(callCount, 0);

    for (size_t i = 0; i < N - 1; ++i) {
        cm.put(handles[i]);
        mDriver->purge();
        EXPECT_EQ(callCount, 0);
    }

    cm.put(handles[N - 1]);
    mDriver->purge();
    EXPECT_EQ(callCount, 1);
}

TEST_F(CallbackManagerTest, TerminatePendingCallbacks) {
    CallbackManager cm(*mDriver);
    int callCount = 0;

    auto callback = [](void* user) {
        auto* counter = static_cast<int*>(user);
        (*counter)++;
    };

    CallbackManager::Handle h = cm.get();
    cm.setCallback(nullptr, callback, &callCount);

    cm.terminate();
    mDriver->purge();
    EXPECT_EQ(callCount, 1);

    // Subsequent put should not invoke the callback again
    cm.put(h);
    mDriver->purge();
    EXPECT_EQ(callCount, 1);
}

TEST_F(CallbackManagerTest, ConcurrentPutAndSetCallbackStress) {
    // Stress test the race between worker threads calling put() and the driver thread calling setCallback()
    constexpr int ITERATIONS = 1000;

    for (int iter = 0; iter < ITERATIONS; ++iter) {
        CallbackManager cm(*mDriver);
        std::atomic_int callCount{0};

        auto callback = [](void* user) {
            auto* counter = static_cast<std::atomic_int*>(user);
            counter->fetch_add(1, std::memory_order_relaxed);
        };

        constexpr int THREAD_COUNT = 4;
        std::vector<CallbackManager::Handle> handles;
        for (int i = 0; i < THREAD_COUNT; ++i) {
            handles.push_back(cm.get());
        }

        std::atomic_bool start{false};
        std::vector<std::thread> workers;

        for (int i = 0; i < THREAD_COUNT; ++i) {
            workers.emplace_back([&cm, handle = handles[i], &start]() mutable {
                while (!start.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }
                cm.put(handle);
            });
        }

        std::thread driverThread([&cm, callback, &callCount, &start]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            cm.setCallback(nullptr, callback, &callCount);
        });

        start.store(true, std::memory_order_release);

        for (auto& w : workers) {
            w.join();
        }
        driverThread.join();

        mDriver->purge();
        ASSERT_EQ(callCount.load(), 1) << "Failed at iteration " << iter;
    }
}

} // namespace
