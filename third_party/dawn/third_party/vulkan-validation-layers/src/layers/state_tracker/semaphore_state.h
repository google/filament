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
#include "state_tracker/submission_reference.h"
#include <future>
#include <map>
#include <shared_mutex>
#include "containers/custom_containers.h"
#include "error_message/error_location.h"

namespace vvl {

class Device;
class Queue;
class Semaphore;

struct SemaphoreInfo {
    SemaphoreInfo(std::shared_ptr<Semaphore> &&sem, uint64_t pl) : semaphore(std::move(sem)), payload(pl) {}
    std::shared_ptr<Semaphore> semaphore;
    uint64_t payload{0};
};

class Semaphore : public RefcountedStateObject {
  public:
    enum OpType {
        kNone,
        kWait,
        kSignal,
        kBinaryAcquire,
    };
    enum Scope {
        kInternal,
        kExternalTemporary,
        kExternalPermanent,
    };

    struct SemOp {
        OpType op_type;
        uint64_t payload;
        SubmissionReference submit;  // Used only by binary semaphores
        std::optional<Func> acquire_command;

        SemOp(OpType op_type, const SubmissionReference &submit, uint64_t payload)
            : op_type(op_type), payload(payload), submit(submit) {}
        SemOp(Func acquire_command, uint64_t payload)
            : op_type(kBinaryAcquire), payload(payload), acquire_command(acquire_command) {}
    };

    struct TimePoint {
        std::optional<SubmissionReference> signal_submit;
        small_vector<SubmissionReference, 1, uint32_t> wait_submits;
        std::optional<Func> acquire_command;
        std::promise<void> completed;
        std::shared_future<void> waiter;

        TimePoint() : completed(), waiter(completed.get_future()) {}
        bool HasSignaler() const { return signal_submit.has_value() || acquire_command.has_value(); }
        bool HasWaiters() const { return !wait_submits.empty(); }
        void Notify() const;
    };

    Semaphore(Device &dev, VkSemaphore handle, const VkSemaphoreCreateInfo *pCreateInfo)
        : Semaphore(dev, handle, vku::FindStructInPNextChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext), pCreateInfo) {}

    std::shared_ptr<const Semaphore> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<Semaphore> shared_from_this() { return SharedFromThisImpl(this); }

    const VulkanTypedHandle *InUse() const override;
    VkSemaphore VkHandle() const { return handle_.Cast<VkSemaphore>(); }
    enum Scope Scope() const;

    // Enqueue a semaphore operation. For binary semaphores, the payload value is generated and
    // returned, so that every semaphore operation has a unique value.
    void EnqueueSignal(const SubmissionReference &signal_submit, uint64_t &payload);
    void EnqueueWait(const SubmissionReference &wait_submit, uint64_t &payload);

    // Enqueue binary semaphore signal from swapchain image acquire command
    void EnqueueAcquire(Func acquire_command);

    // Process wait by retiring timeline timepoints up to the specified payload.
    // If there is un-retired resolving signal then wait until another queue or a host retires timepoints instead.
    // queue_thread determines if this function is called by a queue thread or by the validation object.
    // (validation object has to use {Begin/End}BlockingOperation() when waiting for the timepoint)
    void RetireWait(Queue *current_queue, uint64_t payload, const Location &loc, bool queue_thread = false);

    // Process signal by retiring timeline timepoints up to the specified payload
    void RetireSignal(uint64_t payload);

    // Look for most recent / highest payload operation that matches
    std::optional<SemOp> LastOp(const std::function<bool(OpType op_type, uint64_t payload, bool is_pending)> &filter) const;

    // Returns pending queue submission that signals this binary semaphore.
    std::optional<SubmissionReference> GetPendingBinarySignalSubmission() const;

    // Returns pending queue submission that waits on this binary semaphore.
    std::optional<SubmissionReference> GetPendingBinaryWaitSubmission() const;

    // If a pending binary signal depends on an unresolved timeline wait, this function
    // returns information about the timeline wait; otherwise, it returns an empty result.
    // This is used to validate VUs (such as VUID-vkQueueSubmit-pWaitSemaphores-03238) that have this statement:
    // "and any semaphore signal operations on which it depends must have also been submitted for execution"
    std::optional<SemaphoreInfo> GetPendingBinarySignalTimelineDependency() const;  

    // Current payload value.
    // If a queue submission command is pending execution, then the returned value may immediately be out of date
    uint64_t CurrentPayload() const;

    bool CanBinaryBeSignaled() const;
    bool CanBinaryBeWaited() const;

    void GetLastBinarySignalSource(VkQueue &queue, vvl::Func &acquire_command) const;
    bool HasResolvingTimelineSignal(uint64_t wait_payload) const;

    void Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags);
    void Export(VkExternalSemaphoreHandleTypeFlagBits handle_type);
    std::optional<VkExternalSemaphoreHandleTypeFlagBits> ImportedHandleType() const;

    const VkSemaphoreType type;
    const VkSemaphoreCreateFlags flags;
    const VkExternalSemaphoreHandleTypeFlags export_handle_types;
    const uint64_t initial_value;  // for timelines

#ifdef VK_USE_PLATFORM_METAL_EXT
    static bool GetMetalExport(const VkSemaphoreCreateInfo *info);
    const bool metal_semaphore_export;
#endif  // VK_USE_PLATFORM_METAL_EXT

  private:
    Semaphore(Device &dev, VkSemaphore handle, const VkSemaphoreTypeCreateInfo *type_create_info,
              const VkSemaphoreCreateInfo *pCreateInfo);

    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    // Return true if timepoint has no dependencies and can be retired.
    // If there is unresolved wait then notify signaling queue (if there is registered signal) and return false
    bool CanRetireBinaryWait(TimePoint &timepoint) const;
    bool CanRetireTimelineWait(const vvl::Queue *current_queue, uint64_t payload) const;

    // Mark timepoints up to and including payload as completed (notify waiters) and remove them from timeline
    void RetireTimePoint(uint64_t payload, OpType completed_op, SubmissionReference completed_submit);

    // Waits for the waiter. Unblock parameter must be true if the caller is a validation object and false otherwise.
    // (validation object has to use {Begin/End}BlockingOperation() when waiting for the timepoint)
    void WaitTimePoint(std::shared_future<void> &&waiter, uint64_t payload, bool unblock_validation_object, const Location &loc);

  private:
    enum Scope scope_ { kInternal };
    std::optional<VkExternalSemaphoreHandleTypeFlagBits> imported_handle_type_;  // has value when scope is not kInternal

    // the most recently completed operation
    SemOp completed_;
    // next payload value for binary semaphore operations
    uint64_t next_payload_;

    // Set of pending operations ordered by payload.
    // Timeline operations can be added in any order and multiple wait operations
    // can use the same payload value.
    std::map<uint64_t, TimePoint> timeline_;
    mutable std::shared_mutex lock_;
    Device &dev_data_;
};

}  // namespace vvl
