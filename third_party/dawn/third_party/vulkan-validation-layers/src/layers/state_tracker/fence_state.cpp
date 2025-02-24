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
#include "state_tracker/fence_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/state_tracker.h"

static VkExternalFenceHandleTypeFlags GetExportHandleTypes(const VkFenceCreateInfo *info) {
    auto export_info = vku::FindStructInPNextChain<VkExportFenceCreateInfo>(info->pNext);
    return export_info ? export_info->handleTypes : 0;
}

vvl::Fence::Fence(vvl::Device &dev, VkFence handle, const VkFenceCreateInfo *pCreateInfo)
    : RefcountedStateObject(handle, kVulkanObjectTypeFence),
      flags(pCreateInfo->flags),
      export_handle_types(GetExportHandleTypes(pCreateInfo)),
      state_((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? kRetired : kUnsignaled),
      completed_(),
      waiter_(completed_.get_future()),
      dev_data_(dev) {}

const VulkanTypedHandle *vvl::Fence::InUse() const {
    auto guard = ReadLock();
    // Fence does not have a parent (in the sense of a VVL state object), and the value returned
    // by the base class InUse is not useful for reporting (it is the fence's own handle)
    const bool in_use = RefcountedStateObject::InUse() != nullptr;
    if (!in_use) {
        return nullptr;
    }
    // If the fence is in-use there should be a queue that uses it.
    // NOTE: in-use checks are always with regard to queue operations.
    assert(queue_ != nullptr && "Can't find queue that uses the fence");
    if (queue_) {
        return &queue_->Handle();
    }
    static const VulkanTypedHandle empty{};
    return &empty;
}

bool vvl::Fence::EnqueueSignal(vvl::Queue *queue_state, uint64_t next_seq) {
    auto guard = WriteLock();
    if (scope_ != kInternal) {
        return true;
    }
    // Mark fence in use
    state_ = kInflight;
    queue_ = queue_state;
    seq_ = next_seq;
    return false;
}

// Called from a non-queue operation, such as vkWaitForFences()|
void vvl::Fence::NotifyAndWait(const Location &loc) {
    std::shared_future<void> waiter;
    AcquireFenceSync acquire_fence_sync;
    {
        // Hold the lock only while updating members, but not
        // while waiting
        auto guard = WriteLock();
        if (state_ == kInflight) {
            if (queue_) {
                queue_->Notify(seq_);
                waiter = waiter_;
            } else {
                state_ = kRetired;
                completed_.set_value();
                queue_ = nullptr;
                seq_ = 0;
            }
            acquire_fence_sync = std::move(acquire_fence_sync_);
            acquire_fence_sync_ = AcquireFenceSync{};
        }
    }
    if (waiter.valid()) {
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            dev_data_.LogError(
                "INTERNAL-ERROR-VkFence-state-timeout", Handle(), loc,
                "The Validation Layers hit a timeout waiting for fence state to update (this is most likely a validation bug).");
        }
    }
    for (const auto &submission_ref : acquire_fence_sync.submission_refs) {
        submission_ref.queue->NotifyAndWait(loc, submission_ref.seq);
    }
}

// Retire from a queue operation
void vvl::Fence::Retire() {
    auto guard = WriteLock();
    if (state_ == kInflight) {
        state_ = kRetired;
        completed_.set_value();
        queue_ = nullptr;
        seq_ = 0;
    }
}

void vvl::Fence::Reset() {
    auto guard = WriteLock();
    queue_ = nullptr;
    seq_ = 0;
    // spec: If any member of pFences currently has its payload imported with temporary permanence,
    // that fence’s prior permanent payload is first restored. The remaining operations described
    // therefore operate on the restored payload.
    if (scope_ == kExternalTemporary) {
        scope_ = kInternal;
        imported_handle_type_.reset();
    }
    state_ = kUnsignaled;
    completed_ = std::promise<void>();
    waiter_ = std::shared_future<void>(completed_.get_future());
    acquire_fence_sync_ = AcquireFenceSync{};
}

void vvl::Fence::Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kExternalPermanent) {
        if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && (flags & VK_FENCE_IMPORT_TEMPORARY_BIT) == 0) {
            scope_ = kExternalPermanent;
        } else if (scope_ == kInternal) {
            scope_ = kExternalTemporary;
        }
    }
    imported_handle_type_ = handle_type;
}

void vvl::Fence::Export(VkExternalFenceHandleTypeFlagBits handle_type) {
    auto guard = WriteLock();
    if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Export with reference transference becomes external
        scope_ = kExternalPermanent;
    } else {
        // Export with copy transference has a side effect of resetting the fence
        if (scope_ == kExternalTemporary) {
            scope_ = kInternal;
            imported_handle_type_.reset();
        }
        state_ = kUnsignaled;
        completed_ = std::promise<void>();
        waiter_ = std::shared_future<void>(completed_.get_future());
    }
}

std::optional<VkExternalFenceHandleTypeFlagBits> vvl::Fence::ImportedHandleType() const {
    auto guard = ReadLock();

    // Sanity check: fence imported -> scope is not internal
    assert(!imported_handle_type_.has_value() || scope_ != kInternal);

    return imported_handle_type_;
}

void vvl::Fence::SetAcquireFenceSync(const AcquireFenceSync &acquire_fence_sync) {
    auto guard = WriteLock();

    // An attempt to overwrite existing acquire fence sync is a bug
    assert(acquire_fence_sync.submission_refs.empty() || acquire_fence_sync_.submission_refs.empty());

    acquire_fence_sync_ = acquire_fence_sync;
}
