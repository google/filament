/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include "state_tracker/state_object.h"
#include "state_tracker/fence_state.h"
#include "state_tracker/semaphore_state.h"
#include <condition_variable>
#include <deque>
#include <future>
#include <thread>
#include <vector>
#include <string>
#include "error_message/error_location.h"

namespace vvl {

class CommandBuffer;
class Device;
class Queue;

struct CommandBufferSubmission {
    std::shared_ptr<vvl::CommandBuffer> cb;
    // Specifically made for GPU-AV, for it has unique problems: Error reporting is done *after*
    // command buffer submissions, not at Pre/PostCall time.
    // Contrary to sync val, GPU-AV cannot just look at `GetQueueState()->cmdbuf_label_stack`
    // to construct an initial label stack. sync-val can do that because validation and error reporting is done
    // *before* a command buffer list is submitted: validation is performed one command buffer at a time,
    // and `GetQueueState()->cmdbuf_label_stack` is updated between those validations.
    // When GPU-AV starts doing error reporting, when command buffers have completed,
    // the label stack info stored in Queue state is lost.
    // => GPU-AV needs to track this initial label stack per command buffer submission.
    std::vector<std::string> initial_label_stack;

    CommandBufferSubmission(std::shared_ptr<vvl::CommandBuffer> cb, std::vector<std::string> initial_label_stack)
        : cb(std::move(cb)), initial_label_stack(std::move(initial_label_stack)) {}
    CommandBufferSubmission(CommandBufferSubmission &&other)
        : cb(std::move(other.cb)), initial_label_stack(std::move(other.initial_label_stack)) {}
    CommandBufferSubmission &operator=(const CommandBufferSubmission &other) = default;
    CommandBufferSubmission(const CommandBufferSubmission &) = default;
};

struct QueueSubmission {
    QueueSubmission(const Location &loc_) : loc(loc_), completed(), waiter(completed.get_future()) {}

    bool end_batch{false};
    std::vector<vvl::CommandBufferSubmission> cb_submissions{};

    std::vector<SemaphoreInfo> wait_semaphores;
    std::vector<SemaphoreInfo> signal_semaphores;
    std::shared_ptr<Fence> fence;
    LocationCapture loc;
    uint64_t seq{0};
    uint32_t perf_submit_pass{0};
    std::promise<void> completed;
    std::shared_future<void> waiter;

    void AddCommandBuffer(std::shared_ptr<vvl::CommandBuffer> cb_state, std::vector<std::string> initial_label_stack) {
        cb_submissions.emplace_back(std::move(cb_state), std::move(initial_label_stack));
    }

    void AddSignalSemaphore(std::shared_ptr<Semaphore> &&semaphore_state, uint64_t value) {
        signal_semaphores.emplace_back(std::move(semaphore_state), value);
    }

    void AddWaitSemaphore(std::shared_ptr<Semaphore> &&semaphore_state, uint64_t value) {
        wait_semaphores.emplace_back(std::move(semaphore_state), value);
    }

    void AddFence(std::shared_ptr<Fence> &&fence_state) { fence = std::move(fence_state); }

    void EndUse();
    void BeginUse();
};

// This timeout is for all queue threads to update their state after we know
// (via being in a PostRecord call) that a fence, semaphore or wait for idle has
// completed. Hitting it is almost a certainly a bug in this code.
static inline std::chrono::time_point<std::chrono::steady_clock> GetCondWaitTimeout() {
    return std::chrono::steady_clock::now() + std::chrono::seconds(10);
}

struct PreSubmitResult {
    uint64_t last_submission_seq = 0;

    bool has_external_fence = false;
    uint64_t submission_with_external_fence_seq = 0;
};

class Queue : public StateObject {
  public:
    Queue(Device &dev_data, VkQueue handle, uint32_t family_index, uint32_t queue_index, VkDeviceQueueCreateFlags flags,
          const VkQueueFamilyProperties &queueFamilyProperties)
        : StateObject(handle, kVulkanObjectTypeQueue),
          queue_family_index(family_index),
          queue_index(queue_index),
          create_flags(flags),
          queue_family_properties(queueFamilyProperties),
          dev_data_(dev_data) {}

    ~Queue() { Destroy(); }
    void Destroy() override;

    VkQueue VkHandle() const { return handle_.Cast<VkQueue>(); }

    // called from the various PreCallRecordQueueSubmit() methods
    virtual PreSubmitResult PreSubmit(std::vector<QueueSubmission> &&submissions);
    // called from the various PostCallRecordQueueSubmit() methods
    void PostSubmit();

    // Tell the queue thread that submissions up to and including the submission with
    // sequence number until_seq have finished. kU64Max means to finish all submissions.
    void Notify(uint64_t until_seq = kU64Max);

    // Wait for the queue thread to finish processing submissions with sequence numbers
    // up to and including until_seq. kU64Max means to finish all submissions.
    void Wait(const Location &loc, uint64_t until_seq = kU64Max);

    // Helper that combines Notify and Wait
    void NotifyAndWait(const Location &loc, uint64_t until_seq = kU64Max);

    // Find a timeline wait that does not have a resolving signal submitted yet.
    // Check submissions up to and including until_seq.
    std::optional<SemaphoreInfo> FindTimelineWaitWithoutResolvingSignal(uint64_t until_seq) const;

  public:
    // Queue family index. As queueFamilyIndex parameter in vkGetDeviceQueue.
    const uint32_t queue_family_index;

    // Index of the queue within a queue family. As queueIndex parameter in vkGetDeviceQueue.
    const uint32_t queue_index;

    const VkDeviceQueueCreateFlags create_flags;
    const VkQueueFamilyProperties queue_family_properties;

    // Track command buffer label stack accross all command buffers submitted to this queue.
    // Access to this variable relies on external queue synchronization.
    std::vector<std::string> cmdbuf_label_stack;

    // Track the last closed label. It is used in the error messages to help locate unbalanced vkCmdEndDebugUtilsLabelEXT command.
    // Access to this variable relies on external queue synchronization.
    std::string last_closed_cmdbuf_label;

    // Stop per-queue label tracking after the first label mismatch error.
    // Access to this variable relies on external queue synchronization.
    bool found_unbalanced_cmdbuf_label = false;

  protected:
    // called from the various PostCallRecordQueueSubmit() methods
    virtual void PostSubmit(QueueSubmission &submission) {}
    // called when the worker thread decides a submissions has finished executing
    virtual void Retire(QueueSubmission &submission);

  private:
    uint32_t timeline_wait_count_ = 0;

  private:
    using LockGuard = std::unique_lock<std::mutex>;
    void ThreadFunc();
    QueueSubmission *NextSubmission();
    LockGuard Lock() const { return LockGuard(lock_); }

    Device &dev_data_;

    // state related to submitting to the queue, all data members must
    // be accessed with lock_ held
    std::unique_ptr<std::thread> thread_;
    std::deque<QueueSubmission> submissions_;
    std::atomic<uint64_t> seq_{0};
    uint64_t request_seq_{0};
    bool exit_thread_{false};
    mutable std::mutex lock_;
    // condition to wake up the queue's thread
    std::condition_variable cond_;
};
}  // namespace vvl
