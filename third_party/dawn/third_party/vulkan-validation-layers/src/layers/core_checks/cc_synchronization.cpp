/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <algorithm>
#include <assert.h>
#include <string>
#include <set>

#include <vulkan/vk_enum_string_helper.h>
#include "core_checks/cc_synchronization.h"
#include "core_checks/core_validation.h"
#include "sync/sync_utils.h"
#include "sync/sync_vuid_maps.h"
#include "generated/enum_flag_bits.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/fence_state.h"
#include "state_tracker/semaphore_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/render_pass_state.h"

using sync_utils::BufferBarrier;
using sync_utils::ImageBarrier;
using sync_utils::MemoryBarrier;
using sync_utils::OwnershipTransferBarrier;

ReadLockGuard CoreChecks::ReadLock() const {
    if (global_settings.fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard CoreChecks::WriteLock() {
    if (global_settings.fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

struct TimelineMaxDiffCheck {
    TimelineMaxDiffCheck(uint64_t value_, uint64_t max_diff_) : value(value_), max_diff(max_diff_) {}

    // compute the differents between 2 timeline values, without rollover if the difference is greater than INT64_MAX
    uint64_t AbsDiff(uint64_t a, uint64_t b) { return a > b ? a - b : b - a; }

    bool operator()(const vvl::Semaphore::OpType, uint64_t payload, bool is_pending) { return AbsDiff(value, payload) > max_diff; }

    uint64_t value;
    uint64_t max_diff;
};

bool SemaphoreSubmitState::CanWaitBinary(const vvl::Semaphore &semaphore_state) const {
    assert(semaphore_state.type == VK_SEMAPHORE_TYPE_BINARY);
    // Check if current submission has signaled or unsignaled the semaphore
    if (const bool *signaling_state = vvl::Find(binary_signaling_state, semaphore_state.VkHandle())) {
        const bool signaled = *signaling_state;
        return signaled;  // signaled => can wait
    }
    // Query semaphore object (state set by previous submissions)
    return semaphore_state.CanBinaryBeWaited();
}

bool SemaphoreSubmitState::CanSignalBinary(const vvl::Semaphore &semaphore_state, VkQueue &other_queue,
                                           vvl::Func &other_acquire_command) const {
    assert(semaphore_state.type == VK_SEMAPHORE_TYPE_BINARY);
    // Check if current submission has signaled or unsignaled the semaphore
    if (const bool *signaling_state = vvl::Find(binary_signaling_state, semaphore_state.VkHandle())) {
        const bool signaled = *signaling_state;
        if (!signaled) {
            return true;  // not signaled => can signal
        }
        other_queue = queue;
        other_acquire_command = vvl::Func::Empty;
        return false;  // already signaled => can't signal
    }
    // Query semaphore object (state set by previous submissions)
    if (semaphore_state.CanBinaryBeSignaled()) {
        return true;
    }
    semaphore_state.GetLastBinarySignalSource(other_queue, other_acquire_command);
    return false;
}

VkQueue SemaphoreSubmitState::AnotherQueueWaits(const vvl::Semaphore &semaphore_state) const {
    // VUID-vkQueueSubmit-pWaitSemaphores-00068 (and similar VUs):
    // "When a semaphore wait operation referring to a binary semaphore defined
    //  by any element of the pWaitSemaphores member of any element of pSubmits
    //  executes on queue, there must be no other queues waiting on the same semaphore"
    auto pending_wait_submit = semaphore_state.GetPendingBinaryWaitSubmission();
    if (pending_wait_submit && pending_wait_submit->queue->VkHandle() != queue) {
        return pending_wait_submit->queue->VkHandle();
    }
    return VK_NULL_HANDLE;
}

bool SemaphoreSubmitState::CheckSemaphoreValue(
    const vvl::Semaphore &semaphore_state, std::string &where, uint64_t &bad_value,
    std::function<bool(const vvl::Semaphore::OpType, uint64_t, bool is_pending)> compare_func) {
    auto current_signal = timeline_signals.find(semaphore_state.VkHandle());
    // NOTE: for purposes of validation, duplicate operations in the same submission are not yet pending.
    if (current_signal != timeline_signals.end()) {
        if (compare_func(vvl::Semaphore::kSignal, current_signal->second, false)) {
            where = "current submit's signal";
            bad_value = current_signal->second;
            return true;
        }
    }
    auto current_wait = timeline_waits.find(semaphore_state.VkHandle());
    if (current_wait != timeline_waits.end()) {
        if (compare_func(vvl::Semaphore::kWait, current_wait->second, false)) {
            where = "current submit's wait";
            bad_value = current_wait->second;
            return true;
        }
    }
    auto pending = semaphore_state.LastOp(compare_func);
    if (pending) {
        if (pending->payload == semaphore_state.CurrentPayload()) {
            where = "current";
        } else {
            where = pending->op_type == vvl::Semaphore::OpType::kSignal ? "pending signal" : "pending wait";
        }
        bad_value = pending->payload;
        return true;
    }
    return false;
}

bool SemaphoreSubmitState::ValidateBinaryWait(const Location &loc, VkQueue queue, const vvl::Semaphore &semaphore_state) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    bool skip = false;
    auto semaphore = semaphore_state.VkHandle();
    if ((semaphore_state.Scope() == vvl::Semaphore::kInternal || internal_semaphores.count(semaphore))) {
        if (VkQueue other_queue = AnotherQueueWaits(semaphore_state)) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kOtherQueueWaiting);
            const LogObjectList objlist(semaphore, queue, other_queue);
            skip |= core.LogError(vuid, objlist, loc, "queue (%s) is already waiting on semaphore (%s).",
                                  core.FormatHandle(other_queue).c_str(), core.FormatHandle(semaphore).c_str());
        } else if (!CanWaitBinary(semaphore_state)) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kBinaryCannotBeSignalled);
            const LogObjectList objlist(semaphore, queue);
            skip |= core.LogError(vuid, objlist, loc, "queue (%s) is waiting on semaphore (%s) that has no way to be signaled.",
                                  core.FormatHandle(queue).c_str(), core.FormatHandle(semaphore).c_str());
        } else if (auto timeline_wait_info = semaphore_state.GetPendingBinarySignalTimelineDependency()) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kBinaryCannotBeSignalled);
            const LogObjectList objlist(semaphore_state.Handle(), timeline_wait_info->semaphore->Handle(), queue);
            skip |= core.LogError(
                vuid, objlist, loc,
                "queue (%s) is waiting on binary semaphore (%s) that has an associated signal but it depends on timeline semaphore "
                "wait (%s, wait value = %" PRIu64 ") that does not have resolving signal submitted yet.",
                core.FormatHandle(queue).c_str(), core.FormatHandle(semaphore).c_str(),
                core.FormatHandle(timeline_wait_info->semaphore->VkHandle()).c_str(), timeline_wait_info->payload);
        } else {
            binary_signaling_state[semaphore] = false;
        }
    } else if (semaphore_state.Scope() == vvl::Semaphore::kExternalTemporary) {
        internal_semaphores.insert(semaphore);
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateWaitSemaphore(const Location &wait_semaphore_loc, const vvl::Semaphore &semaphore_state,
                                                 uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;

    switch (semaphore_state.type) {
        case VK_SEMAPHORE_TYPE_BINARY:
            skip |= ValidateBinaryWait(wait_semaphore_loc, queue, semaphore_state);
            break;
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            TimelineMaxDiffCheck exceeds_max_diff(value, core.phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            const VkSemaphore handle = semaphore_state.VkHandle();
            if (CheckSemaphoreValue(semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(wait_semaphore_loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core.LogError(vuid, handle, wait_semaphore_loc,
                                      "value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").", value,
                                      where.c_str(), core.FormatHandle(handle).c_str(), bad_value);
                break;
            }
            timeline_waits[handle] = value;
        } break;
        default:
            break;
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateSignalSemaphore(const Location &signal_semaphore_loc, const vvl::Semaphore &semaphore_state,
                                                   uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;
    const VkSemaphore handle = semaphore_state.VkHandle();
    LogObjectList objlist(handle, queue);

    switch (semaphore_state.type) {
        case VK_SEMAPHORE_TYPE_BINARY: {
            if ((semaphore_state.Scope() == vvl::Semaphore::kInternal || internal_semaphores.count(handle))) {
                VkQueue other_queue = VK_NULL_HANDLE;
                vvl::Func other_command = vvl::Func::Empty;
                if (!CanSignalBinary(semaphore_state, other_queue, other_command)) {
                    std::stringstream initiator;
                    if (other_command != vvl::Func::Empty) {
                        initiator << String(other_command);
                    }
                    if (other_queue != VK_NULL_HANDLE) {
                        if (other_command != vvl::Func::Empty) {
                            initiator << " on ";
                        }
                        initiator << core.FormatHandle(other_queue);
                        objlist.add(other_queue);
                    }
                    const auto &vuid = GetQueueSubmitVUID(signal_semaphore_loc, sync_vuid_maps::SubmitError::kSemAlreadySignalled);
                    skip |= core.LogError(
                        vuid, objlist, signal_semaphore_loc,
                        "(%s) is being signaled by %s, but it was previously signaled by %s and has not since been waited on",
                        core.FormatHandle(handle).c_str(), core.FormatHandle(queue).c_str(), initiator.str().c_str());
                } else {
                    binary_signaling_state[handle] = true;
                }
            }
            break;
        }
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            auto must_be_greater = [value](const vvl::Semaphore::OpType op_type, uint64_t payload, bool is_pending) {
                if (op_type != vvl::Semaphore::OpType::kSignal) {
                    return false;
                }
                // duplicate signal values are never allowed.
                if (value == payload) {
                    return true;
                }
                // exact value ordering cannot be determined until execution time
                return !is_pending && value < payload;
            };
            if (CheckSemaphoreValue(semaphore_state, where, bad_value, must_be_greater)) {
                const auto &vuid = GetQueueSubmitVUID(signal_semaphore_loc, SubmitError::kTimelineSemSmallValue);
                skip |= core.LogError(
                    vuid, objlist, signal_semaphore_loc,
                    "signal value (%" PRIu64 ") in %s must be greater than %s timeline semaphore %s value (%" PRIu64 ")", value,
                    core.FormatHandle(queue).c_str(), where.c_str(), core.FormatHandle(handle).c_str(), bad_value);
                break;
            }
            TimelineMaxDiffCheck exceeds_max_diff(value, core.phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(signal_semaphore_loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core.LogError(vuid, objlist, signal_semaphore_loc,
                                      "value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").", value,
                                      where.c_str(), core.FormatHandle(handle).c_str(), bad_value);
                break;
            }
            timeline_signals[handle] = value;
            break;
        }
        default:
            break;
    }
    return skip;
}

bool CoreChecks::ValidateStageMaskHost(const LogObjectList &objlist, const Location &stage_mask_loc,
                                       VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;
    if ((stageMask & VK_PIPELINE_STAGE_HOST_BIT) != 0) {
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(stage_mask_loc, sync_vuid_maps::SubmitError::kHostStageMask);
        skip |= LogError(vuid, objlist, stage_mask_loc,
                         "must not include VK_PIPELINE_STAGE_HOST_BIT as the stage can't be invoked inside a command buffer.");
    }
    return skip;
}

bool CoreChecks::ValidateFenceForSubmit(const vvl::Fence &fence_state, const char *inflight_vuid, const char *retired_vuid,
                                        const LogObjectList &objlist, const Location &loc) const {
    bool skip = false;

    if (fence_state.Scope() == vvl::Fence::kInternal) {
        switch (fence_state.State()) {
            case vvl::Fence::kInflight:
                skip |= LogError(inflight_vuid, objlist, loc, "(%s) is already in use by another submission.",
                                 FormatHandle(fence_state.Handle()).c_str());
                break;
            case vvl::Fence::kRetired:
                skip |= LogError(retired_vuid, objlist, loc,
                                 "(%s) submitted in SIGNALED state. Fences must be reset before being submitted",
                                 FormatHandle(fence_state.Handle()).c_str());
                break;
            default:
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo &submit,
                                             const Location &submit_loc) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (const auto d3d12_fence_submit_info = vku::FindStructInPNextChain<VkD3D12FenceSubmitInfoKHR>(submit.pNext)) {
        if (d3d12_fence_submit_info->waitSemaphoreValuesCount != submit.waitSemaphoreCount) {
            skip |= LogError("VUID-VkD3D12FenceSubmitInfoKHR-waitSemaphoreValuesCount-00079", state.queue, submit_loc,
                             "contains an instance of VkD3D12FenceSubmitInfoKHR, but its waitSemaphoreValuesCount (%" PRIu32
                             ") is different than %s (%" PRIu32 ").",
                             d3d12_fence_submit_info->waitSemaphoreValuesCount,
                             submit_loc.dot(Field::waitSemaphoreCount).Fields().c_str(), submit.waitSemaphoreCount);
        }
        if (d3d12_fence_submit_info->signalSemaphoreValuesCount != submit.signalSemaphoreCount) {
            skip |= LogError("VUID-VkD3D12FenceSubmitInfoKHR-signalSemaphoreValuesCount-00080", state.queue, submit_loc,
                             "contains an instance of VkD3D12FenceSubmitInfoKHR, but its signalSemaphoreValuesCount (%" PRIu32
                             ") is different than %s (%" PRIu32 ").",
                             d3d12_fence_submit_info->signalSemaphoreValuesCount,
                             submit_loc.dot(Field::signalSemaphoreCount).Fields().c_str(), submit.signalSemaphoreCount);
        }
    }
#endif
    auto *timeline_semaphore_submit_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        if (submit.pWaitDstStageMask) {
            const LogObjectList objlist(semaphore, state.queue);
            auto stage_mask_loc = submit_loc.dot(Field::pWaitDstStageMask, i);
            skip |= ValidatePipelineStage(objlist, stage_mask_loc, state.queue_flags, submit.pWaitDstStageMask[i]);
            skip |= ValidateStageMaskHost(objlist, stage_mask_loc, submit.pWaitDstStageMask[i]);
        }
        auto semaphore_state = Get<vvl::Semaphore>(semaphore);
        ASSERT_AND_CONTINUE(semaphore_state);

        auto wait_semaphore_loc = submit_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkSubmitInfo-pWaitSemaphores-03239", semaphore, wait_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo.",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkSubmitInfo-pNext-03240", semaphore, wait_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::waitSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->waitSemaphoreValueCount,
                    submit_loc.dot(Field::waitSemaphoreCount).Fields().c_str(), submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(wait_semaphore_loc, *semaphore_state, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<vvl::Semaphore>(semaphore);
        ASSERT_AND_CONTINUE(semaphore_state);

        auto signal_semaphore_loc = submit_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkSubmitInfo-pWaitSemaphores-03239", semaphore, signal_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkSubmitInfo-pNext-03241", semaphore, signal_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::signalSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->signalSemaphoreValueCount,
                    submit_loc.dot(Field::signalSemaphoreCount).Fields().c_str(), submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(signal_semaphore_loc, *semaphore_state, value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo2 &submit,
                                             const Location &submit_loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < submit.waitSemaphoreInfoCount; ++i) {
        const auto &wait_info = submit.pWaitSemaphoreInfos[i];
        Location wait_info_loc = submit_loc.dot(Field::pWaitSemaphoreInfos, i);
        const LogObjectList objlist(wait_info.semaphore, state.queue);
        skip |= ValidatePipelineStage(objlist, wait_info_loc.dot(Field::stageMask), state.queue_flags, wait_info.stageMask);
        skip |= ValidateStageMaskHost(objlist, wait_info_loc.dot(Field::stageMask), wait_info.stageMask);

        auto semaphore_state = Get<vvl::Semaphore>(wait_info.semaphore);
        ASSERT_AND_CONTINUE(semaphore_state);

        skip |= state.ValidateWaitSemaphore(wait_info_loc.dot(Field::semaphore), *semaphore_state, wait_info.value);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            for (uint32_t sig_index = 0; sig_index < submit.signalSemaphoreInfoCount; sig_index++) {
                const auto &sig_info = submit.pSignalSemaphoreInfos[sig_index];
                if (wait_info.semaphore == sig_info.semaphore && wait_info.value >= sig_info.value) {
                    Location sig_loc = submit_loc.dot(Field::pSignalSemaphoreInfos, sig_index);
                    skip |= LogError("VUID-VkSubmitInfo2-semaphore-03881", objlist, wait_info_loc.dot(Field::value),
                                     "(%" PRIu64 ") is less or equal to %s (%" PRIu64 ").", wait_info.value,
                                     sig_loc.dot(Field::value).Fields().c_str(), sig_info.value);
                }
            }
        }
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreInfoCount; ++i) {
        const auto &sem_info = submit.pSignalSemaphoreInfos[i];
        auto signal_info_loc = submit_loc.dot(Field::pSignalSemaphoreInfos, i);
        const LogObjectList objlist(sem_info.semaphore, state.queue);
        skip |= ValidatePipelineStage(objlist, signal_info_loc.dot(Field::stageMask), state.queue_flags, sem_info.stageMask);
        skip |= ValidateStageMaskHost(objlist, signal_info_loc.dot(Field::stageMask), sem_info.stageMask);
        if (auto semaphore_state = Get<vvl::Semaphore>(sem_info.semaphore)) {
            skip |= state.ValidateSignalSemaphore(signal_info_loc.dot(Field::semaphore), *semaphore_state, sem_info.value);
        }
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkBindSparseInfo &submit,
                                             const Location &submit_loc) const {
    bool skip = false;
    auto *timeline_semaphore_submit_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        const LogObjectList objlist(semaphore, state.queue);
        // NOTE: there are no stage masks in bind sparse submissions
        auto semaphore_state = Get<vvl::Semaphore>(semaphore);
        ASSERT_AND_CONTINUE(semaphore_state);

        auto wait_semaphore_loc = submit_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkBindSparseInfo-pWaitSemaphores-03246", semaphore, wait_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkBindSparseInfo-pNext-03247", semaphore, wait_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::waitSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->waitSemaphoreValueCount,
                    submit_loc.dot(Field::waitSemaphoreCount).Fields().c_str(), submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(wait_semaphore_loc, *semaphore_state, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<vvl::Semaphore>(semaphore);
        ASSERT_AND_CONTINUE(semaphore_state);

        auto signal_semaphore_loc = submit_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkBindSparseInfo-pWaitSemaphores-03246", semaphore, signal_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkBindSparseInfo-pNext-03248", semaphore, signal_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::signalSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->signalSemaphoreValueCount,
                    submit_loc.dot(Field::signalSemaphoreCount).Fields().c_str(), submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(signal_semaphore_loc, *semaphore_state, value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkFence *pFence,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    auto fence_export_info = vku::FindStructInPNextChain<VkExportFenceCreateInfo>(pCreateInfo->pNext);
    if (fence_export_info && fence_export_info->handleTypes != 0) {
        VkExternalFenceProperties external_properties = vku::InitStructHelper();
        bool export_supported = true;
        // Check export support
        auto check_export_support = [&](VkExternalFenceHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalFenceInfo external_info = vku::InitStructHelper();
            external_info.handleType = flag;
            DispatchGetPhysicalDeviceExternalFencePropertiesHelper(api_version, physical_device, &external_info,
                                                                   &external_properties);
            if ((external_properties.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) == 0) {
                export_supported = false;
                skip |= LogError("VUID-VkExportFenceCreateInfo-handleTypes-01446", device,
                                 create_info_loc.pNext(Struct::VkExportFenceCreateInfo, Field::handleTypes),
                                 "(%s) does not support VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT.",
                                 string_VkExternalFenceHandleTypeFlagBits(flag));
            }
        };
        IterateFlags<VkExternalFenceHandleTypeFlagBits>(fence_export_info->handleTypes, check_export_support);
        // Check handle types compatibility
        if (export_supported &&
            (fence_export_info->handleTypes & external_properties.compatibleHandleTypes) != fence_export_info->handleTypes) {
            skip |= LogError("VUID-VkExportFenceCreateInfo-handleTypes-01446", device,
                             create_info_loc.pNext(Struct::VkExportFenceCreateInfo, Field::handleTypes),
                             "(%s) are not reported as compatible by vkGetPhysicalDeviceExternalFenceProperties (%s).",
                             string_VkExternalFenceHandleTypeFlags(fence_export_info->handleTypes).c_str(),
                             string_VkExternalFenceHandleTypeFlags(external_properties.compatibleHandleTypes).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_type_create_info = vku::FindStructInPNextChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext);
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    if (sem_type_create_info) {
        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE && !enabled_features.timelineSemaphore) {
            skip |= LogError("VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252", device,
                             create_info_loc.dot(Field::semaphoreType),
                             "is VK_SEMAPHORE_TYPE_TIMELINE, but timelineSemaphore feature was not enabled.");
        }

        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_BINARY && sem_type_create_info->initialValue != 0) {
            skip |=
                LogError("VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279", device, create_info_loc.dot(Field::semaphoreType),
                         "is VK_SEMAPHORE_TYPE_BINARY, but initialValue is %" PRIu64 ".", sem_type_create_info->initialValue);
        }
    }

    auto sem_export_info = vku::FindStructInPNextChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
    if (sem_export_info && sem_export_info->handleTypes != 0) {
        VkExternalSemaphoreProperties external_properties = vku::InitStructHelper();
        bool export_supported = true;
        // Check export support
        auto check_export_support = [&](VkExternalSemaphoreHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalSemaphoreInfo external_info = vku::InitStructHelper();
            external_info.handleType = flag;
            DispatchGetPhysicalDeviceExternalSemaphorePropertiesHelper(api_version, physical_device, &external_info,
                                                                       &external_properties);
            if ((external_properties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) == 0) {
                export_supported = false;
                skip |= LogError("VUID-VkExportSemaphoreCreateInfo-handleTypes-01124", device,
                                 create_info_loc.pNext(Struct::VkExportSemaphoreCreateInfo, Field::handleTypes),
                                 "(%s) does not support VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT.",
                                 string_VkExternalSemaphoreHandleTypeFlagBits(flag));
            }
        };
        IterateFlags<VkExternalSemaphoreHandleTypeFlagBits>(sem_export_info->handleTypes, check_export_support);
        // Check handle types compatibility
        if (export_supported &&
            (sem_export_info->handleTypes & external_properties.compatibleHandleTypes) != sem_export_info->handleTypes) {
            skip |= LogError("VUID-VkExportSemaphoreCreateInfo-handleTypes-01124", device,
                             create_info_loc.pNext(Struct::VkExportSemaphoreCreateInfo, Field::handleTypes),
                             "(%s) are not reported as compatible by vkGetPhysicalDeviceExternalSemaphoreProperties (%s).",
                             string_VkExternalSemaphoreHandleTypeFlags(sem_export_info->handleTypes).c_str(),
                             string_VkExternalSemaphoreHandleTypeFlags(external_properties.compatibleHandleTypes).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateWaitSemaphores(device, pWaitInfo, timeout, error_obj);
}

bool CoreChecks::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                               const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        auto semaphore_state = Get<vvl::Semaphore>(pWaitInfo->pSemaphores[i]);
        ASSERT_AND_CONTINUE(semaphore_state);
        if (semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError("VUID-VkSemaphoreWaitInfo-pSemaphores-03256", pWaitInfo->pSemaphores[i],
                             error_obj.location.dot(Field::pWaitInfo).dot(Field::pSemaphores, i), "%s was created with %s",
                             FormatHandle(pWaitInfo->pSemaphores[i]).c_str(), string_VkSemaphoreType(semaphore_state->type));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator,
                                             const ErrorObject &error_obj) const {
    bool skip = false;
    auto fence_state = Get<vvl::Fence>(fence);
    if (fence_state && fence_state->Scope() == vvl::Fence::kInternal && fence_state->State() == vvl::Fence::kInflight) {
        skip |= ValidateObjectNotInUse(fence_state.get(), error_obj.location, "VUID-vkDestroyFence-fence-01120");
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto fence_state = Get<vvl::Fence>(pFences[i]);
        ASSERT_AND_CONTINUE(fence_state);
        if (fence_state->Scope() == vvl::Fence::kInternal && fence_state->State() == vvl::Fence::kInflight) {
            skip |= LogError("VUID-vkResetFences-pFences-01123", pFences[i], error_obj.location.dot(Field::pFences, i),
                             "(%s) is in use.", FormatHandle(pFences[i]).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto sema_node = Get<vvl::Semaphore>(semaphore)) {
        skip |= ValidateObjectNotInUse(sema_node.get(), error_obj.location, "VUID-vkDestroySemaphore-semaphore-05149");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator,
                                             const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto event_state = Get<vvl::Event>(event)) {
        skip |= ValidateObjectNotInUse(event_state.get(), error_obj.location.dot(Field::event), "VUID-vkDestroyEvent-event-01145");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto sampler_state = Get<vvl::Sampler>(sampler)) {
        skip |= ValidateObjectNotInUse(sampler_state.get(), error_obj.location.dot(Field::sampler),
                                       "VUID-vkDestroySampler-sampler-01082");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                            const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    const Location stage_mask_loc = error_obj.location.dot(Field::stageMask);
    const LogObjectList objlist(commandBuffer);
    skip |= ValidatePipelineStage(objlist, stage_mask_loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(objlist, stage_mask_loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                             const ErrorObject &error_obj) const {
    const LogObjectList objlist(commandBuffer, event);

    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdSetEvent2-synchronization2-03824", commandBuffer, error_obj.location,
                         "synchronization2 feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfo);
    if (pDependencyInfo->dependencyFlags != 0) {
        skip |= LogError("VUID-vkCmdSetEvent2-dependencyFlags-03825", objlist, dep_info_loc.dot(Field::dependencyFlags),
                         "(%s) must be 0.", string_VkDependencyFlags(pDependencyInfo->dependencyFlags).c_str());
    }
    skip |= ValidateDependencyInfo(objlist, dep_info_loc, *cb_state, *pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfoKHR *pDependencyInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    const LogObjectList objlist(commandBuffer);
    const Location stage_mask_loc = error_obj.location.dot(Field::stageMask);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineStage(objlist, stage_mask_loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(objlist, stage_mask_loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                               const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    const LogObjectList objlist(commandBuffer);
    const Location stage_mask_loc = error_obj.location.dot(Field::stageMask);

    bool skip = false;
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdResetEvent2-synchronization2-03829", commandBuffer, error_obj.location,
                         "the synchronization2 feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineStage(objlist, stage_mask_loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(objlist, stage_mask_loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

struct RenderPassDepState {
    using Field = vvl::Field;

    const CoreChecks &core;
    const std::string vuid;
    uint32_t active_subpass;
    const VkRenderPass rp_handle;
    const VkPipelineStageFlags2KHR disabled_features;
    const std::vector<uint32_t> &self_dependencies;
    const vku::safe_VkSubpassDependency2 *dependencies;

    RenderPassDepState(const CoreChecks &c, const std::string &v, uint32_t subpass, const VkRenderPass handle,
                       const DeviceFeatures &features, const DeviceExtensions &device_extensions,
                       const std::vector<uint32_t> &self_deps, const vku::safe_VkSubpassDependency2 *deps)
        : core(c),
          vuid(v),
          active_subpass(subpass),
          rp_handle(handle),
          disabled_features(sync_utils::DisabledPipelineStages(features, device_extensions)),
          self_dependencies(self_deps),
          dependencies(deps) {}

    VkMemoryBarrier2 GetSubPassDepBarrier(const vku::safe_VkSubpassDependency2 &dep) const {
        // "If a VkMemoryBarrier2 is included in the pNext chain, srcStageMask, dstStageMask,
        // srcAccessMask, and dstAccessMask parameters are ignored. The synchronization and
        // access scopes instead are defined by the parameters of VkMemoryBarrier2."
        if (const auto override_barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(dep.pNext)) {
            return *override_barrier;
        }

        VkMemoryBarrier2 barrier = vku::InitStructHelper();
        barrier.srcStageMask = dep.srcStageMask;
        barrier.dstStageMask = dep.dstStageMask;
        barrier.srcAccessMask = dep.srcAccessMask;
        barrier.dstAccessMask = dep.dstAccessMask;
        return barrier;
    }

    bool ValidateStage(const Location &barrier_loc, VkPipelineStageFlags2 src_stage_mask,
                       VkPipelineStageFlags2 dst_stage_mask) const {
        // Look for srcStageMask + dstStageMask superset in any self-dependency
        for (const auto self_dep_index : self_dependencies) {
            const auto subpass_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);

            const auto subpass_src_stages =
                sync_utils::ExpandPipelineStages(subpass_dep.srcStageMask, sync_utils::kAllQueueTypes, disabled_features);
            const auto barrier_src_stages =
                sync_utils::ExpandPipelineStages(src_stage_mask, sync_utils::kAllQueueTypes, disabled_features);

            const auto subpass_dst_stages =
                sync_utils::ExpandPipelineStages(subpass_dep.dstStageMask, sync_utils::kAllQueueTypes, disabled_features);
            const auto barrier_dst_stages =
                sync_utils::ExpandPipelineStages(dst_stage_mask, sync_utils::kAllQueueTypes, disabled_features);

            const bool is_subset = (barrier_src_stages == (subpass_src_stages & barrier_src_stages)) &&
                                   (barrier_dst_stages == (subpass_dst_stages & barrier_dst_stages));
            if (is_subset) return false;  // subset is found, return skip value (false)
        }
        return core.LogError(vuid, rp_handle, barrier_loc.dot(Field::srcStageMask),
                             "(%s) and dstStageMask (%s) is not a subset of subpass dependency's srcStageMask and dstStageMask for "
                             "any self-dependency of subpass %" PRIu32 " of %s.",
                             string_VkPipelineStageFlags2(src_stage_mask).c_str(),
                             string_VkPipelineStageFlags2(dst_stage_mask).c_str(), active_subpass,
                             core.FormatHandle(rp_handle).c_str());
    }

    bool ValidateAccess(const Location &barrier_loc, VkAccessFlags2 src_access_mask, VkAccessFlags2 dst_access_mask) const {
        // Look for srcAccessMask + dstAccessMask superset in any self-dependency
        for (const auto self_dep_index : self_dependencies) {
            const auto subpass_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);
            const bool is_subset = (src_access_mask == (subpass_dep.srcAccessMask & src_access_mask)) &&
                                   (dst_access_mask == (subpass_dep.dstAccessMask & dst_access_mask));
            if (is_subset) return false;  // subset is found, return skip value (false)
        }
        return core.LogError(vuid, rp_handle, barrier_loc.dot(Field::srcAccessMask),
                             "(%s) and dstAccessMask (%s) is not a subset of subpass dependency's srcAccessMask and dstAccessMask "
                             "of subpass %" PRIu32 " of %s.",
                             string_VkAccessFlags2(src_access_mask).c_str(), string_VkAccessFlags2(dst_access_mask).c_str(),
                             active_subpass, core.FormatHandle(rp_handle).c_str());
    }

    bool ValidateDependencyFlag(const Location &dep_flags_loc, VkDependencyFlags dependency_flags) const {
        for (const auto self_dep_index : self_dependencies) {
            const auto &subpass_dep = dependencies[self_dep_index];
            const bool match = subpass_dep.dependencyFlags == dependency_flags;
            if (match) return false;  // match is found, return skip value (false)
        }
        return core.LogError(vuid, rp_handle, dep_flags_loc,
                             "(%s) does not equal VkSubpassDependency dependencyFlags value for any "
                             "self-dependency of subpass %" PRIu32 " of %s.",
                             string_VkDependencyFlags(dependency_flags).c_str(), active_subpass,
                             core.FormatHandle(rp_handle).c_str());
    }
};

// If inside a renderpass, validate
bool CoreChecks::ValidateRenderPassPipelineStage(VkRenderPass render_pass, const Location &loc,
                                                 VkPipelineStageFlags2 src_stage_mask, VkPipelineStageFlags2 dst_stage_mask) const {
    bool skip = false;
    const VkPipelineStageFlags2 graphics_stages = syncAllCommandStagesByQueueFlags().at(VK_QUEUE_GRAPHICS_BIT);
    const VkPipelineStageFlags2 src_diff =
        sync_utils::ExpandPipelineStages(src_stage_mask, VK_QUEUE_GRAPHICS_BIT) & ~graphics_stages;
    const VkPipelineStageFlags2 dst_diff =
        sync_utils::ExpandPipelineStages(dst_stage_mask, VK_QUEUE_GRAPHICS_BIT) & ~graphics_stages;
    if (src_diff != 0) {
        const char *vuid = loc.function == Func::vkCmdPipelineBarrier ? "VUID-vkCmdPipelineBarrier-None-07892"
                                                                      : "VUID-vkCmdPipelineBarrier2-None-07892";
        skip |= LogError(vuid, render_pass, loc.dot(Field::srcStageMask), "contains non graphics stage %s.",
                         string_VkPipelineStageFlags2(src_diff).c_str());
    }
    if (dst_diff != 0) {
        const char *vuid = loc.function == Func::vkCmdPipelineBarrier ? "VUID-vkCmdPipelineBarrier-None-07892"
                                                                      : "VUID-vkCmdPipelineBarrier2-None-07892";
        skip |= LogError(vuid, render_pass, loc.dot(Field::dstStageMask), "contains non graphics stage %s.",
                         string_VkPipelineStageFlags2(dst_diff).c_str());
    }
    return skip;
}

// Validate VUs for Pipeline Barriers that are within a renderPass
// Pre: cb_state->active_render_pass must be a pointer to valid renderPass state
bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const vvl::CommandBuffer &cb_state,
                                                    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                                    VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                                    const VkMemoryBarrier *mem_barriers, uint32_t buffer_mem_barrier_count,
                                                    const VkBufferMemoryBarrier *buffer_mem_barriers,
                                                    uint32_t image_mem_barrier_count,
                                                    const VkImageMemoryBarrier *image_barriers) const {
    bool skip = false;
    const vvl::RenderPass *rp_state = cb_state.active_render_pass.get();
    ASSERT_AND_RETURN_SKIP(rp_state);
    RenderPassDepState state(*this, "VUID-vkCmdPipelineBarrier-None-07889", cb_state.GetActiveSubpass(), rp_state->VkHandle(),
                             enabled_features, extensions, rp_state->self_dependencies[cb_state.GetActiveSubpass()],
                             rp_state->create_info.pDependencies);
    if (state.self_dependencies.empty()) {
        skip |= LogError("VUID-vkCmdPipelineBarrier-None-07889", state.rp_handle, outer_loc,
                         "Barriers cannot be set during subpass %" PRIu32 " of %s with no self-dependency specified.",
                         state.active_subpass, FormatHandle(state.rp_handle).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->create_info.pSubpasses[state.active_subpass];
    skip |= state.ValidateStage(outer_loc, src_stage_mask, dst_stage_mask);
    skip |= ValidateRenderPassPipelineStage(state.rp_handle, outer_loc, src_stage_mask, dst_stage_mask);

    if (0 != buffer_mem_barrier_count) {
        skip |= LogError("VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178", state.rp_handle,
                         outer_loc.dot(Field::bufferMemoryBarrierCount), "is non-zero (%" PRIu32 ") for subpass %" PRIu32 " of %s.",
                         buffer_mem_barrier_count, state.active_subpass, FormatHandle(rp_state->Handle()).c_str());
    }
    for (uint32_t i = 0; i < mem_barrier_count; ++i) {
        const auto &mem_barrier = mem_barriers[i];
        const Location barrier_loc = outer_loc.dot(Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        skip |= state.ValidateAccess(barrier_loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }

    for (uint32_t i = 0; i < image_mem_barrier_count; ++i) {
        const auto img_barrier = ImageBarrier(image_barriers[i], src_stage_mask, dst_stage_mask);
        const Location barrier_loc = outer_loc.dot(Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        skip |= state.ValidateAccess(barrier_loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (img_barrier.srcQueueFamilyIndex != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182", state.rp_handle,
                             barrier_loc.dot(Field::srcQueueFamilyIndex),
                             "is %" PRIu32 " and dstQueueFamilyIndex is %" PRIu32 " but they must be equal.",
                             img_barrier.srcQueueFamilyIndex, img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (cb_state.activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(barrier_loc, cb_state, *cb_state.activeFramebuffer, state.active_subpass,
                                                   sub_desc, state.rp_handle, img_barrier);
        }
    }

    if (GetBitSetCount(sub_desc.viewMask) > 1 && ((dependency_flags & VK_DEPENDENCY_VIEW_LOCAL_BIT) == 0)) {
        skip |= LogError("VUID-vkCmdPipelineBarrier-None-07893", state.rp_handle, outer_loc.dot(Field::dependencyFlags),
                         "%s is missing VK_DEPENDENCY_VIEW_LOCAL_BIT and subpass %" PRIu32 " has viewMasks 0x%" PRIx32 ".",
                         string_VkDependencyFlags(dependency_flags).c_str(), state.active_subpass, sub_desc.viewMask);
    }

    skip |= state.ValidateDependencyFlag(outer_loc.dot(Field::dependencyFlags), dependency_flags);
    return skip;
}

bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const vvl::CommandBuffer &cb_state,
                                                    const VkDependencyInfo &dep_info) const {
    bool skip = false;
    const vvl::RenderPass *rp_state = cb_state.active_render_pass.get();
    if (!rp_state || rp_state->UsesDynamicRendering()) {
        return skip;
    }
    RenderPassDepState state(*this, "VUID-vkCmdPipelineBarrier2-None-07889", cb_state.GetActiveSubpass(), rp_state->VkHandle(),
                             enabled_features, extensions, rp_state->self_dependencies[cb_state.GetActiveSubpass()],
                             rp_state->create_info.pDependencies);

    if (state.self_dependencies.empty()) {
        skip |= LogError(state.vuid, state.rp_handle, outer_loc,
                         "Barriers cannot be set during subpass %" PRIu32 " of %s with no self-dependency specified.",
                         state.active_subpass, FormatHandle(rp_state->Handle()).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->create_info.pSubpasses[state.active_subpass];
    for (uint32_t i = 0; i < dep_info.memoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info.pMemoryBarriers[i];
        const Location barrier_loc = outer_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        skip |= state.ValidateStage(barrier_loc, mem_barrier.srcStageMask, mem_barrier.dstStageMask);
        skip |= state.ValidateAccess(barrier_loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
        skip |= ValidateRenderPassPipelineStage(state.rp_handle, outer_loc, mem_barrier.srcStageMask, mem_barrier.dstStageMask);
    }
    if (0 != dep_info.bufferMemoryBarrierCount) {
        skip |= LogError("VUID-vkCmdPipelineBarrier2-bufferMemoryBarrierCount-01178", state.rp_handle,
                         outer_loc.dot(Field::bufferMemoryBarrierCount), "is non-zero (%" PRIu32 ") for subpass %" PRIu32 " of %s.",
                         dep_info.bufferMemoryBarrierCount, state.active_subpass, FormatHandle(state.rp_handle).c_str());
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; ++i) {
        const auto img_barrier = ImageBarrier(dep_info.pImageMemoryBarriers[i]);
        const Location barrier_loc = outer_loc.dot(Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);

        skip |= state.ValidateStage(barrier_loc, img_barrier.srcStageMask, img_barrier.dstStageMask);
        skip |= state.ValidateAccess(barrier_loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);
        skip |= ValidateRenderPassPipelineStage(state.rp_handle, outer_loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (img_barrier.srcQueueFamilyIndex != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-01182", state.rp_handle,
                             barrier_loc.dot(Field::srcQueueFamilyIndex),
                             "is %" PRIu32 " and dstQueueFamilyIndex is %" PRIu32 " but they must be equal.",
                             img_barrier.srcQueueFamilyIndex, img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (cb_state.activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(barrier_loc, cb_state, *cb_state.activeFramebuffer, state.active_subpass,
                                                   sub_desc, state.rp_handle, img_barrier);
        }
    }

    if (GetBitSetCount(sub_desc.viewMask) > 1 && ((dep_info.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) == 0)) {
        skip |= LogError("VUID-vkCmdPipelineBarrier2-None-07893", state.rp_handle, outer_loc.dot(Field::dependencyFlags),
                         "%s is missing VK_DEPENDENCY_VIEW_LOCAL_BIT and subpass %" PRIu32 " has viewMasks 0x%" PRIx32 ".",
                         string_VkDependencyFlags(dep_info.dependencyFlags).c_str(), state.active_subpass, sub_desc.viewMask);
    }

    skip |= state.ValidateDependencyFlag(outer_loc.dot(Field::dependencyFlags), dep_info.dependencyFlags);
    return skip;
}

bool CoreChecks::ValidateStageMasksAgainstQueueCapabilities(const LogObjectList &objlist, const Location &stage_mask_loc,
                                                            VkQueueFlags queue_flags, VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    // these are always allowed by queues, calls that restrict them have dedicated VUs.
    stage_mask &= ~(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT | VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
                    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_HOST_BIT);
    if (stage_mask == 0) {
        return skip;
    }

    static const std::array<std::pair<VkPipelineStageFlags2KHR, VkQueueFlags>, 4> metaFlags{
        {{VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT},
         {VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT},
         {VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT, VK_QUEUE_GRAPHICS_BIT},
         {VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_QUEUE_GRAPHICS_BIT}}};

    for (const auto &entry : metaFlags) {
        if (((entry.first & stage_mask) != 0) && ((entry.second & queue_flags) == 0)) {
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(stage_mask_loc, entry.first);
            skip |= LogError(vuid, objlist, stage_mask_loc,
                             "(%s) is not compatible with the queue family properties (%s) of this command buffer.",
                             sync_utils::StringPipelineStageFlags(entry.first).c_str(), string_VkQueueFlags(queue_flags).c_str());
        }
        stage_mask &= ~entry.first;
    }
    if (stage_mask == 0) {
        return skip;
    }

    auto supported_flags = sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, queue_flags);

    auto bad_flags = stage_mask & ~supported_flags;

    // Lookup each bit in the stagemask and check for overlap between its table bits and queue_flags
    for (size_t i = 0; i < sizeof(bad_flags) * 8; i++) {
        VkPipelineStageFlags2KHR bit = (1ULL << i) & bad_flags;
        if (bit) {
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(stage_mask_loc, bit);
            skip |= LogError(vuid, objlist, stage_mask_loc,
                             "(%s) is not compatible with the queue family properties (%s) of this command buffer.",
                             sync_utils::StringPipelineStageFlags(bit).c_str(), string_VkQueueFlags(queue_flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStageFeatureEnables(const LogObjectList &objlist, const Location &stage_mask_loc,
                                                     VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    if (!enabled_features.synchronization2 && stage_mask == 0) {
        const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(stage_mask_loc, 0, extensions);
        skip |= LogError(vuid, objlist, stage_mask_loc, "must not be 0 unless synchronization2 is enabled.");
    }

    auto disabled_stages = sync_utils::DisabledPipelineStages(enabled_features, extensions);
    auto bad_bits = stage_mask & disabled_stages;
    if (bad_bits == 0) {
        return skip;
    }
    for (size_t i = 0; i < sizeof(bad_bits) * 8; i++) {
        VkPipelineStageFlags2KHR bit = 1ULL << i;
        if (bit & bad_bits) {
            const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(stage_mask_loc, bit, extensions);
            skip |=
                LogError(vuid, objlist, stage_mask_loc, "includes %s when the device does not have %s feature enabled.",
                         sync_utils::StringPipelineStageFlags(bit).c_str(), sync_vuid_maps::GetFeatureNameMap().at(bit).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStage(const LogObjectList &objlist, const Location &stage_mask_loc, VkQueueFlags queue_flags,
                                       VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    skip |= ValidateStageMasksAgainstQueueCapabilities(objlist, stage_mask_loc, queue_flags, stage_mask);
    skip |= ValidatePipelineStageFeatureEnables(objlist, stage_mask_loc, stage_mask);
    return skip;
}

bool CoreChecks::ValidateAccessMask(const LogObjectList &objlist, const Location &access_mask_loc, const Location &stage_mask_loc,
                                    VkQueueFlags queue_flags, VkAccessFlags2KHR access_mask,
                                    VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;

    const auto expanded_pipeline_stages = sync_utils::ExpandPipelineStages(stage_mask, queue_flags);

    if (!enabled_features.rayQuery && (access_mask & VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR)) {
        const auto illegal_pipeline_stages = AllVkPipelineShaderStageBits2 & ~VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
        if (stage_mask & illegal_pipeline_stages) {
            // Select right vuid based on enabled extensions
            const auto &vuid = sync_vuid_maps::GetAccessMaskRayQueryVUIDSelector(access_mask_loc, extensions);
            skip |= LogError(vuid, objlist, stage_mask_loc, "contains pipeline stages %s.",
                             sync_utils::StringPipelineStageFlags(stage_mask).c_str());
        }
    }

    // Early out if all commands set
    if ((stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) != 0) return skip;

    // or if only generic memory accesses are specified (or we got a 0 mask)
    access_mask &= ~(VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
    if (access_mask == 0) return skip;

    const auto valid_accesses = sync_utils::CompatibleAccessMask(expanded_pipeline_stages);
    const auto bad_accesses = (access_mask & ~valid_accesses);
    if (bad_accesses == 0) {
        return skip;
    }

    for (size_t i = 0; i < sizeof(bad_accesses) * 8; i++) {
        VkAccessFlags2KHR bit = (1ULL << i);
        if (bad_accesses & bit) {
            const auto &vuid = sync_vuid_maps::GetBadAccessFlagsVUID(access_mask_loc, bit);
            skip |= LogError(vuid, objlist, access_mask_loc, "(%s) is not supported by stage mask (%s).",
                             sync_utils::StringAccessFlags(bit).c_str(), sync_utils::StringPipelineStageFlags(stage_mask).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateWaitEventsAtSubmit(vvl::Func command, const vvl::CommandBuffer &cb_state, size_t eventCount,
                                            size_t firstEventIndex, VkPipelineStageFlags2 sourceStageMask,
                                            const EventMap &local_event_signal_info, VkQueue waiting_queue, const Location &loc) {
    bool skip = false;
    const vvl::Device &state_data = cb_state.dev_data;
    VkPipelineStageFlags2KHR stage_mask = 0;
    const auto max_event = std::min((firstEventIndex + eventCount), cb_state.events.size());
    for (size_t event_index = firstEventIndex; event_index < max_event; ++event_index) {
        auto event = cb_state.events[event_index];

        // The event signal map tracks src_stage from the last SetEvent within the
        // *current* queue submission. If the current submission does not have
        // SetEvent before WaitEvents then we need to find the last SetEvent (if any)
        // in the previous submissions to the same queue. This information is
        // conveniently stored in the vvl::Event object itself (after each queue
        // submit, vvl::CommandBuffer::Submit() updates vvl::Event, so it contains
        // the last src_stage from that submission).
        if (const auto *event_info = vvl::Find(local_event_signal_info, event)) {
            stage_mask |= event_info->src_stage_mask;
            // The "set event" is found in the current submission (the same queue); there can't be inter-queue usage errors
        } else {
            auto event_state = state_data.Get<vvl::Event>(event);
            if (!event_state) continue;
            stage_mask |= event_state->signal_src_stage_mask;

            if (event_state->signaling_queue != VK_NULL_HANDLE && event_state->signaling_queue != waiting_queue) {
                const LogObjectList objlist(cb_state.Handle(), event, event_state->signaling_queue, waiting_queue);
                skip |= state_data.LogError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue", objlist, Location(command),
                                            "waits for event %s on the queue %s but the event was signaled on a different queue %s",
                                            state_data.FormatHandle(event).c_str(), state_data.FormatHandle(waiting_queue).c_str(),
                                            state_data.FormatHandle(event_state->signaling_queue).c_str());
            }
        }
    }
    // TODO: Need to validate that host_bit is only set if set event is called
    // but set event can be called at any time.
    if (sourceStageMask != stage_mask && sourceStageMask != (stage_mask | VK_PIPELINE_STAGE_HOST_BIT)) {
        skip |= state_data.LogError(
            "VUID-vkCmdWaitEvents-srcStageMask-parameter", cb_state.Handle(), loc,
            "Submitting cmdbuffer with call to VkCmdWaitEvents using srcStageMask %s which must be the bitwise OR of the stageMask "
            "parameters used in calls to vkCmdSetEvent and VK_PIPELINE_STAGE_HOST_BIT if used with vkSetEvent but instead is %s.",
            string_VkPipelineStageFlags2(sourceStageMask).c_str(), string_VkPipelineStageFlags2(stage_mask).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                              const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    auto queue_flags = cb_state->GetQueueFlags();
    const LogObjectList objlist(commandBuffer);

    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::dstStageMask), queue_flags, dstStageMask);

    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateBarriers(error_obj.location, *cb_state, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (uint32_t i = 0; i < bufferMemoryBarrierCount; ++i) {
        if (pBufferMemoryBarriers[i].srcQueueFamilyIndex != pBufferMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803", commandBuffer,
                             error_obj.location.dot(Field::pBufferMemoryBarriers, i),
                             "has different srcQueueFamilyIndex (%" PRIu32 ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             pBufferMemoryBarriers[i].srcQueueFamilyIndex, pBufferMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }
    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        if (pImageMemoryBarriers[i].srcQueueFamilyIndex != pImageMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803", commandBuffer,
                             error_obj.location.dot(Field::pImageMemoryBarriers, i),
                             "has different srcQueueFamilyIndex (%" PRIu32 ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             pImageMemoryBarriers[i].srcQueueFamilyIndex, pImageMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }

    if (cb_state->active_render_pass && ((srcStageMask & VK_PIPELINE_STAGE_HOST_BIT) != 0)) {
        skip |= LogError("VUID-vkCmdWaitEvents-srcStageMask-07308", commandBuffer, error_obj.location.dot(Field::srcStageMask),
                         "is %s.", sync_utils::StringPipelineStageFlags(srcStageMask).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                               const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    bool skip = false;
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdWaitEvents2-synchronization2-03836", commandBuffer, error_obj.location,
                         "the synchronization2 feature was not enabled.");
    }
    for (uint32_t i = 0; (i < eventCount) && !skip; i++) {
        const LogObjectList objlist(commandBuffer, pEvents[i]);
        const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfos, i);
        // TODO - likely to rework VU in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7118
        if ((pDependencyInfos[i].dependencyFlags & VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR) != 0) {
            if (!enabled_features.maintenance8) {
                skip = LogError(
                    "VUID-vkCmdWaitEvents2-maintenance8-10205", objlist, dep_info_loc.dot(Field::dependencyFlags),
                    "VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR is used, but maintenance8 feature "
                    "was not enabled.");
            }
        } else if (pDependencyInfos[i].dependencyFlags != 0) {
            skip |= LogError("VUID-vkCmdWaitEvents2-dependencyFlags-10394", objlist, dep_info_loc.dot(Field::dependencyFlags),
                             "(%s) must be 0.", string_VkDependencyFlags(pDependencyInfos[i].dependencyFlags).c_str());
        }
        skip |= ValidateDependencyInfo(objlist, dep_info_loc, *cb_state, pDependencyInfos[i]);
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

void CoreChecks::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                            VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                            uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                            uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                            uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                            const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                             pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                             imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    TransitionImageLayouts(*cb_state, imageMemoryBarrierCount, pImageMemoryBarriers, sourceStageMask, dstStageMask);
}

void CoreChecks::RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos, Func command) {
    // don't hold read lock during the base class method
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        TransitionImageLayouts(*cb_state, dep_info.imageMemoryBarrierCount, dep_info.pImageMemoryBarriers);
    }
}

void CoreChecks::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void CoreChecks::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj.location.function);
}

void CoreChecks::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    RecordBarriers(record_obj.location.function, *cb_state, sourceStageMask, dstStageMask, bufferMemoryBarrierCount,
                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(record_obj.location.function, *cb_state, dep_info);
    }
}

void CoreChecks::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(record_obj.location.function, *cb_state, dep_info);
    }
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    const LogObjectList objlist(commandBuffer);
    auto queue_flags = cb_state->GetQueueFlags();

    if (!enabled_features.maintenance8 &&
        (dependencyFlags & VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR)) {
        skip = LogError("VUID-vkCmdPipelineBarrier-maintenance8-10206", objlist, error_obj.location.dot(Field::dependencyFlags),
                        "VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR is used, but maintenance8 feature "
                        "was not enabled.");
    }

    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::dstStageMask), queue_flags, dstStageMask);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (cb_state->active_render_pass && !cb_state->active_render_pass->UsesDynamicRendering()) {
        skip |= ValidateRenderPassPipelineBarriers(error_obj.location, *cb_state, srcStageMask, dstStageMask, dependencyFlags,
                                                   memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip =
                LogError("VUID-vkCmdPipelineBarrier-dependencyFlags-01186", objlist, error_obj.location.dot(Field::dependencyFlags),
                         "VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance.");
        }
    }
    if (cb_state->active_render_pass && cb_state->active_render_pass->UsesDynamicRendering()) {
        // In dynamic rendering, vkCmdPipelineBarrier is only allowed for VK_EXT_shader_tile_image
        skip |= ValidateShaderTileImageBarriers(objlist, error_obj.location, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                                                bufferMemoryBarrierCount, imageMemoryBarrierCount, pImageMemoryBarriers,
                                                srcStageMask, dstStageMask);
    }
    skip |= ValidateBarriers(error_obj.location, *cb_state, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    const LogObjectList objlist(commandBuffer);

    const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfo);
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdPipelineBarrier2-synchronization2-03848", commandBuffer, error_obj.location,
                         "the synchronization2 feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (cb_state->active_render_pass) {
        skip |= ValidateRenderPassPipelineBarriers(dep_info_loc, *cb_state, *pDependencyInfo);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (pDependencyInfo->dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip |= LogError("VUID-vkCmdPipelineBarrier2-dependencyFlags-01186", objlist, dep_info_loc.dot(Field::dependencyFlags),
                             "VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance.");
        }
    }
    if (cb_state->active_render_pass && cb_state->active_render_pass->UsesDynamicRendering()) {
        // In dynamic rendering, vkCmdPipelineBarrier2 is only allowed for  VK_EXT_shader_tile_image
        skip |= ValidateShaderTileImageBarriers(objlist, dep_info_loc, *pDependencyInfo);
    }
    skip |= ValidateDependencyInfo(objlist, dep_info_loc, *cb_state, *pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                 const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                                  pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                  imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);

    RecordBarriers(record_obj.location.function, *cb_state, srcStageMask, dstStageMask, bufferMemoryBarrierCount,
                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    TransitionImageLayouts(*cb_state, imageMemoryBarrierCount, pImageMemoryBarriers, srcStageMask, dstStageMask);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                     const RecordObject &record_obj) {
    PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                  const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    RecordBarriers(record_obj.location.function, *cb_state, *pDependencyInfo);
    TransitionImageLayouts(*cb_state, pDependencyInfo->imageMemoryBarrierCount, pDependencyInfo->pImageMemoryBarriers);
}

bool CoreChecks::PreCallValidateSetEvent(VkDevice device, VkEvent event, const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto event_state = Get<vvl::Event>(event)) {
        if (event_state->InUse()) {
            skip |= LogError("VUID-vkSetEvent-event-09543", event, error_obj.location.dot(Field::event),
                             "(%s) that is already in use by a command buffer.", FormatHandle(event).c_str());
        }
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT) {
            skip |= LogError("VUID-vkSetEvent-event-03941", event, error_obj.location.dot(Field::event),
                             "(%s) was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT.", FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetEvent(VkDevice device, VkEvent event, const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto event_state = Get<vvl::Event>(event)) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT) {
            skip |= LogError("VUID-vkResetEvent-event-03823", event, error_obj.location.dot(Field::event),
                             "(%s) was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT.", FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetEventStatus(VkDevice device, VkEvent event, const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto event_state = Get<vvl::Event>(event)) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT) {
            skip |= LogError("VUID-vkGetEventStatus-event-03940", event, error_obj.location.dot(Field::event),
                             "(%s) was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT.", FormatHandle(event).c_str());
        }
    }
    return skip;
}
bool CoreChecks::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const Location signal_loc = error_obj.location.dot(Field::pSignalInfo);
    auto semaphore_state = Get<vvl::Semaphore>(pSignalInfo->semaphore);
    ASSERT_AND_RETURN_SKIP(semaphore_state);

    if (semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError("VUID-VkSemaphoreSignalInfo-semaphore-03257", pSignalInfo->semaphore, signal_loc.dot(Field::semaphore),
                         "%s was created with %s.", FormatHandle(pSignalInfo->semaphore).c_str(),
                         string_VkSemaphoreType(semaphore_state->type));
        return skip;
    }

    const auto current_payload = semaphore_state->CurrentPayload();
    if (current_payload >= pSignalInfo->value) {
        skip |= LogError("VUID-VkSemaphoreSignalInfo-value-03258", pSignalInfo->semaphore, signal_loc.dot(Field::value),
                         "(%" PRIu64 ") must be greater than current semaphore %s value (%" PRIu64 ").", pSignalInfo->value,
                         FormatHandle(pSignalInfo->semaphore).c_str(), current_payload);
        return skip;
    }
    auto exceeds_pending = [pSignalInfo](const vvl::Semaphore::OpType op_type, uint64_t payload, bool is_pending) {
        return is_pending && op_type == vvl::Semaphore::OpType::kSignal && pSignalInfo->value >= payload;
    };
    auto last_op = semaphore_state->LastOp(exceeds_pending);
    if (last_op) {
        skip |= LogError("VUID-VkSemaphoreSignalInfo-value-03259", pSignalInfo->semaphore, signal_loc.dot(Field::value),
                         "(%" PRIu64 ") must be less than value of any pending signal operation (%" PRIu64 ") for semaphore %s.",
                         pSignalInfo->value, last_op->payload, FormatHandle(pSignalInfo->semaphore).c_str());
        return skip;
    }

    uint64_t bad_value = 0;
    const char *where = nullptr;
    TimelineMaxDiffCheck exceeds_max_diff(pSignalInfo->value, phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
    last_op = semaphore_state->LastOp(exceeds_max_diff);
    if (last_op) {
        bad_value = last_op->payload;
        if (last_op->payload == semaphore_state->CurrentPayload()) {
            where = "current";
        } else {
            where = "pending";
        }
    }
    if (where) {
        const Location loc = error_obj.location.dot(Struct::VkSemaphoreSignalInfo, Field::value);
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, sync_vuid_maps::SubmitError::kTimelineSemMaxDiff);
        skip |= LogError(vuid, semaphore_state->Handle(), loc,
                         "(%" PRIu64 ") exceeds limit regarding %s semaphore %s payload (%" PRIu64 ").", pSignalInfo->value,
                         FormatHandle(*semaphore_state).c_str(), where, bad_value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                   const ErrorObject &error_obj) const {
    return PreCallValidateSignalSemaphore(device, pSignalInfo, error_obj);
}

bool CoreChecks::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    auto semaphore_state = Get<vvl::Semaphore>(semaphore);
    ASSERT_AND_RETURN_SKIP(semaphore_state);
    if (semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError("VUID-vkGetSemaphoreCounterValue-semaphore-03255", semaphore, error_obj.location.dot(Field::semaphore),
                         "%s was created with %s.", FormatHandle(semaphore).c_str(), string_VkSemaphoreType(semaphore_state->type));
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue, error_obj);
}

// VkSubpassDependency validation happens when vkCreateRenderPass() is called.
// Dependencies between subpasses can only use pipeline stages compatible with VK_QUEUE_GRAPHICS_BIT,
// for external subpasses we don't have a yet command buffer so we have to assume all of them are valid.
static inline VkQueueFlags SubpassToQueueFlags(uint32_t subpass) {
    return subpass == VK_SUBPASS_EXTERNAL ? sync_utils::kAllQueueTypes : static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT);
}

bool CoreChecks::ValidateSubpassDependency(const ErrorObject &error_obj, const Location &in_loc,
                                           const VkSubpassDependency2 &dependency) const {
    bool skip = false;

    if (dependency.dependencyFlags & VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR) {
        const bool use_rp2 = error_obj.location.function != Func::vkCreateRenderPass;
        auto vuid = use_rp2 ? "VUID-VkSubpassDependency2-dependencyFlags-10204" : "VUID-VkSubpassDependency-dependencyFlags-10203";
        skip |= LogError(vuid, device, in_loc.dot(Field::dependencyFlags),
                         "contains VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR");
    }

    VkMemoryBarrier2 converted_barrier;
    const auto *mem_barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(dependency.pNext);
    const Location loc = mem_barrier ? in_loc.dot(Field::pNext) : in_loc;

    if (mem_barrier) {
        converted_barrier = *mem_barrier;
    } else {
        // use the subpass dependency flags, upconverted into wider synchronization2 fields.
        converted_barrier.srcStageMask = dependency.srcStageMask;
        converted_barrier.dstStageMask = dependency.dstStageMask;
        converted_barrier.srcAccessMask = dependency.srcAccessMask;
        converted_barrier.dstAccessMask = dependency.dstAccessMask;
    }
    auto src_queue_flags = SubpassToQueueFlags(dependency.srcSubpass);
    skip |= ValidatePipelineStage(error_obj.objlist, loc.dot(Field::srcStageMask), src_queue_flags, converted_barrier.srcStageMask);
    skip |= ValidateAccessMask(error_obj.objlist, loc.dot(Field::srcAccessMask), loc.dot(Field::srcStageMask), src_queue_flags,
                               converted_barrier.srcAccessMask, converted_barrier.srcStageMask);

    auto dst_queue_flags = SubpassToQueueFlags(dependency.dstSubpass);
    skip |= ValidatePipelineStage(error_obj.objlist, loc.dot(Field::dstStageMask), dst_queue_flags, converted_barrier.dstStageMask);
    skip |= ValidateAccessMask(error_obj.objlist, loc.dot(Field::dstAccessMask), loc.dot(Field::dstStageMask), dst_queue_flags,
                               converted_barrier.dstAccessMask, converted_barrier.dstStageMask);
    return skip;
}

// Verify an ImageMemoryBarrier's old/new ImageLayouts are compatible with the Image's ImageUsageFlags.
bool CoreChecks::ValidateBarrierLayoutToImageUsage(const Location &layout_loc, VkImage image, VkImageLayout layout,
                                                   VkImageUsageFlags usage_flags) const {
    bool skip = false;
    bool is_error = false;
    switch (layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0);
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0);
            break;
        // alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            // alias VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
            is_error = ((usage_flags & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            is_error = ((usage_flags & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) == 0);
            is_error |= ((usage_flags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0);
            is_error |= ((usage_flags & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT) == 0);
            break;
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
            is_error = !IsShaderTileImageUsageValid(usage_flags);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
            is_error = ((usage_flags & (VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR |
                                        VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR)) == 0);
            break;
        default:
            // Other VkImageLayout values do not have VUs defined in this context.
            break;
    }

    if (is_error) {
        const auto &vuid = sync_vuid_maps::GetBadImageLayoutVUID(layout_loc, layout);
        skip |= LogError(vuid, image, layout_loc, "(%s) is not compatible with %s usage flags %s.", string_VkImageLayout(layout),
                         FormatHandle(image).c_str(), string_VkImageUsageFlags(usage_flags).c_str());
    }
    return skip;
}

std::vector<uint32_t> GetUsedAttachments(const vvl::CommandBuffer &cb_state) {
    std::set<uint32_t> unique;

    for (size_t i = 0; i < cb_state.rendering_attachments.color_locations.size(); ++i) {
        const uint32_t unmapped_color_attachment = cb_state.rendering_attachments.color_locations[i];
        if (unmapped_color_attachment != VK_ATTACHMENT_UNUSED) {
            unique.insert(unmapped_color_attachment);
        }
    }

    for (size_t i = 0; i < cb_state.rendering_attachments.color_indexes.size(); ++i) {
        const uint32_t unmapped_color_index = cb_state.rendering_attachments.color_indexes[i];
        if (unmapped_color_index != VK_ATTACHMENT_UNUSED) {
            unique.insert(unmapped_color_index);
        }
    }

    if (cb_state.rendering_attachments.depth_index) {
        unique.insert(*cb_state.rendering_attachments.depth_index);
    }
    if (cb_state.rendering_attachments.stencil_index) {
        unique.insert(*cb_state.rendering_attachments.stencil_index);
    }

    std::vector<uint32_t> attachments;
    for (auto x : unique) {
        attachments.push_back(x);
    }
    return attachments;
}

// Verify image barriers are compatible with the images they reference.
bool CoreChecks::ValidateBarriersToImages(const Location &barrier_loc, const vvl::CommandBuffer &cb_state,
                                          const ImageBarrier &img_barrier,
                                          vvl::CommandBuffer::ImageLayoutMap &layout_updates_state) const {
    bool skip = false;
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;

    const auto &current_map = cb_state.GetImageLayoutMap();

    {
        auto image_state = Get<vvl::Image>(img_barrier.image);
        ASSERT_AND_RETURN_SKIP(image_state);

        auto image_loc = barrier_loc.dot(Field::image);

        if ((img_barrier.srcQueueFamilyIndex != img_barrier.dstQueueFamilyIndex) ||
            (img_barrier.oldLayout != img_barrier.newLayout)) {
            VkImageUsageFlags usage_flags = image_state->create_info.usage;
            skip |= ValidateBarrierLayoutToImageUsage(barrier_loc.dot(Field::oldLayout), img_barrier.image, img_barrier.oldLayout,
                                                      usage_flags);
            skip |= ValidateBarrierLayoutToImageUsage(barrier_loc.dot(Field::newLayout), img_barrier.image, img_barrier.newLayout,
                                                      usage_flags);
        }

        // Make sure layout is able to be transitioned, currently only presented shared presentable images are locked
        if (image_state->layout_locked) {
            // TODO: waiting for VUID https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5078
            skip |= LogError("UNASSIGNED-barrier-shared-presentable", img_barrier.image, image_loc,
                             "(%s) is a shared presentable and attempting to transition from layout %s to layout %s, but image has "
                             "already been presented and cannot have its layout transitioned.",
                             FormatHandle(img_barrier.image).c_str(), string_VkImageLayout(img_barrier.oldLayout),
                             string_VkImageLayout(img_barrier.newLayout));
        }

        const VkImageCreateInfo &image_create_info = image_state->create_info;
        const VkFormat image_format = image_create_info.format;
        const VkImageAspectFlags aspect_mask = img_barrier.subresourceRange.aspectMask;
        const bool has_depth_mask = (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
        const bool has_stencil_mask = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;

        if (vkuFormatIsDepthAndStencil(image_format)) {
            if (enabled_features.separateDepthStencilLayouts) {
                if (!has_depth_mask && !has_stencil_mask) {
                    auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotDepthOrStencilAspect);
                    skip |=
                        LogError(vuid, img_barrier.image, image_loc, "(%s) has depth/stencil format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
                }
            } else {
                if (!has_depth_mask || !has_stencil_mask) {
                    auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotDepthAndStencilAspect);
                    skip |=
                        LogError(vuid, img_barrier.image, image_loc, "(%s) has depth/stencil format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
                }
            }
        }

        if (has_depth_mask) {
            if (IsImageLayoutStencilOnly(img_barrier.oldLayout) || IsImageLayoutStencilOnly(img_barrier.newLayout)) {
                auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kSeparateDepthWithStencilLayout);
                skip |= LogError(
                    vuid, img_barrier.image, image_loc,
                    "(%s) has stencil format %s has depth aspect with stencil only layouts, oldLayout = %s and newLayout = %s.",
                    FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                    string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
            }
        }
        if (has_stencil_mask) {
            if (IsImageLayoutDepthOnly(img_barrier.oldLayout) || IsImageLayoutDepthOnly(img_barrier.newLayout)) {
                auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kSeparateStencilhWithDepthLayout);
                skip |= LogError(
                    vuid, img_barrier.image, image_loc,
                    "(%s) has depth format %s has stencil aspect with depth only layouts, oldLayout = %s and newLayout = %s.",
                    FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                    string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
            }
        }

        if (!enabled_features.dynamicRenderingLocalRead) {
            if (img_barrier.newLayout == VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ) {
                auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kDynamicRenderingLocalReadNew);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) cannot have newLayout = %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkImageLayout(img_barrier.newLayout));
            }
            if (img_barrier.oldLayout == VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ) {
                auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kDynamicRenderingLocalReadOld);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) cannot have oldLayout = %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkImageLayout(img_barrier.oldLayout));
            }
        }

        if (img_barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            // TODO: Set memory invalid which is in mem_tracker currently
        } else if (!IsQueueFamilyExternal(img_barrier.srcQueueFamilyIndex)) {
            skip |= UpdateCommandBufferImageLayoutMap(cb_state, image_loc, img_barrier, current_map, layout_updates_state);
        }

        const vvl::RenderPass *rp_state = cb_state.active_render_pass.get();
        if (enabled_features.dynamicRenderingLocalRead && rp_state) {
            const auto &img_barrier_image = img_barrier.image;
            const auto &rendering_info = rp_state->dynamic_rendering_begin_rendering_info;
            std::vector<uint32_t> used_attachments(GetUsedAttachments(cb_state));

            for (auto color_attachment_idx : used_attachments) {
                if (color_attachment_idx >= rendering_info.colorAttachmentCount) {
                    continue;
                }
                const auto &color_attachment = rendering_info.pColorAttachments[color_attachment_idx];
                if (color_attachment.imageView == VK_NULL_HANDLE) {
                    continue;
                }
                const auto image_view_state = Get<vvl::ImageView>(color_attachment.imageView);
                ASSERT_AND_CONTINUE(image_view_state);
                const auto &image_view_image_state = image_view_state->image_state;

                if (img_barrier_image == image_view_image_state->VkHandle()) {
                    auto guard = image_view_image_state->layout_range_map->ReadLock();

                    for (const auto &entry : *image_view_image_state->layout_range_map) {
                        if (entry.second != VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ && entry.second != VK_IMAGE_LAYOUT_GENERAL) {
                            const auto &vuid = sync_vuid_maps::GetShaderTileImageVUID(
                                barrier_loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageLayout);
                            skip |= LogError(vuid, img_barrier.image, barrier_loc, "image layout is %s.",
                                             string_VkImageLayout(entry.second));
                        }
                    }
                }
            }
        }

        // checks color format and (single-plane or non-disjoint)
        // if ycbcr extension is not supported then single-plane and non-disjoint are always both true

        if (vkuFormatIsColor(image_format) && (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT)) {
            if (!vkuFormatIsMultiplane(image_format)) {
                const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotColorAspectSinglePlane);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) has color format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            } else if (!image_state->disjoint) {
                const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotColorAspectNonDisjoint);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) has color format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            }
        }

        if ((vkuFormatIsMultiplane(image_format)) && (image_state->disjoint == true)) {
            if (!IsValidPlaneAspect(image_format, aspect_mask) && ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) == 0)) {
                const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kBadMultiplanarAspect);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) has Multiplane format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            }
        }
    }
    return skip;
}

// Verify image barrier image state and that the image is consistent with FB image
bool CoreChecks::ValidateImageBarrierAttachment(const Location &barrier_loc, const vvl::CommandBuffer &cb_state,
                                                const vvl::Framebuffer &fb_state, uint32_t active_subpass,
                                                const vku::safe_VkSubpassDescription2 &sub_desc, const VkRenderPass rp_handle,
                                                const ImageBarrier &img_barrier, const vvl::CommandBuffer *primary_cb_state) const {
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;

    bool skip = false;
    const auto img_bar_image = img_barrier.image;
    bool image_match = false;
    bool sub_image_found = false;  // Do we find a corresponding subpass description
    VkImageLayout sub_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t attach_index = 0;
    uint64_t image_ahb_format = 0;
    const Location image_loc = barrier_loc.dot(Field::image);
    // Verify that a framebuffer image matches barrier image
    const auto attachment_count = fb_state.create_info.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
        auto view_state = primary_cb_state ? primary_cb_state->GetActiveAttachmentImageViewState(attachment)
                                           : cb_state.GetActiveAttachmentImageViewState(attachment);
        if (view_state && (img_bar_image == view_state->create_info.image)) {
            image_match = true;
            attach_index = attachment;
            image_ahb_format = view_state->image_state->ahb_format;
            break;
        }
    }
    if (image_match) {  // Make sure subpass is referring to matching attachment
        if (sub_desc.pDepthStencilAttachment && sub_desc.pDepthStencilAttachment->attachment == attach_index) {
            sub_image_layout = sub_desc.pDepthStencilAttachment->layout;
            sub_image_found = true;
        }
        if (!sub_image_found) {
            const auto *resolve = vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(sub_desc.pNext);
            if (resolve && resolve->pDepthStencilResolveAttachment &&
                resolve->pDepthStencilResolveAttachment->attachment == attach_index) {
                sub_image_layout = resolve->pDepthStencilResolveAttachment->layout;
                sub_image_found = true;
            }
        }
        if (!sub_image_found) {
            for (uint32_t j = 0; j < sub_desc.colorAttachmentCount; ++j) {
                if (sub_desc.pColorAttachments && sub_desc.pColorAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pColorAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
                // Will also catch a "color resolve" attachment
                if (!sub_image_found && sub_desc.pResolveAttachments &&
                    sub_desc.pResolveAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pResolveAttachments[j].layout;
                    sub_image_found = true;
                    if (image_ahb_format == 0) {
                        const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassMismatchAhbZero);
                        skip |= LogError(vuid, rp_handle, image_loc,
                                         "(%s) for subpass %" PRIu32 " was not created with an externalFormat.",
                                         FormatHandle(img_bar_image).c_str(), active_subpass);
                    } else if (sub_desc.pColorAttachments && sub_desc.pColorAttachments[0].attachment != VK_ATTACHMENT_UNUSED) {
                        const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassMismatchColorUnused);
                        skip |=
                            LogError(vuid, rp_handle, image_loc,
                                     "(%s) for subpass %" PRIu32 " the pColorAttachments[0].attachment is %" PRIu32
                                     " instead of VK_ATTACHMENT_UNUSED.",
                                     FormatHandle(img_bar_image).c_str(), active_subpass, sub_desc.pColorAttachments[0].attachment);
                    }
                    break;
                }
            }
        }
        if (!sub_image_found) {
            const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassMismatch);
            skip |= LogError(vuid, rp_handle, image_loc,
                             "(%s) is not referenced by the VkSubpassDescription for active subpass (%" PRIu32 ") of current %s.",
                             FormatHandle(img_bar_image).c_str(), active_subpass, FormatHandle(rp_handle).c_str());
        }

    } else {  // !image_match
        const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassMismatch);
        skip |= LogError(vuid, fb_state.Handle(), image_loc, "(%s) does not match an image from the current %s.",
                         FormatHandle(img_bar_image).c_str(), FormatHandle(fb_state.Handle()).c_str());
    }
    if (img_barrier.oldLayout != img_barrier.newLayout) {
        const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassLayoutChange);
        skip |= LogError(vuid, cb_state.Handle(), barrier_loc.dot(Field::oldLayout),
                         "is %s and newLayout is %s, but %s is being executed within a render pass instance.",
                         string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout),
                         FormatHandle(img_barrier.image).c_str());
    } else {
        if (sub_image_found && sub_image_layout != img_barrier.oldLayout) {
            const LogObjectList objlist(rp_handle, img_bar_image);
            const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassLayoutChange);
            skip |= LogError(vuid, objlist, image_loc,
                             "(%s) is referenced by the VkSubpassDescription for active "
                             "subpass (%" PRIu32 ") of current %s as having layout %s, but image barrier has layout %s.",
                             FormatHandle(img_bar_image).c_str(), active_subpass, FormatHandle(rp_handle).c_str(),
                             string_VkImageLayout(sub_image_layout), string_VkImageLayout(img_barrier.oldLayout));
        }
    }
    return skip;
}

void CoreChecks::EnqueueSubmitTimeValidateImageBarrierAttachment(const Location &loc, vvl::CommandBuffer &cb_state,
                                                                 const ImageBarrier &barrier) {
    // Secondary CBs can have null framebuffer so queue up validation in that case 'til FB is known
    const vvl::RenderPass *rp_state = cb_state.active_render_pass.get();
    if (rp_state && (VK_NULL_HANDLE == cb_state.activeFramebuffer) && cb_state.IsSecondary()) {
        const auto active_subpass = cb_state.GetActiveSubpass();
        if (active_subpass < rp_state->create_info.subpassCount) {
            const auto &sub_desc = rp_state->create_info.pSubpasses[active_subpass];
            // Secondary CB case w/o FB specified delay validation
            auto *this_ptr = this;  // Required for older compilers with c++20 compatibility
            vvl::LocationCapture loc_capture(loc);
            const VkRenderPass render_pass = rp_state->VkHandle();
            cb_state.cmd_execute_commands_functions.emplace_back(
                [this_ptr, loc_capture, active_subpass, sub_desc, render_pass, barrier](
                    const vvl::CommandBuffer &secondary_cb, const vvl::CommandBuffer *primary_cb, const vvl::Framebuffer *fb) {
                    if (!fb) return false;
                    return this_ptr->ValidateImageBarrierAttachment(loc_capture.Get(), secondary_cb, *fb, active_subpass, sub_desc,
                                                                    render_pass, barrier, primary_cb);
                });
        }
    }
}

static bool IsQueueFamilyValid(const vvl::Device &device_data, uint32_t queue_family) {
    return (queue_family < static_cast<uint32_t>(device_data.physical_device_state->queue_family_properties.size()));
}

static bool IsQueueFamilySpecial(uint32_t queue_family) {
    return IsQueueFamilyExternal(queue_family) || (queue_family == VK_QUEUE_FAMILY_IGNORED);
}

static const char *GetFamilyAnnotation(const vvl::Device &device_data, uint32_t family) {
    switch (family) {
        case VK_QUEUE_FAMILY_EXTERNAL:
            return " (VK_QUEUE_FAMILY_EXTERNAL)";
        case VK_QUEUE_FAMILY_FOREIGN_EXT:
            return " (VK_QUEUE_FAMILY_FOREIGN_EXT)";
        case VK_QUEUE_FAMILY_IGNORED:
            return " (VK_QUEUE_FAMILY_IGNORED)";
        default:
            if (!IsQueueFamilyValid(device_data, family)) {
                return " (invalid queue family index)";
            }
            return "";
    }
}

bool CoreChecks::ValidateHostStage(const LogObjectList &objlist, const Location &barrier_loc,
                                   const OwnershipTransferBarrier &barrier) const {
    bool skip = false;
    // src/dst queue families should be equal if HOST_BIT is used
    if (barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex) {
        const bool is_sync2 = barrier_loc.structure == vvl::Struct::VkBufferMemoryBarrier2 ||
                              barrier_loc.structure == vvl::Struct::VkImageMemoryBarrier2;
        auto stage_field = vvl::Field::Empty;
        if (barrier.srcStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            stage_field = vvl::Field::srcStageMask;
        } else if (barrier.dstStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            stage_field = vvl::Field::dstStageMask;
        }
        if (stage_field != vvl::Field::Empty) {
            const auto &vuid = sync_vuid_maps::GetBarrierQueueVUID(barrier_loc, sync_vuid_maps::QueueError::kHostStage);
            const Location stage_loc = is_sync2 ? barrier_loc.dot(stage_field) : Location(barrier_loc.function, stage_field);
            skip |= LogError(vuid, objlist, stage_loc,
                             "is %s but srcQueueFamilyIndex (%" PRIu32 ") != dstQueueFamilyIndex (%" PRIu32 ").",
                             is_sync2 ? "VK_PIPELINE_STAGE_2_HOST_BIT" : "VK_PIPELINE_STAGE_HOST_BIT", barrier.srcQueueFamilyIndex,
                             barrier.dstQueueFamilyIndex);
        }
    }
    return skip;
}

void CoreChecks::RecordBarrierValidationInfo(const Location &barrier_loc, vvl::CommandBuffer &cb_state,
                                             const BufferBarrier &barrier,
                                             QFOTransferBarrierSets<QFOBufferTransferBarrier> &barrier_sets) {
    if (IsOwnershipTransfer(barrier)) {
        if (auto buffer = Get<vvl::Buffer>(barrier.buffer)) {
            if (cb_state.IsReleaseOp(barrier) && !IsQueueFamilyExternal(barrier.dstQueueFamilyIndex)) {
                barrier_sets.release.emplace(barrier);
            } else if (cb_state.IsAcquireOp(barrier) && !IsQueueFamilyExternal(barrier.srcQueueFamilyIndex)) {
                barrier_sets.acquire.emplace(barrier);
            }
        }
    }
}

void CoreChecks::RecordBarrierValidationInfo(const Location &barrier_loc, vvl::CommandBuffer &cb_state,
                                             const ImageBarrier &image_barrier,
                                             QFOTransferBarrierSets<QFOImageTransferBarrier> &barrier_sets) {
    if (IsOwnershipTransfer(image_barrier)) {
        if (auto image = Get<vvl::Image>(image_barrier.image)) {
            ImageBarrier barrier = image_barrier;
            barrier.subresourceRange = NormalizeSubresourceRange(image->create_info, image_barrier.subresourceRange);

            if (cb_state.IsReleaseOp(barrier) && !IsQueueFamilyExternal(barrier.dstQueueFamilyIndex)) {
                barrier_sets.release.emplace(barrier);
            } else if (cb_state.IsAcquireOp(barrier) && !IsQueueFamilyExternal(barrier.srcQueueFamilyIndex)) {
                barrier_sets.acquire.emplace(barrier);
            }
        }
    }
}

void CoreChecks::RecordBarriers(Func func_name, vvl::CommandBuffer &cb_state, VkPipelineStageFlags src_stage_mask,
                                VkPipelineStageFlags dst_stage_mask, uint32_t bufferBarrierCount,
                                const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                const VkImageMemoryBarrier *pImageMemBarriers) {
    for (uint32_t i = 0; i < bufferBarrierCount; i++) {
        Location barrier_loc(func_name, Struct::VkBufferMemoryBarrier, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(pBufferMemBarriers[i], src_stage_mask, dst_stage_mask);
        RecordBarrierValidationInfo(barrier_loc, cb_state, barrier, cb_state.qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; i++) {
        Location barrier_loc(func_name, Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        const ImageBarrier img_barrier(pImageMemBarriers[i], src_stage_mask, dst_stage_mask);
        RecordBarrierValidationInfo(barrier_loc, cb_state, img_barrier, cb_state.qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(barrier_loc, cb_state, img_barrier);
    }
}

void CoreChecks::RecordBarriers(Func func_name, vvl::CommandBuffer &cb_state, const VkDependencyInfo &dep_info) {
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        Location barrier_loc(func_name, Struct::VkBufferMemoryBarrier2, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(dep_info.pBufferMemoryBarriers[i]);
        RecordBarrierValidationInfo(barrier_loc, cb_state, barrier, cb_state.qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        Location barrier_loc(func_name, Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);
        const ImageBarrier img_barrier(dep_info.pImageMemoryBarriers[i]);
        RecordBarrierValidationInfo(barrier_loc, cb_state, img_barrier, cb_state.qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(barrier_loc, cb_state, img_barrier);
    }
}

template <typename TransferBarrier, typename Scoreboard>
bool CoreChecks::ValidateAndUpdateQFOScoreboard(const vvl::CommandBuffer &cb_state, const char *operation,
                                                const TransferBarrier &barrier, Scoreboard *scoreboard, const Location &loc) const {
    // Record to the scoreboard or report that we have a duplication
    bool skip = false;
    auto inserted = scoreboard->emplace(barrier, &cb_state);
    if (!inserted.second && inserted.first->second != &cb_state) {
        // This is a duplication (but don't report duplicates from the same CB, as we do that at record time
        const LogObjectList objlist(cb_state.Handle(), barrier.handle, inserted.first->second->Handle());
        skip |= LogWarning(TransferBarrier::DuplicateQFOInSubmit(), objlist, loc,
                           "%s %s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                           " duplicates existing barrier submitted in this batch from %s.",
                           TransferBarrier::BarrierName(), operation, TransferBarrier::HandleName(),
                           FormatHandle(barrier.handle).c_str(), barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex,
                           FormatHandle(inserted.first->second->Handle()).c_str());
    }
    return skip;
}

template <typename TransferBarrier>
bool CoreChecks::ValidateQueuedQFOTransferBarriers(const vvl::CommandBuffer &cb_state,
                                                   QFOTransferCBScoreboards<TransferBarrier> *scoreboards,
                                                   const GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers,
                                                   const Location &loc) const {
    bool skip = false;
    const auto &cb_barriers = cb_state.GetQFOBarrierSets(TransferBarrier());
    const char *barrier_name = TransferBarrier::BarrierName();
    const char *handle_name = TransferBarrier::HandleName();
    // No release should have an extant duplicate (WARNING)
    for (const auto &release : cb_barriers.release) {
        // Check the global pending release barriers
        const auto set_it = global_release_barriers.find(release.handle);
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            const auto found = set_for_handle.find(release);
            if (found != set_for_handle.cend()) {
                skip |= LogWarning(TransferBarrier::DuplicateQFOSubmitted(), cb_state.Handle(), loc,
                                   "%s releasing queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                                   " to dstQueueFamilyIndex %" PRIu32
                                   " duplicates existing barrier queued for execution, without intervening acquire operation.",
                                   barrier_name, handle_name, FormatHandle(found->handle).c_str(), found->srcQueueFamilyIndex,
                                   found->dstQueueFamilyIndex);
            }
        }
        skip |= ValidateAndUpdateQFOScoreboard(cb_state, "releasing", release, &scoreboards->release, loc);
    }
    // Each acquire must have a matching release (ERROR)
    for (const auto &acquire : cb_barriers.acquire) {
        const auto set_it = global_release_barriers.find(acquire.handle);
        bool matching_release_found = false;
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            matching_release_found = set_for_handle.find(acquire) != set_for_handle.cend();
        }
        if (!matching_release_found) {
            const char *vuid = (loc.function == vvl::Func::vkQueueSubmit) ? "VUID-vkQueueSubmit-pSubmits-02207"
                                                                          : "VUID-vkQueueSubmit2-commandBuffer-03879";
            skip |= LogError(vuid, cb_state.Handle(), loc,
                             "in submitted command buffer %s acquiring ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                             " to dstQueueFamilyIndex %" PRIu32 " has no matching release barrier queued for execution.",
                             barrier_name, handle_name, FormatHandle(acquire.handle).c_str(), acquire.srcQueueFamilyIndex,
                             acquire.dstQueueFamilyIndex);
        }
        skip |= ValidateAndUpdateQFOScoreboard(cb_state, "acquiring", acquire, &scoreboards->acquire, loc);
    }
    return skip;
}

bool CoreChecks::ValidateQueuedQFOTransfers(const vvl::CommandBuffer &cb_state,
                                            QFOTransferCBScoreboards<QFOImageTransferBarrier> *qfo_image_scoreboards,
                                            QFOTransferCBScoreboards<QFOBufferTransferBarrier> *qfo_buffer_scoreboards,
                                            const Location &loc) const {
    bool skip = false;
    skip |= ValidateQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state, qfo_image_scoreboards,
                                                                       qfo_release_image_barrier_map, loc);
    skip |= ValidateQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state, qfo_buffer_scoreboards,
                                                                        qfo_release_buffer_barrier_map, loc);
    return skip;
}

template <typename TransferBarrier>
void RecordQueuedQFOTransferBarriers(QFOTransferBarrierSets<TransferBarrier> &cb_barriers,
                                     GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers) {
    // Add release barriers from this submit to the global map
    for (const auto &release : cb_barriers.release) {
        // the global barrier list is mapped by resource handle to allow cleanup on resource destruction
        // NOTE: vvl::concurrent_ordered_map::find() makes a thread safe copy of the result, so we must
        // copy back after updating.
        auto iter = global_release_barriers.find(release.handle);
        iter->second.insert(release);
        global_release_barriers.insert_or_assign(release.handle, iter->second);
    }

    // Erase acquired barriers from this submit from the global map -- essentially marking releases as consumed
    for (const auto &acquire : cb_barriers.acquire) {
        // NOTE: We're not using [] because we don't want to create entries for missing releases
        auto set_it = global_release_barriers.find(acquire.handle);
        if (set_it != global_release_barriers.end()) {
            QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            set_for_handle.erase(acquire);
            if (set_for_handle.empty()) {  // Clean up empty sets
                global_release_barriers.erase(acquire.handle);
            } else {
                // NOTE: vvl::concurrent_ordered_map::find() makes a thread safe copy of the result, so we must
                // copy back after updating.
                global_release_barriers.insert_or_assign(acquire.handle, set_for_handle);
            }
        }
    }
}

void CoreChecks::RecordQueuedQFOTransfers(vvl::CommandBuffer &cb_state) {
    RecordQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state.qfo_transfer_image_barriers, qfo_release_image_barrier_map);
    RecordQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state.qfo_transfer_buffer_barriers,
                                                              qfo_release_buffer_barrier_map);
}

template <typename Barrier, typename TransferBarrier>
bool CoreChecks::ValidateQFOTransferBarrierUniqueness(const Location &barrier_loc, const vvl::CommandBuffer &cb_state,
                                                      const Barrier &barrier,
                                                      const QFOTransferBarrierSets<TransferBarrier> &barrier_sets) const {
    bool skip = false;
    const char *handle_name = TransferBarrier::HandleName();
    const char *transfer_type = nullptr;
    if (!IsOwnershipTransfer(barrier)) {
        return skip;
    }
    const TransferBarrier *barrier_record = nullptr;
    if (cb_state.IsReleaseOp(barrier) && !IsQueueFamilyExternal(barrier.dstQueueFamilyIndex)) {
        const auto found = barrier_sets.release.find(barrier);
        if (found != barrier_sets.release.cend()) {
            barrier_record = &(*found);
            transfer_type = "releasing";
        }
    } else if (cb_state.IsAcquireOp(barrier) && !IsQueueFamilyExternal(barrier.srcQueueFamilyIndex)) {
        const auto found = barrier_sets.acquire.find(barrier);
        if (found != barrier_sets.acquire.cend()) {
            barrier_record = &(*found);
            transfer_type = "acquiring";
        }
    }
    if (barrier_record != nullptr) {
        skip |= LogWarning(TransferBarrier::DuplicateQFOInCB(), cb_state.Handle(), barrier_loc,
                           "%s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                           " duplicates existing barrier recorded in this command buffer.",
                           transfer_type, handle_name, FormatHandle(barrier_record->handle).c_str(),
                           barrier_record->srcQueueFamilyIndex, barrier_record->dstQueueFamilyIndex);
    }
    return skip;
}

bool CoreChecks::ValidateBarrierQueueFamilies(const LogObjectList &objects, const Location &barrier_loc, const Location &field_loc,
                                              const OwnershipTransferBarrier &barrier, const VulkanTypedHandle &resource_handle,
                                              VkSharingMode sharing_mode, uint32_t command_pool_queue_family) const {
    bool skip = false;
    using sync_vuid_maps::QueueError;

    auto log_queue_family_error = [sharing_mode, resource_handle, &barrier_loc, &field_loc, device_data_ = this,
                                   objects_ = objects](QueueError vu_index, uint32_t family, const char *param_name) -> bool {
        const std::string &vuid = GetBarrierQueueVUID(field_loc, vu_index);
        const char *annotation = GetFamilyAnnotation(*device_data_, family);
        return device_data_->LogError(
            vuid, objects_, barrier_loc, "barrier using %s created with sharingMode %s, has %s %" PRIu32 "%s. %s",
            device_data_->FormatHandle(resource_handle).c_str(), string_VkSharingMode(sharing_mode), param_name, family, annotation,
            sync_vuid_maps::GetQueueErrorSummaryMap().at(vu_index).c_str());
    };
    const auto src_queue_family = barrier.srcQueueFamilyIndex;
    const auto dst_queue_family = barrier.dstQueueFamilyIndex;

    if (!IsExtEnabled(extensions.vk_khr_external_memory)) {
        if (src_queue_family == VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= log_queue_family_error(QueueError::kSrcNoExternalExt, src_queue_family, "srcQueueFamilyIndex");
        } else if (dst_queue_family == VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= log_queue_family_error(QueueError::kDstNoExternalExt, dst_queue_family, "dstQueueFamilyIndex");
        }

        if (sharing_mode == VK_SHARING_MODE_EXCLUSIVE && src_queue_family != dst_queue_family) {
            if (!IsQueueFamilyValid(*this, src_queue_family)) {
                skip |= log_queue_family_error(QueueError::kExclusiveSrc, src_queue_family, "srcQueueFamilyIndex");
            }
            if (!IsQueueFamilyValid(*this, dst_queue_family)) {
                skip |= log_queue_family_error(QueueError::kExclusiveDst, dst_queue_family, "dstQueueFamilyIndex");
            }
        }
    } else {
        if (sharing_mode == VK_SHARING_MODE_EXCLUSIVE && src_queue_family != dst_queue_family) {
            if (!(IsQueueFamilyValid(*this, src_queue_family) || IsQueueFamilySpecial(src_queue_family))) {
                skip |= log_queue_family_error(QueueError::kExclusiveSrc, src_queue_family, "srcQueueFamilyIndex");
            }
            if (!(IsQueueFamilyValid(*this, dst_queue_family) || IsQueueFamilySpecial(dst_queue_family))) {
                skip |= log_queue_family_error(QueueError::kExclusiveDst, dst_queue_family, "dstQueueFamilyIndex");
            }
        }
    }

    if (!IsExtEnabled(extensions.vk_ext_queue_family_foreign)) {
        if (src_queue_family == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            skip |= log_queue_family_error(QueueError::kSrcNoForeignExt, src_queue_family, "srcQueueFamilyIndex");
        } else if (dst_queue_family == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            skip |= log_queue_family_error(QueueError::kDstNoForeignExt, dst_queue_family, "dstQueueFamilyIndex");
        }
    }

    if (!enabled_features.synchronization2 && sharing_mode == VK_SHARING_MODE_CONCURRENT) {
        if (src_queue_family != VK_QUEUE_FAMILY_IGNORED && src_queue_family != VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= log_queue_family_error(QueueError::kSync1ConcurrentSrc, src_queue_family, "srcQueueFamilyIndex");
        } else if (dst_queue_family != VK_QUEUE_FAMILY_IGNORED && dst_queue_family != VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= log_queue_family_error(QueueError::kSync1ConcurrentDst, dst_queue_family, "dstQueueFamilyIndex");
        } else if (src_queue_family != VK_QUEUE_FAMILY_IGNORED && dst_queue_family != VK_QUEUE_FAMILY_IGNORED) {
            const std::string &vuid = GetBarrierQueueVUID(field_loc, QueueError::kSync1ConcurrentNoIgnored);
            const char *src_annotation = GetFamilyAnnotation(*this, src_queue_family);
            const char *dst_annotation = GetFamilyAnnotation(*this, dst_queue_family);
            // Log both src and dst queue families
            skip |= LogError(vuid, objects, barrier_loc,
                             "barrier using %s created with sharingMode %s, has srcQueueFamilyIndex %" PRIu32
                             "%s and dstQueueFamilyIndex %" PRIu32
                             "%s. Source or destination queue family must be VK_QUEUE_FAMILY_IGNORED.",
                             FormatHandle(resource_handle).c_str(), string_VkSharingMode(sharing_mode), src_queue_family,
                             src_annotation, dst_queue_family, dst_annotation);
        }
    }

    if (sharing_mode == VK_SHARING_MODE_EXCLUSIVE && IsOwnershipTransfer(barrier)) {
        if (src_queue_family != command_pool_queue_family && dst_queue_family != command_pool_queue_family) {
            const std::string vuid = GetBarrierQueueVUID(barrier_loc, sync_vuid_maps::QueueError::kSubmitQueueMustMatchSrcOrDst);
            const char *src_annotation = GetFamilyAnnotation(*this, src_queue_family);
            const char *dst_annotation = GetFamilyAnnotation(*this, dst_queue_family);
            skip |= LogError(
                vuid, objects, barrier_loc,
                "has srcQueueFamilyIndex %" PRIu32 "%s and dstQueueFamilyIndex %" PRIu32
                "%s. The command buffer's command pool is associated with family index %" PRIu32
                ". Source or destination queue family must match queue family associated with the command buffer's command pool.",
                src_queue_family, src_annotation, dst_queue_family, dst_annotation, command_pool_queue_family);
        }
    }

    skip |= ValidateHostStage(objects, barrier_loc, barrier);
    return skip;
}

bool CoreChecks::ValidateBufferBarrier(const LogObjectList &objects, const Location &barrier_loc,
                                       const vvl::CommandBuffer &cb_state, const BufferBarrier &mem_barrier) const {
    using sync_vuid_maps::BufferError;
    using sync_vuid_maps::GetBufferBarrierVUID;

    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(barrier_loc, cb_state, mem_barrier, cb_state.qfo_transfer_buffer_barriers);

    // Validate buffer barrier queue family indices
    if (auto buffer_state = Get<vvl::Buffer>(mem_barrier.buffer)) {
        auto buf_loc = barrier_loc.dot(Field::buffer);
        const auto &mem_vuid = GetBufferBarrierVUID(buf_loc, BufferError::kNoMemory);
        skip |= ValidateMemoryIsBoundToBuffer(cb_state.VkHandle(), *buffer_state, buf_loc, mem_vuid.c_str());

        skip |= ValidateBarrierQueueFamilies(objects, barrier_loc, buf_loc, mem_barrier, buffer_state->Handle(),
                                             buffer_state->create_info.sharingMode, cb_state.command_pool->queueFamilyIndex);

        auto buffer_size = buffer_state->create_info.size;
        if (mem_barrier.offset >= buffer_size) {
            auto offset_loc = barrier_loc.dot(Field::offset);
            const auto &vuid = GetBufferBarrierVUID(offset_loc, BufferError::kOffsetTooBig);
            skip |=
                LogError(vuid, objects, offset_loc, "%s has offset 0x%" PRIx64 " which is not less than total size 0x%" PRIx64 ".",
                         FormatHandle(mem_barrier.buffer).c_str(), HandleToUint64(mem_barrier.offset), HandleToUint64(buffer_size));
        } else if (mem_barrier.size != VK_WHOLE_SIZE && (mem_barrier.offset + mem_barrier.size > buffer_size)) {
            auto size_loc = barrier_loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeOutOfRange);
            skip |=
                LogError(vuid, objects, size_loc,
                         "%s has offset 0x%" PRIx64 " and size 0x%" PRIx64 " whose sum is greater than total size 0x%" PRIx64 ".",
                         FormatHandle(mem_barrier.buffer).c_str(), HandleToUint64(mem_barrier.offset),
                         HandleToUint64(mem_barrier.size), HandleToUint64(buffer_size));
        }
        if (mem_barrier.size == 0) {
            auto size_loc = barrier_loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeZero);
            skip |= LogError(vuid, objects, barrier_loc, "%s has a size of 0.", FormatHandle(mem_barrier.buffer).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageBarrier(const LogObjectList &objects, const Location &barrier_loc, const vvl::CommandBuffer &cb_state,
                                      const ImageBarrier &mem_barrier) const {
    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(barrier_loc, cb_state, mem_barrier, cb_state.qfo_transfer_image_barriers);
    const VkImageLayout old_layout = mem_barrier.oldLayout;
    const VkImageLayout new_layout = mem_barrier.newLayout;

    bool is_ilt = true;
    if (enabled_features.synchronization2) {
        is_ilt = old_layout != new_layout;
    } else {
        if (old_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL || old_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadSync2OldLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::oldLayout),
                             "is %s, but the synchronization2 feature was not enabled.", string_VkImageLayout(old_layout));
        }
        if (new_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL || new_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadSync2NewLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::newLayout),
                             "is %s, but the synchronization2 feature was not enabled.", string_VkImageLayout(new_layout));
        }
    }

    if (is_ilt) {
        if (new_layout == VK_IMAGE_LAYOUT_UNDEFINED || new_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::newLayout), "is %s.", string_VkImageLayout(new_layout));
        }
    }

    if (new_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
        if (!enabled_features.attachmentFeedbackLoopLayout) {
            const auto &vuid =
                sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadAttFeedbackLoopLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::newLayout),
                             "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, but the attachmentFeedbackLoopLayout "
                             "feature was not enabled.");
        }
    }

    if (auto image_data = Get<vvl::Image>(mem_barrier.image)) {
        auto image_loc = barrier_loc.dot(Field::image);
        // TODO - use LocationVuidAdapter
        const auto &vuid_no_memory = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kNoMemory);
        skip |= ValidateMemoryIsBoundToImage(objects, *image_data, image_loc, vuid_no_memory.c_str());

        skip |= ValidateBarrierQueueFamilies(objects, barrier_loc, image_loc, mem_barrier, image_data->Handle(),
                                             image_data->create_info.sharingMode, cb_state.command_pool->queueFamilyIndex);

        const auto &vuid_aspect = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kAspectMask);
        skip |=
            ValidateImageAspectMask(image_data->VkHandle(), image_data->create_info.format, mem_barrier.subresourceRange.aspectMask,
                                    image_data->disjoint, image_loc, vuid_aspect.c_str());

        skip |= ValidateImageBarrierSubresourceRange(image_data->create_info, mem_barrier.subresourceRange, objects,
                                                     barrier_loc.dot(Field::subresourceRange));
    }
    return skip;
}

bool CoreChecks::ValidateBarriers(const Location &outer_loc, const vvl::CommandBuffer &cb_state,
                                  VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                  uint32_t memBarrierCount, const VkMemoryBarrier *pMemBarriers, uint32_t bufferBarrierCount,
                                  const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                  const VkImageMemoryBarrier *pImageMemBarriers) const {
    bool skip = false;
    LogObjectList objects(cb_state.Handle());

    // Tracks duplicate layout transition for image barriers.
    // Keeps state between ValidateBarriersToImages calls.
    vvl::CommandBuffer::ImageLayoutMap layout_updates_state;

    for (uint32_t i = 0; i < memBarrierCount; ++i) {
        const Location barrier_loc = outer_loc.dot(Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        const MemoryBarrier barrier(pMemBarriers[i], src_stage_mask, dst_stage_mask);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; ++i) {
        const Location barrier_loc = outer_loc.dot(Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        const ImageBarrier barrier(pImageMemBarriers[i], src_stage_mask, dst_stage_mask);
        const OwnershipTransferOp transfer_op = barrier.TransferOp(cb_state.command_pool->queueFamilyIndex);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier, transfer_op);
        skip |= ValidateImageBarrier(objects, barrier_loc, cb_state, barrier);
        skip |= ValidateBarriersToImages(barrier_loc, cb_state, barrier, layout_updates_state);
    }
    for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
        const Location barrier_loc = outer_loc.dot(Struct::VkBufferMemoryBarrier, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(pBufferMemBarriers[i], src_stage_mask, dst_stage_mask);
        const OwnershipTransferOp transfer_op = barrier.TransferOp(cb_state.command_pool->queueFamilyIndex);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier, transfer_op);
        skip |= ValidateBufferBarrier(objects, barrier_loc, cb_state, barrier);
    }
    return skip;
}

bool CoreChecks::ValidateDependencyInfo(const LogObjectList &objects, const Location &dep_info_loc,
                                        const vvl::CommandBuffer &cb_state, const VkDependencyInfo &dep_info) const {
    bool skip = false;

    // Tracks duplicate layout transition for image barriers.
    // Keeps state between ValidateBarriersToImages calls.
    vvl::CommandBuffer::ImageLayoutMap layout_updates_state;

    for (uint32_t i = 0; i < dep_info.memoryBarrierCount; ++i) {
        const Location barrier_loc = dep_info_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        const MemoryBarrier barrier(dep_info.pMemoryBarriers[i]);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier);
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; ++i) {
        const Location barrier_loc = dep_info_loc.dot(Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);
        const ImageBarrier barrier(dep_info.pImageMemoryBarriers[i]);
        const OwnershipTransferOp transfer_op = barrier.TransferOp(cb_state.command_pool->queueFamilyIndex);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier, transfer_op, dep_info.dependencyFlags);
        skip |= ValidateImageBarrier(objects, barrier_loc, cb_state, barrier);
        skip |= ValidateBarriersToImages(barrier_loc, cb_state, barrier, layout_updates_state);
    }
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; ++i) {
        const Location barrier_loc = dep_info_loc.dot(Struct::VkBufferMemoryBarrier2, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(dep_info.pBufferMemoryBarriers[i]);
        const OwnershipTransferOp transfer_op = barrier.TransferOp(cb_state.command_pool->queueFamilyIndex);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier, transfer_op, dep_info.dependencyFlags);
        skip |= ValidateBufferBarrier(objects, barrier_loc, cb_state, barrier);
    }

    return skip;
}

bool CoreChecks::ValidatePipelineStageForShaderTileImage(const LogObjectList &objlist, const Location &loc,
                                                         VkPipelineStageFlags2KHR stage_mask,
                                                         VkDependencyFlags dependency_flags) const {
    bool skip = false;
    if (HasNonFramebufferStagePipelineStageFlags(stage_mask)) {
        const auto &vuid =
            sync_vuid_maps::GetShaderTileImageVUID(loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageFramebufferSpace);

        skip |= LogError(vuid, objlist, loc, "(%s) is restricted to framebuffer space stages (%s).",
                         sync_utils::StringPipelineStageFlags(stage_mask).c_str(),
                         sync_utils::StringPipelineStageFlags(kFramebufferStagePipelineStageFlags).c_str());
    }
    if (HasFramebufferStagePipelineStageFlags(stage_mask) && loc.field == Field::srcStageMask &&
        (dependency_flags & VK_DEPENDENCY_BY_REGION_BIT) != VK_DEPENDENCY_BY_REGION_BIT) {
        const auto &vuid =
            sync_vuid_maps::GetShaderTileImageVUID(loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageDependencyFlags);
        skip |= LogError(vuid, objlist, loc, "must contain VK_DEPENDENCY_BY_REGION_BIT.");
    }

    return skip;
}

bool CoreChecks::IsShaderTileImageUsageValid(VkImageUsageFlags image_usage) const {
    bool valid = false;

    valid |= ((image_usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0);
    valid |= (((image_usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) &&
              (image_usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) != 0);

    return valid;
}

bool CoreChecks::ValidateShaderTileImageBarriers(const LogObjectList &objlist, const Location &outer_loc,
                                                 const VkDependencyInfo &dep_info) const {
    bool skip = false;

    skip |= ValidateShaderTileImageCommon(objlist, outer_loc, dep_info.dependencyFlags, dep_info.bufferMemoryBarrierCount,
                                          dep_info.imageMemoryBarrierCount);

    for (uint32_t i = 0; i < dep_info.memoryBarrierCount; ++i) {
        const Location loc = outer_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        const auto &mem_barrier = dep_info.pMemoryBarriers[i];
        skip |= ValidatePipelineStageForShaderTileImage(objlist, loc.dot(Field::srcStageMask), mem_barrier.srcStageMask,
                                                        dep_info.dependencyFlags);
        skip |= ValidatePipelineStageForShaderTileImage(objlist, loc.dot(Field::dstStageMask), mem_barrier.dstStageMask,
                                                        dep_info.dependencyFlags);
    }

    return skip;
}

bool CoreChecks::ValidateShaderTileImageBarriers(const LogObjectList &objlist, const Location &outer_loc,
                                                 VkDependencyFlags dependency_flags, uint32_t memory_barrier_count,
                                                 const VkMemoryBarrier *memory_barriers, uint32_t buffer_barrier_count,
                                                 uint32_t image_barrier_count, const VkImageMemoryBarrier *image_barriers,
                                                 VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask) const {
    bool skip = false;

    skip |= ValidateShaderTileImageCommon(objlist, outer_loc, dependency_flags, buffer_barrier_count, image_barrier_count);
    skip |= ValidatePipelineStageForShaderTileImage(objlist, outer_loc.dot(Field::srcStageMask), src_stage_mask, dependency_flags);
    skip |= ValidatePipelineStageForShaderTileImage(objlist, outer_loc.dot(Field::dstStageMask), dst_stage_mask, dependency_flags);

    return skip;
}

bool CoreChecks::ValidateShaderTileImageCommon(const LogObjectList &objlist, const Location &outer_loc,
                                               VkDependencyFlags dependency_flags, uint32_t buffer_barrier_count,
                                               uint32_t image_barrier_count) const {
    bool skip = false;

    // Check shader tile image features
    const bool features_enabled = enabled_features.shaderTileImageColorReadAccess ||
                                  enabled_features.shaderTileImageDepthReadAccess || enabled_features.dynamicRenderingLocalRead;
    if (!features_enabled) {
        const auto &feature_error_vuid =
            sync_vuid_maps::GetShaderTileImageVUID(outer_loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageFeatureError);
        skip |= LogError(feature_error_vuid, objlist, outer_loc,
                         "can not be called inside a dynamic rendering instance. This can be fixed by enabling the "
                         "VK_KHR_dynamic_rendering_local_read or VK_EXT_shader_tile_image features.");
    }

    if (!enabled_features.dynamicRenderingLocalRead) {
        if (buffer_barrier_count != 0 || image_barrier_count != 0) {
            const auto &buf_img_vuid = sync_vuid_maps::GetShaderTileImageVUID(
                outer_loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageNoBuffersOrImages);
            skip |= LogError(buf_img_vuid, objlist, outer_loc,
                             "can only include memory barriers, while application specify image barrier count %" PRIu32
                             " and buffer barrier count %" PRIu32,
                             image_barrier_count, buffer_barrier_count);
        }
    }

    return skip;
}

bool CoreChecks::ValidateMemoryBarrier(const LogObjectList &objects, const Location &barrier_loc,
                                       const vvl::CommandBuffer &cb_state, const MemoryBarrier &barrier,
                                       OwnershipTransferOp ownership_transfer_op, VkDependencyFlags dependency_flags) const {
    bool skip = false;
    const VkQueueFlags queue_flags = cb_state.GetQueueFlags();
    const bool is_sync2 =
        IsValueIn(barrier_loc.structure, {Struct::VkMemoryBarrier2, Struct::VkBufferMemoryBarrier2, Struct::VkImageMemoryBarrier2});

    // Validate Sync2 stages in this function because they are defined per barrier structure.
    // Sync1 stages are shared by all barriers (vkCmdPipelineBarrier api) and are validated once per barrier command call.
    if (is_sync2) {
        const bool allow_all_stages =
            (barrier_loc.function == vvl::Func::vkCmdPipelineBarrier2 ||
             barrier_loc.function == vvl::Func::vkCmdPipelineBarrier2KHR) &&
            (dependency_flags & VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR) != 0;

        if (ownership_transfer_op != OwnershipTransferOp::acquire || allow_all_stages) {
            skip |= ValidatePipelineStage(objects, barrier_loc.dot(Field::srcStageMask), queue_flags, barrier.srcStageMask);
        }
        if (ownership_transfer_op != OwnershipTransferOp::release || allow_all_stages) {
            skip |= ValidatePipelineStage(objects, barrier_loc.dot(Field::dstStageMask), queue_flags, barrier.dstStageMask);
        }
    }

    if (ownership_transfer_op != OwnershipTransferOp::acquire) {
        skip |= ValidateAccessMask(objects, barrier_loc.dot(Field::srcAccessMask), barrier_loc.dot(Field::srcStageMask),
                                   queue_flags, barrier.srcAccessMask, barrier.srcStageMask);
    }
    if (ownership_transfer_op != OwnershipTransferOp::release) {
        skip |= ValidateAccessMask(objects, barrier_loc.dot(Field::dstAccessMask), barrier_loc.dot(Field::dstStageMask),
                                   queue_flags, barrier.dstAccessMask, barrier.dstStageMask);
    }

    if (barrier_loc.function == Func::vkCmdSetEvent2 || barrier_loc.function == Func::vkCmdSetEvent2KHR) {
        if (barrier.srcStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            skip |= LogError("VUID-vkCmdSetEvent2-srcStageMask-09391", objects, barrier_loc.dot(Field::srcStageMask),
                             "is VK_PIPELINE_STAGE_2_HOST_BIT.");
        }
        if (barrier.dstStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            skip |= LogError("VUID-vkCmdSetEvent2-dstStageMask-09392", objects, barrier_loc.dot(Field::dstStageMask),
                             "is VK_PIPELINE_STAGE_2_HOST_BIT.");
        }
    } else if (barrier_loc.function == Func::vkCmdWaitEvents2 || barrier_loc.function == Func::vkCmdWaitEvents2KHR) {
        if (barrier.srcStageMask == VK_PIPELINE_STAGE_2_HOST_BIT && cb_state.active_render_pass) {
            skip |= LogError("VUID-vkCmdWaitEvents2-dependencyFlags-03844", objects, barrier_loc.dot(Field::srcStageMask),
                             "is VK_PIPELINE_STAGE_2_HOST_BIT inside the render pass.");
        }
    }
    return skip;
}
