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

#include "ThreadPriorityOverride.h"

#include <utils/JobSystem.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#if defined(__APPLE__)
#include <pthread.h>
#include <sys/qos.h>
#elif defined(__ANDROID__)
#include <sys/resource.h>
#include <unistd.h>

#ifndef ANDROID_PRIORITY_URGENT_DISPLAY
#define ANDROID_PRIORITY_URGENT_DISPLAY (-8)
#endif
#ifndef ANDROID_PRIORITY_DISPLAY
#define ANDROID_PRIORITY_DISPLAY (-4)
#endif
#ifndef ANDROID_PRIORITY_NORMAL
#define ANDROID_PRIORITY_NORMAL (0)
#endif
#ifndef ANDROID_PRIORITY_BACKGROUND
#define ANDROID_PRIORITY_BACKGROUND (10)
#endif
#elif defined(WIN32)
#include <windows.h>
#endif

using namespace filament::backend;
using namespace utils;

namespace {

constexpr bool isPriorityOverrideSupported() {
#if defined(__APPLE__) || defined(__ANDROID__) || defined(WIN32)
    return true;
#else
    return false;
#endif
}

enum class PriorityLevel {
    BACKGROUND,
    DISPLAY,
    DEFAULT_OR_NORMAL
};

PriorityLevel getCurrentThreadPriorityLevel() {
#if defined(__APPLE__)
    qos_class_t const qosClass = qos_class_self();
    if (qosClass == QOS_CLASS_USER_INTERACTIVE) {
        return PriorityLevel::DISPLAY;
    } else if (qosClass == QOS_CLASS_BACKGROUND) {
        return PriorityLevel::BACKGROUND;
    }
    return PriorityLevel::DEFAULT_OR_NORMAL;
#elif defined(__ANDROID__)
    errno = 0;
    int const nice = getpriority(PRIO_PROCESS, 0);
    if (nice <= ANDROID_PRIORITY_DISPLAY) {
        return PriorityLevel::DISPLAY;
    } else if (nice >= ANDROID_PRIORITY_BACKGROUND) {
        return PriorityLevel::BACKGROUND;
    }
    return PriorityLevel::DEFAULT_OR_NORMAL;
#elif defined(WIN32)
    int const prio = GetThreadPriority(GetCurrentThread());
    if (prio >= THREAD_PRIORITY_HIGHEST) {
        return PriorityLevel::DISPLAY;
    } else if (prio <= THREAD_PRIORITY_BELOW_NORMAL) {
        return PriorityLevel::BACKGROUND;
    }
    return PriorityLevel::DEFAULT_OR_NORMAL;
#else
    return PriorityLevel::DEFAULT_OR_NORMAL;
#endif
}

} // namespace

TEST(ThreadPriorityOverrideTest, DisabledOverrideIsNoOp) {
    JobSystem::setThreadPriority(JobSystem::Priority::DISPLAY);
    PriorityLevel const initialPriority = getCurrentThreadPriorityLevel();

    ThreadPriorityOverride override(false);
    EXPECT_FALSE(override.isOverrideRequested());
    EXPECT_FALSE(override.isOverridden());

    override.registerCurrentThread();
    EXPECT_FALSE(override.isOverridden());
    EXPECT_EQ(getCurrentThreadPriorityLevel(), initialPriority);

    override.startOverride();
    EXPECT_FALSE(override.isOverrideRequested());
    EXPECT_FALSE(override.isOverridden());
    EXPECT_EQ(getCurrentThreadPriorityLevel(), initialPriority);

    override.restorePriority();
    EXPECT_FALSE(override.isOverridden());
    EXPECT_EQ(getCurrentThreadPriorityLevel(), initialPriority);

    override.endOverride();
    EXPECT_FALSE(override.isOverridden());
    EXPECT_EQ(getCurrentThreadPriorityLevel(), initialPriority);
}

TEST(ThreadPriorityOverrideTest, InlineExecutionPreservesPriority) {
    // Simulates a job dequeued and executed inline on the calling/driver thread.
    JobSystem::setThreadPriority(JobSystem::Priority::DISPLAY);
    PriorityLevel const initialPriority = getCurrentThreadPriorityLevel();

    ThreadPriorityOverride override(true);
    EXPECT_FALSE(override.isOverrideRequested());
    EXPECT_FALSE(override.isOverridden());

    override.registerCurrentThread();
    EXPECT_FALSE(override.isOverridden());

    // Since startOverride() was never called, restorePriority() must be a no-op.
    override.restorePriority();
    EXPECT_FALSE(override.isOverridden());
    EXPECT_EQ(getCurrentThreadPriorityLevel(), initialPriority);
}

TEST(ThreadPriorityOverrideTest, WorkerFirstRegistrationHandshake) {
    ThreadPriorityOverride override(true);
    std::atomic<bool> workerRegistered{false};
    std::atomic<bool> overrideStarted{false};
    std::atomic<bool> workerObservedOverride{false};

    EXPECT_FALSE(override.isOverrideRequested());
    EXPECT_FALSE(override.isOverridden());

    std::thread worker([&]() {
        JobSystem::setThreadPriority(JobSystem::Priority::BACKGROUND);
        override.registerCurrentThread();
        workerRegistered.store(true, std::memory_order_release);

        // Wait until the waiting thread has applied the override
        while (!overrideStarted.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        // Record whether the override is active
        workerObservedOverride.store(override.isOverridden(), std::memory_order_relaxed);

        // Restore priority upon completing the job
        override.restorePriority();
    });

    // Wait until worker registers its thread
    while (!workerRegistered.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }

    override.startOverride();
    EXPECT_TRUE(override.isOverrideRequested());
    if constexpr (isPriorityOverrideSupported()) {
        EXPECT_TRUE(override.isOverridden());
    } else {
        EXPECT_FALSE(override.isOverridden());
    }
    overrideStarted.store(true, std::memory_order_release);

    worker.join();
    if constexpr (isPriorityOverrideSupported()) {
        EXPECT_TRUE(workerObservedOverride.load());
    } else {
        EXPECT_FALSE(workerObservedOverride.load());
    }

    override.endOverride();
    EXPECT_FALSE(override.isOverridden());
}

TEST(ThreadPriorityOverrideTest, WaiterFirstRegistrationHandshake) {
    ThreadPriorityOverride override(true);
    std::atomic<bool> overrideStarted{false};
    std::atomic<bool> workerObservedOverride{false};

    EXPECT_FALSE(override.isOverrideRequested());
    EXPECT_FALSE(override.isOverridden());

    // The waiting thread calls startOverride() before worker registers
    override.startOverride();
    EXPECT_TRUE(override.isOverrideRequested());
    EXPECT_FALSE(override.isOverridden()); // Worker not yet registered
    overrideStarted.store(true, std::memory_order_release);

    std::thread worker([&]() {
        JobSystem::setThreadPriority(JobSystem::Priority::BACKGROUND);

        // Worker registers after startOverride() was already called
        override.registerCurrentThread();

        // Priority should have been escalated immediately during registration on supported platforms
        workerObservedOverride.store(override.isOverridden(), std::memory_order_relaxed);

        // Restore priority upon completing the job
        override.restorePriority();
    });

    worker.join();
    if constexpr (isPriorityOverrideSupported()) {
        EXPECT_TRUE(workerObservedOverride.load());
    } else {
        EXPECT_FALSE(workerObservedOverride.load());
    }

    override.endOverride();
    EXPECT_FALSE(override.isOverridden());
}

TEST(ThreadPriorityOverrideTest, PreservesNormalInitialPriority) {
    ThreadPriorityOverride override(true);
    std::atomic<bool> workerRegistered{false};
    std::atomic<bool> overrideStarted{false};
    std::atomic<PriorityLevel> workerPriorityAfterRestore{PriorityLevel::BACKGROUND};

    std::thread worker([&]() {
        JobSystem::setThreadPriority(JobSystem::Priority::NORMAL);
        PriorityLevel const initialPriority = getCurrentThreadPriorityLevel();
        override.registerCurrentThread();
        workerRegistered.store(true, std::memory_order_release);

        while (!overrideStarted.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        // Restore priority upon completing the job; must return to NORMAL, not BACKGROUND
        override.restorePriority();
        workerPriorityAfterRestore.store(getCurrentThreadPriorityLevel(), std::memory_order_relaxed);
    });

    while (!workerRegistered.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }

    override.startOverride();
    overrideStarted.store(true, std::memory_order_release);

    worker.join();
    override.endOverride();

    EXPECT_EQ(workerPriorityAfterRestore.load(), PriorityLevel::DEFAULT_OR_NORMAL);
}

TEST(ThreadPriorityOverrideTest, ConcurrentRegistrationAndOverrideStress) {
    // Stress test the bidirectional handshake across multiple iterations
    constexpr size_t ITERATIONS = 100;
    std::atomic<size_t> completedIterations{0};

    for (size_t i = 0; i < ITERATIONS; ++i) {
        ThreadPriorityOverride override(true);
        std::atomic<bool> startFlag{false};
        std::atomic<bool> workerFinished{false};

        std::thread worker([&]() {
            while (!startFlag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            JobSystem::setThreadPriority(JobSystem::Priority::BACKGROUND);
            override.registerCurrentThread();
            override.restorePriority();
            workerFinished.store(true, std::memory_order_release);
        });

        std::thread waiter([&]() {
            while (!startFlag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            override.startOverride();
            while (!workerFinished.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            override.endOverride();
        });

        // Release both threads simultaneously to maximize race conditions
        startFlag.store(true, std::memory_order_release);

        worker.join();
        waiter.join();

        EXPECT_FALSE(override.isOverridden());
        completedIterations.fetch_add(1, std::memory_order_relaxed);
    }

    EXPECT_EQ(completedIterations.load(), ITERATIONS);
}

TEST(ThreadPriorityOverrideTest, DestructorCleansUpUnendedOverride) {
    // Verifies that if endOverride() is never called, the destructor cleans up safely
    JobSystem::setThreadPriority(JobSystem::Priority::BACKGROUND);
    PriorityLevel const initialPriority = getCurrentThreadPriorityLevel();
    {
        ThreadPriorityOverride override(true);
        override.registerCurrentThread();
        override.startOverride();
        if constexpr (isPriorityOverrideSupported()) {
            EXPECT_TRUE(override.isOverridden());
        } else {
            EXPECT_FALSE(override.isOverridden());
        }
        // Destructor runs here without calling endOverride()
    }
    EXPECT_EQ(getCurrentThreadPriorityLevel(), initialPriority);
    JobSystem::setThreadPriority(JobSystem::Priority::NORMAL);
}

TEST(ThreadPriorityOverrideTest, LateOverrideCleanedUpByEndOverride) {
    // Verifies that if startOverride() executes after restorePriority() has already run,
    // endOverride() restores the worker thread back to its original baseline priority.
    ThreadPriorityOverride override(true);
    std::atomic<bool> workerRestored{false};
    std::atomic<bool> overrideStarted{false};
    std::atomic<bool> overrideEnded{false};
    std::atomic<PriorityLevel> initialPriority{PriorityLevel::DEFAULT_OR_NORMAL};
    std::atomic<PriorityLevel> workerPriorityAfterEndOverride{PriorityLevel::DISPLAY};

    std::thread worker([&]() {
        JobSystem::setThreadPriority(JobSystem::Priority::BACKGROUND);
        initialPriority.store(getCurrentThreadPriorityLevel(), std::memory_order_relaxed);
        override.registerCurrentThread();

        // 1. Worker calls restorePriority() BEFORE startOverride() has executed.
        override.restorePriority();
        workerRestored.store(true, std::memory_order_release);

        // 2. Wait until waiter executes late startOverride().
        while (!overrideStarted.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        // 3. Wait until waiter calls endOverride().
        while (!overrideEnded.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        workerPriorityAfterEndOverride.store(getCurrentThreadPriorityLevel(), std::memory_order_relaxed);
        JobSystem::setThreadPriority(JobSystem::Priority::NORMAL);
    });

    // Wait until worker has completed its restorePriority() call.
    while (!workerRestored.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }

    // Waiter triggers late startOverride() after worker's restorePriority() finished.
    override.startOverride();
    if constexpr (isPriorityOverrideSupported()) {
        EXPECT_TRUE(override.isOverridden());
    }
    overrideStarted.store(true, std::memory_order_release);

    // endOverride() must restore the worker's thread priority on Android & Windows.
    override.endOverride();
    overrideEnded.store(true, std::memory_order_release);
    EXPECT_FALSE(override.isOverridden());

    worker.join();
    EXPECT_EQ(workerPriorityAfterEndOverride.load(), initialPriority.load());
}




