/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"
#include <vector>

// Helper designed to quickly make a renderPass/framebuffer that only has a single Subpass.
// The goal is to keep the class simple as possible.
//
// Interface:
//   We use a seperate class for RenderPass1 and RenderPass2, but want to not have them diverage, so we keep an interface class
//
// Common usage:
//   RenderPassSingleSubpass rp(*this);
//   rp.AddAttachmentDescription(input_format); // sets description[0]
//   rp.AddAttachmentDescription(color_format); // sets description[1]
//   rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});                  // sets reference[0]
//   rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}); // sets reference[1]
//   rp.AddInputAttachment(0); // index maps to reference[0]
//   rp.AddColorAttachment(1); // index maps to reference[1]
//   rp.CreateRenderPass();
class InterfaceRenderPassSingleSubpass {
  public:
    InterfaceRenderPassSingleSubpass(VkLayerTest &test, vkt::Device *device = nullptr);
    virtual ~InterfaceRenderPassSingleSubpass() { Destroy(); }

    VkRenderPass Handle() { return render_pass_; }

    // Parameters are ordered from most likely to be custom values
    // Most tests don't need to worry about the Load/Store ops as we never read the values
    virtual void AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                          VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL,
                                          VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE) = 0;
    // Overload for setting samples count
    virtual void AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples,
                                          VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                          VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL) = 0;

    // Pass in index to VkAttachmentReference
    virtual void AddInputAttachment(uint32_t index) = 0;
    virtual void AddColorAttachment(uint32_t index) = 0;
    virtual void AddResolveAttachment(uint32_t index) = 0;
    virtual void AddDepthStencilAttachment(uint32_t index) = 0;

    virtual void AddSubpassDependency(VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VkAccessFlags srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VkDependencyFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT) = 0;

    virtual void CreateRenderPass(void *pNext = nullptr) = 0;

    // Explicit destroy for those tests that need to test render pass lifetime
    void Destroy() { render_pass_.destroy(); };

  protected:
    VkLayerTest &layer_test_;
    vkt::Device *device_;

    vkt::RenderPass render_pass_;
};

class RenderPassSingleSubpass : public InterfaceRenderPassSingleSubpass {
  public:
    RenderPassSingleSubpass(VkLayerTest &test, vkt::Device *device = nullptr);

    VkRenderPassCreateInfo GetCreateInfo();

    // Ordered from most likely to be custom vs will use defauly
    void AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                  VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
    // Overload for setting sampler count
    void AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples,
                                  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL);

    void AddAttachmentReference(VkAttachmentReference reference);

    // Pass in index to VkAttachmentReference
    void AddInputAttachment(uint32_t index);
    void AddColorAttachment(uint32_t index);
    void AddResolveAttachment(uint32_t index);
    void AddDepthStencilAttachment(uint32_t index);

    void AddSubpassDependency(VkSubpassDependency dependency);
    void AddSubpassDependency(VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VkAccessFlags srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                              VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                              VkDependencyFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT);

    void CreateRenderPass(void *pNext = nullptr);

  private:
    VkRenderPassCreateInfo rp_create_info_;

    std::vector<VkAttachmentDescription> attachment_descriptions_;

    std::vector<VkAttachmentReference> attachments_references_;  // global pool
    std::vector<VkAttachmentReference> input_attachments_;
    std::vector<VkAttachmentReference> color_attachments_;
    VkAttachmentReference resolve_attachment_;
    VkAttachmentReference ds_attachment_;

    VkSubpassDescription subpass_description_;
    VkSubpassDependency subpass_dependency_;
};

class RenderPass2SingleSubpass : public InterfaceRenderPassSingleSubpass {
  public:
    RenderPass2SingleSubpass(VkLayerTest &test, vkt::Device *device = nullptr);

    VkRenderPassCreateInfo2 GetCreateInfo();

    // Ordered from most likely to be custom vs will use defauly
    void AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                  VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
    // Overload for setting sampler count
    void AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples,
                                  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL);

    // Have a seperate set function to keep AddAttachmentDescription simple (very few things extend the AttachmentDescription)
    void SetAttachmentDescriptionPNext(uint32_t index, void *pNext);

    void AddAttachmentReference(uint32_t attachment, VkImageLayout layout, VkImageAspectFlags aspect_mask = 0,
                                void *pNext = nullptr);

    // Pass in index to VkAttachmentReference
    void AddInputAttachment(uint32_t index);
    void AddColorAttachment(uint32_t index);
    void AddResolveAttachment(uint32_t index);
    void AddDepthStencilAttachment(uint32_t index);
    // VK_KHR_depth_stencil_resolve
    void AddDepthStencilResolveAttachment(uint32_t index, VkResolveModeFlagBits depth_resolve_mode,
                                          VkResolveModeFlagBits stencil_resolve_mode);
    // VK_KHR_fragment_shading_rate
    void AddFragmentShadingRateAttachment(uint32_t index, VkExtent2D texel_size);

    void SetViewMask(uint32_t view_mask) { subpass_description_.viewMask = view_mask; }

    void AddSubpassDependency(VkSubpassDependency2 dependency);
    void AddSubpassDependency(VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VkAccessFlags srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                              VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                              VkDependencyFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT);

    void CreateRenderPass(void *pNext = nullptr);

  private:
    VkRenderPassCreateInfo2 rp_create_info_;

    std::vector<VkAttachmentDescription2> attachment_descriptions_;

    std::vector<VkAttachmentReference2> attachments_references_;  // global pool
    std::vector<VkAttachmentReference2> input_attachments_;
    std::vector<VkAttachmentReference2> color_attachments_;
    VkAttachmentReference2 resolve_attachment_;
    VkAttachmentReference2 ds_attachment_;

    VkSubpassDescriptionDepthStencilResolve ds_attachment_resolve_;
    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment_;

    VkSubpassDescription2 subpass_description_;
    VkSubpassDependency2 subpass_dependency_;
};