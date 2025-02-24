/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#include "state_tracker/render_pass_state.h"
#include "utils/convert_utils.h"
#include "state_tracker/image_state.h"

static const VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

static VkSubpassDependency2 ImplicitDependencyFromExternal(uint32_t subpass) {
    VkSubpassDependency2 from_external = {VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                                          nullptr,
                                          VK_SUBPASS_EXTERNAL,
                                          subpass,
                                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                          0,
                                          VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                          0,
                                          0};
    return from_external;
}

static VkSubpassDependency2 ImplicitDependencyToExternal(uint32_t subpass) {
    VkSubpassDependency2 to_external = {VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                                        nullptr,
                                        subpass,
                                        VK_SUBPASS_EXTERNAL,
                                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                        0,
                                        0,
                                        0};
    return to_external;
}
// NOTE: The functions below are only called from the vvl::RenderPass constructor, and use const_cast<> to set up
// members that never change after construction is finished.
static void RecordRenderPassDAG(const VkRenderPassCreateInfo2 *pCreateInfo, vvl::RenderPass *render_pass) {
    auto &subpass_to_node = const_cast<vvl::RenderPass::DAGNodeVec &>(render_pass->subpass_to_node);
    subpass_to_node.resize(pCreateInfo->subpassCount);
    auto &self_dependencies = const_cast<vvl::RenderPass::SelfDepVec &>(render_pass->self_dependencies);
    self_dependencies.resize(pCreateInfo->subpassCount);
    auto &subpass_dependencies = const_cast<vvl::RenderPass::SubpassGraphVec &>(render_pass->subpass_dependencies);
    subpass_dependencies.resize(pCreateInfo->subpassCount);

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        subpass_to_node[i].pass = i;
        self_dependencies[i].clear();
        subpass_dependencies[i].pass = i;
    }
    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const auto &dependency = pCreateInfo->pDependencies[i];
        const auto src_subpass = dependency.srcSubpass;
        const auto dst_subpass = dependency.dstSubpass;
        if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass != VK_SUBPASS_EXTERNAL)) {
            if (dependency.srcSubpass == dependency.dstSubpass) {
                self_dependencies[dependency.srcSubpass].push_back(i);
            } else {
                subpass_to_node[dependency.dstSubpass].prev.push_back(dependency.srcSubpass);
                subpass_to_node[dependency.srcSubpass].next.push_back(dependency.dstSubpass);
            }
        }
        if (src_subpass == VK_SUBPASS_EXTERNAL) {
            assert(dst_subpass != VK_SUBPASS_EXTERNAL);  // this is invalid per VUID-VkSubpassDependency-srcSubpass-00865
            subpass_dependencies[dst_subpass].barrier_from_external.emplace_back(&dependency);
        } else if (dst_subpass == VK_SUBPASS_EXTERNAL) {
            subpass_dependencies[src_subpass].barrier_to_external.emplace_back(&dependency);
        } else if (dependency.srcSubpass != dependency.dstSubpass) {
            // ignore self dependencies in prev and next
            subpass_dependencies[src_subpass].next[&subpass_dependencies[dst_subpass]].emplace_back(&dependency);
            subpass_dependencies[dst_subpass].prev[&subpass_dependencies[src_subpass]].emplace_back(&dependency);
        }
    }

    // If no barriers to external are provided for a given subpass, add them.
    for (auto &subpass_dep : subpass_dependencies) {
        const uint32_t pass = subpass_dep.pass;
        if (subpass_dep.barrier_from_external.empty()) {
            // Add implicit from barrier if they're aren't any
            subpass_dep.implicit_barrier_from_external =
                std::make_unique<VkSubpassDependency2>(ImplicitDependencyFromExternal(pass));
            subpass_dep.barrier_from_external.emplace_back(subpass_dep.implicit_barrier_from_external.get());
        }
        if (subpass_dep.barrier_to_external.empty()) {
            // Add implicit to barrier  if they're aren't any
            subpass_dep.implicit_barrier_to_external = std::make_unique<VkSubpassDependency2>(ImplicitDependencyToExternal(pass));
            subpass_dep.barrier_to_external.emplace_back(subpass_dep.implicit_barrier_to_external.get());
        }
    }

    //
    // Determine "asynchrononous" subpassess
    // syncronization is only interested in asyncronous stages *earlier* that the current one... so we'll only look towards those.
    // NOTE: This is O(N^3), which we could shrink to O(N^2logN) using sets instead of arrays, but given that N is likely to be
    // small and the K for |= from the prev is must less than for set, we'll accept the brute force.
    std::vector<std::vector<bool>> pass_depends(pCreateInfo->subpassCount);
    for (uint32_t i = 1; i < pCreateInfo->subpassCount; ++i) {
        auto &depends = pass_depends[i];
        depends.resize(i);
        auto &subpass_dep = subpass_dependencies[i];
        for (const auto &prev : subpass_dep.prev) {
            const auto prev_pass = prev.first->pass;
            const auto &prev_depends = pass_depends[prev_pass];
            for (uint32_t j = 0; j < prev_pass; j++) {
                depends[j] = depends[j] || prev_depends[j];
            }
            depends[prev_pass] = true;
        }
        for (uint32_t pass = 0; pass < subpass_dep.pass; pass++) {
            if (!depends[pass]) {
                subpass_dep.async.push_back(pass);
            }
        }
    }
}

struct AttachmentTracker {  // This is really only of local interest, but a bit big for a lambda
    vvl::RenderPass *const rp;
    vvl::RenderPass::SubpassVec &first;
    vvl::RenderPass::FirstIsTransitionVec &first_is_transition;
    vvl::RenderPass::SubpassVec &last;
    vvl::RenderPass::TransitionVec &subpass_transitions;
    vvl::RenderPass::FirstReadMap &first_read;
    const uint32_t attachment_count;
    std::vector<VkImageLayout> attachment_layout;
    std::vector<std::vector<VkImageLayout>> subpass_attachment_layout;
    explicit AttachmentTracker(vvl::RenderPass *render_pass)
        : rp(render_pass),
          first(const_cast<vvl::RenderPass::SubpassVec &>(rp->attachment_first_subpass)),
          first_is_transition(const_cast<vvl::RenderPass::FirstIsTransitionVec &>(rp->attachment_first_is_transition)),
          last(const_cast<vvl::RenderPass::SubpassVec &>(rp->attachment_last_subpass)),
          subpass_transitions(const_cast<vvl::RenderPass::TransitionVec &>(rp->subpass_transitions)),
          first_read(const_cast<vvl::RenderPass::FirstReadMap &>(rp->attachment_first_read)),
          attachment_count(rp->create_info.attachmentCount),
          attachment_layout(),
          subpass_attachment_layout() {
        first.resize(attachment_count, VK_SUBPASS_EXTERNAL);
        first_is_transition.resize(attachment_count, false);
        last.resize(attachment_count, VK_SUBPASS_EXTERNAL);
        subpass_transitions.resize(rp->create_info.subpassCount + 1);  // Add an extra for EndRenderPass
        attachment_layout.reserve(attachment_count);
        subpass_attachment_layout.resize(rp->create_info.subpassCount);
        for (auto &subpass_layouts : subpass_attachment_layout) {
            subpass_layouts.resize(attachment_count, kInvalidLayout);
        }

        for (uint32_t j = 0; j < attachment_count; j++) {
            attachment_layout.push_back(rp->create_info.pAttachments[j].initialLayout);
        }
    }

    void Update(uint32_t subpass, const uint32_t *preserved, uint32_t count) {
        // for preserved attachment, preserve the layout from the most recent (max subpass) dependency
        // or initial, if none

        // max_prev is invariant across attachments
        uint32_t max_prev = VK_SUBPASS_EXTERNAL;
        for (const auto &prev : rp->subpass_dependencies[subpass].prev) {
            const auto prev_pass = prev.first->pass;
            max_prev = (max_prev == VK_SUBPASS_EXTERNAL) ? prev_pass : std::max(prev_pass, max_prev);
        }

        for (const auto attachment : vvl::make_span(preserved, count)) {
            if (max_prev == VK_SUBPASS_EXTERNAL) {
                subpass_attachment_layout[subpass][attachment] = rp->create_info.pAttachments[attachment].initialLayout;
            } else {
                subpass_attachment_layout[subpass][attachment] = subpass_attachment_layout[max_prev][attachment];
            }
        }
    }

    void Update(uint32_t subpass, const VkAttachmentReference2 *attach_ref, uint32_t count, bool is_read) {
        if (nullptr == attach_ref) return;
        for (uint32_t j = 0; j < count; ++j) {
            const auto attachment = attach_ref[j].attachment;
            if (attachment != VK_ATTACHMENT_UNUSED) {
                const auto layout = attach_ref[j].layout;
                // Take advantage of the fact that insert won't overwrite, so we'll only write the first time.
                first_read.emplace(attachment, is_read);
                const auto initial_layout = rp->create_info.pAttachments[attachment].initialLayout;
                bool no_external_transition = true;
                if (first[attachment] == VK_SUBPASS_EXTERNAL) {
                    first[attachment] = subpass;
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout);
                        first_is_transition[attachment] = true;
                        no_external_transition = false;
                    }
                }
                last[attachment] = subpass;

                for (const auto &prev : rp->subpass_dependencies[subpass].prev) {
                    const auto prev_pass = prev.first->pass;
                    const auto prev_layout = subpass_attachment_layout[prev_pass][attachment];
                    if ((prev_layout != kInvalidLayout) && (prev_layout != layout)) {
                        subpass_transitions[subpass].emplace_back(prev_pass, attachment, prev_layout, layout);
                    }
                }

                if (no_external_transition && (rp->subpass_dependencies[subpass].prev.empty())) {
                    // This will insert a layout transition when dependencies are missing between first and subsequent use
                    // but is consistent with the idea of an implicit external dependency
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout);
                    }
                }

                attachment_layout[attachment] = layout;
                subpass_attachment_layout[subpass][attachment] = layout;
            }
        }
    }
    void FinalTransitions() {
        auto &final_transitions = subpass_transitions[rp->create_info.subpassCount];

        for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
            const auto final_layout = rp->create_info.pAttachments[attachment].finalLayout;
            // Add final transitions for attachments that were used and change layout.
            if ((last[attachment] != VK_SUBPASS_EXTERNAL) && final_layout != attachment_layout[attachment]) {
                final_transitions.emplace_back(last[attachment], attachment, attachment_layout[attachment], final_layout);
            }
        }
    }
};

static void InitRenderPassState(vvl::RenderPass *render_pass) {
    auto create_info = render_pass->create_info.ptr();

    RecordRenderPassDAG(create_info, render_pass);

    AttachmentTracker attachment_tracker(render_pass);

    for (uint32_t subpass_index = 0; subpass_index < create_info->subpassCount; ++subpass_index) {
        const VkSubpassDescription2 &subpass = create_info->pSubpasses[subpass_index];
        attachment_tracker.Update(subpass_index, subpass.pColorAttachments, subpass.colorAttachmentCount, false);
        attachment_tracker.Update(subpass_index, subpass.pResolveAttachments, subpass.colorAttachmentCount, false);
        attachment_tracker.Update(subpass_index, subpass.pDepthStencilAttachment, 1, false);
        attachment_tracker.Update(subpass_index, subpass.pInputAttachments, subpass.inputAttachmentCount, true);
        attachment_tracker.Update(subpass_index, subpass.pPreserveAttachments, subpass.preserveAttachmentCount);

        // From the spec
        // If the VkSubpassDescription2::viewMask member of any element of pSubpasses is not zero, multiview functionality is
        // considered to be enabled for this render pass.
        (*const_cast<bool *>(&render_pass->has_multiview_enabled)) |= (subpass.viewMask != 0);
    }
    attachment_tracker.FinalTransitions();
}

static bool IsDynamicRenderingMultiviewEnabled(const VkRenderingInfo *rendering_info) {
    return rendering_info && rendering_info->viewMask != 0u;
}

namespace vvl {

RenderPass::RenderPass(VkRenderPass handle, VkRenderPassCreateInfo2 const *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      create_info(pCreateInfo) {
    InitRenderPassState(this);
}

static vku::safe_VkRenderPassCreateInfo2 ConvertCreateInfo(const VkRenderPassCreateInfo &create_info) {
    vku::safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(create_info);
    return create_info_2;
}

RenderPass::RenderPass(VkRenderPass handle, VkRenderPassCreateInfo const *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      create_info(ConvertCreateInfo(*pCreateInfo)) {
    InitRenderPassState(this);
}

const VkPipelineRenderingCreateInfo VkPipelineRenderingCreateInfo_default = {
    VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, nullptr, 0, 0, nullptr, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};

RenderPass::RenderPass(VkPipelineRenderingCreateInfo const *pPipelineRenderingCreateInfo, bool rasterization_enabled)
    : StateObject(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(true),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      rasterization_enabled(rasterization_enabled),
      dynamic_pipeline_rendering_create_info((pPipelineRenderingCreateInfo && rasterization_enabled)
                                                 ? pPipelineRenderingCreateInfo
                                                 : &VkPipelineRenderingCreateInfo_default) {}

bool RenderPass::UsesColorAttachment(uint32_t subpass_num) const {
    bool result = false;

    if (subpass_num < create_info.subpassCount) {
        const auto &subpass = create_info.pSubpasses[subpass_num];

        for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
            if (subpass.pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                result = true;
                break;
            }
        }

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        // VK_ANDROID_external_format_resolve allows for the only color attachment to be VK_ATTACHMENT_UNUSED
        // but in this case, it will use the resolve attachment as color attachment. Which means that we do
        // actually use color attachments
        if (subpass.pResolveAttachments != nullptr) {
            for (uint32_t i = 0; i < subpass.colorAttachmentCount && !result; ++i) {
                uint32_t resolveAttachmentIndex = subpass.pResolveAttachments[i].attachment;
                const void *resolveAtatchmentPNextChain = create_info.pAttachments[resolveAttachmentIndex].pNext;
                if (vku::FindStructInPNextChain<VkExternalFormatANDROID>(resolveAtatchmentPNextChain)) result = true;
            }
        }
#endif
    }
    return result;
}

bool RenderPass::UsesDepthStencilAttachment(uint32_t subpass_num) const {
    bool result = false;
    if (subpass_num < create_info.subpassCount) {
        const auto &subpass = create_info.pSubpasses[subpass_num];
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            result = true;
        }
    }
    return result;
}

// vkspec.html#renderpass-noattachments
bool RenderPass::UsesNoAttachment(uint32_t subpass) const {
    // If using dynamic rendering, there is no subpass, so return 'false'
    return !UsesColorAttachment(subpass) && !UsesDepthStencilAttachment(subpass) && !UsesDynamicRendering();
}

uint32_t RenderPass::GetDynamicRenderingColorAttachmentCount() const {
    if (use_dynamic_rendering_inherited) {
        return inheritance_rendering_info.colorAttachmentCount;
    } else if (use_dynamic_rendering) {
        return dynamic_rendering_begin_rendering_info.colorAttachmentCount;
    }
    return 0;
}

uint32_t RenderPass::GetDynamicRenderingViewMask() const {
    if (use_dynamic_rendering_inherited) {
        return inheritance_rendering_info.viewMask;
    } else if (use_dynamic_rendering) {
        return dynamic_rendering_begin_rendering_info.viewMask;
    }
    return 0;
}

uint32_t RenderPass::GetViewMaskBits(uint32_t subpass) const {
    if (use_dynamic_rendering_inherited) {
        return GetBitSetCount(inheritance_rendering_info.viewMask);
    } else if (use_dynamic_rendering) {
        return GetBitSetCount(dynamic_rendering_begin_rendering_info.viewMask);
    } else {
        const auto *subpass_desc = &create_info.pSubpasses[subpass];
        if (subpass_desc) {
            return GetBitSetCount(subpass_desc->viewMask);
        }
    }
    return 0;
}

const VkMultisampledRenderToSingleSampledInfoEXT *RenderPass::GetMSRTSSInfo(uint32_t subpass) const {
    if (UsesDynamicRendering()) {
        return vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(
            dynamic_rendering_begin_rendering_info.pNext);
    }
    return vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(create_info.pSubpasses[subpass].pNext);
}

RenderPass::RenderPass(VkRenderingInfo const *pRenderingInfo, bool rasterization_enabled)
    : StateObject(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(true),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(
          IsDynamicRenderingMultiviewEnabled((pRenderingInfo && rasterization_enabled) ? pRenderingInfo : nullptr)),
      rasterization_enabled(rasterization_enabled),
      dynamic_rendering_begin_rendering_info((pRenderingInfo && rasterization_enabled) ? pRenderingInfo : nullptr) {}

RenderPass::RenderPass(VkCommandBufferInheritanceRenderingInfo const *pInheritanceRenderingInfo)
    : StateObject(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(true),
      has_multiview_enabled(false),
      inheritance_rendering_info(pInheritanceRenderingInfo) {}

Framebuffer::Framebuffer(VkFramebuffer handle, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RenderPass> &&rpstate,
                         std::vector<std::shared_ptr<vvl::ImageView>> &&attachments)
    : StateObject(handle, kVulkanObjectTypeFramebuffer),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      rp_state(rpstate),
      attachments_view_state(std::move(attachments)) {}

void Framebuffer::LinkChildNodes() {
    // Connect child node(s), which cannot safely be done in the constructor.
    for (auto &a : attachments_view_state) {
        a->AddParent(this);
    }
}

void Framebuffer::Destroy() {
    for (auto &view : attachments_view_state) {
        view->RemoveParent(this);
    }
    attachments_view_state.clear();
    StateObject::Destroy();
}

}  // namespace vvl
