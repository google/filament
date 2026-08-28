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

#include <atomic>

#if defined(__APPLE__)
#include <pthread.h>
#include <sys/qos.h>
#elif defined(__ANDROID__)
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

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

namespace filament::backend {

ThreadPriorityOverride::ThreadPriorityOverride(bool const enabled) noexcept
    : mEnabled(enabled) {
}

ThreadPriorityOverride::~ThreadPriorityOverride() noexcept {
#if defined(__APPLE__)
    // Release any active QoS override handle that was not torn down by endOverride().
    // Uses relaxed ordering because the destructor has exclusive ownership of the object.
    pthread_override_t override = mOverride.exchange(nullptr, std::memory_order_relaxed);
    if (override) {
        pthread_override_qos_class_end_np(override);
    }
#elif defined(__ANDROID__)
    // Restore original priority if the override was active and not previously restored.
    if (mOverridden.exchange(false, std::memory_order_relaxed)) {
        pid_t const tid = mTid.load(std::memory_order_relaxed);
        if (tid) {
            int const originalNice = mOriginalPriority.load(std::memory_order_relaxed);
            setpriority(PRIO_PROCESS, tid, originalNice);
        }
    }
#elif defined(WIN32)
    // Restore original priority if the override was active and not previously restored.
    if (mOverridden.exchange(false, std::memory_order_relaxed)) {
        HANDLE const thread = mThread.load(std::memory_order_relaxed);
        if (thread) {
            int const originalPriority = mOriginalPriority.load(std::memory_order_relaxed);
            SetThreadPriority(thread, originalPriority);
        }
    }
    // Close the duplicated thread handle allocated in registerCurrentThread().
    HANDLE thread = mThread.load(std::memory_order_relaxed);
    if (thread) {
        CloseHandle(thread);
    }
#endif
}

void ThreadPriorityOverride::registerCurrentThread() noexcept {
    if (!mEnabled) {
        return;
    }
#if defined(__APPLE__)
    pthread_t const self = pthread_self();
    mThread.store(self, std::memory_order_seq_cst);
    if (mOverrideRequested.load(std::memory_order_seq_cst)) {
        applyOverride(self);
    }
#elif defined(__ANDROID__)
    pid_t const tid = gettid();
    errno = 0;
    int const originalNice = getpriority(PRIO_PROCESS, 0);
    if (errno == 0) {
        mOriginalPriority.store(originalNice, std::memory_order_seq_cst);
    }
    mTid.store(tid, std::memory_order_seq_cst);
    if (mOverrideRequested.load(std::memory_order_seq_cst)) {
        setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_DISPLAY);
        mOverridden.store(true, std::memory_order_seq_cst);
    }
#elif defined(WIN32)
    HANDLE hRealThread = nullptr;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hRealThread,
            0, FALSE, DUPLICATE_SAME_ACCESS);
    int const originalPriority = GetThreadPriority(GetCurrentThread());
    if (originalPriority != THREAD_PRIORITY_ERROR_RETURN) {
        mOriginalPriority.store(originalPriority, std::memory_order_seq_cst);
    }
    mThread.store(hRealThread, std::memory_order_seq_cst);
    if (hRealThread && mOverrideRequested.load(std::memory_order_seq_cst)) {
        SetThreadPriority(hRealThread, THREAD_PRIORITY_HIGHEST);
        mOverridden.store(true, std::memory_order_seq_cst);
    }
#endif
}

void ThreadPriorityOverride::startOverride() noexcept {
    if (!mEnabled) {
        return;
    }
    mOverrideRequested.store(true, std::memory_order_seq_cst);
#if defined(__APPLE__)
    pthread_t const thread = mThread.load(std::memory_order_seq_cst);
    if (thread) {
        applyOverride(thread);
    }
#elif defined(__ANDROID__)
    pid_t const tid = mTid.load(std::memory_order_seq_cst);
    if (tid) {
        setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_DISPLAY);
        mOverridden.store(true, std::memory_order_seq_cst);
    }
#elif defined(WIN32)
    HANDLE const thread = mThread.load(std::memory_order_seq_cst);
    if (thread) {
        SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
        mOverridden.store(true, std::memory_order_seq_cst);
    }
#endif
}

void ThreadPriorityOverride::endOverride() noexcept {
    if (!mEnabled) {
        return;
    }
#if defined(__APPLE__)
    pthread_override_t const override = mOverride.exchange(nullptr, std::memory_order_seq_cst);
    if (override) {
        pthread_override_qos_class_end_np(override);
    }
#elif defined(__ANDROID__)
    if (mOverridden.exchange(false, std::memory_order_seq_cst)) {
        pid_t const tid = mTid.load(std::memory_order_seq_cst);
        if (tid) {
            int const originalNice = mOriginalPriority.load(std::memory_order_seq_cst);
            setpriority(PRIO_PROCESS, tid, originalNice);
        }
    }
#elif defined(WIN32)
    if (mOverridden.exchange(false, std::memory_order_seq_cst)) {
        HANDLE const thread = mThread.load(std::memory_order_seq_cst);
        if (thread) {
            int const originalPriority = mOriginalPriority.load(std::memory_order_seq_cst);
            SetThreadPriority(thread, originalPriority);
        }
    }
#endif
}

void ThreadPriorityOverride::restorePriority() noexcept {
    if (!mEnabled) {
        return;
    }
#if defined(__ANDROID__)
    if (mOverridden.exchange(false, std::memory_order_seq_cst)) {
        int const originalNice = mOriginalPriority.load(std::memory_order_seq_cst);
        // 0 specifies the calling thread (the worker thread).
        setpriority(PRIO_PROCESS, 0, originalNice);
    }
#elif defined(WIN32)
    if (mOverridden.exchange(false, std::memory_order_seq_cst)) {
        HANDLE const thread = mThread.load(std::memory_order_seq_cst);
        if (thread) {
            int const originalPriority = mOriginalPriority.load(std::memory_order_seq_cst);
            SetThreadPriority(thread, originalPriority);
        }
    }
#endif
}

bool ThreadPriorityOverride::isOverrideRequested() const noexcept {
    return mOverrideRequested.load(std::memory_order_acquire);
}

bool ThreadPriorityOverride::isOverridden() const noexcept {
#if defined(__APPLE__)
    return mOverride.load(std::memory_order_acquire) != nullptr;
#elif defined(__ANDROID__) || defined(WIN32)
    return mOverridden.load(std::memory_order_acquire);
#else
    return false;
#endif
}

#if defined(__APPLE__)
void ThreadPriorityOverride::applyOverride(pthread_t const thread) noexcept {
    pthread_override_t expected = nullptr;
    pthread_override_t const overrideQos =
            pthread_override_qos_class_start_np(thread, QOS_CLASS_USER_INTERACTIVE, 0);
    if (overrideQos) {
        if (!mOverride.compare_exchange_strong(expected, overrideQos, std::memory_order_seq_cst)) {
            pthread_override_qos_class_end_np(overrideQos);
        }
    }
}
#endif

} // namespace filament::backend
