/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "utils/vk_layer_utils.h"

static ShaderObjectStage inline ConvertToShaderObjectStage(VkShaderStageFlagBits stage) {
    if (stage == VK_SHADER_STAGE_VERTEX_BIT) return ShaderObjectStage::VERTEX;
    if (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) return ShaderObjectStage::TESSELLATION_CONTROL;
    if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) return ShaderObjectStage::TESSELLATION_EVALUATION;
    if (stage == VK_SHADER_STAGE_GEOMETRY_BIT) return ShaderObjectStage::GEOMETRY;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) return ShaderObjectStage::FRAGMENT;
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) return ShaderObjectStage::COMPUTE;
    if (stage == VK_SHADER_STAGE_TASK_BIT_EXT) return ShaderObjectStage::TASK;
    if (stage == VK_SHADER_STAGE_MESH_BIT_EXT) return ShaderObjectStage::MESH;

    assert(false);

    return ShaderObjectStage::LAST;
}

// For Traditional RenderPasses, the index is simply the index into the VkRenderPassCreateInfo::pAttachments,
// but for dynamic rendering, there is no "standard" way to map the index, instead we have our own custom indexing and it is not
// obvious at all to the user where it came from
std::string AttachmentInfo::Describe(AttachmentSource source, uint32_t index) const {
    std::ostringstream ss;
    auto type_string = [](Type type) {
        switch (type) {
            case Type::Input:
                return "Input";
            case Type::Color:
                return "Color";
            case Type::ColorResolve:
                return "Color Resolve";
            case Type::DepthStencil:
                return "Depth Stencil";
            case Type::Depth:
                return "Depth";
            case Type::DepthResolve:
                return "Depth Resolve";
            case Type::Stencil:
                return "Stencil";
            case Type::StencilResolve:
                return "Stencil Resolve";
            default:
                break;
        }
        return "Unknown Type";
    };

    if (source == AttachmentSource::DynamicRendering) {
        ss << "VkRenderingInfo::";
        if (type == Type::Color) {
            ss << "pColorAttachments[" << index << "].imageView";
        } else if (type == Type::ColorResolve) {
            // This assumes the caller calculated the correct index with GetDynamicColorResolveAttachmentImageIndex
            ss << "pColorAttachments[" << index << "].resolveImageView";
        } else if (type == Type::Depth) {
            ss << "pDepthAttachment.imageView";
        } else if (type == Type::DepthResolve) {
            ss << "pStencilAttachment.resolveImageView";
        } else if (type == Type::Stencil) {
            ss << "pDepthAttachment.imageView";
        } else if (type == Type::StencilResolve) {
            ss << "pStencilAttachment.resolveImageView";
        }
    } else {
        ss << "VkRenderPassCreateInfo::pAttachments[" << index << "] (" << type_string(type) << ")";
    }
    return ss.str();
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkEventCreateInfo *info) {
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

namespace vvl {

Event::Event(VkEvent handle, const VkEventCreateInfo *create_info)
    : StateObject(handle, kVulkanObjectTypeEvent),
      flags(create_info->flags)
#ifdef VK_USE_PLATFORM_METAL_EXT
      ,
      metal_event_export(GetMetalExport(create_info))
#endif  // VK_USE_PLATFORM_METAL_EXT
{
}

CommandPool::CommandPool(Device &dev, VkCommandPool handle, const VkCommandPoolCreateInfo *create_info, VkQueueFlags flags)
    : StateObject(handle, kVulkanObjectTypeCommandPool),
      dev_data(dev),
      createFlags(create_info->flags),
      queueFamilyIndex(create_info->queueFamilyIndex),
      queue_flags(flags),
      unprotected((create_info->flags & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) == 0) {}

void CommandPool::Allocate(const VkCommandBufferAllocateInfo *allocate_info, const VkCommandBuffer *command_buffers) {
    for (uint32_t i = 0; i < allocate_info->commandBufferCount; i++) {
        auto new_cb = dev_data.CreateCmdBufferState(command_buffers[i], allocate_info, this);
        commandBuffers.emplace(command_buffers[i], new_cb.get());
        dev_data.Add(std::move(new_cb));
    }
}

void CommandPool::Free(uint32_t count, const VkCommandBuffer *command_buffers) {
    for (uint32_t i = 0; i < count; i++) {
        auto iter = commandBuffers.find(command_buffers[i]);
        if (iter != commandBuffers.end()) {
            dev_data.Destroy<CommandBuffer>(iter->first);
            commandBuffers.erase(iter);
        }
    }
}

void CommandPool::Reset(const Location &loc) {
    for (auto &entry : commandBuffers) {
        auto guard = entry.second->WriteLock();
        entry.second->Reset(loc);
    }
}

void CommandPool::Destroy() {
    for (auto &entry : commandBuffers) {
        dev_data.Destroy<CommandBuffer>(entry.first);
    }
    commandBuffers.clear();
    StateObject::Destroy();
}

void CommandBuffer::SetActiveSubpass(uint32_t subpass) {
    active_subpass_ = subpass;
    // Always reset stored rasterization samples count
    active_subpass_sample_count_ = std::nullopt;
}

CommandBuffer::CommandBuffer(Device &dev, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *allocate_info,
                             const vvl::CommandPool *pool)
    : RefcountedStateObject(handle, kVulkanObjectTypeCommandBuffer),
      allocate_info(*allocate_info),
      command_pool(pool),
      dev_data(dev),
      unprotected(pool->unprotected),
      lastBound({*this, *this, *this}) {
    ResetCBState();
}

// Get the image viewstate for a given framebuffer attachment
vvl::ImageView *CommandBuffer::GetActiveAttachmentImageViewState(uint32_t index) {
    assert(!active_attachments.empty() && index != VK_ATTACHMENT_UNUSED && (index < active_attachments.size()));
    return active_attachments[index].image_view;
}

// Get the image viewstate for a given framebuffer attachment
const vvl::ImageView *CommandBuffer::GetActiveAttachmentImageViewState(uint32_t index) const {
    if (active_attachments.empty() || index == VK_ATTACHMENT_UNUSED || (index >= active_attachments.size())) {
        return nullptr;
    }
    return active_attachments[index].image_view;
}

void CommandBuffer::AddChild(std::shared_ptr<StateObject> &child_node) {
    assert(child_node);
    if (child_node->AddParent(this)) {
        object_bindings.insert(child_node);
    }
}

void CommandBuffer::RemoveChild(std::shared_ptr<StateObject> &child_node) {
    assert(child_node);
    child_node->RemoveParent(this);
    object_bindings.erase(child_node);
}

// Reset the command buffer state
// Maintain the createInfo and set state to CB_NEW, but clear all other state
void CommandBuffer::ResetCBState() {
    // Remove object bindings
    for (const auto &obj : object_bindings) {
        obj->RemoveParent(this);
    }
    object_bindings.clear();
    broken_bindings.clear();

    // Reset CB state (note that createInfo is not cleared)
    memset(&beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    memset(&inheritanceInfo, 0, sizeof(VkCommandBufferInheritanceInfo));
    has_draw_cmd = false;
    has_dispatch_cmd = false;
    has_trace_rays_cmd = false;
    has_build_as_cmd = false;
    hasRenderPassInstance = false;
    suspendsRenderPassInstance = false;
    resumesRenderPassInstance = false;
    state = CbState::New;
    command_count = 0;
    submitCount = 0;
    image_layout_change_count = 1;  // Start at 1. 0 is insert value for validation cache versions, s.t. new == dirty

    dynamic_state_status.cb.reset();
    dynamic_state_status.pipeline.reset();
    dynamic_state_status.rtx_stack_size_cb = false;
    dynamic_state_status.rtx_stack_size_pipeline = false;
    CBDynamicFlags all;
    dynamic_state_value.reset(all.set());
    memset(&invalidated_state_pipe, 0, sizeof(VkPipeline) * CB_DYNAMIC_STATE_STATUS_NUM);

    inheritedViewportDepths.clear();
    usedViewportScissorCount = 0;
    pipelineStaticViewportCount = 0;
    pipelineStaticScissorCount = 0;
    viewportMask = 0;
    viewportWithCountMask = 0;
    scissorMask = 0;
    scissorWithCountMask = 0;
    trashedViewportMask = 0;
    trashedScissorMask = 0;
    trashedViewportCount = false;
    trashedScissorCount = false;
    usedDynamicViewportCount = false;
    usedDynamicScissorCount = false;
    dirtyStaticState = false;

    active_render_pass = nullptr;
    sample_locations_begin_info = nullptr;
    attachment_source = AttachmentSource::Empty;
    active_attachments.clear();
    active_subpasses.clear();
    active_color_attachments_index.clear();
    has_render_pass_striped = false;
    striped_count = 0;
    activeSubpassContents = VK_SUBPASS_CONTENTS_INLINE;
    SetActiveSubpass(0);
    rendering_attachments.Reset();
    waitedEvents.clear();
    events.clear();
    writeEventsBeforeWait.clear();
    activeQueries.clear();
    startedQueries.clear();
    renderPassQueries.clear();
    image_layout_map.clear();
    aliased_image_layout_map.clear();
    current_vertex_buffer_binding_info.clear();
    primaryCommandBuffer = VK_NULL_HANDLE;
    linkedCommandBuffers.clear();
    queue_submit_functions.clear();
    queue_submit_functions_after_render_pass.clear();
    cmd_execute_commands_functions.clear();
    event_updates.clear();
    query_updates.clear();

    for (auto &item : lastBound) {
        item.Reset();
    }
    activeFramebuffer = VK_NULL_HANDLE;
    index_buffer_binding.reset();

    qfo_transfer_image_barriers.Reset();
    qfo_transfer_buffer_barriers.Reset();

    // Clean up video specific states
    bound_video_session = nullptr;
    bound_video_session_parameters = nullptr;
    bound_video_picture_resources.clear();
    video_encode_quality_level.reset();
    video_session_updates.clear();

    // Clean up the label data
    debug_label.Reset();
    label_stack_depth_ = 0;
    label_commands_.clear();

    push_constant_data_chunks.clear();
    push_constant_latest_used_layout.fill(VK_NULL_HANDLE);
    push_constant_ranges_layout.reset();

    nesting_level = 0;

    transform_feedback_active = false;
    transform_feedback_buffers_bound = 0;

    // Clean up the label data
    dev_data.debug_report->ResetCmdDebugUtilsLabel(VkHandle());

    push_constant_data_chunks.clear();
    push_constant_latest_used_layout.fill(VK_NULL_HANDLE);
    push_constant_ranges_layout = nullptr;
}

void CommandBuffer::Reset(const Location &loc) {
    ResetCBState();
    // Remove reverse command buffer links.
    Invalidate(true);
}

// Track which resources are in-flight by atomically incrementing their "in_use" count
void CommandBuffer::IncrementResources() {
    submitCount++;
}

// Discussed in details in https://github.com/KhronosGroup/Vulkan-Docs/issues/1081
// Internal discussion and CTS were written to prove that this is not called after an incompatible vkCmdBindPipeline
// "Binding a pipeline with a layout that is not compatible with the push constant layout does not disturb the push constant values"
//
// vkCmdBindDescriptorSet has nothing to do with push constants and don't need to call this after neither
//
// Part of this assumes apps at draw/dispatch/traceRays/etc time will have it properly compatible or else other VU will be triggered
void CommandBuffer::ResetPushConstantRangesLayoutIfIncompatible(const vvl::PipelineLayout &pipeline_layout_state) {
    if (push_constant_ranges_layout == pipeline_layout_state.push_constant_ranges_layout) {
        return;
    }

    push_constant_data_chunks.clear();
    push_constant_latest_used_layout.fill(VK_NULL_HANDLE);
    push_constant_ranges_layout = pipeline_layout_state.push_constant_ranges_layout;
}

void CommandBuffer::Destroy() {
    // Remove the cb debug labels
    dev_data.debug_report->EraseCmdDebugUtilsLabel(VkHandle());
    {
        auto guard = WriteLock();
        ResetCBState();
    }
    StateObject::Destroy();
}

void CommandBuffer::NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {
    {
        auto guard = WriteLock();
        assert(!invalid_nodes.empty());
        // Save all of the vulkan handles between the command buffer and the now invalid node
        LogObjectList log_list;
        for (auto &obj : invalid_nodes) {
            log_list.add(obj->Handle());
        }

        bool found_invalid = false;
        for (auto &obj : invalid_nodes) {
            // Only record a broken binding if one of the nodes in the invalid chain is still
            // being tracked by the command buffer. This is to try to avoid race conditions
            // caused by separate CommandBuffer and StateObject::parent_nodes locking.
            if (object_bindings.erase(obj)) {
                obj->RemoveParent(this);
                found_invalid = true;
            }
            switch (obj->Type()) {
                case kVulkanObjectTypeCommandBuffer:
                    if (unlink) {
                        linkedCommandBuffers.erase(static_cast<CommandBuffer *>(obj.get()));
                    }
                    break;
                case kVulkanObjectTypeImage:
                    if (unlink) {
                        image_layout_map.erase(obj->Handle().Cast<VkImage>());
                    }
                    break;
                default:
                    break;
            }
        }
        if (found_invalid) {
            if (state == CbState::Recording) {
                state = CbState::InvalidIncomplete;
            } else if (state == CbState::Recorded) {
                state = CbState::InvalidComplete;
            }
            broken_bindings.emplace(invalid_nodes[0]->Handle(), log_list);
        }
    }
    StateObject::NotifyInvalidate(invalid_nodes, unlink);
}

const CommandBuffer::ImageLayoutMap &CommandBuffer::GetImageLayoutMap() const { return image_layout_map; }

// The const variant only need the image as it is the key for the map
std::shared_ptr<const ImageLayoutRegistry> CommandBuffer::GetImageLayoutRegistry(VkImage image) const {
    auto it = image_layout_map.find(image);
    if (it == image_layout_map.cend()) {
        return nullptr;
    }
    return it->second;
}

// The non-const variant only needs the image state, as the factory requires it to construct a new entry
std::shared_ptr<ImageLayoutRegistry> CommandBuffer::GetOrCreateImageLayoutRegistry(const vvl::Image &image_state) {
    // Make sure we don't create a nullptr keyed entry for a zombie Image
    if (image_state.Destroyed() || !image_state.layout_range_map) {
        return nullptr;
    }
    auto iter = image_layout_map.find(image_state.VkHandle());
    if (iter != image_layout_map.end() && iter->second && image_state.GetId() == iter->second->GetImageId()) {
        return iter->second;
    }
    std::shared_ptr<ImageLayoutRegistry> image_layout_registry;
    if (image_state.CanAlias()) {
        // Aliasing images need to share the same local layout map.
        // Since they use the same global layout state, use it as a key
        // for the local state. We don't need a lock on the global range
        // map to do a lookup based on its pointer.
        const auto *global_layout_map = image_state.layout_range_map.get();
        auto alias_iter = aliased_image_layout_map.find(global_layout_map);
        if (alias_iter != aliased_image_layout_map.end()) {
            image_layout_registry = alias_iter->second;
        } else {
            image_layout_registry = std::make_shared<ImageLayoutRegistry>(image_state);
            // Save the local layout map for the next aliased image.
            // The global layout map pointer is only used as a key into the local lookup
            // table so it doesn't need to be locked.
            aliased_image_layout_map.emplace(global_layout_map, image_layout_registry);
        }

    } else {
        image_layout_registry = std::make_shared<ImageLayoutRegistry>(image_state);
    }
    if (iter != image_layout_map.end()) {
        // overwrite the stale entry
        iter->second = image_layout_registry;
    } else {
        // add a new entry
        image_layout_map.insert({image_state.VkHandle(), image_layout_registry});
    }
    return image_layout_registry;
}

static bool SetQueryState(const QueryObject &object, QueryState value, QueryMap *localQueryToStateMap) {
    (*localQueryToStateMap)[object] = value;
    return false;
}

void CommandBuffer::BeginQuery(const QueryObject &query_obj) {
    activeQueries.insert(query_obj);
    startedQueries.insert(query_obj);
    query_updates.emplace_back([query_obj](CommandBuffer &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                           uint32_t perfQueryPass, QueryMap *localQueryToStateMap) {
        SetQueryState(QueryObject(query_obj, perfQueryPass), QUERYSTATE_RUNNING, localQueryToStateMap);
        return false;
    });
    updatedQueries.insert(query_obj);
    if (query_obj.inside_render_pass) {
        renderPassQueries.insert(query_obj);
    }
}

void CommandBuffer::EndQuery(const QueryObject &query_obj) {
    activeQueries.erase(query_obj);
    query_updates.emplace_back([query_obj](CommandBuffer &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                           uint32_t perfQueryPass, QueryMap *localQueryToStateMap) {
        return SetQueryState(QueryObject(query_obj, perfQueryPass), QUERYSTATE_ENDED, localQueryToStateMap);
    });
    updatedQueries.insert(query_obj);
    if (query_obj.inside_render_pass) {
        renderPassQueries.erase(query_obj);
    }
}

bool CommandBuffer::UpdatesQuery(const QueryObject &query_obj) const {
    // Clear out the perf_pass from the caller because it isn't known when the command buffer is recorded.
    auto key = query_obj;
    key.perf_pass = 0;
    for (auto *sub_cb : linkedCommandBuffers) {
        if (sub_cb->updatedQueries.find(key) != sub_cb->updatedQueries.end()) {
            return true;
        }
    }
    return updatedQueries.find(key) != updatedQueries.end();
}

static bool SetQueryStateMulti(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t perfPass, QueryState value,
                               QueryMap *localQueryToStateMap) {
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryObject query_obj = {queryPool, firstQuery + i, perfPass};
        (*localQueryToStateMap)[query_obj] = value;
    }
    return false;
}

void CommandBuffer::EndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    for (uint32_t slot = firstQuery; slot < (firstQuery + queryCount); slot++) {
        QueryObject query_obj = {queryPool, slot};
        activeQueries.erase(query_obj);
        updatedQueries.insert(query_obj);
    }
    query_updates.emplace_back([queryPool, firstQuery, queryCount](CommandBuffer &cb_state_arg, bool do_validate,
                                                                   VkQueryPool &firstPerfQueryPool, uint32_t perfQueryPass,
                                                                   QueryMap *localQueryToStateMap) {
        return SetQueryStateMulti(queryPool, firstQuery, queryCount, perfQueryPass, QUERYSTATE_ENDED, localQueryToStateMap);
    });
}

void CommandBuffer::ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    for (uint32_t slot = firstQuery; slot < (firstQuery + queryCount); slot++) {
        QueryObject query_obj = {queryPool, slot};
        updatedQueries.insert(query_obj);
    }

    query_updates.emplace_back([queryPool, firstQuery, queryCount](CommandBuffer &cb_state_arg, bool do_validate,
                                                                   VkQueryPool &firstPerfQueryPool, uint32_t perfQueryPass,
                                                                   QueryMap *localQueryToStateMap) {
        return SetQueryStateMulti(queryPool, firstQuery, queryCount, perfQueryPass, QUERYSTATE_RESET, localQueryToStateMap);
    });
}

void CommandBuffer::UpdateSubpassAttachments() {
    ASSERT_AND_RETURN(active_render_pass);
    const auto &subpass = active_render_pass->create_info.pSubpasses[GetActiveSubpass()];
    assert(active_subpasses.size() == active_attachments.size());

    for (uint32_t index = 0; index < subpass.inputAttachmentCount; ++index) {
        const uint32_t attachment_index = subpass.pInputAttachments[index].attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::Input;
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            active_subpasses[attachment_index].layout = subpass.pInputAttachments[index].layout;
            active_subpasses[attachment_index].aspectMask = subpass.pInputAttachments[index].aspectMask;
        }
    }

    for (uint32_t index = 0; index < subpass.colorAttachmentCount; ++index) {
        const uint32_t attachment_index = subpass.pColorAttachments[index].attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::Color;
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            active_subpasses[attachment_index].layout = subpass.pColorAttachments[index].layout;
            active_subpasses[attachment_index].aspectMask = subpass.pColorAttachments[index].aspectMask;
            active_color_attachments_index.insert(index);
        }
        if (subpass.pResolveAttachments) {
            const uint32_t attachment_index2 = subpass.pResolveAttachments[index].attachment;
            if (attachment_index2 != VK_ATTACHMENT_UNUSED) {
                active_attachments[attachment_index2].type = AttachmentInfo::Type::ColorResolve;
                active_subpasses[attachment_index2].used = true;
                active_subpasses[attachment_index2].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                active_subpasses[attachment_index2].layout = subpass.pResolveAttachments[index].layout;
                active_subpasses[attachment_index2].aspectMask = subpass.pResolveAttachments[index].aspectMask;
            }
        }
    }

    if (subpass.pDepthStencilAttachment) {
        const uint32_t attachment_index = subpass.pDepthStencilAttachment->attachment;
        if (attachment_index != VK_ATTACHMENT_UNUSED) {
            active_attachments[attachment_index].type = AttachmentInfo::Type::DepthStencil;
            active_subpasses[attachment_index].used = true;
            active_subpasses[attachment_index].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            active_subpasses[attachment_index].layout = subpass.pDepthStencilAttachment->layout;
            active_subpasses[attachment_index].aspectMask = subpass.pDepthStencilAttachment->aspectMask;
        }
    }
}

// For non Dynamic Renderpass we update the attachments
void CommandBuffer::UpdateAttachmentsView(const VkRenderPassBeginInfo *pRenderPassBegin) {
    const bool imageless = (activeFramebuffer->create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0;
    const VkRenderPassAttachmentBeginInfo *attachment_info_struct = nullptr;
    if (pRenderPassBegin) attachment_info_struct = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);

    for (uint32_t i = 0; i < active_attachments.size(); ++i) {
        if (imageless) {
            if (attachment_info_struct && i < attachment_info_struct->attachmentCount) {
                active_attachments[i].image_view = dev_data.Get<vvl::ImageView>(attachment_info_struct->pAttachments[i]).get();
            }
        } else {
            active_attachments[i].image_view = activeFramebuffer->attachments_view_state[i].get();
        }
    }

    // While updating the subpass we will set the active_attachments type
    UpdateSubpassAttachments();
}

void CommandBuffer::BeginRenderPass(Func command, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassContents contents) {
    RecordCmd(command);
    activeFramebuffer = dev_data.Get<vvl::Framebuffer>(pRenderPassBegin->framebuffer);
    active_render_pass = dev_data.Get<vvl::RenderPass>(pRenderPassBegin->renderPass);
    render_area = pRenderPassBegin->renderArea;
    SetActiveSubpass(0);
    activeSubpassContents = contents;
    renderPassQueries.clear();

    // Connect this RP to cmdBuffer
    if (!dev_data.disabled[command_buffer_state]) {
        AddChild(active_render_pass);
    }

    sample_locations_begin_info = vku::FindStructInPNextChain<VkRenderPassSampleLocationsBeginInfoEXT>(pRenderPassBegin->pNext);

    auto rp_striped_begin = vku::FindStructInPNextChain<VkRenderPassStripeBeginInfoARM>(pRenderPassBegin->pNext);
    if (rp_striped_begin) {
        has_render_pass_striped = true;
        striped_count += rp_striped_begin->stripeInfoCount;
    }

    // Spec states that after BeginRenderPass all resources should be rebound
    if (active_render_pass->has_multiview_enabled) {
        UnbindResources();
    }

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(pRenderPassBegin->pNext);
    render_pass_device_mask = chained_device_group_struct ? chained_device_group_struct->deviceMask : initial_device_mask;

    attachment_source = AttachmentSource::RenderPass;
    active_subpasses.clear();
    active_attachments.clear();

    if (activeFramebuffer) {
        active_subpasses.resize(activeFramebuffer->create_info.attachmentCount);
        active_attachments.resize(activeFramebuffer->create_info.attachmentCount);
        UpdateAttachmentsView(pRenderPassBegin);

        // Connect this framebuffer and its children to this cmdBuffer
        AddChild(activeFramebuffer);
    }
}

void CommandBuffer::NextSubpass(Func command, VkSubpassContents contents) {
    RecordCmd(command);
    SetActiveSubpass(GetActiveSubpass() + 1);
    activeSubpassContents = contents;
    ASSERT_AND_RETURN(active_render_pass);

    if (activeFramebuffer) {
        active_subpasses.clear();
        active_subpasses.resize(activeFramebuffer->create_info.attachmentCount);

        if (GetActiveSubpass() < active_render_pass->create_info.subpassCount) {
            UpdateSubpassAttachments();
        }
    }

    // Spec states that after NextSubpass all resources should be rebound
    if (active_render_pass->has_multiview_enabled) {
        UnbindResources();
    }
}

void CommandBuffer::EndRenderPass(Func command) {
    RecordCmd(command);
    active_render_pass = nullptr;
    attachment_source = AttachmentSource::Empty;
    active_attachments.clear();
    active_subpasses.clear();
    active_color_attachments_index.clear();
    SetActiveSubpass(0);
    activeFramebuffer = VK_NULL_HANDLE;
    sample_locations_begin_info = nullptr;
}

void CommandBuffer::BeginRendering(Func command, const VkRenderingInfo *pRenderingInfo) {
    RecordCmd(command);
    active_render_pass = std::make_shared<vvl::RenderPass>(pRenderingInfo, true);
    render_area = pRenderingInfo->renderArea;
    renderPassQueries.clear();

    rendering_attachments.Reset();
    rendering_attachments.color_locations.resize(pRenderingInfo->colorAttachmentCount);
    rendering_attachments.color_indexes.resize(pRenderingInfo->colorAttachmentCount);

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(pRenderingInfo->pNext);
    render_pass_device_mask = chained_device_group_struct ? chained_device_group_struct->deviceMask : initial_device_mask;

    auto rp_striped_begin = vku::FindStructInPNextChain<VkRenderPassStripeBeginInfoARM>(pRenderingInfo->pNext);
    if (rp_striped_begin) {
        has_render_pass_striped = true;
        striped_count += rp_striped_begin->stripeInfoCount;
    }

    activeSubpassContents = ((pRenderingInfo->flags & VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT)
                                 ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                 : VK_SUBPASS_CONTENTS_INLINE);

    // Handle flags for dynamic rendering
    if (!hasRenderPassInstance && pRenderingInfo->flags & VK_RENDERING_RESUMING_BIT) {
        resumesRenderPassInstance = true;
    }
    suspendsRenderPassInstance = (pRenderingInfo->flags & VK_RENDERING_SUSPENDING_BIT) > 0;
    hasRenderPassInstance = true;

    attachment_source = AttachmentSource::DynamicRendering;
    active_attachments.clear();
    // add 2 for the Depth and Stencil
    // multiple by 2 because every attachment might have a resolve
    // Currently reserve the maximum possible size for |active_attachments| so when looping, we NEED to check for null
    uint32_t attachment_count = (pRenderingInfo->colorAttachmentCount + 2) * 2;

    active_attachments.resize(attachment_count);

    for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
        active_color_attachments_index.insert(i);

        // Default from spec
        rendering_attachments.color_locations[i] = i;
        rendering_attachments.color_indexes[i] = i;

        if (pRenderingInfo->pColorAttachments[i].imageView != VK_NULL_HANDLE) {
            auto &color_attachment = active_attachments[GetDynamicColorAttachmentImageIndex(i)];
            color_attachment.image_view = dev_data.Get<vvl::ImageView>(pRenderingInfo->pColorAttachments[i].imageView).get();
            color_attachment.type = AttachmentInfo::Type::Color;
            if (pRenderingInfo->pColorAttachments[i].resolveMode != VK_RESOLVE_MODE_NONE &&
                pRenderingInfo->pColorAttachments[i].resolveImageView != VK_NULL_HANDLE) {
                auto &resolve_attachment = active_attachments[GetDynamicColorResolveAttachmentImageIndex(i)];
                resolve_attachment.image_view = dev_data.Get<vvl::ImageView>(pRenderingInfo->pColorAttachments[i].imageView).get();
                resolve_attachment.type = AttachmentInfo::Type::ColorResolve;
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
        auto &depth_attachment = active_attachments[GetDynamicDepthAttachmentImageIndex()];
        depth_attachment.image_view = dev_data.Get<vvl::ImageView>(pRenderingInfo->pDepthAttachment->imageView).get();
        depth_attachment.type = AttachmentInfo::Type::Depth;
        if (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE &&
            pRenderingInfo->pDepthAttachment->resolveImageView != VK_NULL_HANDLE) {
            auto &resolve_attachment = active_attachments[GetDynamicDepthResolveAttachmentImageIndex()];
            resolve_attachment.image_view = dev_data.Get<vvl::ImageView>(pRenderingInfo->pDepthAttachment->imageView).get();
            resolve_attachment.type = AttachmentInfo::Type::DepthResolve;
        }
    }

    if (pRenderingInfo->pStencilAttachment && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
        auto &stencil_attachment = active_attachments[GetDynamicStencilAttachmentImageIndex()];
        stencil_attachment.image_view = dev_data.Get<vvl::ImageView>(pRenderingInfo->pStencilAttachment->imageView).get();
        stencil_attachment.type = AttachmentInfo::Type::Stencil;
        if (pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE &&
            pRenderingInfo->pStencilAttachment->resolveImageView != VK_NULL_HANDLE) {
            auto &resolve_attachment = active_attachments[GetDynamicStencilResolveAttachmentImageIndex()];
            resolve_attachment.image_view = dev_data.Get<vvl::ImageView>(pRenderingInfo->pStencilAttachment->imageView).get();
            resolve_attachment.type = AttachmentInfo::Type::StencilResolve;
        }
    }
}

void CommandBuffer::EndRendering(Func command) {
    RecordCmd(command);
    active_render_pass = nullptr;
    active_color_attachments_index.clear();
}

void CommandBuffer::BeginVideoCoding(const VkVideoBeginCodingInfoKHR *pBeginInfo) {
    RecordCmd(Func::vkCmdBeginVideoCodingKHR);
    bound_video_session = dev_data.Get<vvl::VideoSession>(pBeginInfo->videoSession);
    bound_video_session_parameters = dev_data.Get<vvl::VideoSessionParameters>(pBeginInfo->videoSessionParameters);

    if (bound_video_session) {
        // Connect this video session to cmdBuffer
        if (!dev_data.disabled[command_buffer_state]) {
            AddChild(bound_video_session);
        }
    }

    if (bound_video_session_parameters) {
        // Connect this video session parameters object to cmdBuffer
        if (!dev_data.disabled[command_buffer_state]) {
            AddChild(bound_video_session_parameters);
        }
    }

    if (bound_video_session->IsEncode()) {
        video_encode_rate_control_state = VideoEncodeRateControlState(bound_video_session->GetCodecOp(), pBeginInfo);
        video_encode_quality_level.reset();
    }

    if (pBeginInfo->referenceSlotCount > 0) {
        size_t deactivated_slot_count = 0;

        for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
            // Initialize the set of bound video picture resources
            if (pBeginInfo->pReferenceSlots[i].pPictureResource != nullptr) {
                int32_t slot_index = pBeginInfo->pReferenceSlots[i].slotIndex;
                vvl::VideoPictureResource res(dev_data, *pBeginInfo->pReferenceSlots[i].pPictureResource);
                bound_video_picture_resources.emplace(std::make_pair(res, slot_index));
            }

            if (pBeginInfo->pReferenceSlots[i].slotIndex >= 0 && pBeginInfo->pReferenceSlots[i].pPictureResource == nullptr) {
                deactivated_slot_count++;
            }
        }

        if (deactivated_slot_count > 0) {
            std::vector<int32_t> deactivated_slots{};
            deactivated_slots.reserve(deactivated_slot_count);
            for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
                if (pBeginInfo->pReferenceSlots[i].slotIndex >= 0 && pBeginInfo->pReferenceSlots[i].pPictureResource == nullptr) {
                    deactivated_slots.emplace_back(pBeginInfo->pReferenceSlots[i].slotIndex);
                }
            }

            // Enqueue submission time DPB slot deactivation
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [deactivated_slots](const Device &dev_data, const vvl::VideoSession *vs_state,
                                    vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    for (const auto &slot_index : deactivated_slots) {
                        dev_state.Deactivate(slot_index);
                    }
                    return false;
                });
        }
    }
}

void CommandBuffer::EndVideoCoding(const VkVideoEndCodingInfoKHR *pEndCodingInfo) {
    RecordCmd(Func::vkCmdEndVideoCodingKHR);
    bound_video_session = nullptr;
    bound_video_session_parameters = nullptr;
    bound_video_picture_resources.clear();
    video_encode_quality_level.reset();
}

void CommandBuffer::ControlVideoCoding(const VkVideoCodingControlInfoKHR *pControlInfo) {
    RecordCmd(Func::vkCmdControlVideoCodingKHR);

    if (pControlInfo && bound_video_session) {
        if (pControlInfo->flags & VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR) {
            // Remove DPB slot index association for bound video picture resources
            for (auto &binding : bound_video_picture_resources) {
                binding.second = -1;
            }

            // Enqueue submission time video session state reset/initialization
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [](const Device &dev_data, const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
                   bool do_validate) {
                    dev_state.Reset();
                    return false;
                });
        }

        if (bound_video_session->IsEncode() && pControlInfo->flags & VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR) {
            auto state = VideoEncodeRateControlState(bound_video_session->GetCodecOp(), pControlInfo);
            if (state) {
                video_encode_rate_control_state = state;

                // Enqueue rate control specific device state changes
                video_session_updates[bound_video_session->VkHandle()].emplace_back(
                    [state](const Device &dev_data, const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
                            bool do_validate) {
                        dev_state.SetRateControlState(state);
                        return false;
                    });
            }
        }

        if (bound_video_session->IsEncode() && pControlInfo->flags & VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR) {
            auto quality_level_info = vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(pControlInfo->pNext);
            if (quality_level_info != nullptr) {
                uint32_t quality_level = quality_level_info->qualityLevel;
                video_encode_quality_level = quality_level;

                // Enqueue encode quality level device state change
                video_session_updates[bound_video_session->VkHandle()].emplace_back(
                    [quality_level](const Device &dev_data, const vvl::VideoSession *vs_state,
                                    vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                        dev_state.SetEncodeQualityLevel(quality_level);
                        return false;
                    });
            }
        }
    }
}

void vvl::CommandBuffer::EnqueueUpdateVideoInlineQueries(const VkVideoInlineQueryInfoKHR &query_info) {
    query_updates.emplace_back([query_info](vvl::CommandBuffer &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                            uint32_t perfQueryPass, QueryMap *localQueryToStateMap) {
        for (uint32_t i = 0; i < query_info.queryCount; i++) {
            SetQueryState(QueryObject(query_info.queryPool, query_info.firstQuery + i), QUERYSTATE_ENDED, localQueryToStateMap);
        }
        return false;
    });
    for (uint32_t i = 0; i < query_info.queryCount; i++) {
        updatedQueries.insert(QueryObject(query_info.queryPool, query_info.firstQuery + i));
    }
}

void CommandBuffer::DecodeVideo(const VkVideoDecodeInfoKHR *pDecodeInfo) {
    RecordCmd(Func::vkCmdDecodeVideoKHR);

    if (bound_video_session && pDecodeInfo) {
        if (pDecodeInfo->pSetupReferenceSlot && pDecodeInfo->pSetupReferenceSlot->pPictureResource) {
            vvl::VideoReferenceSlot setup_slot(dev_data, *bound_video_session->profile, *pDecodeInfo->pSetupReferenceSlot);

            // Update bound video picture resource DPB slot index association
            bound_video_picture_resources[setup_slot.resource] = setup_slot.index;

            // Enqueue submission time reference slot setup or invalidation
            bool reference_setup_requested = bound_video_session->ReferenceSetupRequested(*pDecodeInfo);
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [setup_slot, reference_setup_requested](const Device &dev_data, const vvl::VideoSession *vs_state,
                                                        vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    if (reference_setup_requested) {
                        dev_state.Activate(setup_slot.index, setup_slot.picture_id, setup_slot.resource);
                    } else {
                        dev_state.Invalidate(setup_slot.index, setup_slot.picture_id);
                    }
                    return false;
                });
        }

        // Update active query indices
        for (auto &query : activeQueries) {
            uint32_t op_count = bound_video_session->GetVideoDecodeOperationCount(pDecodeInfo);
            query.active_query_index += op_count;
        }

        // Update inline queries
        if (bound_video_session->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
            const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pDecodeInfo->pNext);
            if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
                EnqueueUpdateVideoInlineQueries(*inline_query_info);
            }
        }
    }
}

void vvl::CommandBuffer::EncodeVideo(const VkVideoEncodeInfoKHR *pEncodeInfo) {
    RecordCmd(Func::vkCmdEncodeVideoKHR);

    if (bound_video_session && pEncodeInfo) {
        if (pEncodeInfo->pSetupReferenceSlot && pEncodeInfo->pSetupReferenceSlot->pPictureResource) {
            vvl::VideoReferenceSlot setup_slot(dev_data, *bound_video_session->profile, *pEncodeInfo->pSetupReferenceSlot);

            // Update bound video picture resource DPB slot index association
            bound_video_picture_resources[setup_slot.resource] = setup_slot.index;

            // Enqueue submission time reference slot setup or invalidation
            bool reference_setup_requested = bound_video_session->ReferenceSetupRequested(*pEncodeInfo);
            video_session_updates[bound_video_session->VkHandle()].emplace_back(
                [setup_slot, reference_setup_requested](const Device &dev_data, const vvl::VideoSession *vs_state,
                                                        vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    if (reference_setup_requested) {
                        dev_state.Activate(setup_slot.index, setup_slot.picture_id, setup_slot.resource);
                    } else {
                        dev_state.Invalidate(setup_slot.index, setup_slot.picture_id);
                    }
                    return false;
                });
        }

        // Update active query indices
        for (auto &query : activeQueries) {
            uint32_t op_count = bound_video_session->GetVideoEncodeOperationCount(pEncodeInfo);
            query.active_query_index += op_count;
        }

        // Update inline queries
        if (bound_video_session->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
            const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pEncodeInfo->pNext);
            if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
                EnqueueUpdateVideoInlineQueries(*inline_query_info);
            }
        }
    }
}

void CommandBuffer::Begin(const VkCommandBufferBeginInfo *pBeginInfo) {
    if (CbState::Recorded == state || CbState::InvalidComplete == state) {
        Location loc(Func::vkBeginCommandBuffer);
        Reset(loc);
    }

    // Set updated state here in case implicit reset occurs above
    state = CbState::Recording;
    ASSERT_AND_RETURN(pBeginInfo);
    beginInfo = *pBeginInfo;
    if (beginInfo.pInheritanceInfo && IsSecondary()) {
        inheritanceInfo = *(beginInfo.pInheritanceInfo);
        beginInfo.pInheritanceInfo = &inheritanceInfo;
        // If we are a secondary command-buffer and inheriting.  Update the items we should inherit.
        if (beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
            if (beginInfo.pInheritanceInfo->renderPass) {
                active_render_pass = dev_data.Get<vvl::RenderPass>(beginInfo.pInheritanceInfo->renderPass);
                SetActiveSubpass(beginInfo.pInheritanceInfo->subpass);

                if (beginInfo.pInheritanceInfo->framebuffer) {
                    activeFramebuffer = dev_data.Get<vvl::Framebuffer>(beginInfo.pInheritanceInfo->framebuffer);
                    attachment_source = AttachmentSource::Inheritance;
                    active_subpasses.clear();
                    active_attachments.clear();

                    if (activeFramebuffer) {
                        active_subpasses.resize(activeFramebuffer->create_info.attachmentCount);
                        active_attachments.resize(activeFramebuffer->create_info.attachmentCount);
                        UpdateAttachmentsView(nullptr);

                        // Connect this framebuffer and its children to this cmdBuffer
                        if (!dev_data.disabled[command_buffer_state]) {
                            AddChild(activeFramebuffer);
                        }
                    }
                }
            } else {
                auto inheritance_rendering_info =
                    vku::FindStructInPNextChain<VkCommandBufferInheritanceRenderingInfo>(beginInfo.pInheritanceInfo->pNext);
                if (inheritance_rendering_info) {
                    active_render_pass = std::make_shared<vvl::RenderPass>(inheritance_rendering_info);
                }
            }

            // Check for VkCommandBufferInheritanceViewportScissorInfoNV (VK_NV_inherited_viewport_scissor)
            auto p_inherited_viewport_scissor_info =
                vku::FindStructInPNextChain<VkCommandBufferInheritanceViewportScissorInfoNV>(beginInfo.pInheritanceInfo->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && p_inherited_viewport_scissor_info->viewportScissor2D) {
                auto pViewportDepths = p_inherited_viewport_scissor_info->pViewportDepths;
                inheritedViewportDepths.assign(pViewportDepths,
                                               pViewportDepths + p_inherited_viewport_scissor_info->viewportDepthCount);
            }
        }
    }

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupCommandBufferBeginInfo>(pBeginInfo->pNext);
    if (chained_device_group_struct) {
        initial_device_mask = chained_device_group_struct->deviceMask;
    } else {
        initial_device_mask = (1 << dev_data.physical_device_count) - 1;
    }
    performance_lock_acquired = dev_data.performance_lock_acquired;
    updatedQueries.clear();
}

void CommandBuffer::End(VkResult result) {
    if (VK_SUCCESS == result) {
        state = CbState::Recorded;
    }
}

void CommandBuffer::ExecuteCommands(vvl::span<const VkCommandBuffer> secondary_command_buffers) {
    RecordCmd(Func::vkCmdExecuteCommands);
    for (const VkCommandBuffer sub_command_buffer : secondary_command_buffers) {
        auto sub_cb_state = dev_data.GetWrite<CommandBuffer>(sub_command_buffer);
        ASSERT_AND_RETURN(sub_cb_state);
        if (!(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // TODO: Because this is a state change, clearing the VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT needs to be moved
                // from the validation step to the recording step
                beginInfo.flags &= ~VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            }
        }

        // Propagate inital layout and current layout state to the primary cmd buffer
        // NOTE: The update/population of the image_layout_map is done in CoreChecks, but for other classes derived from
        // Device these maps will be empty, so leaving the propagation in the the state tracker should be a no-op
        // for those other classes.
        for (const auto &[image, image_layout_registry] : sub_cb_state->image_layout_map) {
            const auto image_state = dev_data.Get<vvl::Image>(image);
            if (!image_state || image_state->Destroyed() || !image_layout_registry ||
                image_state->GetId() != image_layout_registry->GetImageId()) {
                continue;
            }
            auto cb_image_layout_registry = GetOrCreateImageLayoutRegistry(*image_state);
            if (cb_image_layout_registry) {
                cb_image_layout_registry->UpdateFrom(*image_layout_registry);
            }
        }

        sub_cb_state->primaryCommandBuffer = VkHandle();
        linkedCommandBuffers.insert(sub_cb_state.get());
        AddChild(sub_cb_state);
        // Add a query update that runs all the query updates that happen in the sub command buffer.
        // This avoids locking ambiguity because primary command buffers are locked when these
        // callbacks run, but secondary command buffers are not.
        query_updates.emplace_back([sub_command_buffer](CommandBuffer &cb_state_arg, bool do_validate,
                                                        VkQueryPool &firstPerfQueryPool, uint32_t perfQueryPass,
                                                        QueryMap *localQueryToStateMap) {
            bool skip = false;
            auto sub_cb_state_arg = cb_state_arg.dev_data.GetWrite<CommandBuffer>(sub_command_buffer);
            for (auto &function : sub_cb_state_arg->query_updates) {
                skip |= function(*sub_cb_state_arg, do_validate, firstPerfQueryPool, perfQueryPass, localQueryToStateMap);
            }
            return skip;
        });
        for (auto &function : sub_cb_state->event_updates) {
            event_updates.push_back(function);
        }
        for (auto &event : sub_cb_state->events) {
            events.push_back(event);
        }
        for (auto &function : sub_cb_state->queue_submit_functions) {
            queue_submit_functions.push_back(function);
        }

        // State is trashed after executing secondary command buffers.
        // Importantly, this function runs after CoreChecks::PreCallValidateCmdExecuteCommands.
        trashedViewportMask = vvl::MaxTypeValue(trashedViewportMask);
        trashedScissorMask = vvl::MaxTypeValue(trashedScissorMask);
        trashedViewportCount = true;
        trashedScissorCount = true;

        // Pass along if any commands are used in the secondary command buffer
        has_draw_cmd |= sub_cb_state->has_draw_cmd;
        has_dispatch_cmd |= sub_cb_state->has_dispatch_cmd;
        has_trace_rays_cmd |= sub_cb_state->has_trace_rays_cmd;
        has_build_as_cmd |= sub_cb_state->has_build_as_cmd;

        // Handle secondary command buffer updates for dynamic rendering
        if (!hasRenderPassInstance) {
            resumesRenderPassInstance = sub_cb_state->resumesRenderPassInstance;
        }
        if (!sub_cb_state->active_render_pass) {
            suspendsRenderPassInstance = sub_cb_state->suspendsRenderPassInstance;
            hasRenderPassInstance |= sub_cb_state->hasRenderPassInstance;
        }

        if (sub_cb_state->IsSecondary()) {
            nesting_level = std::max(nesting_level, sub_cb_state->nesting_level + 1);
        }

        label_stack_depth_ += sub_cb_state->label_stack_depth_;
        label_commands_.insert(label_commands_.end(), sub_cb_state->label_commands_.begin(), sub_cb_state->label_commands_.end());
    }
}

void CommandBuffer::PushDescriptorSetState(VkPipelineBindPoint pipelineBindPoint, const vvl::PipelineLayout &pipeline_layout,
                                           vvl::Func bound_command, uint32_t set, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet *pDescriptorWrites) {
    // Short circuit invalid updates
    if ((set >= pipeline_layout.set_layouts.size()) || !pipeline_layout.set_layouts[set] ||
        !pipeline_layout.set_layouts[set]->IsPushDescriptor()) {
        return;
    }

    // We need a descriptor set to update the bindings with, compatible with the passed layout
    const auto &dsl = pipeline_layout.set_layouts[set];
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    auto &last_bound = lastBound[lv_bind_point];
    auto &push_descriptor_set = last_bound.push_descriptor_set;
    // If we are disturbing the current push_desriptor_set clear it
    if (!push_descriptor_set || !last_bound.IsBoundSetCompatible(set, pipeline_layout)) {
        last_bound.UnbindAndResetPushDescriptorSet(dev_data.CreateDescriptorSet(VK_NULL_HANDLE, nullptr, dsl, 0));
    }

    UpdateLastBoundDescriptorSets(pipelineBindPoint, pipeline_layout, bound_command, set, 1, nullptr, push_descriptor_set, 0,
                                  nullptr);
    last_bound.desc_set_pipeline_layout = pipeline_layout.VkHandle();

    // Now that we have either the new or extant push_descriptor set ... do the write updates against it
    push_descriptor_set->PerformPushDescriptorsUpdate(descriptorWriteCount, pDescriptorWrites);
}

// Generic function to handle state update for all CmdDraw* type functions
void CommandBuffer::UpdateDrawCmd(Func command) {
    has_draw_cmd = true;
    UpdatePipelineState(command, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

// Generic function to handle state update for all CmdDispatch* type functions
void CommandBuffer::UpdateDispatchCmd(Func command) {
    has_dispatch_cmd = true;
    UpdatePipelineState(command, VK_PIPELINE_BIND_POINT_COMPUTE);
}

// Generic function to handle state update for all CmdTraceRay* type functions
void CommandBuffer::UpdateTraceRayCmd(Func command) {
    has_trace_rays_cmd = true;
    UpdatePipelineState(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

// Generic function to handle state update for all Provoking functions calls (draw/dispatch/traceray/etc)
void CommandBuffer::UpdatePipelineState(Func command, const VkPipelineBindPoint bind_point) {
    RecordCmd(command);

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto &last_bound = lastBound[lv_bind_point];
    vvl::Pipeline *pipe = last_bound.pipeline_state;
    if (!pipe) {
        return;
    }

    // Update the consumed viewport/scissor count.
    {
        usedViewportScissorCount = std::max({usedViewportScissorCount, pipelineStaticViewportCount, pipelineStaticScissorCount});
        usedDynamicViewportCount |= pipe->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        usedDynamicScissorCount |= pipe->IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    }

    if (pipe->IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        SetActiveSubpassRasterizationSampleCount(dynamic_state_value.rasterization_samples);
    }

    if (last_bound.desc_set_pipeline_layout != VK_NULL_HANDLE) {
        for (const auto &[set_index, binding_req_map] : pipe->active_slots) {
            if (set_index >= last_bound.ds_slots.size()) {
                continue;
            }
            auto &ds_slot = last_bound.ds_slots[set_index];
            // Pull the set node
            auto &descriptor_set = ds_slot.ds_state;
            if (!descriptor_set) {
                continue;
            }

            // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor binding

            // We can skip updating the state if "nothing" has changed since the last validation.
            // See CoreChecks::ValidateActionState for more details.
            const bool need_update =  // Update if descriptor set (or contents) has changed
                ds_slot.validated_set != descriptor_set.get() ||
                ds_slot.validated_set_change_count != descriptor_set->GetChangeCount() ||
                (!dev_data.disabled[image_layout_validation] &&
                 ds_slot.validated_set_image_layout_change_count != image_layout_change_count);
            if (need_update) {
                if (!dev_data.disabled[command_buffer_state] && !descriptor_set->IsPushDescriptor()) {
                    AddChild(descriptor_set);
                }

                // Bind this set and its active descriptor resources to the command buffer
                descriptor_set->UpdateDrawStates(&dev_data, *this, binding_req_map);

                ds_slot.validated_set = descriptor_set.get();
                ds_slot.validated_set_change_count = descriptor_set->GetChangeCount();
                ds_slot.validated_set_image_layout_change_count = image_layout_change_count;
            }
        }
    }
}

// Helper for descriptor set (and buffer) updates.
static bool PushDescriptorCleanup(LastBound &last_bound, uint32_t set_idx) {
    // All uses are from loops over ds_slots, but just in case..
    assert(set_idx < last_bound.ds_slots.size());

    auto descriptor_set = last_bound.ds_slots[set_idx].ds_state.get();
    if (descriptor_set && descriptor_set->IsPushDescriptor()) {
        assert(descriptor_set == last_bound.push_descriptor_set.get());
        last_bound.push_descriptor_set = nullptr;
        return true;
    }
    return true;
}

// Update pipeline_layout bind points applying the "Pipeline Layout Compatibility" rules.
// One of pDescriptorSets or push_descriptor_set should be nullptr, indicating whether this
// is called for CmdBindDescriptorSets or CmdPushDescriptorSet.
void CommandBuffer::UpdateLastBoundDescriptorSets(VkPipelineBindPoint pipeline_bind_point,
                                                  const vvl::PipelineLayout &pipeline_layout, vvl::Func bound_command,
                                                  uint32_t first_set, uint32_t set_count, const VkDescriptorSet *pDescriptorSets,
                                                  std::shared_ptr<vvl::DescriptorSet> &push_descriptor_set,
                                                  uint32_t dynamic_offset_count, const uint32_t *p_dynamic_offsets) {
    ASSERT_AND_RETURN((pDescriptorSets == nullptr) ^ (push_descriptor_set == nullptr));

    uint32_t required_size = first_set + set_count;
    const uint32_t last_binding_index = required_size - 1;
    assert(last_binding_index < pipeline_layout.set_compat_ids.size());

    // Some useful shorthand
    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto &last_bound = lastBound[lv_bind_point];
    last_bound.desc_set_pipeline_layout = pipeline_layout.VkHandle();
    last_bound.desc_set_bound_command = bound_command;
    auto &pipe_compat_ids = pipeline_layout.set_compat_ids;
    // Resize binding arrays
    if (last_binding_index >= last_bound.ds_slots.size()) {
        last_bound.ds_slots.resize(required_size);
    }
    const uint32_t current_size = static_cast<uint32_t>(last_bound.ds_slots.size());

    // Clean up the "disturbed" before and after the range to be set
    if (required_size < current_size) {
        if (last_bound.ds_slots[last_binding_index].compat_id_for_set != pipe_compat_ids[last_binding_index]) {
            // We're disturbing those after last, we'll shrink below, but first need to check for and cleanup the push_descriptor
            for (auto set_idx = required_size; set_idx < current_size; ++set_idx) {
                if (PushDescriptorCleanup(last_bound, set_idx)) {
                    break;
                }
            }
        } else {
            // We're not disturbing past last, so leave the upper binding data alone.
            required_size = current_size;
        }
    }

    // We resize if we need more set entries or if those past "last" are disturbed
    if (required_size != current_size) {
        last_bound.ds_slots.resize(required_size);
    }

    // For any previously bound sets, need to set them to "invalid" if they were disturbed by this update
    for (uint32_t set_idx = 0; set_idx < first_set; ++set_idx) {
        auto &ds_slot = last_bound.ds_slots[set_idx];
        if (ds_slot.compat_id_for_set != pipe_compat_ids[set_idx]) {
            PushDescriptorCleanup(last_bound, set_idx);
            ds_slot.Reset();
            ds_slot.compat_id_for_set = pipe_compat_ids[set_idx];
        }
    }

    // Now update the bound sets with the input sets
    const uint32_t *input_dynamic_offsets = p_dynamic_offsets;  // "read" pointer for dynamic offset data
    for (uint32_t input_idx = 0; input_idx < set_count; input_idx++) {
        auto set_idx = input_idx + first_set;  // set_idx is index within layout, input_idx is index within input descriptor sets
        auto &ds_slot = last_bound.ds_slots[set_idx];
        auto descriptor_set =
            push_descriptor_set ? push_descriptor_set : dev_data.Get<vvl::DescriptorSet>(pDescriptorSets[input_idx]);

        ds_slot.Reset();
        // Record binding (or push)
        if (descriptor_set != last_bound.push_descriptor_set) {
            // Only cleanup the push descriptors if they aren't the currently used set.
            PushDescriptorCleanup(last_bound, set_idx);
        }
        ds_slot.ds_state = descriptor_set;
        ds_slot.compat_id_for_set = pipe_compat_ids[set_idx];  // compat ids are canonical *per* set index

        if (descriptor_set) {
            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            // TODO: Add logic for tracking push_descriptor offsets (here or in caller)
            if (set_dynamic_descriptor_count && input_dynamic_offsets) {
                const uint32_t *end_offset = input_dynamic_offsets + set_dynamic_descriptor_count;
                ds_slot.dynamic_offsets = std::vector<uint32_t>(input_dynamic_offsets, end_offset);
                input_dynamic_offsets = end_offset;
                assert(input_dynamic_offsets <= (p_dynamic_offsets + dynamic_offset_count));
            } else {
                ds_slot.dynamic_offsets.clear();
            }
        }
    }
}

void CommandBuffer::UpdateLastBoundDescriptorBuffers(VkPipelineBindPoint pipeline_bind_point,
                                                     const vvl::PipelineLayout &pipeline_layout, uint32_t first_set,
                                                     uint32_t set_count, const uint32_t *buffer_indicies,
                                                     const VkDeviceSize *buffer_offsets) {
    uint32_t required_size = first_set + set_count;
    const uint32_t last_binding_index = required_size - 1;
    assert(last_binding_index < pipeline_layout.set_compat_ids.size());

    // Some useful shorthand
    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto &last_bound = lastBound[lv_bind_point];
    last_bound.desc_set_pipeline_layout = pipeline_layout.VkHandle();
    auto &pipe_compat_ids = pipeline_layout.set_compat_ids;
    // Resize binding arrays
    if (last_binding_index >= last_bound.ds_slots.size()) {
        last_bound.ds_slots.resize(required_size);
    }
    const uint32_t current_size = static_cast<uint32_t>(last_bound.ds_slots.size());

    // Clean up the "disturbed" before and after the range to be set
    if (required_size < current_size) {
        if (last_bound.ds_slots[last_binding_index].compat_id_for_set != pipe_compat_ids[last_binding_index]) {
            // We're disturbing those after last, we'll shrink below, but first need to check for and cleanup the push_descriptor
            for (auto set_idx = required_size; set_idx < current_size; ++set_idx) {
                if (PushDescriptorCleanup(last_bound, set_idx)) {
                    break;
                }
            }
        } else {
            // We're not disturbing past last, so leave the upper binding data alone.
            required_size = current_size;
        }
    }

    // We resize if we need more set entries or if those past "last" are disturbed
    if (required_size != current_size) {
        last_bound.ds_slots.resize(required_size);
    }

    // For any previously bound sets, need to set them to "invalid" if they were disturbed by this update
    for (uint32_t set_idx = 0; set_idx < first_set; ++set_idx) {
        PushDescriptorCleanup(last_bound, set_idx);
        last_bound.ds_slots[set_idx].Reset();
    }

    // Now update the bound sets with the input sets
    for (uint32_t input_idx = 0; input_idx < set_count; input_idx++) {
        auto set_idx = input_idx + first_set;  // set_idx is index within layout, input_idx is index within input descriptor sets
        auto &ds_slot = last_bound.ds_slots[set_idx];
        ds_slot.Reset();

        // Record binding
        ds_slot.descriptor_buffer_binding = {buffer_indicies[input_idx], buffer_offsets[input_idx]};
        ds_slot.compat_id_for_set = pipe_compat_ids[set_idx];  // compat ids are canonical *per* set index
    }
}

// Set image layout for given VkImageSubresourceRange struct
void CommandBuffer::SetImageLayout(const vvl::Image &image_state, const VkImageSubresourceRange &image_subresource_range,
                                   VkImageLayout layout, VkImageLayout expected_layout) {
    auto image_layout_registry = GetOrCreateImageLayoutRegistry(image_state);
    if (image_layout_registry &&
        image_layout_registry->SetSubresourceRangeLayout(*this, image_subresource_range, layout, expected_layout)) {
        image_layout_change_count++;  // Change the version of this data to force revalidation
    }
}

// Set the initial image layout for all slices of an image view
void CommandBuffer::SetImageViewInitialLayout(const vvl::ImageView &view_state, VkImageLayout layout) {
    if (dev_data.disabled[image_layout_validation]) {
        return;
    }
    vvl::Image *image_state = view_state.image_state.get();
    auto image_layout_registry =
        (image_state && !image_state->Destroyed()) ? GetOrCreateImageLayoutRegistry(*image_state) : nullptr;
    if (image_layout_registry) {
        image_layout_registry->SetSubresourceRangeInitialLayout(*this, layout, view_state);
    }
}

// Set the initial image layout for a passed non-normalized subresource range
void CommandBuffer::SetImageInitialLayout(const vvl::Image &image_state, const VkImageSubresourceRange &range,
                                          VkImageLayout layout) {
    auto image_layout_registry = GetOrCreateImageLayoutRegistry(image_state);
    if (image_layout_registry) {
        image_layout_registry->SetSubresourceRangeInitialLayout(*this, image_state.NormalizeSubresourceRange(range), layout);
    }
}

void CommandBuffer::SetImageInitialLayout(VkImage image, const VkImageSubresourceRange &range, VkImageLayout layout) {
    auto image_state = dev_data.Get<vvl::Image>(image);
    ASSERT_AND_RETURN(image_state);
    SetImageInitialLayout(*image_state, range, layout);
}

void CommandBuffer::SetImageInitialLayout(const vvl::Image &image_state, const VkImageSubresourceLayers &layers,
                                          VkImageLayout layout) {
    SetImageInitialLayout(image_state, RangeFromLayers(layers), layout);
}

// Set image layout for all slices of an image view
void CommandBuffer::SetImageViewLayout(const vvl::ImageView &view_state, VkImageLayout layout, VkImageLayout layoutStencil) {
    const vvl::Image *image_state = view_state.image_state.get();

    VkImageSubresourceRange sub_range = view_state.normalized_subresource_range;

    if (sub_range.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) && layoutStencil != kInvalidLayout) {
        sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        SetImageLayout(*image_state, sub_range, layout);
        sub_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        SetImageLayout(*image_state, sub_range, layoutStencil);
    } else {
        // If layoutStencil is kInvalidLayout (meaning no separate depth/stencil layout), image view format has both depth and
        // stencil aspects, and subresource has only one of aspect out of depth or stencil, then the missing aspect will also be
        // transitioned and thus must be included explicitly
        if (const VkFormat format = view_state.create_info.format; vkuFormatIsDepthAndStencil(format)) {
            if (layoutStencil == kInvalidLayout &&
                (sub_range.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
                sub_range.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        SetImageLayout(*image_state, sub_range, layout);
    }
}

void CommandBuffer::RecordCmd(Func command) { command_count++; }

void CommandBuffer::RecordStateCmd(Func command, CBDynamicState state) {
    RecordCmd(command);
    RecordDynamicState(state);

    vvl::Pipeline *pipeline = GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (pipeline && !pipeline->IsDynamic(state)) {
        dirtyStaticState = true;
    }
}

void CommandBuffer::RecordDynamicState(CBDynamicState state) {
    dynamic_state_status.cb.set(state);
    dynamic_state_status.pipeline.set(state);
    dynamic_state_status.history.set(state);
}

void CommandBuffer::RecordTransferCmd(Func command, std::shared_ptr<Bindable> &&buf1, std::shared_ptr<Bindable> &&buf2) {
    RecordCmd(command);
    if (buf1) {
        AddChild(buf1);
    }
    if (buf2) {
        AddChild(buf2);
    }
}

void CommandBuffer::RecordSetEvent(Func command, VkEvent event, VkPipelineStageFlags2KHR stageMask) {
    RecordCmd(command);
    if (!dev_data.disabled[command_buffer_state]) {
        auto event_state = dev_data.Get<vvl::Event>(event);
        if (event_state) {
            AddChild(event_state);
        }
    }
    events.push_back(event);
    if (!waitedEvents.count(event)) {
        writeEventsBeforeWait.push_back(event);
    }
    event_updates.emplace_back(
        [event, stageMask](CommandBuffer &, bool do_validate, EventMap &local_event_signal_info, VkQueue, const Location &loc) {
            local_event_signal_info[event] = EventInfo{stageMask, true};
            return false;  // skip
        });
}

void CommandBuffer::RecordResetEvent(Func command, VkEvent event, VkPipelineStageFlags2KHR stageMask) {
    RecordCmd(command);
    if (!dev_data.disabled[command_buffer_state]) {
        auto event_state = dev_data.Get<vvl::Event>(event);
        if (event_state) {
            AddChild(event_state);
        }
    }
    events.push_back(event);
    if (!waitedEvents.count(event)) {
        writeEventsBeforeWait.push_back(event);
    }

    event_updates.emplace_back(
        [event](CommandBuffer &, bool do_validate, EventMap &local_event_signal_info, VkQueue, const Location &loc) {
            local_event_signal_info[event] = EventInfo{VK_PIPELINE_STAGE_2_NONE, false};
            return false;  // skip
        });
}

void CommandBuffer::RecordWaitEvents(Func command, uint32_t eventCount, const VkEvent *pEvents,
                                     VkPipelineStageFlags2KHR src_stage_mask) {
    RecordCmd(command);
    for (uint32_t i = 0; i < eventCount; ++i) {
        if (!dev_data.disabled[command_buffer_state]) {
            auto event_state = dev_data.Get<vvl::Event>(pEvents[i]);
            if (event_state) {
                AddChild(event_state);
            }
        }
        waitedEvents.insert(pEvents[i]);
        events.push_back(pEvents[i]);
    }
}

void CommandBuffer::RecordBarriers(uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                   uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                   uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    if (dev_data.disabled[command_buffer_state]) return;

    for (uint32_t i = 0; i < bufferMemoryBarrierCount; i++) {
        auto buffer_state = dev_data.Get<vvl::Buffer>(pBufferMemoryBarriers[i].buffer);
        if (buffer_state) {
            AddChild(buffer_state);
        }
    }
    for (uint32_t i = 0; i < imageMemoryBarrierCount; i++) {
        auto image_state = dev_data.Get<vvl::Image>(pImageMemoryBarriers[i].image);
        if (image_state) {
            AddChild(image_state);
        }
    }
}

void CommandBuffer::RecordBarriers(const VkDependencyInfo &dep_info) {
    if (dev_data.disabled[command_buffer_state]) return;

    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        auto buffer_state = dev_data.Get<vvl::Buffer>(dep_info.pBufferMemoryBarriers[i].buffer);
        if (buffer_state) {
            AddChild(buffer_state);
        }
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        auto image_state = dev_data.Get<vvl::Image>(dep_info.pImageMemoryBarriers[i].image);
        if (image_state) {
            AddChild(image_state);
        }
    }
}

void CommandBuffer::RecordWriteTimestamp(Func command, VkPipelineStageFlags2KHR pipelineStage, VkQueryPool queryPool,
                                         uint32_t slot) {
    RecordCmd(command);
    if (dev_data.disabled[query_validation]) return;

    if (!dev_data.disabled[command_buffer_state]) {
        auto pool_state = dev_data.Get<vvl::QueryPool>(queryPool);
        AddChild(pool_state);
    }
    QueryObject query_obj = {queryPool, slot};
    EndQuery(query_obj);
}

void CommandBuffer::Submit(VkQueue queue, uint32_t perf_submit_pass, const Location &loc) {
    // Update vvl::QueryPool with a query state at the end of the command buffer.
    // Ultimately, it tracks the final query state for the entire submission.
    {
        VkQueryPool first_pool = VK_NULL_HANDLE;
        QueryMap local_query_to_state_map;
        for (auto &function : query_updates) {
            function(*this, /*do_validate*/ false, first_pool, perf_submit_pass, &local_query_to_state_map);
        }
        for (const auto &[query_object, query_state] : local_query_to_state_map) {
            auto query_pool_state = dev_data.Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            query_pool_state->SetQueryState(query_object.slot, query_object.perf_pass, query_state);
        }
    }

    // Update vvl::Event with src_stage from the last recorded SetEvent.
    // Ultimately, it tracks the last SetEvent for the entire submission.
    {
        EventMap local_event_signal_info;
        for (const auto &function : event_updates) {
            function(*this, /*do_validate*/ false, local_event_signal_info,
                     VK_NULL_HANDLE /* when do_validate is false then wait handler is inactive */, loc);
        }
        for (const auto &[event, info] : local_event_signal_info) {
            auto event_state = dev_data.Get<vvl::Event>(event);
            event_state->signaled = info.signal;
            event_state->signal_src_stage_mask = info.src_stage_mask;
            event_state->signaling_queue = queue;
        }
    }

    for (const auto &it : video_session_updates) {
        auto video_session_state = dev_data.Get<vvl::VideoSession>(it.first);
        auto device_state = video_session_state->DeviceStateWrite();
        for (const auto &function : it.second) {
            function(dev_data, video_session_state.get(), *device_state, /*do_validate*/ false);
        }
    }
}

void CommandBuffer::Retire(uint32_t perf_submit_pass, const std::function<bool(const QueryObject &)> &is_query_updated_after) {
    QueryMap local_query_to_state_map;
    VkQueryPool first_pool = VK_NULL_HANDLE;
    for (auto &function : query_updates) {
        function(*this, /*do_validate*/ false, first_pool, perf_submit_pass, &local_query_to_state_map);
    }

    for (const auto &[query_object, query_state] : local_query_to_state_map) {
        if (query_state == QUERYSTATE_ENDED && !is_query_updated_after(query_object)) {
            auto query_pool_state = dev_data.Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            query_pool_state->SetQueryState(query_object.slot, query_object.perf_pass, QUERYSTATE_AVAILABLE);
        }
    }
}

uint32_t CommandBuffer::GetDynamicColorAttachmentCount() const {
    if (active_render_pass) {
        if (active_render_pass->use_dynamic_rendering_inherited) {
            return active_render_pass->inheritance_rendering_info.colorAttachmentCount;
        }
        if (active_render_pass->use_dynamic_rendering) {
            return active_render_pass->dynamic_rendering_begin_rendering_info.colorAttachmentCount;
        }
    }
    return 0;
}

bool CommandBuffer::HasValidDynamicDepthAttachment() const {
    if (active_render_pass) {
        if (active_render_pass->use_dynamic_rendering_inherited) {
            return active_render_pass->inheritance_rendering_info.depthAttachmentFormat != VK_FORMAT_UNDEFINED;
        }
        if (active_render_pass->use_dynamic_rendering) {
            return active_render_pass->dynamic_rendering_begin_rendering_info.pDepthAttachment != nullptr;
        }
    }
    return false;
}
bool CommandBuffer::HasValidDynamicStencilAttachment() const {
    if (active_render_pass) {
        if (active_render_pass->use_dynamic_rendering_inherited) {
            return active_render_pass->inheritance_rendering_info.stencilAttachmentFormat != VK_FORMAT_UNDEFINED;
        }
        if (active_render_pass->use_dynamic_rendering) {
            return active_render_pass->dynamic_rendering_begin_rendering_info.pStencilAttachment != nullptr;
        }
    }
    return false;
}
bool CommandBuffer::HasExternalFormatResolveAttachment() const {
    if (active_render_pass && active_render_pass->use_dynamic_rendering &&
        active_render_pass->dynamic_rendering_begin_rendering_info.colorAttachmentCount > 0) {
        return active_render_pass->dynamic_rendering_begin_rendering_info.pColorAttachments->resolveMode ==
               VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID;
    }
    return false;
}

void CommandBuffer::BindShader(VkShaderStageFlagBits shader_stage, vvl::ShaderObject *shader_object_state) {
    const VkPipelineBindPoint pipeline_bind_point = ConvertToPipelineBindPoint(shader_stage);
    auto &last_bound_state = lastBound[ConvertToLvlBindPoint(pipeline_bind_point)];
    const auto stage_index = static_cast<uint32_t>(ConvertToShaderObjectStage(shader_stage));
    last_bound_state.shader_object_bound[stage_index] = true;
    last_bound_state.shader_object_states[stage_index] = shader_object_state;
}

void CommandBuffer::UnbindResources() {
    // Vertex and index buffers
    index_buffer_binding.reset();
    current_vertex_buffer_binding_info.clear();

    // Push constants
    push_constant_data_chunks.clear();
    push_constant_latest_used_layout.fill(VK_NULL_HANDLE);
    push_constant_ranges_layout.reset();

    // Reset status of cb to force rebinding of all resources
    // Index buffer included
    dynamic_state_status.cb.reset();
    dynamic_state_status.pipeline.reset();
    dynamic_state_status.rtx_stack_size_cb = false;
    dynamic_state_status.rtx_stack_size_pipeline = false;

    // Pipeline and descriptor sets
    lastBound[BindPoint_Graphics].Reset();
}

LogObjectList CommandBuffer::GetObjectList(VkShaderStageFlagBits stage) const {
    LogObjectList objlist(handle_);
    const auto lv_bind_point = ConvertToLvlBindPoint(stage);
    const auto &last_bound = lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (pipeline_state) {
        objlist.add(pipeline_state->Handle());
    } else if (VkShaderEXT shader = last_bound.GetShader(ConvertToShaderObjectStage(stage))) {
        objlist.add(shader);
    }
    return objlist;
}

LogObjectList CommandBuffer::GetObjectList(VkPipelineBindPoint pipeline_bind_point) const {
    LogObjectList objlist(handle_);
    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    const auto &last_bound = lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (pipeline_state) {
        objlist.add(pipeline_state->Handle());
    } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::COMPUTE)) {
            objlist.add(shader);
        }
    } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        // If using non-compute, need to check all graphics stages
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::VERTEX)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::TESSELLATION_CONTROL)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::TESSELLATION_EVALUATION)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::GEOMETRY)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::FRAGMENT)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::MESH)) {
            objlist.add(shader);
        }
        if (VkShaderEXT shader = last_bound.GetShader(ShaderObjectStage::TASK)) {
            objlist.add(shader);
        }
    }
    return objlist;
}

vvl::Pipeline *CommandBuffer::GetCurrentPipeline(VkPipelineBindPoint pipelineBindPoint) const {
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    return lastBound[lv_bind_point].pipeline_state;
}

void CommandBuffer::GetCurrentPipelineAndDesriptorSets(VkPipelineBindPoint pipelineBindPoint, const vvl::Pipeline **rtn_pipe,
                                                       const std::vector<LastBound::DescriptorSetSlot> **rtn_sets) const {
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    const auto &last_bound = lastBound[lv_bind_point];
    if (!last_bound.pipeline_state) return;
    *rtn_pipe = last_bound.pipeline_state;
    *rtn_sets = &(last_bound.ds_slots);
}

void CommandBuffer::BeginLabel(const char *label_name) {
    ++label_stack_depth_;
    label_commands_.emplace_back(LabelCommand{true, label_name});
}

void CommandBuffer::EndLabel() {
    --label_stack_depth_;
    label_commands_.emplace_back(LabelCommand{false, std::string()});
}

void CommandBuffer::ReplayLabelCommands(const vvl::span<const LabelCommand> &label_commands,
                                        std::vector<std::string> &label_stack) {
    for (const LabelCommand &command : label_commands) {
        if (command.begin) {
            label_stack.emplace_back(command.label_name.empty() ? "(empty label)" : command.label_name);
        } else if (!label_stack.empty()) {
            // The above condition is needed for several reasons. On the primary command buffer level
            // the labels are not necessary balanced. And if the empty stack is detected in the context
            // where it is an error, then it will be reported by the core validation, but we still need
            // a safety check.
            label_stack.pop_back();
        }
    }
}

std::string CommandBuffer::GetDebugRegionName(const std::vector<LabelCommand> &label_commands, uint32_t label_command_index,
                                              const std::vector<std::string> &initial_label_stack) {
    if (label_command_index >= label_commands.size()) {
        // Can happen due to core validation error when in-use command buffer was re-recorded.
        // It's a bug if this happens in a valid vulkan program.
        return {};
    }
    auto label_commands_to_replay = vvl::make_span(label_commands.data(), label_command_index + 1);
    auto label_stack = initial_label_stack;
    vvl::CommandBuffer::ReplayLabelCommands(label_commands_to_replay, label_stack);

    // Build up complete debug region name from all enclosing regions
    std::string debug_region;
    for (const std::string &label_name : label_stack) {
        if (!debug_region.empty()) {
            debug_region += "::";
        }
        debug_region += label_name;
    }
    return debug_region;
}

std::string CommandBuffer::DescribeInvalidatedState(CBDynamicState dynamic_state) const {
    std::stringstream ss;
    if (dynamic_state_status.history[dynamic_state] && !dynamic_state_status.cb[dynamic_state]) {
        ss << " (There was a call to vkCmdBindPipeline";
        if (auto pipeline = dev_data.Get<vvl::Pipeline>(invalidated_state_pipe[dynamic_state])) {
            ss << " with " << dev_data.FormatHandle(*pipeline);
        }
        ss << " that didn't have " << DynamicStateToString(dynamic_state) << " and invalidated the prior "
           << DescribeDynamicStateCommand(dynamic_state) << " call)";
    }
    return ss.str();
}

}  // namespace vvl
