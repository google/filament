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
#include "state_tracker/semaphore_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/state_tracker.h"

static bool CanSignalBinarySemaphoreAfterOperation(vvl::Semaphore::OpType op_type) {
    return op_type == vvl::Semaphore::kNone || op_type == vvl::Semaphore::kWait;
}

static bool CanWaitBinarySemaphoreAfterOperation(vvl::Semaphore::OpType op_type) {
    return op_type == vvl::Semaphore::kSignal || op_type == vvl::Semaphore::kBinaryAcquire;
}

static VkExternalSemaphoreHandleTypeFlags GetExportHandleTypes(const VkSemaphoreCreateInfo *pCreateInfo) {
    auto export_info = vku::FindStructInPNextChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
    return export_info ? export_info->handleTypes : 0;
}

void vvl::Semaphore::TimePoint::Notify() const {
    assert(signal_submit.has_value() && signal_submit->queue);
    signal_submit->queue->Notify(signal_submit->seq);
}

vvl::Semaphore::Semaphore(Device &dev, VkSemaphore handle, const VkSemaphoreTypeCreateInfo *type_create_info,
                          const VkSemaphoreCreateInfo *pCreateInfo)
    : RefcountedStateObject(handle, kVulkanObjectTypeSemaphore),
      type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
      flags(pCreateInfo->flags),
      export_handle_types(GetExportHandleTypes(pCreateInfo)),
      initial_value(type == VK_SEMAPHORE_TYPE_TIMELINE ? type_create_info->initialValue : 0),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_semaphore_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      completed_{type == VK_SEMAPHORE_TYPE_TIMELINE ? kSignal : kNone, SubmissionReference{},
                 type_create_info ? type_create_info->initialValue : 0},
      next_payload_(completed_.payload + 1),
      dev_data_(dev) {
}

const VulkanTypedHandle *vvl::Semaphore::InUse() const {
    auto guard = ReadLock();
    // Semaphore does not have a parent (in the sense of a VVL state object), and the value returned
    // by the base class InUse is not useful for reporting (it is the semaphore's own handle)
    const bool in_use = RefcountedStateObject::InUse() != nullptr;
    if (!in_use) {
        return nullptr;
    }
    // Scan timeline to find the first queue that uses the semaphore
    for (const auto &[_, timepoint] : timeline_) {
        if (timepoint.signal_submit.has_value() && timepoint.signal_submit->queue) {
            return &timepoint.signal_submit->queue->Handle();
        } else {
            for (const SubmissionReference &wait_submit : timepoint.wait_submits) {
                if (wait_submit.queue) {
                    return &wait_submit.queue->Handle();
                }
            }
        }
    }
    // NOTE: In current implementation timepoints represent pending state. In-use tracking
    // can retire timepoint even if submission is still pending, so timeline_ state it's
    // always pending state but empty timeline does not mean there is no pending state.
    // We don't make stronger guarantees because it's enough for in-use tracking.
    // You can use NegativeSyncObject.TimelineSubmitSignalAndInUseTracking to check for
    // a scenario when there is pending submission and timeline is empty.
    //
    // This should be taken into account when semaphore is used by functionality other than
    // in-use tracking. In the following code we check completed_ state in case pending queue
    // cannot be derived from timeline_. It's a bit unconventional. Maybe we need better
    // separation between in-use tracking on other type of functionality. Or maybe it's about
    // better definitions.
    if (completed_.submit.queue) {
        return &completed_.submit.queue->Handle();
    }
    assert(false && "Can't find queue that uses the semaphore");
    static const VulkanTypedHandle empty{};
    return &empty;
}

enum vvl::Semaphore::Scope vvl::Semaphore::Scope() const {
    auto guard = ReadLock();
    return scope_;
}

void vvl::Semaphore::EnqueueSignal(const SubmissionReference &signal_submit, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    }
    // Check there is no existing signal, validation should enforce this
    assert(timeline_.find(payload) == timeline_.end() || !timeline_.find(payload)->second.signal_submit.has_value());

    timeline_[payload].signal_submit.emplace(signal_submit);
}

void vvl::Semaphore::EnqueueWait(const SubmissionReference &wait_submit, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        if (timeline_.empty()) {
            if (scope_ != vvl::Semaphore::kInternal) {
                // for external semaphore mark wait as completed, no guarantee of signal visibility
                completed_ = SemOp(kWait, wait_submit, 0);
                return;
            } else {
                // generate binary payload value from the last completed signals
                assert(completed_.op_type == kSignal);
                payload = completed_.payload;
            }
        } else {
            // generate binary payload value from the most recent pending binary signal
            assert(timeline_.rbegin()->second.HasSignaler());
            payload = timeline_.rbegin()->first;
        }
    }

    if (payload <= completed_.payload) {
        // Signal is already retired and its timepoint removed. Mark wait as completed.
        // NOTE: wait's submission can still be pending, but timepoint lifetime logic
        // is determined by the signal. completed_ is updated when signal is retired.
        // The matching waits should be resolved against completed_ in this case.
        assert(!vvl::Contains(timeline_, payload));
        completed_.op_type = kWait;
        completed_.submit = wait_submit;
        return;
    }

    timeline_[payload].wait_submits.emplace_back(wait_submit);
}

void vvl::Semaphore::EnqueueAcquire(vvl::Func acquire_command) {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = WriteLock();
    auto payload = next_payload_++;
    assert(timeline_.find(payload) == timeline_.end());
    timeline_[payload].acquire_command.emplace(acquire_command);
}

std::optional<vvl::Semaphore::SemOp> vvl::Semaphore::LastOp(const std::function<bool(OpType, uint64_t, bool)> &filter) const {
    auto guard = ReadLock();
    std::optional<SemOp> result;

    for (auto pos = timeline_.rbegin(); pos != timeline_.rend(); ++pos) {
        uint64_t payload = pos->first;
        auto &timepoint = pos->second;
        for (auto &op : timepoint.wait_submits) {
            if (!filter || filter(kWait, payload, true)) {
                result.emplace(SemOp(kWait, op, payload));
                break;
            }
        }
        if (!result && timepoint.signal_submit) {
            // vkSemaphoreSignal can't be a pending operation, it signals immediately
            const bool pending = timepoint.signal_submit->queue != nullptr;

            if (!filter || filter(kSignal, payload, pending)) {
                result.emplace(SemOp(kSignal, *timepoint.signal_submit, payload));
                break;
            }
        }
        if (!result && timepoint.acquire_command && (!filter || filter(kBinaryAcquire, payload, true))) {
            result.emplace(SemOp(*timepoint.acquire_command, payload));
            break;
        }
    }
    if (!result && (!filter || filter(completed_.op_type, completed_.payload, false))) {
        result.emplace(completed_);
    }
    return result;
}

std::optional<vvl::SubmissionReference> vvl::Semaphore::GetPendingBinarySignalSubmission() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const auto &timepoint = timeline_.rbegin()->second;
    const auto &signal_submit = timepoint.signal_submit;

    // Skip signals that are not associated with a queue
    if (signal_submit.has_value() && signal_submit->queue == nullptr) {
        return {};
    }
    return signal_submit;
}

std::optional<vvl::SubmissionReference> vvl::Semaphore::GetPendingBinaryWaitSubmission() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const auto &timepoint = timeline_.rbegin()->second;
    assert(timepoint.wait_submits.empty() || timepoint.wait_submits.size() == 1);

    // No waits
    if (timepoint.wait_submits.empty()) {
        return {};
    }
    // Skip waits that are not associated with a queue
    if (timepoint.wait_submits[0].queue == nullptr) {
        return {};
    }
    return timepoint.wait_submits[0];
}

std::optional<vvl::SemaphoreInfo> vvl::Semaphore::GetPendingBinarySignalTimelineDependency() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const TimePoint &timepoint = timeline_.rbegin()->second;
    assert(timepoint.HasSignaler());
    const auto &signal_submit = timepoint.signal_submit;

    // A signal not associated with a queue cannot be blocked by timeline wait
    // (host signal or image acquire signal)
    if (!signal_submit.has_value() || signal_submit->queue == nullptr) {
        return {};
    }

    return signal_submit->queue->FindTimelineWaitWithoutResolvingSignal(signal_submit->seq);
}

uint64_t vvl::Semaphore::CurrentPayload() const {
    auto guard = ReadLock();
    return completed_.payload;
}

bool vvl::Semaphore::CanBinaryBeSignaled() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return CanSignalBinarySemaphoreAfterOperation(completed_.op_type);
    }
    // Every timeline slot of binary semaphore should contain at least a signal.
    // Wait before signal is not allowed.
    assert(timeline_.rbegin()->second.HasSignaler());

    return timeline_.rbegin()->second.HasWaiters();
}

bool vvl::Semaphore::CanBinaryBeWaited() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return CanWaitBinarySemaphoreAfterOperation(completed_.op_type);
    }

    const TimePoint &timepoint = timeline_.rbegin()->second;

    assert(scope_ == vvl::Semaphore::kInternal);  // Ensured by all calling sites

    // Every timeline slot of binary semaphore should contain at least a signal.
    // Wait before signal is not allowed.
    assert(timepoint.HasSignaler());

    // Can wait if there are no waiters
    return !timepoint.HasWaiters();
}

void vvl::Semaphore::GetLastBinarySignalSource(VkQueue &queue, vvl::Func &acquire_command) const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    queue = VK_NULL_HANDLE;
    acquire_command = vvl::Func::Empty;

    auto guard = ReadLock();
    if (timeline_.empty()) {
        if (completed_.op_type == kSignal && completed_.submit.queue) {
            queue = completed_.submit.queue->VkHandle();
        } else if (completed_.op_type == kBinaryAcquire) {
            acquire_command = *completed_.acquire_command;
        }
    } else {
        const TimePoint &timepoint = timeline_.rbegin()->second;
        if (timepoint.signal_submit.has_value() && timepoint.signal_submit->queue) {
            queue = timepoint.signal_submit->queue->VkHandle();
        } else if (timepoint.acquire_command.has_value()) {
            acquire_command = *timepoint.acquire_command;
        }
    }
}

bool vvl::Semaphore::HasResolvingTimelineSignal(uint64_t wait_payload) const {
    assert(type == VK_SEMAPHORE_TYPE_TIMELINE);
    auto guard = ReadLock();

    // Check if completed payload value (which includes initial value) resolves the wait.
    if (wait_payload <= completed_.payload) {
        return true;
    }

    auto it = timeline_.find(wait_payload);
    assert(it != timeline_.end());  // for each registered wait there is a timepoint
    while (it != timeline_.end()) {
        if (it->second.signal_submit.has_value()) {
            assert(it->first >= wait_payload);  // timepoints are ordered in increasing order
            return true;
        }
        ++it;
    }
    return false;
}

bool vvl::Semaphore::CanRetireBinaryWait(TimePoint &timepoint) const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    // The only allowed configuration when binary semaphore wait does not have a signal
    // is external semaphore. Just retire the wait because there is no guarantee we can
    // track the signal.
    if (!timepoint.signal_submit.has_value()) {
        assert(scope_ != kInternal);
        return true;
    }

    // The resolving signal can only be on another queue (the earlier signals on the
    // current queue are already processed and corresponding timepoints are retired).
    // Initiate forward progress on signaling queue and ask the caller to wait.
    timepoint.Notify();
    return false;
}

bool vvl::Semaphore::CanRetireTimelineWait(const vvl::Queue *current_queue, uint64_t payload) const {
    assert(type == VK_SEMAPHORE_TYPE_TIMELINE);

    // In the correct program the resolving signal is the next signal on the timeline,
    // otherwise this violates the rule of strictly increasing signal values.
    auto it = timeline_.find(payload);
    assert(it != timeline_.end());
    for (; it != timeline_.end(); ++it) {
        const TimePoint &t = it->second;
        if (!t.signal_submit.has_value()) {
            continue;
        }
        // If the next signal is on the waiting (current) queue, it can't be a resolving signal (blocked by wait).
        // QueueSubmissionValidator will also report an error about non-increasing signal values
        if (t.signal_submit->queue != nullptr && t.signal_submit->queue == current_queue) {
            continue;
        }
        // Found the resolving signal
        break;
    }

    // There is always a resolving signal when we reach a retirement phase (CPU successfully finished waiting on GPU).
    // For external semaphore we might not have visibility of this signal. Just retire the wait.
    if (it == timeline_.end()) {
        assert(scope_ != kInternal);
        return true;
    }

    // Found host signal that finishes this wait
    const TimePoint &t = it->second;
    if (t.signal_submit->queue == nullptr) {
        return true;
    }

    // Notify signaling queue and wait for its queue thread
    t.Notify();
    return false;
}

void vvl::Semaphore::RetireWait(vvl::Queue *current_queue, uint64_t payload, const Location &loc, bool queue_thread) {
    std::shared_future<void> waiter;
    {
        auto guard = WriteLock();
        if (payload <= completed_.payload) {
            return;
        }
        if (scope_ != kInternal) {
            if (!vvl::Find(timeline_, payload)) {
                // GetSemaphoreCounterValue for external semaphore might not have a registered timepoint.
                // Add timepoint so we can retire timeline up to that point.
                assert(type == VK_SEMAPHORE_TYPE_TIMELINE);
                timeline_[payload] = TimePoint{};
            }
            if (scope_ == kExternalTemporary) {
                scope_ = kInternal;
                imported_handle_type_.reset();
            }
        }
        TimePoint &timepoint = vvl::FindExisting(timeline_, payload);

        bool retire = false;
        if (timepoint.acquire_command) {
            retire = true;  // There is resolving acquire signal, timepoint can be retired
        } else if (type == VK_SEMAPHORE_TYPE_BINARY) {
            retire = CanRetireBinaryWait(timepoint);
        } else {
            retire = CanRetireTimelineWait(current_queue, payload);
        }
        if (retire) {
            // SemOp::submit is used only by the binary semaphores.
            // Binary semaphores can have at most one wait per timepoint.
            const auto submit_ref = (type == VK_SEMAPHORE_TYPE_BINARY) ? timepoint.wait_submits[0] : SubmissionReference{};

            RetireTimePoint(payload, kWait, submit_ref);
            return;
        }

        // Wait for some other queue or a host operation to retire
        assert(timepoint.waiter.valid());
        // the current timepoint should get destroyed while we're waiting, so copy out the waiter.
        waiter = timepoint.waiter;
    }
    WaitTimePoint(std::move(waiter), payload, !queue_thread, loc);
}

void vvl::Semaphore::RetireSignal(uint64_t payload) {
    auto guard = WriteLock();
    if (payload <= completed_.payload) {
        return;
    }
    TimePoint &timepoint = vvl::FindExisting(timeline_, payload);
    assert(timepoint.signal_submit.has_value());

    OpType completed_op = kSignal;
    SubmissionReference completed_submit = *timepoint.signal_submit;

    // If there is a wait operation then mark it as the last completed instead.
    // The reason to do this here instead on the waiter side (after it is unblocked)
    // is because signal can have larger (timeline) value than corresponding wait value.
    // In this case it's the signal that defines the last completed value.
    if (!timepoint.wait_submits.empty()) {
        completed_op = kWait;
        // SemOp::submit is used only for binary semaphores which can have only single wait
        completed_submit = timepoint.wait_submits[0];
    }

    RetireTimePoint(payload, completed_op, completed_submit);
}

void vvl::Semaphore::RetireTimePoint(uint64_t payload, OpType completed_op, SubmissionReference completed_submit) {
    auto it = timeline_.begin();
    while (it != timeline_.end() && it->first <= payload) {
        assert(it->first > completed_.payload);
        it->second.completed.set_value();
        ++it;
    }
    timeline_.erase(timeline_.begin(), it);
    completed_ = SemOp(completed_op, completed_submit, payload);
}

void vvl::Semaphore::WaitTimePoint(std::shared_future<void> &&waiter, uint64_t payload, bool unblock_validation_object,
                                   const Location &loc) {
    if (unblock_validation_object) {
        dev_data_.BeginBlockingOperation();
    }

    auto result = waiter.wait_until(GetCondWaitTimeout());

    if (unblock_validation_object) {
        dev_data_.EndBlockingOperation();
    }

    if (result != std::future_status::ready) {
        dev_data_.LogError("INTERNAL-ERROR-VkSemaphore-state-timeout", Handle(), loc,
                           "The Validation Layers hit a timeout waiting for timeline semaphore state to update (this is most "
                           "likely a validation bug). completed_.payload=%" PRIu64 " wait_payload=%" PRIu64,
                           completed_.payload, payload);
    }
}

void vvl::Semaphore::Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT || flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) &&
            scope_ == kInternal) {
            scope_ = kExternalTemporary;
        } else {
            scope_ = kExternalPermanent;
        }
    }
    imported_handle_type_ = handle_type;
}

void vvl::Semaphore::Export(VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    if (handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        auto guard = WriteLock();
        scope_ = kExternalPermanent;
    } else {
        assert(type == VK_SEMAPHORE_TYPE_BINARY);  // checked by validation phase
        // Exporting a semaphore payload to a handle with copy transference has the same side effects on the source semaphore's
        // payload as executing a semaphore wait operation
        auto filter = [](const Semaphore::OpType op_type, uint64_t payload, bool is_pending) {
            return is_pending && CanWaitBinarySemaphoreAfterOperation(op_type);
        };
        auto last_op = LastOp(filter);
        if (last_op) {
            EnqueueWait(last_op->submit, last_op->payload);
        }
    }
}

std::optional<VkExternalSemaphoreHandleTypeFlagBits> vvl::Semaphore::ImportedHandleType() const {
    auto guard = ReadLock();

    // Sanity check: semaphore imported -> scope is not internal
    assert(!imported_handle_type_.has_value() || scope_ != kInternal);

    return imported_handle_type_;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
bool vvl::Semaphore::GetMetalExport(const VkSemaphoreCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
