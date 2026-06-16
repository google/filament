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

#ifndef TNT_FILAMENT_BACKEND_THREADPRIORITYOVERRIDE_H
#define TNT_FILAMENT_BACKEND_THREADPRIORITYOVERRIDE_H

#include <utils/JobSystem.h>

#include <atomic>

#if defined(__APPLE__)
#include <pthread.h>
#elif defined(__ANDROID__)
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>

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

/**
 * ThreadPriorityOverride is a cross-platform helper class to mitigate priority inversion.
 *
 * Priority inversion occurs when a high-priority thread (e.g. the main rendering thread)
 * blocks waiting for a low-priority thread (e.g. a background shader compilation thread).
 *
 * This class allows the waiting thread to temporarily escalate the priority of the compiling
 * thread to USER_INTERACTIVE / DISPLAY level. Once the worker finishes its job, the priority
 * is restored back to the baseline background priority.
 *
 * Note: On unsupported platforms (such as standard Linux or WebGL), this class compiles as a no-op.
 */
class ThreadPriorityOverride {
public:
    ThreadPriorityOverride(bool enabled = true) noexcept : mEnabled(enabled) {}

    ~ThreadPriorityOverride() noexcept {
#if defined(WIN32)
        HANDLE thread = mThread.load(std::memory_order_relaxed);
        if (thread) {
            CloseHandle(thread);
        }
#endif
    }

    ThreadPriorityOverride(ThreadPriorityOverride const&) = delete;
    ThreadPriorityOverride& operator=(ThreadPriorityOverride const&) = delete;
    ThreadPriorityOverride(ThreadPriorityOverride&&) = delete;
    ThreadPriorityOverride& operator=(ThreadPriorityOverride&&) = delete;

    /**
     * Called by the worker thread to register its native thread handle/ID.
     * This must be called from the worker thread itself before starting the job.
     */
    void registerCurrentThread() noexcept {
        if (!mEnabled) {
            return;
        }
#if defined(__APPLE__)
        mThread.store(pthread_self(), std::memory_order_release);
#elif defined(__ANDROID__)
        mTid.store(gettid(), std::memory_order_release);
#elif defined(WIN32)
        HANDLE hRealThread = nullptr;
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hRealThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
        mThread.store(hRealThread, std::memory_order_release);
#endif
    }

    /**
     * Called by the waiting thread to temporarily elevate the priority of the registered
     * worker thread to prevent priority inversion.
     */
    void startOverride() noexcept {
        if (!mEnabled) {
            return;
        }
#if defined(__APPLE__)
        pthread_t thread = mThread.load(std::memory_order_acquire);
        if (thread) {
            mOverride = pthread_override_qos_class_start_np(thread, QOS_CLASS_USER_INTERACTIVE, 0);
        }
#elif defined(__ANDROID__)
        pid_t tid = mTid.load(std::memory_order_acquire);
        if (tid) {
            setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_DISPLAY);
        }
#elif defined(WIN32)
        HANDLE thread = mThread.load(std::memory_order_acquire);
        if (thread) {
            SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
        }
#endif
    }

    /**
     * Called by the waiting thread to release the temporary priority elevation.
     * On Apple, this tears down the pthread QoS override.
     */
    void endOverride() noexcept {
        if (!mEnabled) {
            return;
        }
#if defined(__APPLE__)
        if (mOverride) {
            pthread_override_qos_class_end_np(mOverride);
            mOverride = nullptr;
        }
#endif
    }

    /**
     * Called by the worker thread after completing its job to drop its priority
     * back to the default background level.
     * On Android and Windows, this sets the thread nice value/priority back to the target.
     */
    void restoreDefaultPriority(utils::JobSystem::Priority priority) noexcept {
        if (!mEnabled) {
            return;
        }
#if defined(__ANDROID__)
        int niceValue = ANDROID_PRIORITY_NORMAL;
        switch (priority) {
            case utils::JobSystem::Priority::BACKGROUND:
                niceValue = ANDROID_PRIORITY_BACKGROUND;
                break;
            case utils::JobSystem::Priority::NORMAL:
                niceValue = ANDROID_PRIORITY_NORMAL;
                break;
            case utils::JobSystem::Priority::DISPLAY:
                niceValue = ANDROID_PRIORITY_DISPLAY;
                break;
            case utils::JobSystem::Priority::URGENT_DISPLAY:
                niceValue = ANDROID_PRIORITY_URGENT_DISPLAY;
                break;
        }
        setpriority(PRIO_PROCESS, 0, niceValue);
#elif defined(WIN32)
        HANDLE thread = mThread.load(std::memory_order_acquire);
        if (thread) {
            int winPriority = THREAD_PRIORITY_NORMAL;
            switch (priority) {
                case utils::JobSystem::Priority::BACKGROUND:
                    winPriority = THREAD_PRIORITY_BELOW_NORMAL;
                    break;
                case utils::JobSystem::Priority::NORMAL:
                    winPriority = THREAD_PRIORITY_NORMAL;
                    break;
                case utils::JobSystem::Priority::DISPLAY:
                    winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
                    break;
                case utils::JobSystem::Priority::URGENT_DISPLAY:
                    winPriority = THREAD_PRIORITY_HIGHEST;
                    break;
            }
            SetThreadPriority(thread, winPriority);
        }
#endif
    }

private:
    bool const mEnabled = true;
#if defined(__APPLE__)
    std::atomic<pthread_t> mThread{nullptr};
    pthread_override_t mOverride{nullptr};
#elif defined(__ANDROID__)
    std::atomic<pid_t> mTid{0};
#elif defined(WIN32)
    std::atomic<HANDLE> mThread{nullptr};
#endif
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_THREADPRIORITYOVERRIDE_H
