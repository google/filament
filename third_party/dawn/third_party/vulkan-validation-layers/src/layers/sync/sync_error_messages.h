/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "sync/sync_renderpass.h"  // DynamicRenderingInfo::Attachment

#include <vulkan/vulkan.h>
#include <string>

class CommandBufferAccessContext;
class HazardResult;
struct ReportKeyValues;
class QueueBatchContext;

namespace vvl {
class DescriptorSet;
class Device;
class Pipeline;
}  // namespace vvl

namespace syncval {

class ErrorMessages {
  public:
    explicit ErrorMessages(vvl::Device& validator);

    std::string Error(const HazardResult& hazard, const char* description, const CommandBufferAccessContext& cb_context,
                      vvl::Func command) const;

    std::string BufferError(const HazardResult& hazard, VkBuffer buffer, const char* buffer_description,
                            const CommandBufferAccessContext& cb_context, vvl::Func command) const;

    std::string BufferRegionError(const HazardResult& hazard, VkBuffer buffer, uint32_t region_index,
                                  ResourceAccessRange region_range, const CommandBufferAccessContext& cb_context,
                                  const vvl::Func command) const;

    std::string ImageRegionError(const HazardResult& hazard, VkImage image, bool is_src_image, uint32_t region_index,
                                 const CommandBufferAccessContext& cb_context, vvl::Func command) const;

    std::string ImageSubresourceRangeError(const HazardResult& hazard, VkImage image, uint32_t subresource_range_index,
                                           const CommandBufferAccessContext& cb_context, vvl::Func command) const;

    std::string BeginRenderingError(const HazardResult& hazard, const syncval_state::DynamicRenderingInfo::Attachment& attachment,
                                    const CommandBufferAccessContext& cb_context, vvl::Func command) const;

    std::string EndRenderingResolveError(const HazardResult& hazard, const VulkanTypedHandle& image_view_handle,
                                         VkResolveModeFlagBits resolve_mode, const CommandBufferAccessContext& cb_context,
                                         vvl::Func command) const;

    std::string EndRenderingStoreError(const HazardResult& hazard, const VulkanTypedHandle& image_view_handle,
                                       VkAttachmentStoreOp store_op, const CommandBufferAccessContext& cb_context,
                                       vvl::Func command) const;

    std::string DrawDispatchImageError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                       const vvl::ImageView& image_view, const vvl::Pipeline& pipeline,
                                       const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                       VkImageLayout image_layout, uint32_t descriptor_binding, uint32_t binding_index,
                                       vvl::Func command) const;

    std::string DrawDispatchTexelBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                             const vvl::BufferView& buffer_view, const vvl::Pipeline& pipeline,
                                             const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                             uint32_t descriptor_binding, uint32_t binding_index, vvl::Func command) const;

    std::string DrawDispatchBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                        const vvl::Buffer& buffer, const vvl::Pipeline& pipeline,
                                        const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                        uint32_t descriptor_binding, uint32_t binding_index, vvl::Func command) const;

    std::string DrawVertexBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                      const vvl::Buffer& vertex_buffer, vvl::Func command) const;

    std::string DrawIndexBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                     const vvl::Buffer& index_buffer, vvl::Func command) const;

    std::string DrawAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                    const vvl::ImageView& attachment_view, vvl::Func command) const;

    std::string ClearColorAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                          const std::string& subpass_attachment_info, vvl::Func command) const;

    std::string ClearDepthStencilAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 const std::string& subpass_attachment_info, VkImageAspectFlagBits aspect,
                                                 vvl::Func command) const;

    std::string PipelineBarrierError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                     uint32_t image_barrier_index, const vvl::Image& image, vvl::Func command) const;

    std::string WaitEventsError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                uint32_t image_barrier_index, const vvl::Image& image, vvl::Func command) const;

    std::string FirstUseError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                              const CommandBufferAccessContext& recorded_context, uint32_t command_buffer_index,
                              VkCommandBuffer recorded_handle, vvl::Func command) const;

    std::string RenderPassResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, uint32_t subpass,
                                       const char* aspect_name, const char* attachment_name, uint32_t src_attachment,
                                       uint32_t dst_attachment, vvl::Func command) const;

    std::string RenderPassLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard, uint32_t subpass, uint32_t attachment,
                                                                VkImageLayout old_layout, VkImageLayout new_layout,
                                                                uint32_t store_resolve_subpass, vvl::Func command) const;

    std::string RenderPassLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                uint32_t subpass, uint32_t attachment, VkImageLayout old_layout,
                                                VkImageLayout new_layout, vvl::Func command) const;

    std::string RenderPassLoadOpVsLayoutTransitionError(const HazardResult& hazard, uint32_t subpass, uint32_t attachment,
                                                        const char* aspect_name, VkAttachmentLoadOp load_op,
                                                        vvl::Func command) const;

    std::string RenderPassLoadOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, uint32_t subpass,
                                      uint32_t attachment, const char* aspect_name, VkAttachmentLoadOp load_op,
                                      vvl::Func command) const;

    std::string RenderPassStoreOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, uint32_t subpass,
                                       uint32_t attachment, const char* aspect_name, const char* store_op_type_name,
                                       VkAttachmentStoreOp store_op, vvl::Func command) const;

    std::string RenderPassColorAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                               const vvl::ImageView& view, uint32_t attachment, vvl::Func command) const;

    std::string RenderPassDepthStencilAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                      const vvl::ImageView& view, bool is_depth, vvl::Func command) const;

    std::string RenderPassFinalLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard,
                                                                     const CommandBufferAccessContext& cb_context, uint32_t subpass,
                                                                     uint32_t attachment, VkImageLayout old_layout,
                                                                     VkImageLayout new_layout, vvl::Func command) const;

    std::string RenderPassFinalLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                     uint32_t subpass, uint32_t attachment, VkImageLayout old_layout,
                                                     VkImageLayout new_layout, vvl::Func command) const;

    std::string PresentError(const HazardResult& hazard, const QueueBatchContext& batch_context, uint32_t present_index,
                             const VulkanTypedHandle& swapchain_handle, uint32_t image_index, const VulkanTypedHandle& image_handle,
                             vvl::Func command) const;

    std::string VideoReferencePictureError(const HazardResult& hazard, uint32_t reference_picture_index,
                                           const CommandBufferAccessContext& cb_context, vvl::Func command) const;

  private:
    void AddCbContextExtraProperties(const CommandBufferAccessContext& cb_context, ResourceUsageTag tag,
                                     ReportKeyValues& key_values) const;

  private:
    vvl::Device& validator_;
    const bool& extra_properties_;
    const bool& pretty_print_extra_;
};

}  // namespace syncval
