/*
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

#pragma once
#include "sync/sync_commandbuffer.h"
#include "state_tracker/queue_state.h"

struct PresentedImage;
class QueueBatchContext;
struct QueueSubmitCmdState;
class QueueSyncState;
class SyncValidator;

using BatchContextPtr = std::shared_ptr<QueueBatchContext>;
using BatchContextConstPtr = std::shared_ptr<const QueueBatchContext>;
using CommandBufferConstPtr = std::shared_ptr<const syncval_state::CommandBuffer>;

namespace vvl {
class Semaphore;
}  // namespace vvl

struct AcquiredImage {
    std::shared_ptr<const syncval_state::ImageState> image;
    subresource_adapter::ImageRangeGenerator generator;
    ResourceUsageTag present_tag;
    ResourceUsageTag acquire_tag;
    bool Invalid() const;

    AcquiredImage() = default;
    AcquiredImage(const PresentedImage &presented, ResourceUsageTag acq_tag);
};

// Information associated with a semaphore signal
struct SignalInfo {
    // QueueSubmit signal
    SignalInfo(const std::shared_ptr<const vvl::Semaphore> &semaphore_state, const std::shared_ptr<QueueBatchContext> &batch,
               const SyncExecScope &exec_scope, uint64_t timeline_value);

    // SignalSemaphore signal
    SignalInfo(const std::shared_ptr<const vvl::Semaphore> &semaphore_state, uint64_t timeline_value);

    // AcquireNextImage signal
    SignalInfo(const std::shared_ptr<const vvl::Semaphore> &semaphore_state, const PresentedImage &presented,
               ResourceUsageTag acquire_tag);

    // Signaled semaphore. Not null.
    std::shared_ptr<const vvl::Semaphore> semaphore_state;

    // Batch from the signal's first scope. It is null for a host signal (vkSignalSemaphore)
    std::shared_ptr<QueueBatchContext> batch;

    // Use the first_scope.valid_accesses for the first access scope of non-host signals.
    // first_scope.queue is kQueueIdInvalid for a host signal (vkSignalSemaphore)
    SemaphoreScope first_scope;

    // Value signaled by a timeline semaphore
    uint64_t timeline_value = 0;

    // Swapchain specific signal info.
    // Batch field is the batch of the last present for the acquired image.
    // The AcquiredImage further limits the scope of the resolve operation, and the "barrier" will also
    // be special case (updating "PRESENTED" write with "ACQUIRE" read, as well as setting the barrier).
    //
    // NOTE: shared_ptr is used here as a memory saver. AcquiredImage is 224 bytes at the time
    // of writing and is not used in the queue submit signals. If we optimize ImageRangeGenerator
    // memory usage then shared_ptr can be replaced by std::optional to avoid allocation.
    std::shared_ptr<AcquiredImage> acquired_image;
};

// When the timeline wait is resolved, the previous signals can be removed
struct RemoveTimelineSignalsRequest {
    VkSemaphore semaphore = VK_NULL_HANDLE;

    // Remove all signals with a value *less than* this threshold (should not touch signals with equal value).
    uint64_t signal_threshold_value = 0;

    // NOTE: it's possible to have multiple queues with signals that match the wait value.
    // The specification defines that exactly one signal resolves the wait, and in the presence
    // of multiple such signals the implementation may choose any of them. It's the application
    // responsibility to be careful and not to create a race condition in such a scenario.
    //
    // The queue that signaled the resolving signal. Only the signals signaled by this queue should be processed.
    QueueId queue = 0;
};

// The requests to update SyncValidator's registry of binary/timeline signals.
// They are collected during validation phase and are applied in the record phase.
struct SignalsUpdate {
    vvl::unordered_map<VkSemaphore, SignalInfo> binary_signal_requests;
    vvl::unordered_set<VkSemaphore> binary_unsignal_requests;

    vvl::unordered_map<VkSemaphore, std::vector<SignalInfo>> timeline_signals;
    std::vector<RemoveTimelineSignalsRequest> remove_timeline_signals_requests;

    // Register submission batch signals.
    // Return true if at least one timeline signal was registered
    bool RegisterSignals(const BatchContextPtr &batch, const vvl::span<const VkSemaphoreSubmitInfo> &submit_signals);

    // Return resolving binary signal. Empty result in the case of a validation error
    std::optional<SignalInfo> OnBinaryWait(VkSemaphore semaphore);

    // Return resolving timeline signal. Empty result if it is a wait-before-signal
    std::optional<SignalInfo> OnTimelineWait(VkSemaphore semaphore, uint64_t wait_value);

    SignalsUpdate(const SyncValidator &sync_validator) : sync_validator_(sync_validator) {}

  private:
    void OnBinarySignal(const vvl::Semaphore &semaphore_state, const std::shared_ptr<QueueBatchContext> &batch,
                        const VkSemaphoreSubmitInfo &submit_signal);
    // Return false if signal is invalid (non-increasing value)
    bool OnTimelineSignal(const vvl::Semaphore &semaphore_state, const std::shared_ptr<QueueBatchContext> &batch,
                          const VkSemaphoreSubmitInfo &submit_signal);

  private:
    const SyncValidator &sync_validator_;
};

struct FenceHostSyncPoint {
    QueueId queue_id = kQueueIdInvalid;
    ResourceUsageTag tag = 0;
    AcquiredImage acquired;  // Iff queue == invalid and acquired.image valid.
};

struct TimelineHostSyncPoint {
    QueueId queue_id = 0;
    ResourceUsageTag tag = 0;
    uint64_t timeline_value = 0;
};

struct PresentedImageRecord {
    ResourceUsageTag tag;  // the global tag at presentation
    uint32_t image_index;
    uint32_t present_index;
    std::weak_ptr<const syncval_state::Swapchain> swapchain_state;
    std::shared_ptr<const syncval_state::ImageState> image;
};

struct PresentedImage : public PresentedImageRecord {
    std::shared_ptr<QueueBatchContext> batch;
    subresource_adapter::ImageRangeGenerator range_gen;

    PresentedImage() = default;
    void UpdateMemoryAccess(SyncAccessIndex usage, ResourceUsageTag tag, AccessContext &access_context) const;
    PresentedImage(const SyncValidator &sync_state, std::shared_ptr<QueueBatchContext> batch, VkSwapchainKHR swapchain,
                   uint32_t image_index, uint32_t present_index, ResourceUsageTag present_tag_);
    // For non-previsously presented images..
    PresentedImage(std::shared_ptr<const syncval_state::Swapchain> swapchain, uint32_t at_index);
    bool Invalid() const;
    void ExportToSwapchain(SyncValidator &);
    void SetImage(uint32_t at_index);
};
using PresentedImages = std::vector<PresentedImage>;

// Store references to ResourceUsageRecords with global tag range within a batch
class BatchAccessLog {
  public:
    struct BatchRecord {
        const QueueSyncState *queue = nullptr;
        uint64_t submit_index = 0;
        uint32_t batch_index = 0;
        uint32_t cb_index = 0;
        ResourceUsageTag base_tag = 0;
    };

    struct AccessRecord {
        const BatchRecord *batch;
        const ResourceUsageRecord *record;
        const DebugNameProvider *debug_name_provider;
        bool IsValid() const { return batch && record; }
    };

    struct CBSubmitLog : DebugNameProvider {
      public:
        CBSubmitLog() = default;
        CBSubmitLog(const CBSubmitLog &batch) = default;
        CBSubmitLog(CBSubmitLog &&other) = default;
        CBSubmitLog &operator=(const CBSubmitLog &other) = default;
        CBSubmitLog &operator=(CBSubmitLog &&other) = default;
        CBSubmitLog(const BatchRecord &batch, std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs,
                    std::shared_ptr<const CommandExecutionContext::AccessLog> log);
        CBSubmitLog(const BatchRecord &batch, const CommandBufferAccessContext &cb,
                    const std::vector<std::string> &initial_label_stack);
        size_t Size() const { return log_->size(); }
        AccessRecord GetAccessRecord(ResourceUsageTag tag) const;

        // DebugNameProvider
        std::string GetDebugRegionName(const ResourceUsageRecord &record) const override;

      private:
        BatchRecord batch_;
        std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs_;
        std::shared_ptr<const CommandExecutionContext::AccessLog> log_;
        // label stack at the point when command buffer is submitted to the queue
        std::vector<std::string> initial_label_stack_;
    };

    void Import(const BatchRecord &batch, const CommandBufferAccessContext &cb_access,
                const std::vector<std::string> &initial_label_stack);
    void Import(const BatchAccessLog &other);
    void Insert(const BatchRecord &batch, const ResourceUsageRange &range,
                std::shared_ptr<const CommandExecutionContext::AccessLog> log);

    void Trim(const ResourceUsageTagSet &used);
    // AccessRecord lookup is based on global tags
    AccessRecord GetAccessRecord(ResourceUsageTag tag) const;
    BatchAccessLog() {}

  private:
    using CBSubmitLogRangeMap = sparse_container::range_map<ResourceUsageTag, CBSubmitLog>;
    CBSubmitLogRangeMap log_map_;
};

// Batch that has wait-before-signal dependencies.
struct UnresolvedBatch {
    BatchContextPtr batch;
    uint64_t submit_index = 0;
    uint32_t batch_index = 0;
    std::vector<CommandBufferConstPtr> command_buffers;

    // Waits-before-signals that prevent this batch from being resolved.
    // When the wait is resolved it is removed from this list and the batch
    // from the resolving signal is stored in resolved_dependencies array.
    std::vector<VkSemaphoreSubmitInfo> unresolved_waits;

    // The batches from the resolved dependencies. They are used for async validaton.
    // This includes the batches from the resolved waits and also the last batch
    // (prior batch on the same queue).
    std::vector<BatchContextConstPtr> resolved_dependencies;

    // Signals to signal when all the waits are resolved.
    std::vector<VkSemaphoreSubmitInfo> signals;

    // Queue's label stack at the beginning of this batch
    std::vector<std::string> label_stack;
};

// Helper struct to resolve wait-before-signal
struct UnresolvedQueue {
    std::shared_ptr<QueueSyncState> queue_state;
    std::vector<UnresolvedBatch> unresolved_batches;
    // whether unresolved state should be updated for this queue
    bool update_unresolved = false;
};

class QueueBatchContext : public CommandExecutionContext, public std::enable_shared_from_this<QueueBatchContext> {
  public:
    class PresentResourceRecord : public AlternateResourceUsage::RecordBase {
      public:
        using Base_ = AlternateResourceUsage::RecordBase;
        Base_::Record MakeRecord() const override;
        ~PresentResourceRecord() override {}
        PresentResourceRecord(const PresentedImageRecord &presented) : presented_(presented) {}
        std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const override;
        vvl::Func GetCommand() const override { return vvl::Func::vkQueuePresentKHR; }

      private:
        PresentedImageRecord presented_;
    };

    class AcquireResourceRecord : public AlternateResourceUsage::RecordBase {
      public:
        using Base_ = AlternateResourceUsage::RecordBase;
        Base_::Record MakeRecord() const override;
        AcquireResourceRecord(const PresentedImageRecord &presented, ResourceUsageTag tag, vvl::Func command)
            : presented_(presented), acquire_tag_(tag), command_(command) {}
        std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const override;
        vvl::Func GetCommand() const override { return command_; }

      private:
        PresentedImageRecord presented_;
        ResourceUsageTag acquire_tag_;
        vvl::Func command_;
    };

    using Ptr = std::shared_ptr<QueueBatchContext>;
    using ConstPtr = std::shared_ptr<const QueueBatchContext>;

    QueueBatchContext(const SyncValidator &sync_state, const QueueSyncState &queue_state);
    QueueBatchContext(const SyncValidator &sync_state);
    QueueBatchContext() = delete;
    ~QueueBatchContext();
    void Trim();

    std::string FormatUsage(ResourceUsageTagEx tag_ex, ReportKeyValues &extra_properties) const override;
    void AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const override;
    AccessContext *GetCurrentAccessContext() override { return current_access_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_access_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    const QueueSyncState *GetQueueSyncState() { return queue_state_; }
    QueueId GetQueueId() const override;
    ExecutionType Type() const override { return kSubmitted; }
    ResourceUsageRange GetTagRange() const { return tag_range_; }

    ResourceUsageTag SetupBatchTags(uint32_t tag_count);
    void ResetEventsContext() { events_context_.Clear(); }

    // For Submit
    std::vector<BatchContextConstPtr> ResolveSubmitWaits(vvl::span<const VkSemaphoreSubmitInfo> wait_semaphores,
                                                         std::vector<VkSemaphoreSubmitInfo> &unresolved_waits,
                                                         SignalsUpdate &signals_update);

    bool ValidateSubmit(const std::vector<CommandBufferConstPtr> &command_buffers, uint64_t submit_index, uint32_t batch_index,
                        std::vector<std::string> &current_label_stack, const ErrorObject &error_obj);
    void ResolveSubmittedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    // For Present
    std::vector<ConstPtr> ResolvePresentWaits(vvl::span<const VkSemaphore> wait_semaphores, const PresentedImages &presented_images,
                                              SignalsUpdate &signals_update);
    bool DoQueuePresentValidate(const Location &loc, const PresentedImages &presented_images);
    void DoPresentOperations(const PresentedImages &presented_images);
    void LogPresentOperations(const PresentedImages &presented_images, uint64_t submit_index);

    // For Acquire
    void SetupAccessContext(const PresentedImage &presented);
    void DoAcquireOperation(const PresentedImage &presented);
    void LogAcquireOperation(const PresentedImage &presented, vvl::Func command);

    VulkanTypedHandle Handle() const override;

    template <typename Predicate>
    void ApplyPredicatedWait(Predicate &predicate);
    void ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag);
    void ApplyAcquireWait(const AcquiredImage &acquired);
    void OnResourceDestroyed(const ResourceAccessRange &resource_range);

    void BeginRenderPassReplaySetup(ReplayState &replay, const SyncOpBeginRenderPass &begin_op);
    void NextSubpassReplaySetup(ReplayState &replay);
    void EndRenderPassReplayCleanup(ReplayState &replay);

    [[nodiscard]] std::vector<ConstPtr> RegisterAsyncContexts(const std::vector<ConstPtr> &batches_resolved);
    void ResolveLastBatch(const QueueBatchContext::ConstPtr &last_batch);

    void ResolveSubmitSemaphoreWait(const SignalInfo &signal_info, VkPipelineStageFlags2 wait_mask);
    void ImportTags(const QueueBatchContext &from);

  private:
    void ResolvePresentSemaphoreWait(const SignalInfo &signal_info, const PresentedImages &presented_images);

  private:
    const QueueSyncState *queue_state_ = nullptr;
    ResourceUsageRange tag_range_ = ResourceUsageRange(0, 0);  // Range of tags referenced by cbs_referenced

    AccessContext access_context_;
    AccessContext *current_access_context_;
    SyncEventsContext events_context_;
    BatchAccessLog batch_log_;
    std::vector<ResourceUsageTag> queue_sync_tag_;
};

class QueueSyncState {
  public:
    QueueSyncState(const std::shared_ptr<vvl::Queue> &queue_state, QueueId id) : id_(id), queue_state_(queue_state) {}

    VulkanTypedHandle Handle() const { return queue_state_->Handle(); }
    const vvl::Queue *GetQueueState() const { return queue_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_state_->queue_family_properties.queueFlags; }
    QueueId GetQueueId() const { return id_; }
    // Method is const but updates mutable sumbit_index atomically.
    uint64_t ReserveSubmitId() const;

    // Last batch state management.
    // The Validate phase makes a request to update last batch by calling SetPendingLastBatch.
    // Then the Record phase actually updates the last batch by calling ApplyPendingLastBatch.
    // Pending last batch is a mutable state. It relies on the queue external synchronization.
    QueueBatchContext::ConstPtr LastBatch() const { return last_batch_; }
    QueueBatchContext::Ptr LastBatch() { return last_batch_; }
    void SetPendingLastBatch(QueueBatchContext::Ptr &&last) const;
    void ApplyPendingLastBatch();
    QueueBatchContext::Ptr PendingLastBatch() const { return pending_last_batch_; }

    // Unresolved batches state management.
    // The Validate phase makes request to update the list of unresolved batches by calling SetPendingUnresolvedBatches.
    // Then the Record phase actually updates the list of unresolved batches by calling ApplyPendingLastBatch.
    // Pending unresovled batches is a mutable state. It relies on the queue external synchronization.
    const std::vector<UnresolvedBatch> &UnresolvedBatches() const { return unresolved_batches_; }
    void SetPendingUnresolvedBatches(std::vector<UnresolvedBatch> &&unresolved_batches) const;
    void ApplyPendingUnresolvedBatches();
    const std::vector<UnresolvedBatch> &PendingUnresolvedBatches() const { return pending_unresolved_batches_; }

    // Called by the Validate methods to ensure no pending state is left.
    // Pending state is automatically cleared in PostRecord calls,
    // the only exception is when validation error happens.
    void ClearPending() const;

  private:
    const QueueId id_;
    std::shared_ptr<vvl::Queue> queue_state_;
    mutable std::atomic<uint64_t> submit_index_ = 0;

    QueueBatchContext::Ptr last_batch_;

    // The first batch in the unresolved batches list is always due to the wait-before-signal dependency.
    // All subsequent batches from the same queue must also be stored here because they can't be processed
    // until the wait-before-signal dependency is resolved (respect submission order). When the first batch
    // is resolved, we start processing other queued batches until we uncoutner a batch with unresolved
    // wait-before-signal (it becomes the new head of the list) or the list is empty.
    std::vector<UnresolvedBatch> unresolved_batches_;

    mutable QueueBatchContext::Ptr pending_last_batch_;
    mutable std::vector<UnresolvedBatch> pending_unresolved_batches_;
    mutable bool update_unresolved_batches_ = false;
};

struct QueueSubmitCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    SignalsUpdate signals_update;
    QueueSubmitCmdState(const SyncValidator &sync_validator) : signals_update(sync_validator) {}
};
