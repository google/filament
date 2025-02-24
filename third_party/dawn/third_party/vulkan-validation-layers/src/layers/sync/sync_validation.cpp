/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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
#include <memory>
#include <vector>

#include "sync/sync_error_messages.h"
#include "sync/sync_validation.h"
#include "sync/sync_image.h"
#include "state_tracker/buffer_state.h"
#include "utils/convert_utils.h"
#include "utils/ray_tracing_utils.h"
#include "utils/text_utils.h"
#include "vk_layer_config.h"

SyncValidator::~SyncValidator() {
    // Instance level SyncValidator does not have much to say
    const bool device_validation_object = (device != nullptr);
    // Get environment variable. Specify non-zero number to enable
    const auto show_stats_str = GetEnvironment("VK_SYNCVAL_SHOW_STATS");
    const bool show_stats = !show_stats_str.empty() && std::stoul(show_stats_str) != 0;

    if (device_validation_object && show_stats) {
        stats.ReportOnDestruction();
    }
}

bool SyncValidator::SyncError(SyncHazard hazard, const LogObjectList &objlist, const Location &loc,
                              const std::string &error_message) const {
    return LogError(string_SyncHazardVUID(hazard), objlist, loc, "%s", error_message.c_str());
}

ResourceUsageRange SyncValidator::ReserveGlobalTagRange(size_t tag_count) const {
    ResourceUsageRange reserve;
    reserve.begin = tag_limit_.fetch_add(tag_count);
    reserve.end = reserve.begin + tag_count;
    return reserve;
}

void SyncValidator::EnsureTimelineSignalsLimit(uint32_t signals_per_queue_limit, QueueId queue) {
    for (auto &[_, signals] : timeline_signals_) {
        const size_t initial_signal_count = signals.size();
        std::unordered_map<QueueId, uint32_t> signals_per_queue;
        for (const SignalInfo &signal : signals) {
            ++signals_per_queue[signal.first_scope.queue];
        }
        const bool filter_queue = queue != kQueueIdInvalid;
        for (auto it = signals.begin(); it != signals.end();) {
            if (filter_queue && it->first_scope.queue != queue) {
                ++it;
                continue;
            }
            auto &counter = signals_per_queue[it->first_scope.queue];
            if (counter > signals_per_queue_limit) {
                it = signals.erase(it);
                --counter;
            } else {
                ++it;
            }
        }
        stats.RemoveTimelineSignals(uint32_t(initial_signal_count - signals.size()));
    }
}

void SyncValidator::ApplySignalsUpdate(SignalsUpdate &update, const QueueBatchContext::Ptr &last_batch) {
    // NOTE: All conserved QueueBatchContexts need to have their access logs reset to use the global
    // logger and the only conserved QBCs are those referenced by unwaited signals and the last batch.

    for (auto &signal_entry : update.binary_signal_requests) {
        auto &signal_batch = signal_entry.second.batch;
        // Batches retained for signalled semaphore don't need to retain
        // event data, unless it's the last batch in the submit
        if (signal_batch != last_batch) {
            signal_batch->ResetEventsContext();
            // Make sure that retained batches are minimal, and trim
            // after the events contexts has been cleared.
            signal_batch->Trim();
        }
        const VkSemaphore semaphore = signal_entry.first;
        SignalInfo &signal_info = signal_entry.second;
        binary_signals_.insert_or_assign(semaphore, std::move(signal_info));
    }
    for (VkSemaphore semaphore : update.binary_unsignal_requests) {
        binary_signals_.erase(semaphore);
    }
    for (auto &[semaphore, new_signals] : update.timeline_signals) {
        std::vector<SignalInfo> &signals = timeline_signals_[semaphore];
        vvl::Append(signals, new_signals);
        stats.AddTimelineSignals((uint32_t)new_signals.size());

        // Update host sync points
        std::deque<TimelineHostSyncPoint> &host_sync_points = host_waitable_semaphores_[semaphore];
        for (SignalInfo &new_signal : new_signals) {
            if (new_signal.batch) {
                // The lifetimes of the semaphore host sync points are managed by vkWaitSemaphores.
                // kMaxTimelineHostSyncPoints limit is used when the program does not use vkWaitSemaphores.
                // We accumulate up to kMaxTimelineHostSyncPoints of the host sync points per semaphore.
                // Dropping old sync points cannot introduce false positives but may miss a sync hazard.
                // The limit is chosen to be large enough comparing to typical numbers of queue submissions
                // between host synchronization points.
                const uint32_t kMaxTimelineHostSyncPoints = 256;  // max ~6 Kb per semaphore
                if (host_sync_points.size() >= kMaxTimelineHostSyncPoints) {
                    host_sync_points.pop_front();
                }
                // Add a host sync point for this signal
                TimelineHostSyncPoint sync_point;
                assert(new_signal.first_scope.queue != kQueueIdInvalid);
                sync_point.queue_id = new_signal.first_scope.queue;
                sync_point.tag = new_signal.batch->GetTagRange().end - 1;
                sync_point.timeline_value = new_signal.timeline_value;
                host_sync_points.emplace_back(sync_point);
            }
        }
    }
    for (const auto &remove_signals_request : update.remove_timeline_signals_requests) {
        auto &signals = timeline_signals_[remove_signals_request.semaphore];
        for (auto it = signals.begin(); it != signals.end();) {
            const SignalInfo &signal = *it;
            if (signal.first_scope.queue == remove_signals_request.queue &&
                signal.timeline_value < remove_signals_request.signal_threshold_value) {
                it = signals.erase(it);
                stats.RemoveTimelineSignals(1);
                continue;
            }
            ++it;
        }
    }

    // Enforce max signals limit in case timeline is signaled multiple times and never/rarely is waited on.
    // This does not introduce errors/false-positives (check EnsureTimelineSignalsLimit documentation)
    const uint32_t kMaxTimelineSignalsPerQueue = 100;
    EnsureTimelineSignalsLimit(kMaxTimelineSignalsPerQueue);
}

void SyncValidator::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    auto tagged_wait_op = [queue_id, tag](const QueueBatchContext::Ptr &batch) {
        batch->ApplyTaggedWait(queue_id, tag);
        batch->Trim();

        // If there is a *pending* last batch then apply tagged wait for its accesses too.
        // A pending last batch might exist if this wait was initiated between QueueSubmit's
        // Validate and Record phases (from a different thread). The pending last batch might
        // contain *imported* accesses that are in the scope of this wait.
        auto batch_queue_state = batch->GetQueueSyncState();
        auto pending_batch = batch_queue_state ? batch_queue_state->PendingLastBatch() : nullptr;
        if (pending_batch) {
            pending_batch->ApplyTaggedWait(queue_id, tag);
            pending_batch->Trim();
        }
    };
    ForAllQueueBatchContexts(tagged_wait_op);
}

void SyncValidator::ApplyAcquireWait(const AcquiredImage &acquired) {
    auto acq_wait_op = [&acquired](const QueueBatchContext::Ptr &batch) {
        batch->ApplyAcquireWait(acquired);
        batch->Trim();
    };
    ForAllQueueBatchContexts(acq_wait_op);
}

template <typename BatchOp>
void SyncValidator::ForAllQueueBatchContexts(BatchOp &&op) {
    // Get last batch from each queue
    std::vector<QueueBatchContext::Ptr> batch_contexts = GetLastBatches([](auto) { return true; });

    // Get batches from binary signals
    for (auto &[_, signal] : binary_signals_) {
        if (!vvl::Contains(batch_contexts, signal.batch)) {
            batch_contexts.emplace_back(signal.batch);
        }
    }
    // Get batches from timeline signals
    for (auto &[_, signals] : timeline_signals_) {
        for (const auto &signal : signals) {
            if (signal.batch && !vvl::Contains(batch_contexts, signal.batch)) {
                batch_contexts.emplace_back(signal.batch);
            }
        }
    }
    // Get present batches
    ForEachShared<vvl::Swapchain>([&batch_contexts](const std::shared_ptr<vvl::Swapchain> &swapchain) {
        auto sync_swapchain = std::static_pointer_cast<syncval_state::Swapchain>(swapchain);
        sync_swapchain->GetPresentBatches(batch_contexts);
    });

    // Note: The const is to force the reference to const be on all platforms.
    //
    // It's not obivious (nor cross platform consitent), that the batch reference should be const
    // but since it's pointing to the actual *key* for the set it must be. This doesn't make the
    // object the shared pointer is referencing constant however.
    for (const auto &batch : batch_contexts) {
        op(batch);
    }
}

void SyncValidator::UpdateFenceHostSyncPoint(VkFence fence, FenceHostSyncPoint &&sync_point) {
    std::shared_ptr<const vvl::Fence> fence_state = Get<vvl::Fence>(fence);
    if (!vvl::StateObject::Invalid(fence_state)) {
        waitable_fences_[fence_state->VkHandle()] = std::move(sync_point);
    }
}

void SyncValidator::WaitForFence(VkFence fence) {
    auto fence_it = waitable_fences_.find(fence);
    if (fence_it != waitable_fences_.end()) {
        // The fence may no longer be waitable for several valid reasons.
        FenceHostSyncPoint &wait_for = fence_it->second;
        if (wait_for.acquired.Invalid()) {
            // This is just a normal fence wait
            ApplyTaggedWait(wait_for.queue_id, wait_for.tag);
        } else {
            // This a fence wait for a present operation
            ApplyAcquireWait(wait_for.acquired);
        }
        waitable_fences_.erase(fence_it);
    }
}

void SyncValidator::WaitForSemaphore(VkSemaphore semaphore, uint64_t value) {
    std::deque<TimelineHostSyncPoint> *sync_points = vvl::Find(host_waitable_semaphores_, semaphore);
    if (!sync_points) {
        return;
    }
    auto matching_sync_point = [value](const TimelineHostSyncPoint &sync_point) { return sync_point.timeline_value >= value; };
    auto sync_point_it = std::find_if(sync_points->begin(), sync_points->end(), matching_sync_point);
    if (sync_point_it == sync_points->end()) {
        return;
    }

    const TimelineHostSyncPoint &sync_point = *sync_point_it;
    ApplyTaggedWait(sync_point.queue_id, sync_point.tag);

    // Remove signals before the resolving one (keep the resolving signal).
    std::vector<SignalInfo> &signals = timeline_signals_[semaphore];
    const size_t initial_signal_count = signals.size();
    vvl::erase_if(signals, [&sync_point](SignalInfo &signal) {
        return signal.first_scope.queue == sync_point.queue_id && signal.timeline_value < sync_point.timeline_value;
    });
    stats.RemoveTimelineSignals(uint32_t(initial_signal_count - signals.size()));

    // We can remove all sync points that are in the scope of current wait.
    // Subsequent attempts to synchronize on the host with already synchronized
    // timeline values will result in noop.
    sync_points->erase(sync_points->begin(), sync_point_it + 1 /* include resolving sync point too*/);
}

void SyncValidator::UpdateSyncImageMemoryBindState(uint32_t count, const VkBindImageMemoryInfo *infos) {
    for (const auto &info : vvl::make_span(infos, count)) {
        if (VK_NULL_HANDLE == info.image) continue;
        auto image_state = Get<ImageState>(info.image);

        // Need to protect if some VkBindMemoryStatus are not VK_SUCCESS
        if (!image_state->HasBeenBound()) continue;

        if (image_state->IsTiled()) {
            image_state->SetOpaqueBaseAddress(*this);
        }
    }
}

std::shared_ptr<const QueueSyncState> SyncValidator::GetQueueSyncStateShared(VkQueue queue) const {
    for (const auto &queue_sync_state : queue_sync_states_) {
        if (queue_sync_state->GetQueueState()->VkHandle() == queue) {
            return queue_sync_state;
        }
    }
    return {};
}

std::shared_ptr<vvl::CommandBuffer> SyncValidator::CreateCmdBufferState(VkCommandBuffer handle,
                                                                        const VkCommandBufferAllocateInfo *allocate_info,
                                                                        const vvl::CommandPool *cmd_pool) {
    auto cb_state = std::make_shared<syncval_state::CommandBuffer>(*this, handle, allocate_info, cmd_pool);
    if (cb_state) {
        cb_state->access_context.SetSelfReference();
    }
    return std::static_pointer_cast<vvl::CommandBuffer>(cb_state);
}

std::shared_ptr<vvl::Swapchain> SyncValidator::CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info,
                                                                    VkSwapchainKHR handle) {
    return std::static_pointer_cast<vvl::Swapchain>(std::make_shared<syncval_state::Swapchain>(*this, create_info, handle));
}

std::shared_ptr<vvl::Image> SyncValidator::CreateImageState(VkImage handle, const VkImageCreateInfo *create_info,
                                                            VkFormatFeatureFlags2 features) {
    return std::make_shared<ImageState>(*this, handle, create_info, features);
}

std::shared_ptr<vvl::Image> SyncValidator::CreateImageState(VkImage handle, const VkImageCreateInfo *create_info,
                                                            VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                            VkFormatFeatureFlags2 features) {
    return std::make_shared<ImageState>(*this, handle, create_info, swapchain, swapchain_index, features);
}
std::shared_ptr<vvl::ImageView> SyncValidator::CreateImageViewState(
    const std::shared_ptr<vvl::Image> &image_state, VkImageView handle, const VkImageViewCreateInfo *create_info,
    VkFormatFeatureFlags2 format_features, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageViewState>(image_state, handle, create_info, format_features, cubic_props);
}

void SyncValidator::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator,
                                               const RecordObject &record_obj) {
    if (const auto buffer_state = Get<vvl::Buffer>(buffer)) {
        const VkDeviceSize base_address = ResourceBaseAddress(*buffer_state);
        const ResourceAccessRange buffer_range(base_address, base_address + buffer_state->create_info.size);
        auto batch_op = [&buffer_range](const QueueBatchContext::Ptr &batch) {
            batch->OnResourceDestroyed(buffer_range);
            batch->Trim();
        };
        ForAllQueueBatchContexts(batch_op);
    }
    BaseClass::PreCallRecordDestroyBuffer(device, buffer, pAllocator, record_obj);
}

void SyncValidator::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator,
                                              const RecordObject &record_obj) {
    if (const auto image_state = Get<syncval_state::ImageState>(image)) {
        auto batch_op = [&image_state](const QueueBatchContext::Ptr &batch) {
            ImageRangeGen range_gen = image_state->MakeImageRangeGen(image_state->full_range, false);
            for (; range_gen->non_empty(); ++range_gen) {
                const ResourceAccessRange subresource_range = *range_gen;
                batch->OnResourceDestroyed(subresource_range);
            }
            batch->Trim();
        };
        ForAllQueueBatchContexts(batch_op);
    }
    BaseClass::PreCallRecordDestroyImage(device, image, pAllocator, record_obj);
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcBuffer);
                const auto error = error_messages_.BufferRegionError(hazard, srcBuffer, region_index, src_range, *cb_context,
                                                                     error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstBuffer);
                const auto error = error_messages_.BufferRegionError(hazard, dstBuffer, region_index, dst_range, *cb_context,
                                                                     error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto src_tag_ex = src_buffer ? cb_context->AddCommandHandle(tag, src_buffer->Handle()) : ResourceUsageTagEx{tag};

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);
    auto dst_tag_ex = dst_buffer ? cb_context->AddCommandHandle(tag, dst_buffer->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, src_tag_ex);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<vvl::Buffer>(pCopyBufferInfo->srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(pCopyBufferInfo->dstBuffer);

    for (const auto [region_index, copy_region] : vvl::enumerate(pCopyBufferInfo->pRegions, pCopyBufferInfo->regionCount)) {
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                // TODO -- add tag information to log msg when useful.
                // TODO: there are no tests for this error
                const LogObjectList objlist(commandBuffer, pCopyBufferInfo->srcBuffer);
                const auto error = error_messages_.BufferRegionError(hazard, pCopyBufferInfo->srcBuffer, region_index, src_range,
                                                                     *cb_context, error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                // TODO: there are no tests for this error
                const LogObjectList objlist(commandBuffer, pCopyBufferInfo->dstBuffer);
                const auto error = error_messages_.BufferRegionError(hazard, pCopyBufferInfo->dstBuffer, region_index, dst_range,
                                                                     *cb_context, error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, error_obj);
}

void SyncValidator::RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(command);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<vvl::Buffer>(pCopyBufferInfo->srcBuffer);
    auto src_tag_ex = src_buffer ? cb_context->AddCommandHandle(tag, src_buffer->Handle()) : ResourceUsageTagEx{tag};

    auto dst_buffer = Get<vvl::Buffer>(pCopyBufferInfo->dstBuffer);
    auto dst_tag_ex = dst_buffer ? cb_context->AddCommandHandle(tag, dst_buffer->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < pCopyBufferInfo->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfo->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, src_tag_ex);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                   const RecordObject &record_obj) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                const RecordObject &record_obj) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj.location.function);
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const auto error = error_messages_.ImageRegionError(hazard, srcImage, true, region, *cb_access_context,
                                                                    error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const auto error = error_messages_.ImageRegionError(hazard, dstImage, false, region, *cb_access_context,
                                                                    error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto src_tag_ex = src_image ? cb_access_context->AddCommandHandle(tag, src_image->Handle()) : ResourceUsageTagEx{tag};

    auto dst_image = Get<ImageState>(dstImage);
    auto dst_tag_ex = dst_image ? cb_access_context->AddCommandHandle(tag, dst_image->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent,
                                       src_tag_ex);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent,
                                       dst_tag_ex);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(pCopyImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pCopyImageInfo->srcImage);
                const auto error = error_messages_.ImageRegionError(hazard, pCopyImageInfo->srcImage, true, region,
                                                                    *cb_access_context, error_obj.location.function);
                // TODO: this error not covered by the test
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pCopyImageInfo->dstImage);
                const auto error = error_messages_.ImageRegionError(hazard, pCopyImageInfo->dstImage, false, region,
                                                                    *cb_access_context, error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, error_obj);
}

void SyncValidator::RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pCopyImageInfo->srcImage);
    auto src_tag_ex = src_image ? cb_access_context->AddCommandHandle(tag, src_image->Handle()) : ResourceUsageTagEx{tag};

    auto dst_image = Get<ImageState>(pCopyImageInfo->dstImage);
    auto dst_tag_ex = dst_image ? cb_access_context->AddCommandHandle(tag, dst_image->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent,
                                       src_tag_ex);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent,
                                       dst_tag_ex);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                  const RecordObject &record_obj) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                               const RecordObject &record_obj) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj.location.function);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(), srcStageMask,
                                           dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                           pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    skip |= pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(
        record_obj.location.function, *this, cb_access_context->GetQueueFlags(), srcStageMask, dstStageMask, memoryBarrierCount,
        pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                           *pDependencyInfo);
    skip |= pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                        const RecordObject &record_obj) {
    PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                     const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // The state tracker sets up the device state
    BaseClass::PostCreateDevice(pCreateInfo, loc);

    // Returns queues in the same order as advertised by the driver.
    // This allows to have deterministic QueueId between runs that simplifies debugging.
    auto get_sorted_queues = [this]() {
        std::vector<std::shared_ptr<vvl::Queue>> queues;
        ForEachShared<vvl::Queue>([&queues](const std::shared_ptr<vvl::Queue> &queue) { queues.emplace_back(queue); });
        std::sort(queues.begin(), queues.end(), [](const auto &q1, const auto &q2) {
            return (q1->queue_family_index < q2->queue_family_index) ||
                   (q1->queue_family_index == q2->queue_family_index && q1->queue_index < q2->queue_index);
        });
        return queues;
    };
    queue_sync_states_.reserve(Count<vvl::Queue>());
    for (const auto &queue : get_sorted_queues()) {
        queue_sync_states_.emplace_back(std::make_shared<QueueSyncState>(queue, queue_id_limit_++));
    }

    const auto env_debug_command_number = GetEnvironment("VK_SYNCVAL_DEBUG_COMMAND_NUMBER");
    if (!env_debug_command_number.empty()) {
        debug_command_number = static_cast<uint32_t>(std::stoul(env_debug_command_number));
    }
    const auto env_debug_reset_count = GetEnvironment("VK_SYNCVAL_DEBUG_RESET_COUNT");
    if (!env_debug_reset_count.empty()) {
        debug_reset_count = static_cast<uint32_t>(std::stoul(env_debug_reset_count));
    }
    debug_cmdbuf_pattern = GetEnvironment("VK_SYNCVAL_DEBUG_CMDBUF_PATTERN");
    text::ToLower(debug_cmdbuf_pattern);
}

void SyncValidator::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                  const RecordObject &record_obj) {
    BaseClass::PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, record_obj);
    if (record_obj.result != VK_SUCCESS) return;
    assert(!vvl::Contains(timeline_signals_, *pSemaphore));
}

void SyncValidator::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator,
                                                  const RecordObject &record_obj) {
    if (auto sem_state = Get<vvl::Semaphore>(semaphore); sem_state && (sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE)) {
        if (auto it = timeline_signals_.find(semaphore); it != timeline_signals_.end()) {
            stats.RemoveTimelineSignals((uint32_t)it->second.size());
            timeline_signals_.erase(it);
        }
    }
    BaseClass::PreCallRecordDestroySemaphore(device, semaphore, pAllocator, record_obj);
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    if (cb_state) {
        SyncOpBeginRenderPass sync_op(error_obj.location.function, *this, pRenderPassBegin, pSubpassBeginInfo);
        skip |= sync_op.Validate(cb_state->access_context);
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents, const ErrorObject &error_obj) const {
    bool skip = BaseClass::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, error_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = BaseClass::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

void SyncValidator::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                     const RecordObject &record_obj) {
    // The state tracker sets up the command buffer state
    BaseClass::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);

    // Create/initialize the structure that trackers accesses at the command buffer scope.
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    cb_state->access_context.Reset();
}

void SyncValidator::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    if (cb_state) {
        if (!cb_state->IsPrimary()) {
            return;  // [core validation check]: only primary command buffer can begin render pass
        }
        cb_state->access_context.RecordSyncOp<SyncOpBeginRenderPass>(command, *this, pRenderPassBegin, pSubpassBeginInfo);
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, record_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                         const RecordObject &record_obj) {
    PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    SyncOpNextSubpass sync_op(error_obj.location.function, *this, pSubpassBeginInfo, pSubpassEndInfo);
    return sync_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                  const ErrorObject &error_obj) const {
    // Convert to a NextSubpass2
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper();
    return ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, &subpass_end_info, error_obj);
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                      const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    if (!cb_state->IsPrimary()) {
        return;  // [core validation check]: only primary command buffer can start next subpass
    }
    cb_context->RecordSyncOp<SyncOpNextSubpass>(command, *this, pSubpassBeginInfo, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                 const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    RecordCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    auto *cb_context = &cb_state->access_context;

    SyncOpEndRenderPass sync_op(error_obj.location.function, *this, pSubpassEndInfo);
    skip |= sync_op.Validate(*cb_context);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return ValidateCmdEndRenderPass(commandBuffer, nullptr, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                     const ErrorObject &error_obj) const {
    return ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, Func command) {
    // Resolve the all subpass contexts to the command buffer contexts
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    if (!cb_state->IsPrimary()) {
        return;  // [core validation check]: only primary command buffer can end render pass
    }
    cb_context->RecordSyncOp<SyncOpEndRenderPass>(command, *this, pSubpassEndInfo);
}

// Simple heuristic rule to detect WAW operations representing algorithmically safe or increment
// updates to a resource which do not conflict at the byte level.
// TODO: Revisit this rule to see if it needs to be tighter or looser
// TODO: Add programatic control over suppression heuristics
bool SyncValidator::SupressedBoundDescriptorWAW(const HazardResult &hazard) const {
    assert(hazard.IsHazard());
    return hazard.IsWAWHazard();
}

void SyncValidator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, nullptr, record_obj.location.function);
    BaseClass::PostCallRecordCmdEndRenderPass(commandBuffer, record_obj);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                    const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, record_obj.location.function);
    BaseClass::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                       const RecordObject &record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state || !pRenderingInfo) return skip;

    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state(&skip, std::move(cb_state));
    cmd_state->AddRenderingInfo(*this, *pRenderingInfo);

    // We need to set skip, because the TlsGuard destructor is looking at the skip value for RAII cleanup.
    skip |= cmd_state->cb_state->access_context.ValidateBeginRendering(error_obj, *cmd_state);
    return skip;
}

void SyncValidator::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                      const RecordObject &record_obj) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                   const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state;

    assert(cmd_state && cmd_state->cb_state && (cmd_state->cb_state->VkHandle() == commandBuffer));
    // Note: for fine grain locking need to to something other than cast.
    auto cb_state = std::const_pointer_cast<syncval_state::CommandBuffer>(cmd_state->cb_state);
    cb_state->access_context.RecordBeginRendering(*cmd_state, record_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip |= cb_state->access_context.ValidateEndRendering(error_obj);
    return skip;
}

void SyncValidator::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void SyncValidator::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdEndRendering(commandBuffer, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;

    cb_state->access_context.RecordEndRendering(record_obj);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_image = Get<ImageState>(dstImage);

    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        HazardResult hazard;
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range =
                    MakeRange(copy_region.bufferOffset, vvl::GetBufferSizeFromCopyImage(copy_region, dst_image->create_info.format,
                                                                                        dst_image->create_info.arrayLayers));
                hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
                if (hazard.IsHazard()) {
                    // PHASE1 TODO -- add tag information to log msg when useful.
                    const LogObjectList objlist(commandBuffer, srcBuffer);
                    const auto error = error_messages_.BufferRegionError(hazard, srcBuffer, region_index, src_range,
                                                                         *cb_access_context, loc.function);
                    skip |= SyncError(hazard.Hazard(), objlist, loc, error);
                }
            }

            hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                           copy_region.imageExtent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const auto error =
                    error_messages_.ImageRegionError(hazard, dstImage, false, region_index, *cb_access_context, loc.function);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
            if (skip) break;
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        error_obj.location);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                         const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, error_obj.location.dot(Field::pCopyBufferToImageInfo));
}

template <typename RegionType>
void SyncValidator::RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                               Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto src_tag_ex = src_buffer ? cb_access_context->AddCommandHandle(tag, src_buffer->Handle()) : ResourceUsageTagEx{tag};

    auto dst_image = Get<ImageState>(dstImage);
    auto dst_tag_ex = dst_image ? cb_access_context->AddCommandHandle(tag, dst_image->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range =
                    MakeRange(copy_region.bufferOffset, vvl::GetBufferSizeFromCopyImage(copy_region, dst_image->create_info.format,
                                                                                        dst_image->create_info.arrayLayers));
                context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range,
                                           src_tag_ex);
            }
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                       copy_region.imageExtent, dst_tag_ex);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                                    record_obj);
    RecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                               record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                          const RecordObject &record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                       const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, record_obj.location.function);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);
    const VkDeviceMemory dst_memory = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemoryState()->VkHandle() : VK_NULL_HANDLE;
    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                                copy_region.imageExtent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const auto error =
                    error_messages_.ImageRegionError(hazard, srcImage, true, region_index, *cb_access_context, loc.function);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
            if (dst_memory != VK_NULL_HANDLE) {
                ResourceAccessRange dst_range =
                    MakeRange(copy_region.bufferOffset, vvl::GetBufferSizeFromCopyImage(copy_region, src_image->create_info.format,
                                                                                        src_image->create_info.arrayLayers));
                hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
                if (hazard.IsHazard()) {
                    const LogObjectList objlist(commandBuffer, dstBuffer);
                    const auto error = error_messages_.BufferRegionError(hazard, dstBuffer, region_index, dst_range,
                                                                         *cb_access_context, loc.function);
                    skip |= SyncError(hazard.Hazard(), objlist, loc, error);
                }
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        error_obj.location);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                         const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, error_obj.location.dot(Field::pCopyImageToBufferInfo));
}

template <typename RegionType>
void SyncValidator::RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto src_tag_ex = src_image ? cb_access_context->AddCommandHandle(tag, src_image->Handle()) : ResourceUsageTagEx{tag};

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);
    auto dst_tag_ex = dst_buffer ? cb_access_context->AddCommandHandle(tag, dst_buffer->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                       copy_region.imageExtent, src_tag_ex);
            if (dst_buffer) {
                ResourceAccessRange dst_range =
                    MakeRange(copy_region.bufferOffset, vvl::GetBufferSizeFromCopyImage(copy_region, src_image->create_info.format,
                                                                                        src_image->create_info.arrayLayers));
                context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range,
                                           dst_tag_ex);
            }
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                      const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                                    record_obj);
    RecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                               record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                          const RecordObject &record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                       const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, record_obj.location.function);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(blit_region.srcSubresource), offset, extent, false,
                                                SYNC_BLIT_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const auto error =
                    error_messages_.ImageRegionError(hazard, srcImage, true, region, *cb_access_context, loc.function);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
        }

        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(blit_region.dstSubresource), offset, extent, false,
                                                SYNC_BLIT_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const auto error =
                    error_messages_.ImageRegionError(hazard, dstImage, false, region, *cb_access_context, loc.function);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                error_obj.location);
}

bool SyncValidator::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, error_obj.location.dot(Field::pBlitImageInfo));
}

template <typename RegionType>
void SyncValidator::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const RegionType *pRegions, VkFilter filter, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    const auto tag = cb_state->access_context.NextCommandTag(command);
    auto *context = cb_state->access_context.GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto src_tag_ex = src_image ? cb_state->access_context.AddCommandHandle(tag, src_image->Handle()) : ResourceUsageTagEx{tag};

    auto dst_image = Get<ImageState>(dstImage);
    auto dst_tag_ex = dst_image ? cb_state->access_context.AddCommandHandle(tag, dst_image->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            context->UpdateAccessState(*src_image, SYNC_BLIT_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(blit_region.srcSubresource), offset, extent, src_tag_ex);
        }
        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            context->UpdateAccessState(*dst_image, SYNC_BLIT_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(blit_region.dstSubresource), offset, extent, dst_tag_ex);
        }
    }
}

void SyncValidator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit *pRegions, VkFilter filter, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter, record_obj);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                       record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                  const RecordObject &record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                               const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter, record_obj.location.function);
}

bool SyncValidator::ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                           const VkDeviceSize struct_size, const VkBuffer buffer, const VkDeviceSize offset,
                                           const uint32_t drawCount, const uint32_t stride, const Location &loc) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    auto buf_state = Get<vvl::Buffer>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_context.GetCBState().Handle(), buf_state->Handle());
            const auto error = error_messages_.BufferError(hazard, buffer, "indirect", cb_context, loc.function);
            skip |= SyncError(hazard.Hazard(), objlist, loc, error);
        }
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(cb_context.GetCBState().Handle(), buf_state->Handle());
                const auto error = error_messages_.BufferError(hazard, buffer, "indirect", cb_context, loc.function);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
                break;
            }
        }
    }
    return skip;
}

void SyncValidator::RecordIndirectBuffer(CommandBufferAccessContext &cb_context, const ResourceUsageTag tag,
                                         const VkDeviceSize struct_size, const VkBuffer buffer, const VkDeviceSize offset,
                                         const uint32_t drawCount, uint32_t stride) {
    auto buf_state = Get<vvl::Buffer>(buffer);
    auto tag_ex = buf_state ? cb_context.AddCommandHandle(tag, buf_state->Handle()) : ResourceUsageTagEx{tag};

    VkDeviceSize size = struct_size;
    AccessContext &context = *cb_context.GetCurrentAccessContext();
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                                  tag_ex);
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                                      tag_ex);
        }
    }
}

bool SyncValidator::ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context, VkBuffer buffer,
                                        VkDeviceSize offset, const Location &loc) const {
    bool skip = false;

    auto count_buf_state = Get<vvl::Buffer>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    auto hazard = context.DetectHazard(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
    if (hazard.IsHazard()) {
        const LogObjectList objlist(cb_context.GetCBState().Handle(), count_buf_state->Handle());
        const auto error = error_messages_.BufferError(hazard, buffer, "countBuffer", cb_context, loc.function);
        skip |= SyncError(hazard.Hazard(), objlist, loc, error);
    }
    return skip;
}

void SyncValidator::RecordCountBuffer(CommandBufferAccessContext &cb_context, const ResourceUsageTag tag, VkBuffer buffer,
                                      VkDeviceSize offset) {
    auto count_buf_state = Get<vvl::Buffer>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, count_buf_state->Handle());
    AccessContext &context = *cb_context.GetCurrentAccessContext();
    context.UpdateAccessState(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                              tag_ex);
}

bool SyncValidator::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip |= cb_state->access_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                             const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatch(commandBuffer, x, y, z, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
}

bool SyncValidator::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    const auto *context = cb_state->access_context.GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_state->access_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    skip |= ValidateIndirectBuffer(cb_state->access_context, *context, sizeof(VkDispatchIndirectCommand), buffer, offset, 1,
                                   sizeof(VkDispatchIndirectCommand), error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDispatchIndirectCommand), buffer, offset, 1,
                         sizeof(VkDispatchIndirectCommand));
}

bool SyncValidator::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertex(vertexCount, firstVertex, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertex(vertexCount, firstVertex, tag);
    cb_access_context->RecordDrawAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertexIndex(indexCount, firstIndex, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                              record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertexIndex(indexCount, firstIndex, tag);
    cb_access_context->RecordDrawAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride,
                                   error_obj.location);
    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    if (drawCount == 0) return;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertex(?, ?, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount,
                                   stride, error_obj.location);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertexIndex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertexIndex(?, ?, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndirectCommand), buffer, offset, maxDrawCount,
                                   stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, countBuffer, countBufferOffset, error_obj.location);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*cb_access_context, tag, countBuffer, countBufferOffset);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertex(?, ?, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride, record_obj);
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               record_obj.location.function);
}
bool SyncValidator::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, maxDrawCount,
                                   stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, countBuffer, countBufferOffset, error_obj.location);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertexIndex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*cb_access_context, tag, countBuffer, countBufferOffset);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertexIndex(?, ?, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride, record_obj);
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj.location.function);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

bool SyncValidator::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, image);
                const auto error = error_messages_.ImageSubresourceRangeError(hazard, image, index, *cb_access_context,
                                                                              error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue *pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<ImageState>(image);
    if (image_state) {
        cb_access_context->AddCommandHandle(tag, image_state->Handle());
    }

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange *pRanges,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, image);
                const auto error = error_messages_.ImageSubresourceRangeError(hazard, image, index, *cb_access_context,
                                                                              error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges,
                                                         record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<ImageState>(image);
    if (image_state) {
        cb_access_context->AddCommandHandle(tag, image_state->Handle());
    }

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                       const VkClearRect *pRects, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    for (const auto [attachment_index, attachment] : vvl::enumerate(pAttachments, attachmentCount)) {
        Location attachment_loc = error_obj.location.dot(Field::pAttachments, attachment_index);
        for (const auto [rect_index, rect] : vvl::enumerate(pRects, rectCount)) {
            Location rect_loc = attachment_loc.dot(Field::pRects, rect_index);
            skip |= cb_state->access_context.ValidateClearAttachment(rect_loc, attachment, rect);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                     const VkClearRect *pRects, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    auto cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    for (const auto &attachment : vvl::make_span(pAttachments, attachmentCount)) {
        for (const auto &rect : vvl::make_span(pRects, rectCount)) {
            cb_access_context->RecordClearAttachment(tag, attachment, rect);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, queryPool, dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, dstBuffer, "dstBuffer", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }

    // TODO:Track VkQueryPool
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags,
                                                         const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset,
                                                       stride, flags, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }

    // TODO:Track VkQueryPool
}

bool SyncValidator::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_CLEAR_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, dstBuffer, "dstBuffer", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const auto error = error_messages_.ImageRegionError(hazard, srcImage, true, region, *cb_access_context,
                                                                    error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const auto error = error_messages_.ImageRegionError(hazard, dstImage, false, region, *cb_access_context,
                                                                    error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve *pRegions, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                               pRegions, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto src_tag_ex = src_image ? cb_access_context->AddCommandHandle(tag, src_image->Handle()) : ResourceUsageTagEx{tag};

    auto dst_image = Get<ImageState>(dstImage);
    auto dst_tag_ex = dst_image ? cb_access_context->AddCommandHandle(tag, dst_image->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset,
                                       resolve_region.extent, src_tag_ex);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                       resolve_region.extent, dst_tag_ex);
        }
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const Location image_info_loc = error_obj.location.dot(Field::pResolveImageInfo);
    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const Location region_loc = image_info_loc.dot(Field::pRegions, region);
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pResolveImageInfo->srcImage);
                const auto error = error_messages_.ImageRegionError(hazard, pResolveImageInfo->srcImage, true, region,
                                                                    *cb_access_context, error_obj.location.function);
                // TODO: this error is not covered by the test
                skip |= SyncError(hazard.Hazard(), objlist, region_loc, error);
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pResolveImageInfo->dstImage);
                const auto error = error_messages_.ImageRegionError(hazard, pResolveImageInfo->dstImage, false, region,
                                                                    *cb_access_context, error_obj.location.function);
                // TODO: this error is not covered by the test
                skip |= SyncError(hazard.Hazard(), objlist, region_loc, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, error_obj);
}

void SyncValidator::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                                  const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto src_tag_ex = src_image ? cb_access_context->AddCommandHandle(tag, src_image->Handle()) : ResourceUsageTagEx{tag};

    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);
    auto dst_tag_ex = dst_image ? cb_access_context->AddCommandHandle(tag, dst_image->Handle()) : ResourceUsageTagEx{tag};

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset,
                                       resolve_region.extent, src_tag_ex);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                       resolve_region.extent, dst_tag_ex);
        }
    }
}

void SyncValidator::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                     const RecordObject &record_obj) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

bool SyncValidator::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_CLEAR_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, dstBuffer, "dstBuffer", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void *pData, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                           VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const auto error =
                error_messages_.BufferError(hazard, dstBuffer, "dstBuffer", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), dstBuffer, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                         const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return skip;

    const Location decode_info_loc = error_obj.location.dot(Field::pDecodeInfo);

    auto src_buffer = Get<vvl::Buffer>(pDecodeInfo->srcBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange);
        auto hazard = context->DetectHazard(*src_buffer, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, src_range);
        if (hazard.IsHazard()) {
            // TODO: there are no tests for this error
            const auto error = error_messages_.BufferError(hazard, pDecodeInfo->srcBuffer, "bitstream buffer", *cb_access_context,
                                                           error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), src_buffer->Handle(), decode_info_loc.dot(Field::srcBuffer), error);
        }
    }

    auto dst_resource = vvl::VideoPictureResource(*this, pDecodeInfo->dstPictureResource);
    if (dst_resource) {
        auto hazard = context->DetectHazard(*vs_state, dst_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE);
        if (hazard.IsHazard()) {
            const auto error =
                error_messages_.Error(hazard, "decode output picture", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), dst_resource.image_view_state->Handle(),
                              decode_info_loc.dot(Field::dstPictureResource), error);
        }
    }

    if (pDecodeInfo->pSetupReferenceSlot != nullptr && pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(*this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource && (setup_resource != dst_resource)) {
            auto hazard = context->DetectHazard(*vs_state, setup_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE);
            if (hazard.IsHazard()) {
                // TODO: there are no tests for this error
                const auto error =
                    error_messages_.Error(hazard, "reconstructed picture", *cb_access_context, error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), setup_resource.image_view_state->Handle(),
                                  decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource), error);
            }
        }
    }

    for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
        if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(*this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                auto hazard = context->DetectHazard(*vs_state, reference_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ);
                if (hazard.IsHazard()) {
                    const auto error =
                        error_messages_.VideoReferencePictureError(hazard, i, *cb_access_context, error_obj.location.function);
                    skip |= SyncError(hazard.Hazard(), reference_resource.image_view_state->Handle(),
                                      decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource), error);
                }
            }
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                   const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    auto src_buffer = Get<vvl::Buffer>(pDecodeInfo->srcBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange);
        const ResourceUsageTagEx src_tag_ex = cb_access_context->AddCommandHandle(tag, src_buffer->Handle());
        context->UpdateAccessState(*src_buffer, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, SyncOrdering::kNonAttachment, src_range,
                                   src_tag_ex);
    }

    auto dst_resource = vvl::VideoPictureResource(*this, pDecodeInfo->dstPictureResource);
    if (dst_resource) {
        context->UpdateAccessState(*vs_state, dst_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE, tag);
    }

    if (pDecodeInfo->pSetupReferenceSlot != nullptr && pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(*this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource && (setup_resource != dst_resource)) {
            context->UpdateAccessState(*vs_state, setup_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE, tag);
        }
    }

    for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
        if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(*this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                context->UpdateAccessState(*vs_state, reference_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, tag);
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return skip;

    const Location encode_info_loc = error_obj.location.dot(Field::pEncodeInfo);

    auto dst_buffer = Get<vvl::Buffer>(pEncodeInfo->dstBuffer);
    if (dst_buffer) {
        const ResourceAccessRange src_range = MakeRange(*dst_buffer, pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, src_range);
        if (hazard.IsHazard()) {
            const auto error = error_messages_.BufferError(hazard, pEncodeInfo->dstBuffer, "bitstream buffer", *cb_access_context,
                                                           error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), dst_buffer->Handle(), encode_info_loc.dot(Field::dstBuffer), error);
        }
    }

    auto src_resource = vvl::VideoPictureResource(*this, pEncodeInfo->srcPictureResource);
    if (src_resource) {
        auto hazard = context->DetectHazard(*vs_state, src_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ);
        if (hazard.IsHazard()) {
            // TODO: there are no tests for this error
            const auto error =
                error_messages_.Error(hazard, "encode input picture", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), src_resource.image_view_state->Handle(),
                              encode_info_loc.dot(Field::srcPictureResource), error);
        }
    }

    if (pEncodeInfo->pSetupReferenceSlot != nullptr && pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(*this, *pEncodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource) {
            auto hazard = context->DetectHazard(*vs_state, setup_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE);
            if (hazard.IsHazard()) {
                const auto error =
                    error_messages_.Error(hazard, "reconstructed picture", *cb_access_context, error_obj.location.function);
                skip |= SyncError(hazard.Hazard(), setup_resource.image_view_state->Handle(),
                                  encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource), error);
            }
        }
    }

    for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
        if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(*this, *pEncodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                auto hazard = context->DetectHazard(*vs_state, reference_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ);
                if (hazard.IsHazard()) {
                    const auto error =
                        error_messages_.VideoReferencePictureError(hazard, i, *cb_access_context, error_obj.location.function);
                    skip |= SyncError(hazard.Hazard(), reference_resource.image_view_state->Handle(),
                                      encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource), error);
                }
            }
        }
    }

    if (pEncodeInfo->flags & (VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR)) {
        auto quantization_map_info = vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapInfoKHR>(pEncodeInfo);
        if (quantization_map_info) {
            auto image_view_state = Get<ImageViewState>(quantization_map_info->quantizationMap);
            if (image_view_state) {
                VkOffset3D offset = {0, 0, 0};
                VkExtent3D extent = {quantization_map_info->quantizationMapExtent.width,
                                     quantization_map_info->quantizationMapExtent.height, 1};
                auto hazard = context->DetectHazard(*image_view_state, offset, extent, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ,
                                                    SyncOrdering::kOrderingNone);
                if (hazard.IsHazard()) {
                    // TODO: there are no tests for this error
                    const auto error =
                        error_messages_.Error(hazard, "quantization map", *cb_access_context, error_obj.location.function);
                    skip |= SyncError(hazard.Hazard(), image_view_state->Handle(),
                                      encode_info_loc.pNext(Struct::VkVideoEncodeQuantizationMapInfoKHR, Field::quantizationMap),
                                      error);
                }
            }
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                   const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    auto src_buffer = Get<vvl::Buffer>(pEncodeInfo->dstBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange);
        const ResourceUsageTagEx src_tag_ex = cb_access_context->AddCommandHandle(tag, src_buffer->Handle());
        context->UpdateAccessState(*src_buffer, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, SyncOrdering::kNonAttachment, src_range,
                                   src_tag_ex);
    }

    auto src_resource = vvl::VideoPictureResource(*this, pEncodeInfo->srcPictureResource);
    if (src_resource) {
        context->UpdateAccessState(*vs_state, src_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, tag);
    }

    if (pEncodeInfo->pSetupReferenceSlot != nullptr && pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(*this, *pEncodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource) {
            context->UpdateAccessState(*vs_state, setup_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, tag);
        }
    }

    for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
        if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(*this, *pEncodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                context->UpdateAccessState(*vs_state, reference_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, tag);
            }
        }
    }

    if (pEncodeInfo->flags & (VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR)) {
        auto quantization_map_info = vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapInfoKHR>(pEncodeInfo);
        if (quantization_map_info) {
            auto image_view_state = Get<ImageViewState>(quantization_map_info->quantizationMap);
            if (image_view_state) {
                VkOffset3D offset = {0, 0, 0};
                VkExtent3D extent = {quantization_map_info->quantizationMapExtent.width,
                                     quantization_map_info->quantizationMapExtent.height, 1};
                context->UpdateAccessState(*image_view_state, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, SyncOrdering::kOrderingNone,
                                           offset, extent, ResourceUsageTagEx{tag});
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask,
                                             cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR *pDependencyInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo *pDependencyInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    if (!pDependencyInfo) return skip;

    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                  const VkDependencyInfoKHR *pDependencyInfo, const RecordObject &record_obj) {
    PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                               const VkDependencyInfo *pDependencyInfo, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    if (!pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event,
                                             *pDependencyInfo, cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

bool SyncValidator::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask, const ErrorObject &error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

void SyncValidator::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                    VkPipelineStageFlags2KHR stageMask, const RecordObject &record_obj) {
    PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void SyncValidator::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                 const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                    pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return wait_events_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                uint32_t bufferMemoryBarrierCount,
                                                const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
                                              pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                              imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                                               bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                               pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                     const VkDependencyInfoKHR *pDependencyInfos,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

void SyncValidator::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                    const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    pDependencyInfos);
    skip |= wait_events_op.Validate(*cb_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, pDependencyInfos);
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            // TODO: there are no tests for this error
            const auto error =
                error_messages_.BufferError(hazard, dstBuffer, "dstBuffer", *cb_access_context, error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), dstBuffer, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                          const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = BaseClass::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, error_obj);
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    // Heavyweight, but we need a proxy copy of the active command buffer access context
    CommandBufferAccessContext proxy_cb_context(*cb_context, CommandBufferAccessContext::AsProxyContext());

    auto &proxy_label_commands = proxy_cb_context.GetProxyLabelCommands();
    proxy_label_commands = cb_state->GetLabelCommands();

    // Make working copies of the access and events contexts
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        if (cb_index == 0) {
            proxy_cb_context.NextCommandTag(error_obj.location.function, ResourceUsageRecord::SubcommandType::kIndex);
        } else {
            proxy_cb_context.NextSubcommandTag(error_obj.location.function, ResourceUsageRecord::SubcommandType::kIndex);
        }

        const auto recorded_cb = Get<syncval_state::CommandBuffer>(pCommandBuffers[cb_index]);
        if (!recorded_cb) continue;
        const auto *recorded_cb_context = &recorded_cb->access_context;
        assert(recorded_cb_context);

        const ResourceUsageTag base_tag = proxy_cb_context.GetTagCount();
        skip |= ReplayState(proxy_cb_context, *recorded_cb_context, error_obj, cb_index, base_tag).ValidateFirstUse();

        // Update proxy label commands so they can be used by ImportRecordedAccessLog
        const auto &recorded_label_commands = recorded_cb->GetLabelCommands();
        proxy_label_commands.insert(proxy_label_commands.end(), recorded_label_commands.begin(), recorded_label_commands.end());

        // The barriers have already been applied in ValidatFirstUse
        proxy_cb_context.ImportRecordedAccessLog(*recorded_cb_context);
        proxy_cb_context.ResolveExecutedCommandBuffer(*recorded_cb_context->GetCurrentAccessContext(), base_tag);
    }
    proxy_label_commands.clear();

    return skip;
}

void SyncValidator::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        if (const auto recorded_cb = Get<syncval_state::CommandBuffer>(pCommandBuffers[cb_index])) {
            const auto subcommand = ResourceUsageRecord::SubcommandType::kIndex;
            if (cb_index == 0) {
                ResourceUsageTag cb_tag = cb_context->NextCommandTag(record_obj.location.function, subcommand);
                cb_context->AddCommandHandle(cb_tag, recorded_cb->Handle(), cb_index);
            } else {
                ResourceUsageTag cb_tag = cb_context->NextSubcommandTag(record_obj.location.function, subcommand);
                cb_context->AddSubcommandHandle(cb_tag, recorded_cb->Handle(), cb_index);
            }
            cb_context->RecordExecutedCommandBuffer(recorded_cb->access_context);
        }
    }
}

void SyncValidator::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                  const RecordObject &record_obj) {
    BaseClass::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, record_obj);
    if (VK_SUCCESS != record_obj.result) return;
    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image;
    bind_info.memory = memory;
    bind_info.memoryOffset = memoryOffset;
    UpdateSyncImageMemoryBindState(1, &bind_info);
}

void SyncValidator::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                   const RecordObject &record_obj) {
    // Don't check |record_obj.result| as some binds might still be valid
    BaseClass::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);

    UpdateSyncImageMemoryBindState(bindInfoCount, pBindInfos);
}

void SyncValidator::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo *pBindInfos, const RecordObject &record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void SyncValidator::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) {
    BaseClass::PostCallRecordQueueWaitIdle(queue, record_obj);
    if (record_obj.result != VK_SUCCESS || !syncval_settings.submit_time_validation || queue == VK_NULL_HANDLE) {
        return;
    }
    const auto queue_state = GetQueueSyncStateShared(queue);
    if (!queue_state) return;  // Invalid queue
    QueueId waited_queue = queue_state->GetQueueId();
    ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex);

    // For each timeline, remove all signals signaled on the waited queue, except the last one.
    // The last signal is needed to represent the current timeline state.
    EnsureTimelineSignalsLimit(1, waited_queue);

    // Eliminate host waitable objects from the current queue.
    vvl::EraseIf(waitable_fences_, [waited_queue](const auto &sf) { return sf.second.queue_id == waited_queue; });
    for (auto &[semaphore, sync_points] : host_waitable_semaphores_) {
        vvl::EraseIf(sync_points, [waited_queue](const auto &sync_point) { return sync_point.queue_id == waited_queue; });
    }
}

void SyncValidator::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) {
    BaseClass::PostCallRecordDeviceWaitIdle(device, record_obj);

    // We need to treat this a fence waits for all queues... noting that present engine ops will be preserved.
    ForAllQueueBatchContexts(
        [](const QueueBatchContext::Ptr &batch) { batch->ApplyTaggedWait(kQueueAny, ResourceUsageRecord::kMaxIndex); });

    // For each timeline keep only the last signal per queue.
    // The last signal is needed to represent the current timeline state.
    EnsureTimelineSignalsLimit(1);

    // As we we've waited for everything on device, any waits are mooted. (except for acquires)
    vvl::EraseIf(waitable_fences_, [](const auto &waitable) { return waitable.second.acquired.Invalid(); });
    host_waitable_semaphores_.clear();
}

struct QueuePresentCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    SignalsUpdate signals_update;
    PresentedImages presented_images;
    QueuePresentCmdState(const SyncValidator &sync_validator) : signals_update(sync_validator) {}
};

bool SyncValidator::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!syncval_settings.submit_time_validation) return skip;

    ClearPending();

    vvl::TlsGuard<QueuePresentCmdState> cmd_state(&skip, *this);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    QueueBatchContext::ConstPtr last_batch = cmd_state->queue->LastBatch();
    QueueBatchContext::Ptr batch(std::make_shared<QueueBatchContext>(*this, *cmd_state->queue));

    uint32_t present_tag_count = SetupPresentInfo(*pPresentInfo, batch, cmd_state->presented_images);

    const auto wait_semaphores = vvl::make_span(pPresentInfo->pWaitSemaphores, pPresentInfo->waitSemaphoreCount);

    auto resolved_batches = batch->ResolvePresentWaits(wait_semaphores, cmd_state->presented_images, cmd_state->signals_update);

    // Import the previous batch information
    if (last_batch && !vvl::Contains(resolved_batches, last_batch)) {
        batch->ResolveLastBatch(last_batch);
        resolved_batches.emplace_back(std::move(last_batch));
    }

    // The purpose of keeping return value is to ensure async batches are alive during validation.
    // Validation accesses raw pointer to async contexts stored in AsyncReference.
    const auto async_batches = batch->RegisterAsyncContexts(resolved_batches);

    const ResourceUsageTag global_range_start = batch->SetupBatchTags(present_tag_count);
    // Update the present tags (convert to global range)
    for (auto &presented : cmd_state->presented_images) {
        presented.tag += global_range_start;
    }

    skip |= batch->DoQueuePresentValidate(error_obj.location, cmd_state->presented_images);
    batch->DoPresentOperations(cmd_state->presented_images);
    batch->LogPresentOperations(cmd_state->presented_images, submit_id);

    if (!skip) {
        cmd_state->queue->SetPendingLastBatch(std::move(batch));
    }
    return skip;
}

uint32_t SyncValidator::SetupPresentInfo(const VkPresentInfoKHR &present_info, QueueBatchContext::Ptr &batch,
                                         PresentedImages &presented_images) const {
    const VkSwapchainKHR *const swapchains = present_info.pSwapchains;
    const uint32_t *const image_indices = present_info.pImageIndices;
    const uint32_t swapchain_count = present_info.swapchainCount;

    // Create the working list of presented images
    presented_images.reserve(swapchain_count);
    for (uint32_t present_index = 0; present_index < swapchain_count; present_index++) {
        // Note: Given the "EraseIf" implementation for acquire fence waits, each presentation needs a unique tag.
        const ResourceUsageTag tag = presented_images.size();
        presented_images.emplace_back(*this, batch, swapchains[present_index], image_indices[present_index], present_index, tag);
        if (presented_images.back().Invalid()) {
            presented_images.pop_back();
        }
    }
    // Present is tagged for each swapchain.
    return static_cast<uint32_t>(presented_images.size());
}

void SyncValidator::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                  const RecordObject &record_obj) {
    BaseClass::PostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
    if (!syncval_settings.submit_time_validation) return;

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueuePresentCmdState> cmd_state;

    // See vvl::Device::PostCallRecordQueuePresentKHR for spec excerpt supporting
    if (record_obj.result == VK_ERROR_OUT_OF_HOST_MEMORY || record_obj.result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
        record_obj.result == VK_ERROR_DEVICE_LOST) {
        return;
    }

    // Update the state with the data from the validate phase
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));
    ApplySignalsUpdate(cmd_state->signals_update, queue_state->PendingLastBatch());
    for (auto &presented : cmd_state->presented_images) {
        presented.ExportToSwapchain(*this);
    }
    queue_state->ApplyPendingLastBatch();
}

void SyncValidator::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                      VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                      const RecordObject &record_obj) {
    BaseClass::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
    if (!syncval_settings.submit_time_validation) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
}

void SyncValidator::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                       uint32_t *pImageIndex, const RecordObject &record_obj) {
    BaseClass::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, record_obj);
    if (!syncval_settings.submit_time_validation) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex, record_obj);
}

void SyncValidator::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;

    // Get the image out of the presented list and create apppropriate fences/semaphores.
    auto swapchain_state = Get<syncval_state::Swapchain>(swapchain);
    if (vvl::StateObject::Invalid(swapchain_state)) return;  // Invalid acquire calls to be caught in CoreCheck/Parameter validation

    PresentedImage presented = swapchain_state->MovePresentedImage(*pImageIndex);
    if (presented.Invalid()) return;

    // No way to make access safe, so nothing to record
    if ((semaphore == VK_NULL_HANDLE) && (fence == VK_NULL_HANDLE)) return;

    // We create a queue-less QBC for the Semaphore and fences to wait on

    // Note: this is a heavyweight way to deal with the fact that all operation logs live in the QueueBatchContext... and
    // acquire doesn't happen on a queue, but we need a place to put the acquire operation access record.
    auto batch = std::make_shared<QueueBatchContext>(*this);
    batch->SetupAccessContext(presented);
    const ResourceUsageTag acquire_tag = batch->SetupBatchTags(1);
    batch->DoAcquireOperation(presented);
    batch->LogAcquireOperation(presented, record_obj.location.function);

    // Now swap out the present queue batch with the acquired one.
    // Note that fence and signal will read the acquire batch from presented, so this needs to be done before
    // setting up the synchronization
    presented.batch = std::move(batch);

    if (semaphore != VK_NULL_HANDLE) {
        std::shared_ptr<const vvl::Semaphore> sem_state = Get<vvl::Semaphore>(semaphore);
        if (sem_state) {
            // This will ignore any duplicated signal (emplace does not update existing entry),
            // and the core validation reports and error in this case.
            binary_signals_.emplace(sem_state->VkHandle(), SignalInfo(sem_state, presented, acquire_tag));
        }
    }
    if (fence != VK_NULL_HANDLE) {
        FenceHostSyncPoint sync_point;
        sync_point.tag = acquire_tag;
        sync_point.acquired = AcquiredImage(presented, acquire_tag);
        UpdateFenceHostSyncPoint(fence, std::move(sync_point));
    }
}

bool SyncValidator::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                               const ErrorObject &error_obj) const {
    SubmitInfoConverter submit_info(pSubmits, submitCount);
    return ValidateQueueSubmit(queue, submitCount, submit_info.submit_infos2.data(), fence, error_obj);
}

static std::vector<CommandBufferConstPtr> GetCommandBuffers(const vvl::Device &state_tracker, const VkSubmitInfo2 &submit_info) {
    // Collected command buffers have the same indexing as in the input VkSubmitInfo2 for reporting purposes.
    // If Get query returns null, it is stored in the result array to keep original indexing.
    std::vector<CommandBufferConstPtr> command_buffers;
    command_buffers.reserve(submit_info.commandBufferInfoCount);
    for (const auto &cb_info : vvl::make_span(submit_info.pCommandBufferInfos, submit_info.commandBufferInfoCount)) {
        command_buffers.emplace_back(state_tracker.Get<syncval_state::CommandBuffer>(cb_info.commandBuffer));
    }
    return command_buffers;
}

bool SyncValidator::ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                        const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!syncval_settings.submit_time_validation) return skip;

    std::lock_guard lock_guard(queue_submit_mutex_);

    ClearPending();

    QueueSubmitCmdState cmd_state_obj(*this);
    QueueSubmitCmdState* cmd_state = &cmd_state_obj;

    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    auto &queue_sync_state = cmd_state->queue;
    SignalsUpdate &signals_update = cmd_state->signals_update;

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = queue_sync_state->ReserveSubmitId();

    // Update label stack as we progress through batches and command buffers
    auto current_label_stack = queue_sync_state->GetQueueState()->cmdbuf_label_stack;

    BatchContextConstPtr last_batch = queue_sync_state->LastBatch();
    bool has_unresolved_batches = !queue_sync_state->UnresolvedBatches().empty();

    BatchContextPtr new_last_batch;
    std::vector<UnresolvedBatch> new_unresolved_batches;
    bool new_timeline_signals = false;

    for (uint32_t batch_idx = 0; batch_idx < submitCount; batch_idx++) {
        const VkSubmitInfo2 &submit = pSubmits[batch_idx];
        auto batch = std::make_shared<QueueBatchContext>(*this, *queue_sync_state);

        const auto wait_semaphores = vvl::make_span(submit.pWaitSemaphoreInfos, submit.waitSemaphoreInfoCount);
        std::vector<VkSemaphoreSubmitInfo> unresolved_waits;
        auto resolved_batches = batch->ResolveSubmitWaits(wait_semaphores, unresolved_waits, signals_update);

        // Add unresolved batch
        if (has_unresolved_batches || !unresolved_waits.empty()) {
            UnresolvedBatch unresolved_batch;
            unresolved_batch.batch = std::move(batch);
            unresolved_batch.submit_index = submit_id;
            unresolved_batch.batch_index = batch_idx;
            unresolved_batch.command_buffers = GetCommandBuffers(*this, submit);
            unresolved_batch.unresolved_waits = std::move(unresolved_waits);
            unresolved_batch.resolved_dependencies = std::move(resolved_batches);
            if (submit.pSignalSemaphoreInfos && submit.signalSemaphoreInfoCount) {
                const auto last_info = submit.pSignalSemaphoreInfos + submit.signalSemaphoreInfoCount;
                unresolved_batch.signals.assign(submit.pSignalSemaphoreInfos, last_info);
            }
            unresolved_batch.label_stack = current_label_stack;
            new_unresolved_batches.emplace_back(std::move(unresolved_batch));
            has_unresolved_batches = true;
            stats.AddUnresolvedBatch();
            continue;
        }
        new_last_batch = batch;

        // Import the previous batch information
        if (last_batch && !vvl::Contains(resolved_batches, last_batch)) {
            batch->ResolveLastBatch(last_batch);
            resolved_batches.emplace_back(std::move(last_batch));
        }

        // The purpose of keeping return value is to ensure async batches are alive during validation.
        // Validation accesses raw pointer to async contexts stored in AsyncReference.
        // TODO: All syncval tests pass when the return value is ignored. Write a regression test that fails/crashes in this case.
        const auto async_batches = batch->RegisterAsyncContexts(resolved_batches);

        const auto command_buffers = GetCommandBuffers(*this, submit);
        skip |= batch->ValidateSubmit(command_buffers, submit_id, batch_idx, current_label_stack, error_obj);

        const auto submit_signals = vvl::make_span(submit.pSignalSemaphoreInfos, submit.signalSemaphoreInfoCount);
        new_timeline_signals |= signals_update.RegisterSignals(batch, submit_signals);

        // Unless the previous batch was referenced by a signal it will self destruct
        // in the record phase when the last batch is updated.
        last_batch = batch;
    }

    // Schedule state update
    if (new_last_batch) {
        queue_sync_state->SetPendingLastBatch(std::move(new_last_batch));
    }
    if (!new_unresolved_batches.empty()) {
        auto unresolved_batches = queue_sync_state->UnresolvedBatches();
        vvl::Append(unresolved_batches, new_unresolved_batches);
        queue_sync_state->SetPendingUnresolvedBatches(std::move(unresolved_batches));
    }

    // Check if timeline signals resolve existing wait-before-signal dependencies
    if (new_timeline_signals) {
        skip |= PropagateTimelineSignals(signals_update, error_obj);
    }

    if (!skip) {
        const_cast<SyncValidator *>(this)->RecordQueueSubmit(queue, fence, cmd_state);
    }

    // Note that if we skip, guard cleans up for us, but cannot release the reserved tag range
    return skip;
}

bool SyncValidator::PropagateTimelineSignals(SignalsUpdate &signals_update, const ErrorObject &error_obj) const {
    bool skip = false;
    // Initialize per-queue unresolved batches state.
    std::vector<UnresolvedQueue> queues;
    for (const auto &queue_state : queue_sync_states_) {
        if (!queue_state->PendingUnresolvedBatches().empty()) {
            // Pending request defines the final unresolved list (current + new unresolved batches)
            queues.emplace_back(UnresolvedQueue{queue_state, queue_state->PendingUnresolvedBatches()});
        } else if (!queue_state->UnresolvedBatches().empty()) {
            queues.emplace_back(UnresolvedQueue{queue_state, queue_state->UnresolvedBatches()});
        }
    }

    // Each iteration uses registered timeline signals to resolve existing unresolved batches.
    // Each resolved batch can generate new timeline signals which can resolve more unresolved batches on the next iteration.
    // This finishes when all unresolved batches are resolved or when iteration does not generate new timeline signals.
    while (PropagateTimelineSignalsIteration(queues, signals_update, skip, error_obj)) {
        ;
    }

    // Schedule unresolved state update
    for (UnresolvedQueue &queue : queues) {
        if (queue.update_unresolved) {
            queue.queue_state->SetPendingUnresolvedBatches(std::move(queue.unresolved_batches));
        }
    }
    return skip;
}

bool SyncValidator::PropagateTimelineSignalsIteration(std::vector<UnresolvedQueue> &queues, SignalsUpdate &signals_update,
                                                      bool &skip, const ErrorObject &error_obj) const {
    bool has_new_timeline_signals = false;
    for (auto &queue : queues) {
        if (queue.unresolved_batches.empty()) {
            continue;  // all batches for this queue were resolved by previous iterations
        }

        BatchContextPtr last_batch =
            queue.queue_state->PendingLastBatch() ? queue.queue_state->PendingLastBatch() : queue.queue_state->LastBatch();
        const BatchContextPtr initial_last_batch = last_batch;

        while (!queue.unresolved_batches.empty()) {
            auto &unresolved_batch = queue.unresolved_batches.front();

            has_new_timeline_signals |= ProcessUnresolvedBatch(unresolved_batch, signals_update, last_batch, skip, error_obj);

            // Remove processed batch from the (local) unresolved list
            queue.unresolved_batches.erase(queue.unresolved_batches.begin());

            // Propagate change into the queue's (global) unresolved state
            queue.update_unresolved = true;

            stats.RemoveUnresolvedBatch();
        }
        if (last_batch != initial_last_batch) {
            queue.queue_state->SetPendingLastBatch(std::move(last_batch));
        }
    }
    return has_new_timeline_signals;
}

bool SyncValidator::ProcessUnresolvedBatch(UnresolvedBatch &unresolved_batch, SignalsUpdate &signals_update,
                                           BatchContextPtr &last_batch, bool &skip, const ErrorObject &error_obj) const {
    // Resolve waits that have matching signal
    auto it = unresolved_batch.unresolved_waits.begin();
    while (it != unresolved_batch.unresolved_waits.end()) {
        const VkSemaphoreSubmitInfo &wait_info = *it;
        auto resolving_signal = signals_update.OnTimelineWait(wait_info.semaphore, wait_info.value);
        if (!resolving_signal.has_value()) {
            ++it;
            continue;  // resolving signal not found, the wait stays unresolved
        }
        if (resolving_signal->batch) {  // null for host signals
            unresolved_batch.batch->ResolveSubmitSemaphoreWait(*resolving_signal, wait_info.stageMask);
            unresolved_batch.batch->ImportTags(*resolving_signal->batch);
            unresolved_batch.resolved_dependencies.emplace_back(resolving_signal->batch);
        }
        it = unresolved_batch.unresolved_waits.erase(it);
    }

    // This batch still has unresolved waits
    if (!unresolved_batch.unresolved_waits.empty()) {
        return false;  // no new timeline signals were registered
    }

    // Process fully resolved batch
    UnresolvedBatch &ready_batch = unresolved_batch;
    if (last_batch && !vvl::Contains(ready_batch.resolved_dependencies, last_batch)) {
        ready_batch.batch->ResolveLastBatch(last_batch);
        ready_batch.resolved_dependencies.emplace_back(std::move(last_batch));
    }
    last_batch = ready_batch.batch;

    const auto async_batches = ready_batch.batch->RegisterAsyncContexts(ready_batch.resolved_dependencies);

    skip |= ready_batch.batch->ValidateSubmit(ready_batch.command_buffers, ready_batch.submit_index, ready_batch.batch_index,
                                              ready_batch.label_stack, error_obj);

    const auto submit_signals = vvl::make_span(ready_batch.signals.data(), ready_batch.signals.size());
    return signals_update.RegisterSignals(ready_batch.batch, submit_signals);
}

void SyncValidator::RecordQueueSubmit(VkQueue queue, VkFence fence, QueueSubmitCmdState *cmd_state) {
    // If this return is above the TlsGuard, then the Validate phase return must also be.
    if (!syncval_settings.submit_time_validation) return;  // Queue submit validation disabled

    if (!cmd_state->queue) return;  // Validation couldn't find a valid queue object

    // Don't need to look up the queue state again, but we need a non-const version
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));
    ApplySignalsUpdate(cmd_state->signals_update, queue_state->PendingLastBatch());

    // Apply the pending state from the validation phase. Check all queues because timeline signals
    // on the current queue can resolve wait-before-signal batches on other queues.
    for (const auto &qs : queue_sync_states_) {
        qs->ApplyPendingLastBatch();
        qs->ApplyPendingUnresolvedBatches();
    }

    FenceHostSyncPoint sync_point;
    sync_point.queue_id = queue_state->GetQueueId();
    sync_point.tag = ReserveGlobalTagRange(1).begin;
    UpdateFenceHostSyncPoint(fence, std::move(sync_point));
}

bool SyncValidator::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                   VkFence fence, const ErrorObject &error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool SyncValidator::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                                const ErrorObject &error_obj) const {
    return ValidateQueueSubmit(queue, submitCount, pSubmits, fence, error_obj);
}

void SyncValidator::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) {
    BaseClass::PostCallRecordGetFenceStatus(device, fence, record_obj);
    if (!syncval_settings.submit_time_validation) return;
    if (record_obj.result == VK_SUCCESS) {
        // fence is signalled, mark it as waited for
        WaitForFence(fence);
    }
}

void SyncValidator::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                                uint64_t timeout, const RecordObject &record_obj) {
    BaseClass::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, record_obj);
    if (!syncval_settings.submit_time_validation) return;
    if ((record_obj.result == VK_SUCCESS) && ((VK_TRUE == waitAll) || (1 == fenceCount))) {
        // We can only know the pFences have signal if we waited for all of them, or there was only one of them
        for (uint32_t i = 0; i < fenceCount; i++) {
            WaitForFence(pFences[i]);
        }
    }
}

bool SyncValidator::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    if (!syncval_settings.submit_time_validation) {
        return skip;
    }
    ClearPending();
    vvl::TlsGuard<QueueSubmitCmdState> cmd_state(&skip, *this);
    SignalsUpdate &signals_update = cmd_state->signals_update;

    auto semaphore_state = Get<vvl::Semaphore>(pSignalInfo->semaphore);
    if (!semaphore_state) {
        return skip;
    }

    std::vector<SignalInfo> &signals = signals_update.timeline_signals[pSignalInfo->semaphore];

    // Reject invalid signal
    if (!signals.empty() && pSignalInfo->value <= signals.back().timeline_value) {
        return skip;  // [core validation check]: strictly increasing signal values
    }

    signals.emplace_back(SignalInfo(semaphore_state, pSignalInfo->value));
    skip |= PropagateTimelineSignals(signals_update, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                      const ErrorObject &error_obj) const {
    return PreCallValidateSignalSemaphore(device, pSignalInfo, error_obj);
}

void SyncValidator::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                  const RecordObject &record_obj) {
    BaseClass::PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
    if (!syncval_settings.submit_time_validation) {
        return;
    }

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueueSubmitCmdState> cmd_state;

    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    ApplySignalsUpdate(cmd_state->signals_update, nullptr);
    for (const auto &qs : queue_sync_states_) {
        qs->ApplyPendingLastBatch();
        qs->ApplyPendingUnresolvedBatches();
    }
}

void SyncValidator::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                     const RecordObject &record_obj) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void SyncValidator::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                 const RecordObject &record_obj) {
    BaseClass::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
    if (!syncval_settings.submit_time_validation) {
        return;
    }
    const bool wait_all = pWaitInfo->semaphoreCount == 1 || (pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT) == 0;
    if (record_obj.result == VK_SUCCESS && wait_all) {
        for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
            WaitForSemaphore(pWaitInfo->pSemaphores[i], pWaitInfo->pValues[i]);
        }
    }
}

void SyncValidator::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                    const RecordObject &record_obj) {
    PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void SyncValidator::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                           const RecordObject &record_obj) {
    BaseClass::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
    if (!syncval_settings.submit_time_validation) {
        return;
    }
    if (record_obj.result == VK_SUCCESS) {
        WaitForSemaphore(semaphore, *pValue);
    }
}

void SyncValidator::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                              const RecordObject &record_obj) {
    PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void SyncValidator::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                        VkImage *pSwapchainImages, const RecordObject &record_obj) {
    BaseClass::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, record_obj);
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    auto swapchain_state = Get<vvl::Swapchain>(swapchain);

    if (pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            vvl::SwapchainImage &swapchain_image = swapchain_state->images[i];
            if (swapchain_image.image_state) {
                auto *sync_image = static_cast<ImageState *>(swapchain_image.image_state);
                assert(sync_image->IsTiled());  // This is the assumption from the spec, and the implementation relies on it
                sync_image->SetOpaqueBaseAddress(*this);
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = cb_state->access_context;
    auto &context = *cb_context.GetCurrentAccessContext();

    for (uint32_t i = 0; i < infoCount; i++) {
        const auto &info = pInfos[i];
        const auto scratch_buffers = GetBuffersByAddress(info.scratchData.deviceAddress);
        if (scratch_buffers.empty()) {
            continue;
        }
        if (scratch_buffers.size() > 1) {
            continue;  // postpone handling this case until syncval memory aliasing support is ready
        }
        const VkDeviceSize scratch_size = rt::ComputeScratchSize(rt::BuildType::Device, device, info, ppBuildRangeInfos[i]);
        const vvl::Buffer &scratch_buffer = *scratch_buffers[0];

        // Skip invalid configurations
        {
            const sparse_container::range<VkDeviceSize> scratch_range(info.scratchData.deviceAddress,
                                                                      info.scratchData.deviceAddress + scratch_size);
            if (!scratch_buffer.DeviceAddressRange().includes(scratch_range)) {
                continue;  // [core validation check]: invalid scratch range
            }
        }

        const ResourceAccessRange range = MakeRange(info.scratchData.deviceAddress - scratch_buffer.deviceAddress, scratch_size);
        auto hazard = context.DetectHazard(scratch_buffer, SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, scratch_buffer.Handle());
            const auto error = error_messages_.BufferError(hazard, scratch_buffer.VkHandle(), "scratch buffer", cb_context,
                                                           error_obj.location.function);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = cb_state->access_context;
    auto &context = *cb_context.GetCurrentAccessContext();

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);

    for (uint32_t i = 0; i < infoCount; i++) {
        const auto &info = pInfos[i];
        const auto scratch_buffers = GetBuffersByAddress(info.scratchData.deviceAddress);
        if (scratch_buffers.empty()) {
            continue;
        }
        if (scratch_buffers.size() > 1) {
            continue;  // postpone handling this case until syncval memory aliasing support is ready
        }
        const VkDeviceSize scratch_size = rt::ComputeScratchSize(rt::BuildType::Device, device, info, ppBuildRangeInfos[i]);
        const vvl::Buffer &scratch_buffer = *scratch_buffers[0];

        // Skip invalid configurations
        {
            const sparse_container::range<VkDeviceSize> scratch_range(info.scratchData.deviceAddress,
                                                                      info.scratchData.deviceAddress + scratch_size);
            if (!scratch_buffer.DeviceAddressRange().includes(scratch_range)) {
                continue;  // [core validation check]: invalid scratch range
            }
        }

        const ResourceAccessRange range = MakeRange(info.scratchData.deviceAddress - scratch_buffer.deviceAddress, scratch_size);
        auto tag_ex = cb_context.AddCommandHandle(tag, scratch_buffer.Handle());
        context.UpdateAccessState(scratch_buffer, SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE,
                                  SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool syncval_state::ImageState::IsSimplyBound() const {
    bool simple = SimpleBinding(static_cast<const vvl::Bindable &>(*this)) || IsSwapchainImage() || bind_swapchain;

    // If it's not simple we must have an encoder.
    assert(!simple || fragment_encoder.get());

    return simple;
}

void syncval_state::ImageState::SetOpaqueBaseAddress(vvl::Device &dev_data) {
    // This is safe to call if already called to simplify caller logic
    // NOTE: Not asserting IsTiled, as there could in future be other reasons for opaque representations
    if (opaque_base_address_) return;

    VkDeviceSize opaque_base = 0U;  // Fakespace Allocator starts > 0
    auto get_opaque_base = [&opaque_base](const vvl::Image &other) {
        const ImageState &other_sync = static_cast<const ImageState &>(other);
        opaque_base = other_sync.opaque_base_address_;
        return true;
    };
    if (IsSwapchainImage()) {
        AnyAliasBindingOf(bind_swapchain->ObjectBindings(), get_opaque_base);
    } else {
        AnyImageAliasOf(get_opaque_base);
    }
    if (!opaque_base) {
        // The size of the opaque range is based on the SyncVal *internal* representation of the tiled resource, unrelated
        // to the acutal size of the the resource in device memory. If differing representations become possible, the allocated
        // size would need to be changed to those representation's size requirements.
        opaque_base = dev_data.AllocFakeMemory(fragment_encoder->TotalSize());
    }
    opaque_base_address_ = opaque_base;
}

VkDeviceSize syncval_state::ImageState::GetResourceBaseAddress() const {
    if (HasOpaqueMapping()) {
        return GetOpaqueBaseAddress();
    }
    return GetFakeBaseAddress();
}

ImageRangeGen syncval_state::ImageState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                           bool is_depth_sliced) const {
    if (!fragment_encoder || !IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    ImageRangeGen range_gen(*fragment_encoder.get(), subresource_range, base_address, is_depth_sliced);
    return range_gen;
}

ImageRangeGen syncval_state::ImageState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                           const VkOffset3D &offset, const VkExtent3D &extent,
                                                           bool is_depth_sliced) const {
    if (!fragment_encoder || !IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    subresource_adapter::ImageRangeGenerator range_gen(*fragment_encoder.get(), subresource_range, offset, extent, base_address,
                                                       is_depth_sliced);
    return range_gen;
}

syncval_state::ImageViewState::ImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle,
                                              const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2 ff,
                                              const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props)
    : vvl::ImageView(image_state, handle, ci, ff, cubic_props), view_range_gen(MakeImageRangeGen()) {}

ImageRangeGen syncval_state::ImageViewState::MakeImageRangeGen() const {
    return GetImageState()->MakeImageRangeGen(normalized_subresource_range, IsDepthSliced());
}

ImageRangeGen syncval_state::ImageViewState::MakeImageRangeGen(const VkOffset3D &offset, const VkExtent3D &extent,
                                                               VkImageAspectFlags override_depth_stencil_aspect_mask) const {
    if (Invalid()) ImageRangeGen();

    VkImageSubresourceRange subresource_range = normalized_subresource_range;

    if (override_depth_stencil_aspect_mask != 0) {
        assert((override_depth_stencil_aspect_mask & kDepthStencilAspects) == override_depth_stencil_aspect_mask);
        subresource_range.aspectMask = override_depth_stencil_aspect_mask;
    }

    return GetImageState()->MakeImageRangeGen(subresource_range, offset, extent, IsDepthSliced());
}
