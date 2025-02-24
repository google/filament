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

#include <memory>
#include <vulkan/vulkan.h>

#include "sync/sync_common.h"
#include "sync/sync_access_context.h"
#include "sync/sync_commandbuffer.h"
#include "sync/sync_error_messages.h"
#include "sync/sync_stats.h"
#include "sync/sync_submit.h"

namespace syncval {
// sync validation has no instance-level functionality
class Instance : public vvl::Instance {
  public:
    Instance(vvl::dispatch::Instance *dispatch) : vvl::Instance(dispatch, LayerObjectTypeSyncValidation) {}
};
}  // namespace syncval

VALSTATETRACK_DERIVED_STATE_OBJECT(VkImage, syncval_state::ImageState, vvl::Image)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImageView, syncval_state::ImageViewState, vvl::ImageView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, syncval_state::CommandBuffer, vvl::CommandBuffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSwapchainKHR, syncval_state::Swapchain, vvl::Swapchain)

class SyncValidator : public vvl::Device, public SyncStageAccess {
    using BaseClass = vvl::Device;

  public:
    using ImageState = syncval_state::ImageState;
    using ImageViewState = syncval_state::ImageViewState;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    SyncValidator(vvl::dispatch::Device *dev, syncval::Instance *instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeSyncValidation), error_messages_(*this) {}
    ~SyncValidator();

    syncval::ErrorMessages error_messages_;

    // Stats object must be the first member of this class:
    // - it is the first to be constructed: can observe all subsequent syncval stats events
    // - it is the last to be destroyed: ensures there are no unreported syncval stats events.
    mutable syncval_stats::Stats stats;  // Stats object is thread safe

    // Global tag range for submitted command buffers resource usage logs
    // Started the global tag count at 1 s.t. zero are invalid and ResourceUsageTag normalization can just zero them.
    mutable std::atomic<ResourceUsageTag> tag_limit_{1};  // This is reserved in Validation phase, thus mutable and atomic
    ResourceUsageRange ReserveGlobalTagRange(size_t tag_count) const;  // Note that the tag_limit_ is mutable this has side effects

    std::vector<std::shared_ptr<QueueSyncState>> queue_sync_states_;
    QueueId queue_id_limit_ = 0;

    mutable std::mutex queue_submit_mutex_;

    // Semaphore signal registry
    vvl::unordered_map<VkSemaphore, SignalInfo> binary_signals_;
    vvl::unordered_map<VkSemaphore, std::vector<SignalInfo>> timeline_signals_;

    // Host synchronization (vkWaitForFences, vkWaitSemaphores)
    vvl::unordered_map<VkFence, FenceHostSyncPoint> waitable_fences_;
    vvl::unordered_map<VkSemaphore, std::deque<TimelineHostSyncPoint>> host_waitable_semaphores_;

    uint32_t debug_command_number = vvl::kU32Max;
    uint32_t debug_reset_count = 1;
    std::string debug_cmdbuf_pattern;

    bool SyncError(SyncHazard hazard, const LogObjectList &objlist, const Location &loc, const std::string &error_message) const;

    // Ensures that the number of signals per timeline per queue does not exceed the specified limit.
    // If `queue` parameter is specified, then only that queue is checked (used by vkQueueWaitIdle).
    // If the number of signals exceeds the limit, then signals with the smallest values are removed.
    //
    // Note, removing registered signals can't introduce errors/false-positives assuming at least
    // a single signal per timeline is left. That's because if there are more than one matching signal
    // to resolve a wait then the specification defines that only one signal is selected, which one is
    // unspecified. In the current implementation we keep multiple signals per timeline to have additional
    // options of validation, but, for example, keeping only the last signal is sufficient.
    void EnsureTimelineSignalsLimit(uint32_t signals_per_queue_limit, QueueId queue = kQueueIdInvalid);

    // Applies information from update object to binary_signals_/timeline_signals_.
    // The update object is mutable to be able to std::move SignalInfo from it.
    void ApplySignalsUpdate(SignalsUpdate &update, const QueueBatchContext::Ptr &last_batch);

    bool PropagateTimelineSignals(SignalsUpdate &signals_update, const ErrorObject &error_obj) const;

    // Return true if new timeline signals were generated by resolved batches
    bool PropagateTimelineSignalsIteration(std::vector<UnresolvedQueue> &queues, SignalsUpdate &signals_update, bool &skip,
                                           const ErrorObject &error_ob) const;
    bool ProcessUnresolvedBatch(UnresolvedBatch &unresolved_batch, SignalsUpdate &signals_update, BatchContextPtr &last_batch,
                                bool &skip, const ErrorObject &error_obj) const;

    void ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag);
    void ApplyAcquireWait(const AcquiredImage &acquired);

     // Go through every queue batch context and apply synchronization operation
    template <typename BatchOp>
    void ForAllQueueBatchContexts(BatchOp &&op);

    void UpdateFenceHostSyncPoint(VkFence fence, FenceHostSyncPoint &&sync_point);

    void WaitForFence(VkFence fence);
    void WaitForSemaphore(VkSemaphore semaphore, uint64_t value);

    void UpdateSyncImageMemoryBindState(uint32_t count, const VkBindImageMemoryInfo *infos);

    std::shared_ptr<const QueueSyncState> GetQueueSyncStateShared(VkQueue queue) const;
    QueueId GetQueueIdLimit() const { return queue_id_limit_; }

    std::vector<QueueBatchContext::ConstPtr> GetLastBatches(std::function<bool(const QueueBatchContext::ConstPtr &)> filter) const;
    std::vector<QueueBatchContext::Ptr> GetLastBatches(std::function<bool(const QueueBatchContext::ConstPtr &)> filter);
    std::vector<QueueBatchContext::ConstPtr> GetLastPendingBatches(std::function<bool(const QueueBatchContext::ConstPtr &)> filter) const;
    void ClearPending() const;

    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer handle,
                                                             const VkCommandBufferAllocateInfo *allocate_info,
                                                             const vvl::CommandPool *cmd_pool) override;
    std::shared_ptr<vvl::Swapchain> CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info,
                                                         VkSwapchainKHR swapchain) final;
    std::shared_ptr<vvl::Image> CreateImageState(VkImage handle, const VkImageCreateInfo *create_info,
                                                 VkFormatFeatureFlags2 features) final;

    std::shared_ptr<vvl::Image> CreateImageState(VkImage handle, const VkImageCreateInfo *create_info, VkSwapchainKHR swapchain,
                                                 uint32_t swapchain_index, VkFormatFeatureFlags2 features) final;
    std::shared_ptr<vvl::ImageView> CreateImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle,
                                                         const VkImageViewCreateInfo *create_info,
                                                         VkFormatFeatureFlags2 format_features,
                                                         const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) final;
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator,
                                    const RecordObject &record_obj) override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator,
                                   const RecordObject &record_obj) override;

    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                  const VkSubpassBeginInfo *pSubpassBeginInfo, Func command);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                              const VkSubpassEndInfo *pSubpassEndInfo, Func command);
    void RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, Func command);
    bool SupressedBoundDescriptorWAW(const HazardResult &hazard) const;

    void PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) override;

    void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                       const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                       const RecordObject &record_obj) override;
    void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) override;

    bool ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfo *pSubpassBeginInfo, const ErrorObject &error_obj) const;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents, const ErrorObject &error_obj) const override;

    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               const VkSubpassBeginInfo *pSubpassBeginInfo,
                                               const ErrorObject &error_obj) const override;

    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo,
                                            const ErrorObject &error_obj) const override;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions, const ErrorObject &error_obj) const override;

    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy *pRegions, const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                          const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                       const ErrorObject &error_obj) const override;
    void RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo, Func command);
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                        const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                     const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions, const ErrorObject &error_obj) const override;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions,
                                   const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                         const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                      const ErrorObject &error_obj) const override;

    void RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo, Func command);
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                       const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                    const RecordObject &record_obj) override;

    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                           const ErrorObject &error_obj) const override;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                         const RecordObject &record_obj) override;

    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                               const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                            const ErrorObject &error_obj) const override;
    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                             const RecordObject &record_obj) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                          const RecordObject &record_obj) override;

    void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                          const RecordObject &record_obj) override;

    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                          VkSubpassContents contents, const RecordObject &record_obj) override;
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) override;
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) override;

    bool ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                       const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                        const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const override;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject &record_obj) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                       const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                          const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) override;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                  const ErrorObject &error_obj) const;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                          const ErrorObject &error_obj) const override;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                         const RecordObject &record_obj) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                            const RecordObject &record_obj) override;

    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                          const ErrorObject &error_obj) const override;
    void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                           const RecordObject &record_obj) override;
    void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                        const RecordObject &record_obj) override;

    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) override;
    void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) override;

    template <typename RegionType>
    bool ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                      const Location &loc) const;
    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                 const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                              const ErrorObject &error_obj) const override;

    template <typename RegionType>
    void RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, Func command);
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                           const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                               const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                            const RecordObject &record_obj) override;

    template <typename RegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                      const Location &loc) const;
    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                 const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                              const ErrorObject &error_obj) const override;

    template <typename RegionType>
    void RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                    VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions, Func command);
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                           const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                               const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                            const RecordObject &record_obj) override;

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                              const Location &loc) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                         const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                      const ErrorObject &error_obj) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                            Func command);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter,
                                   const RecordObject &record_obj) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                       const RecordObject &record_obj) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                    const RecordObject &record_obj) override;

    bool ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                const VkDeviceSize struct_size, const VkBuffer buffer, const VkDeviceSize offset,
                                const uint32_t drawCount, const uint32_t stride, const Location &loc) const;
    void RecordIndirectBuffer(CommandBufferAccessContext &cb_context, ResourceUsageTag tag, const VkDeviceSize struct_size,
                              const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount, uint32_t stride);

    bool ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context, VkBuffer buffer,
                             VkDeviceSize offset, const Location &loc) const;
    void RecordCountBuffer(CommandBufferAccessContext &cb_context, ResourceUsageTag tag, VkBuffer buffer, VkDeviceSize offset);

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                    const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                  const RecordObject &record_obj) override;

    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject &record_obj) override;

    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                       const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                     const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                      uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const ErrorObject &error_obj) const override;
    void RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride, Func command);
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject &error_obj) const override;
    void RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, Func command);
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue *pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges,
                                                  const ErrorObject &error_obj) const override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) override;

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects,
                                            const ErrorObject &error_obj) const override;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects,
                                          const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags,
                                                const ErrorObject &error_obj) const override;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags, const RecordObject &record_obj) override;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data, const RecordObject &record_obj) override;

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions, const ErrorObject &error_obj) const override;

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve *pRegions, const RecordObject &record_obj) override;

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                            const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                         const ErrorObject &error_obj) const override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                          const RecordObject &record_obj) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                       const RecordObject &record_obj) override;

    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                      VkDeviceSize dataSize, const void *pData, const RecordObject &record_obj) override;

    bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                const ErrorObject &error_obj) const override;
    void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                              VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                              const RecordObject &record_obj) override;

    bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                          const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                        const RecordObject &record_obj) override;
    bool PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                          const ErrorObject &error_obj) const override;
    void PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                        const RecordObject &record_obj) override;

    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const ErrorObject &error_obj) const override;
    void PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                   const RecordObject &record_obj) override;

    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR *pDependencyInfo,
                                        const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                     const ErrorObject &error_obj) const override;
    void PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR *pDependencyInfo,
                                       const RecordObject &record_obj) override;
    void PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                    const RecordObject &record_obj) override;

    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const ErrorObject &error_obj) const override;
    void PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                     const RecordObject &record_obj) override;

    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                          const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                       const ErrorObject &error_obj) const override;
    void PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                         const RecordObject &record_obj) override;
    void PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                      const RecordObject &record_obj) override;

    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                      const ErrorObject &error_obj) const override;
    void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                     VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                     const RecordObject &record_obj) override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                          const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                         const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                       const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) override;
    bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkBuffer dstBuffer,
                                                 VkDeviceSize dstOffset, uint32_t marker,
                                                 const ErrorObject &error_obj) const override;
    void PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkBuffer dstBuffer,
                                               VkDeviceSize dstOffset, uint32_t marker, const RecordObject &record_obj) override;
    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                         const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                        const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                           const RecordObject &record_obj) override;
    void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) override;
    void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) override;

    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                        const ErrorObject &error_obj) const override;
    uint32_t SetupPresentInfo(const VkPresentInfoKHR &present_info, QueueBatchContext::Ptr &batch,
                              PresentedImages &presented_images) const;
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                       const RecordObject &record_obj) override;
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) override;
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo, uint32_t *pImageIndex,
                                            const RecordObject &record_obj) override;
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj);
    bool ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                             const ErrorObject &error_obj) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                    const ErrorObject &error_obj) const override;
    void RecordQueueSubmit(VkQueue queue, VkFence fence, QueueSubmitCmdState *cmd_state);
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                        const ErrorObject &error_obj) const override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                     const ErrorObject &error_obj) const override;
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) override;
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                     uint64_t timeout, const RecordObject &record_obj) override;
    bool PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                        const ErrorObject &error_obj) const override;
    bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                           const ErrorObject &error_obj) const override;
    void PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                       const RecordObject &record_obj) override;
    void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                          const RecordObject &record_obj) override;
    void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                      const RecordObject &record_obj) override;
    void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                         const RecordObject &record_obj) override;
    void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                const RecordObject &record_obj) override;
    void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                   const RecordObject &record_obj) override;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                             VkImage *pSwapchainImages, const RecordObject &record_obj) override;
    bool PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos,
                                                        const RecordObject &record_obj) override;
};
