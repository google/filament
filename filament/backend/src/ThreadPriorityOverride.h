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

#include <atomic>

#if defined(__APPLE__)
#include <pthread.h>
#elif defined(__ANDROID__)
#include <sys/types.h>
#elif defined(WIN32)
#include <windows.h>
#endif

namespace filament::backend {

/**
 * ThreadPriorityOverride is a cross-platform helper class to mitigate priority inversion.
 *
 * Priority inversion occurs when a high-priority thread (e.g., the main rendering / GL driver thread)
 * blocks waiting on a low-priority thread (e.g., a background shader compilation worker).
 *
 * Synchronization Model:
 * ----------------------
 * There are two cooperating threads per override instance:
 * 1. Worker Thread: Executes the compilation job. Calls registerCurrentThread() before starting
 *    and restorePriority() upon completion.
 * 2. Waiting Thread: Needs the result immediately. Calls startOverride() before waiting on the
 *    condition variable and endOverride() after waking up.
 *
 * To avoid race conditions between registration and elevation, a sequentially consistent
 * (std::memory_order_seq_cst) handshake is used between mOverrideRequested and mTid / mThread:
 * - If startOverride() executes first: it sets mOverrideRequested = true. When the worker calls
 *   registerCurrentThread(), it detects mOverrideRequested and escalates its own priority.
 * - If registerCurrentThread() executes first: it stores mTid / mThread. When the waiting thread
 *   calls startOverride(), it detects the registered ID and escalates the worker's priority.
 * - If both run concurrently: sequentially consistent ordering guarantees at least one thread
 *   will observe the other's flag and apply the priority elevation.
 *
 * Calling Thread Protection:
 * --------------------------
 * When a job is dequeued and executed synchronously inline by the waiting/driver thread, or when a
 * background job completes without any thread waiting on it, priority is never escalated.
 * The mOverridden flag guarantees that priority restoration is strictly a no-op unless priority
 * was actually elevated. When priority is restored, it returns to the thread's recorded original
 * baseline priority (rather than a hardcoded background level), preventing calling threads or
 * workers at arbitrary baseline priorities from being demoted.
 *
 * Platform Mechanisms:
 * --------------------
 * - Apple (macOS / iOS): Uses pthread_override_qos_class_start_np / end_np to apply reference-counted
 *   QoS overrides (QOS_CLASS_USER_INTERACTIVE). Atomic handles ensure thread-safe cleanup.
 * - Android: Uses setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_DISPLAY) to elevate the nice value,
 *   and restores it to the recorded original baseline priority upon completion.
 * - Windows: Uses DuplicateHandle and SetThreadPriority(THREAD_PRIORITY_HIGHEST), restoring to the
 *   recorded original baseline priority upon completion.
 * - Other platforms: Compiles as a lightweight no-op.
 */
class ThreadPriorityOverride {
public:
    explicit ThreadPriorityOverride(bool enabled = true) noexcept;
    ~ThreadPriorityOverride() noexcept;

    ThreadPriorityOverride(ThreadPriorityOverride const&) = delete;
    ThreadPriorityOverride& operator=(ThreadPriorityOverride const&) = delete;
    ThreadPriorityOverride(ThreadPriorityOverride&&) = delete;
    ThreadPriorityOverride& operator=(ThreadPriorityOverride&&) = delete;

    /**
     * Called by the worker thread to register its native thread handle/ID and record its
     * current original priority before any override takes place.
     * This must be called from the executing thread before beginning the job.
     */
    void registerCurrentThread() noexcept;

    /**
     * Called by the waiting thread to temporarily elevate the priority of the registered
     * worker thread to prevent priority inversion.
     */
    void startOverride() noexcept;

    /**
     * Called by the waiting thread to release the temporary priority elevation.
     */
    void endOverride() noexcept;

    /**
     * Called by the worker thread after completing its job to drop its priority back to its
     * original baseline level if it was previously overridden.
     */
    void restorePriority() noexcept;

    /**
     * Returns true if an override has been requested by the waiting thread.
     */
    bool isOverrideRequested() const noexcept;

    /**
     * Returns true if priority elevation is currently active on this instance.
     */
    bool isOverridden() const noexcept;

private:
#if defined(__APPLE__)
    void applyOverride(pthread_t thread) noexcept;
#endif

    bool const mEnabled = true;

    // Set to true by the waiting thread in startOverride().
    // Read by registerCurrentThread() to detect early override requests.
    // Memory Order: std::memory_order_seq_cst to prevent reordering with mTid / mThread.
    std::atomic<bool> mOverrideRequested{false};

#if defined(__APPLE__)
    // Native pthread handle of the worker thread.
    // Stored in registerCurrentThread() and read in startOverride().
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<pthread_t> mThread{nullptr};

    // Active QoS override handle returned by pthread_override_qos_class_start_np().
    // Managed atomically to avoid leaks when startOverride() and registerCurrentThread() race.
    // Memory Order: std::memory_order_seq_cst on creation/exchange, relaxed on destruction.
    std::atomic<pthread_override_t> mOverride{nullptr};
#elif defined(__ANDROID__)
    // Native Linux thread ID (TID) of the worker thread from gettid().
    // Stored in registerCurrentThread() and read in startOverride().
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<pid_t> mTid{0};

    // Original nice value of the worker thread recorded during registerCurrentThread().
    // Used to restore the exact baseline priority regardless of what priority it started at.
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<int> mOriginalPriority{0};

    // Tracks whether this worker thread's nice value was escalated to ANDROID_PRIORITY_DISPLAY.
    // Guards restorePriority() so that non-elevated or inline calling threads are untouched.
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<bool> mOverridden{false};
#elif defined(WIN32)
    // Duplicate handle to the worker thread for cross-thread SetThreadPriority() calls.
    // Stored in registerCurrentThread(), closed in ~ThreadPriorityOverride().
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<HANDLE> mThread{nullptr};

    // Original thread priority level recorded during registerCurrentThread().
    // Used to restore the exact baseline priority regardless of what priority it started at.
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<int> mOriginalPriority{0};

    // Tracks whether this thread's priority was escalated to THREAD_PRIORITY_HIGHEST.
    // Guards restorePriority() so that non-elevated or inline calling threads are untouched.
    // Memory Order: std::memory_order_seq_cst.
    std::atomic<bool> mOverridden{false};
#endif
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_THREADPRIORITYOVERRIDE_H
