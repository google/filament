/* Copyright (c) 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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

#include <vulkan/vulkan.h>

#include "sync/sync_common.h"
#include "sync/sync_access_context.h"
#include "sync/sync_op.h"

class CommandExecutionContext;
struct ClearAttachmentInfo;

struct LastBound;

namespace syncval_state {
enum class AttachmentType { kColor, kDepth, kStencil };

struct DynamicRenderingInfo {
    struct Attachment {
        const vku::safe_VkRenderingAttachmentInfo &info;
        std::shared_ptr<const ImageViewState> view;
        std::shared_ptr<const ImageViewState> resolve_view;
        ImageRangeGen view_gen;
        std::optional<ImageRangeGen> resolve_gen;
        AttachmentType type;

        Attachment(const SyncValidator &state, const vku::safe_VkRenderingAttachmentInfo &info, const AttachmentType type_,
                   const VkOffset3D &offset, const VkExtent3D &extent);

        SyncAccessIndex GetLoadUsage() const;
        SyncAccessIndex GetStoreUsage() const;
        SyncOrdering GetOrdering() const;
        Location GetLocation(const Location &loc, uint32_t index = 0) const;
        bool IsWriteable(const LastBound &last_bound_state) const;
        bool IsValid() const { return view.get(); }
    };

    // attachments store references to this info, so make sure this doesn't get moved around.
    DynamicRenderingInfo(const DynamicRenderingInfo &) = delete;
    DynamicRenderingInfo(DynamicRenderingInfo &&) = delete;
    DynamicRenderingInfo &operator=(const DynamicRenderingInfo &) = delete;
    DynamicRenderingInfo &operator=(DynamicRenderingInfo &&) = delete;

    DynamicRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info);
    ClearAttachmentInfo GetClearAttachmentInfo(const VkClearAttachment &clear_attachment, const VkClearRect &rect) const;
    vku::safe_VkRenderingInfo info;
    std::vector<Attachment> attachments;  // All attachments (with internal typing)
};

struct BeginRenderingCmdState {
    BeginRenderingCmdState(std::shared_ptr<const syncval_state::CommandBuffer> &&cb_state_) : cb_state(std::move(cb_state_)) {}
    void AddRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info);
    const DynamicRenderingInfo &GetRenderingInfo() const;
    std::shared_ptr<const CommandBuffer> cb_state;
    std::unique_ptr<DynamicRenderingInfo> info;
};
}  // namespace syncval_state

void InitSubpassContexts(VkQueueFlags queue_flags, const vvl::RenderPass &rp_state, const AccessContext *external_context,
                         std::vector<AccessContext> &subpass_contexts);

struct ClearAttachmentInfo {
    using ImageViewState = syncval_state::ImageViewState;

    const ImageViewState *view = nullptr;
    VkImageAspectFlags aspects_to_clear = {};
    VkImageSubresourceRange subresource_range{};
    VkOffset3D offset = {};
    VkExtent3D extent = {};
    uint32_t attachment_index = VK_ATTACHMENT_UNUSED;
    uint32_t subpass = 0;

    static VkImageSubresourceRange RestrictSubresourceRange(const VkClearRect &clear_rect, const ImageViewState &view);
    static VkImageAspectFlags GetAspectsToClear(VkImageAspectFlags clear_aspect_mask, const ImageViewState &view);
    ClearAttachmentInfo() = default;
    ClearAttachmentInfo(const VkClearAttachment &clear_attachment, const VkClearRect &rect, const ImageViewState &view_,
                        uint32_t attachment_index_ = VK_ATTACHMENT_UNUSED /* renderpass instance only */,
                        uint32_t subpass_ = 0 /* renderpass instance only */);

    // ClearAttachmentInfo can be invalid for several reasons based on the VkClearAttachment and the rendering
    // attachment state, including some caught by the constructor.  Consumers *must* check validity before use
    bool IsValid() const { return view && (aspects_to_clear != 0U) && (subresource_range.layerCount != 0U); }
    std::string GetSubpassAttachmentText() const;
};

class RenderPassAccessContext {
  public:
    static AttachmentViewGenVector CreateAttachmentViewGen(
        const VkRect2D &render_area, const std::vector<const syncval_state::ImageViewState *> &attachment_views);
    RenderPassAccessContext() : rp_state_(nullptr), render_area_(VkRect2D()), current_subpass_(0) {}
    RenderPassAccessContext(const vvl::RenderPass &rp_state, const VkRect2D &render_area, VkQueueFlags queue_flags,
                            const std::vector<const syncval_state::ImageViewState *> &attachment_views,
                            const AccessContext *external_context);

    static bool ValidateLayoutTransitions(const CommandBufferAccessContext &cb_context, const AccessContext &access_context,
                                          const vvl::RenderPass &rp_state, const VkRect2D &render_area, uint32_t subpass,
                                          const AttachmentViewGenVector &attachment_views, vvl::Func command);

    static bool ValidateLoadOperation(const CommandBufferAccessContext &cb_context, const AccessContext &access_context,
                                      const vvl::RenderPass &rp_state, const VkRect2D &render_area, uint32_t subpass,
                                      const AttachmentViewGenVector &attachment_views, vvl::Func command);

    bool ValidateStoreOperation(const CommandBufferAccessContext &cb_context, vvl::Func command) const;
    bool ValidateResolveOperations(const CommandBufferAccessContext &cb_context, vvl::Func command) const;

    static void UpdateAttachmentResolveAccess(const vvl::RenderPass &rp_state, const AttachmentViewGenVector &attachment_views,
                                              uint32_t subpass, const ResourceUsageTag tag, AccessContext access_context);

    static void UpdateAttachmentStoreAccess(const vvl::RenderPass &rp_state, const AttachmentViewGenVector &attachment_views,
                                            uint32_t subpass, const ResourceUsageTag tag, AccessContext &access_context);

    static void RecordLayoutTransitions(const vvl::RenderPass &rp_state, uint32_t subpass,
                                        const AttachmentViewGenVector &attachment_views, const ResourceUsageTag tag,
                                        AccessContext &access_context);

    bool ValidateDrawSubpassAttachment(const CommandBufferAccessContext &cb_context, vvl::Func command) const;
    void RecordDrawSubpassAttachment(const vvl::CommandBuffer &cmd_buffer, ResourceUsageTag tag);

    uint32_t GetAttachmentIndex(const VkClearAttachment &clear_attachment) const;
    ClearAttachmentInfo GetClearAttachmentInfo(const VkClearAttachment &clear_attachment, const VkClearRect &rect) const;

    bool ValidateNextSubpass(const CommandBufferAccessContext &cb_context, vvl::Func command) const;
    bool ValidateEndRenderPass(const CommandBufferAccessContext &cb_context, vvl::Func command) const;
    bool ValidateFinalSubpassLayoutTransitions(const CommandBufferAccessContext &cb_context, vvl::Func command) const;

    void RecordLayoutTransitions(ResourceUsageTag tag);
    void RecordLoadOperations(ResourceUsageTag tag);
    void RecordBeginRenderPass(ResourceUsageTag tag, ResourceUsageTag load_tag);
    void RecordNextSubpass(ResourceUsageTag store_tag, ResourceUsageTag barrier_tag, ResourceUsageTag load_tag);
    void RecordEndRenderPass(AccessContext *external_context, ResourceUsageTag store_tag, ResourceUsageTag barrier_tag);

    AccessContext &CurrentContext() { return subpass_contexts_[current_subpass_]; }
    const AccessContext &CurrentContext() const { return subpass_contexts_[current_subpass_]; }
    const std::vector<AccessContext> &GetContexts() const { return subpass_contexts_; }
    uint32_t GetCurrentSubpass() const { return current_subpass_; }
    const vvl::RenderPass *GetRenderPassState() const { return rp_state_; }
    AccessContext *CreateStoreResolveProxy() const;

  private:
    const vvl::RenderPass *rp_state_;
    const VkRect2D render_area_;
    uint32_t current_subpass_;
    std::vector<AccessContext> subpass_contexts_;
    AttachmentViewGenVector attachment_views_;
};
