/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "utils/convert_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

class NegativeRenderPass : public VkLayerTest {};

TEST_F(NegativeRenderPass, AttachmentIndexOutOfRange) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // There are no attachments, but refer to attachment 0.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);

    // "... must be less than the total number of attachments ..."
    TestRenderPassCreate(m_errorMonitor, *m_device, rp.GetCreateInfo(), true, "VUID-VkRenderPassCreateInfo-attachment-00834",
                         "VUID-VkRenderPassCreateInfo2-attachment-03051");
}

TEST_F(NegativeRenderPass, AttachmentReadOnlyButCleared) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    const bool maintenance2Supported = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    VkAttachmentDescription description = {0,
                                           ds_format,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // loadOp
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // stencilLoadOp
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);

    // Test both cases when rp2 is not supported

    // Set loadOp to clear
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-00836",
                         "VUID-VkRenderPassCreateInfo2-pAttachments-02522");

    if (maintenance2Supported == true) {
        depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-01566",
                             "VUID-VkRenderPassCreateInfo2-pAttachments-02522");
    }

    description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // reset

    // Set stencilLoadOp to clear
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-02511",
                         "VUID-VkRenderPassCreateInfo2-pAttachments-02523");

    if (maintenance2Supported == true) {
        depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-01567",
                             "VUID-VkRenderPassCreateInfo2-pAttachments-02523");
    }

    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // reset
}

TEST_F(NegativeRenderPass, AttachmentMismatchingLayoutsColor) {
    TEST_DESCRIPTION("Attachment is used simultaneously as two color attachments with different layouts.");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);

    TestRenderPassCreate(m_errorMonitor, *m_device, rp.GetCreateInfo(), true, "VUID-VkSubpassDescription-layout-02519",
                         "VUID-VkSubpassDescription2-layout-02528");
}

TEST_F(NegativeRenderPass, AttachmentDescriptionFinalLayout) {
    TEST_DESCRIPTION("VkAttachmentDescription's finalLayout must not be UNDEFINED or PREINITIALIZED");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);  // required for invalid enums
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAttachmentReference attach_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-00843");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-00843");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto depth_format = FindSupportedDepthOnlyFormat(Gpu());
    auto stencil_format = FindSupportedStencilOnlyFormat(Gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    // Depth
    {
        attach_desc.format = depth_format;

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285");

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // Stencil
    {
        attach_desc.format = stencil_format;

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                             "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                             "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285");

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
}

TEST_F(NegativeRenderPass, AttachmentDescriptionFinalLayoutSeperateDS) {
    TEST_DESCRIPTION("VkAttachmentDescription's finalLayout must not be UNDEFINED or PREINITIALIZED");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAttachmentReference attach_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-00843");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-00843");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto depth_format = FindSupportedDepthOnlyFormat(Gpu());
    auto depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());
    auto stencil_format = FindSupportedStencilOnlyFormat(Gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                         "VUID-VkAttachmentDescription2-format-03286");
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                         "VUID-VkAttachmentDescription2-format-03286");
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                         "VUID-VkAttachmentDescription2-format-03286");
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                         "VUID-VkAttachmentDescription2-format-03286");

    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                         "VUID-VkAttachmentDescription2-format-03287");
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                         "VUID-VkAttachmentDescription2-format-03287");
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                         "VUID-VkAttachmentDescription2-format-03287");
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                         "VUID-VkAttachmentDescription2-format-03287");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    // depth
    {
        attach_desc.format = depth_format;

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03290",
                             "VUID-VkAttachmentDescription2-format-03290");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03290",
                             "VUID-VkAttachmentDescription2-format-03290");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03291",
                             "VUID-VkAttachmentDescription2-format-03291");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03291",
                             "VUID-VkAttachmentDescription2-format-03291");

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // stencil
    {
        attach_desc.format = stencil_format;

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03292",
                             "VUID-VkAttachmentDescription2-format-06247");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03292",
                             "VUID-VkAttachmentDescription2-format-06247");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03293",
                             "VUID-VkAttachmentDescription2-format-06248");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03293",
                             "VUID-VkAttachmentDescription2-format-06248");

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    if (rp2Supported) {
        attach_desc.format = depth_stencil_format;
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

        VkAttachmentDescriptionStencilLayout attachment_description_stencil_layout = vku::InitStructHelper();
        attachment_description_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_GENERAL;
        auto rpci2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);
        rpci2.pAttachments[0].pNext = &attachment_description_stencil_layout;

        VkImageLayout forbidden_layouts[] = {
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        };
        auto forbidden_layouts_array_size = sizeof(forbidden_layouts) / sizeof(forbidden_layouts[0]);

        for (size_t i = 0; i < forbidden_layouts_array_size; ++i) {
            attachment_description_stencil_layout.stencilInitialLayout = forbidden_layouts[i];
            TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(),
                                     {"VUID-VkAttachmentDescriptionStencilLayout-stencilInitialLayout-03308"});
        }
        attachment_description_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        for (size_t i = 0; i < forbidden_layouts_array_size; ++i) {
            attachment_description_stencil_layout.stencilFinalLayout = forbidden_layouts[i];
            TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(),
                                     {"VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03309"});
        }
        attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(),
                                 {"VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310"});
        attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(),
                                 {"VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310"});

        rpci2.pAttachments[0].pNext = nullptr;
    }
}

TEST_F(NegativeRenderPass, AttachmentDescriptionFinalLayoutSync2) {
    TEST_DESCRIPTION("VkAttachmentDescription's finalLayout must not be UNDEFINED or PREINITIALIZED");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);  // required for invalid enums
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAttachmentReference attach_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-00843");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-00843");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());
    auto stencil_format = FindSupportedStencilOnlyFormat(Gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    // Test invalid layouts for color formats
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03280",
                         "VUID-VkAttachmentDescription2-format-03280");

    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03282",
                         "VUID-VkAttachmentDescription2-format-03282");

    // invalid formats without synchronization2
    {
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-synchronization2-06908",
                             "VUID-VkAttachmentDescription2-synchronization2-06908");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-synchronization2-06908",
                             "VUID-VkAttachmentDescription2-synchronization2-06908");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-synchronization2-06909",
                             "VUID-VkAttachmentDescription2-synchronization2-06909");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-synchronization2-06909",
                             "VUID-VkAttachmentDescription2-synchronization2-06909");

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // Test invalid layouts for depth/stencil format
    {
        attach_desc.format = depth_stencil_format;
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03281",
                             "VUID-VkAttachmentDescription2-format-03281");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03283",
                             "VUID-VkAttachmentDescription2-format-03283");
    }
}

TEST_F(NegativeRenderPass, AttachmentsMisc) {
    TEST_DESCRIPTION(
        "Ensure that CreateRenderPass produces the expected validation errors when a subpass's attachments violate the valid usage "
        "conditions.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    std::vector<VkAttachmentDescription> attachments = {
        // input attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth attachment
        {0, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // preserve attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth non-resolve attachment
        {0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };

    std::vector<VkAttachmentReference> input = {
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    std::vector<VkAttachmentReference> color = {
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    std::vector<VkAttachmentReference> resolve = {
        {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    std::vector<uint32_t> preserve = {5};
    std::vector<VkAttachmentReference> depth_1bit = {
        {6, VK_IMAGE_LAYOUT_GENERAL},
        {6, VK_IMAGE_LAYOUT_GENERAL},
    };

    VkSubpassDescription subpass = {0,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    size32(input),
                                    input.data(),
                                    size32(color),
                                    color.data(),
                                    resolve.data(),
                                    &depth,
                                    size32(preserve),
                                    preserve.data()};

    auto rpci =
        vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachments), attachments.data(), 1u, &subpass, 0u, nullptr);

    // Test too many color attachments
    const uint32_t max_color_attachments = m_device->Physical().limits_.maxColorAttachments;
    const uint32_t too_big_max_attachments = 65536 + 1;  // let's say this is too much to allocate
    if (max_color_attachments >= too_big_max_attachments) {
        printf("VkPhysicalDeviceLimits::maxColorAttachments is too large to practically test against -- skipping part of test.\n");
    } else {
        std::vector<VkAttachmentReference> too_many_colors(max_color_attachments + 1, color[0]);
        VkSubpassDescription test_subpass = subpass;
        test_subpass.colorAttachmentCount = size32(too_many_colors);
        test_subpass.pColorAttachments = too_many_colors.data();
        test_subpass.pResolveAttachments = NULL;
        VkRenderPassCreateInfo test_rpci = rpci;
        test_rpci.pSubpasses = &test_subpass;

        TestRenderPassCreate(m_errorMonitor, *m_device, test_rpci, rp2Supported,
                             "VUID-VkSubpassDescription-colorAttachmentCount-00845",
                             "VUID-VkSubpassDescription2-colorAttachmentCount-03063");
    }

    // Test sample count mismatch between color buffers
    attachments[subpass.pColorAttachments[1].attachment].samples = VK_SAMPLE_COUNT_8_BIT;
    depth.attachment = VK_ATTACHMENT_UNUSED;  // Avoids triggering 01418

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pColorAttachments-09430",
                         "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872");

    depth.attachment = 3;
    attachments[subpass.pColorAttachments[1].attachment].samples = attachments[subpass.pColorAttachments[0].attachment].samples;

    // Test sample count mismatch between color buffers and depth buffer
    attachments[subpass.pDepthStencilAttachment->attachment].samples = VK_SAMPLE_COUNT_8_BIT;
    subpass.colorAttachmentCount = 1;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pDepthStencilAttachment-01418",
                         "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872");

    attachments[subpass.pDepthStencilAttachment->attachment].samples = attachments[subpass.pColorAttachments[0].attachment].samples;
    subpass.colorAttachmentCount = size32(color);

    // Test resolve attachment with UNUSED color attachment
    color[0].attachment = VK_ATTACHMENT_UNUSED;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pResolveAttachments-00847",
                         "VUID-VkSubpassDescription2-externalFormatResolve-09335");

    color[0].attachment = 1;

    // Test resolve from a single-sampled color attachment
    attachments[subpass.pColorAttachments[0].attachment].samples = VK_SAMPLE_COUNT_1_BIT;
    subpass.colorAttachmentCount = 1;           // avoid mismatch (00337), and avoid double report
    subpass.pDepthStencilAttachment = nullptr;  // avoid mismatch (01418)

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pResolveAttachments-00848",
                         "VUID-VkSubpassDescription2-externalFormatResolve-09338");

    attachments[subpass.pColorAttachments[0].attachment].samples = VK_SAMPLE_COUNT_4_BIT;
    subpass.colorAttachmentCount = size32(color);
    subpass.pDepthStencilAttachment = &depth;

    // Test resolve to a multi-sampled resolve attachment
    attachments[subpass.pResolveAttachments[0].attachment].samples = VK_SAMPLE_COUNT_4_BIT;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pResolveAttachments-00849",
                         "VUID-VkSubpassDescription2-pResolveAttachments-03067");

    attachments[subpass.pResolveAttachments[0].attachment].samples = VK_SAMPLE_COUNT_1_BIT;

    // Test with color/resolve format mismatch
    attachments[subpass.pColorAttachments[0].attachment].format = VK_FORMAT_R8G8B8A8_SRGB;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pResolveAttachments-00850",
                         "VUID-VkSubpassDescription2-externalFormatResolve-09339");

    attachments[subpass.pColorAttachments[0].attachment].format = attachments[subpass.pResolveAttachments[0].attachment].format;

    // Test for UNUSED preserve attachments
    preserve[0] = VK_ATTACHMENT_UNUSED;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-attachment-00853",
                         "VUID-VkSubpassDescription2-attachment-03073");

    preserve[0] = 5;
    // Test for preserve attachments used elsewhere in the subpass
    color[0].attachment = preserve[0];

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pPreserveAttachments-00854",
                         "VUID-VkSubpassDescription2-pPreserveAttachments-03074");

    color[0].attachment = 1;
    input[0].attachment = 0;
    input[0].layout = VK_IMAGE_LAYOUT_GENERAL;

    // Test for attachment used first as input with loadOp=CLEAR
    {
        std::vector<VkSubpassDescription> subpasses = {subpass, subpass, subpass};
        subpasses[0].inputAttachmentCount = 0;
        subpasses[1].inputAttachmentCount = 0;
        attachments[input[0].attachment].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        auto rpci_multipass = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachments), attachments.data(),
                                                                      size32(subpasses), subpasses.data(), 0u, nullptr);

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci_multipass, rp2Supported, "VUID-VkSubpassDescription-loadOp-00846",
                             "VUID-VkSubpassDescription2-loadOp-03064");

        attachments[input[0].attachment].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    // Test for depthStencil and color pointing to same attachment
    {
        // Both use same VkAttachmentReference
        VkSubpassDescription subpass_same = {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, depth_1bit.data(), nullptr, depth_1bit.data(), 0, nullptr};

        VkRenderPassCreateInfo rpci_same = rpci;
        rpci_same.pSubpasses = &subpass_same;

        // only test rp1 so can ignore the expected 2nd error
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription-pColorAttachments-02648");
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci_same, false, "VUID-VkSubpassDescription-pDepthStencilAttachment-04438",
                             nullptr);

        if (rp2Supported) {
            auto create_info2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci_same);
            m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pColorAttachments-02898");
            TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *create_info2.ptr(),
                                     {"VUID-VkSubpassDescription2-pDepthStencilAttachment-04440"});
        }

        // Same test but use 2 different VkAttachmentReference to point to same attachment
        subpass_same.pDepthStencilAttachment = &depth_1bit.data()[1];

        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription-pColorAttachments-02648");
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci_same, false, "VUID-VkSubpassDescription-pDepthStencilAttachment-04438",
                             nullptr);

        if (rp2Supported) {
            auto create_info2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci_same);
            m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pColorAttachments-02898");
            TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *create_info2.ptr(),
                                     {"VUID-VkSubpassDescription2-pDepthStencilAttachment-04440"});
        }
    }
}

TEST_F(NegativeRenderPass, ShaderResolveQCOM) {
    TEST_DESCRIPTION("Ensure RenderPass create meets the requirements for QCOM_render_pass_shader_resolve");

    AddRequiredExtensions(VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    std::vector<VkAttachmentDescription> attachments = {
        // input attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth attachment
        {0, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    std::vector<VkAttachmentReference> input = {
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    std::vector<VkAttachmentReference> color = {
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::vector<VkAttachmentReference> resolve = {
        {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, size32(input), input.data(), size32(color), color.data(), nullptr, &depth, 0, nullptr};

    std::vector<VkSubpassDependency> dependency = {
        {0, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_MEMORY_WRITE_BIT,
         VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT},
    };

    auto rpci =
        vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachments), attachments.data(), 1u, &subpass, 0u, nullptr);

    // Create a resolve subpass where the pResolveattachments are not VK_ATTACHMENT_UNUSED
    VkSubpassDescription test_subpass = subpass;
    test_subpass.pResolveAttachments = resolve.data();
    test_subpass.flags = VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM;
    VkRenderPassCreateInfo test_rpci = rpci;
    test_rpci.pSubpasses = &test_subpass;

    TestRenderPassCreate(m_errorMonitor, *m_device, test_rpci, rp2Supported, "VUID-VkSubpassDescription-flags-03341",
                         "VUID-VkRenderPassCreateInfo2-flags-04907");

    // Create a resolve subpass which is not the last subpass in the subpass dependency chain.
    {
        VkSubpassDescription subpasses[2] = {subpass, subpass};
        subpasses[0].pResolveAttachments = nullptr;
        subpasses[0].flags = VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM;
        subpasses[1].pResolveAttachments = nullptr;
        subpasses[1].flags = 0;

        auto test2_rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachments), attachments.data(), 2u,
                                                                  subpasses, size32(dependency), dependency.data());

        TestRenderPassCreate(m_errorMonitor, *m_device, test2_rpci, rp2Supported, "VUID-VkSubpassDescription-flags-03343",
                             "VUID-VkRenderPassCreateInfo2-flags-04909");
    }
}

TEST_F(NegativeRenderPass, AttachmentReferenceLayout) {
    TEST_DESCRIPTION("Attachment reference uses PREINITIALIZED or UNDEFINED layouts");

    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    const std::array attachments = {
        VkAttachmentDescription{0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        VkAttachmentDescription{0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
    };
    std::array refs = {
        VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},          // color
        VkAttachmentReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},  // depth stencil
    };
    const std::array subpasses = {
        VkSubpassDescription{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &refs[0], nullptr, &refs[1], 0, nullptr},
    };

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, static_cast<uint32_t>(attachments.size()), attachments.data(),
                                                        static_cast<uint32_t>(subpasses.size()), subpasses.data(), 0u, nullptr);

    // Use UNDEFINED layout
    refs[0].layout = VK_IMAGE_LAYOUT_UNDEFINED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentReference-layout-03077",
                         "VUID-VkAttachmentReference2-layout-03077");

    // Use PREINITIALIZED layout
    refs[0].layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentReference-layout-03077",
                         "VUID-VkAttachmentReference2-layout-03077");

    if (rp2Supported) {
        auto rpci2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);

        // set valid values to start
        rpci2.pSubpasses[0].pColorAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(
            *m_errorMonitor, *m_device, *rpci2.ptr(),
            {"VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313", "VUID-VkRenderPassCreateInfo2-attachment-06244"});
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TestRenderPass2KHRCreate(
            *m_errorMonitor, *m_device, *rpci2.ptr(),
            {"VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313", "VUID-VkRenderPassCreateInfo2-attachment-06244"});

        rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(
            *m_errorMonitor, *m_device, *rpci2.ptr(),
            {"VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313", "VUID-VkRenderPassCreateInfo2-attachment-06245"});
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPass2KHRCreate(
            *m_errorMonitor, *m_device, *rpci2.ptr(),
            {"VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313", "VUID-VkRenderPassCreateInfo2-attachment-06245"});
    }

    // test RenderPass 1
    refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    refs[1].layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-attachment-06244");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkAttachmentReference-separateDepthStencilLayouts-03313",
                         nullptr);
}

TEST_F(NegativeRenderPass, AttachmentReferenceLayoutSeparateDepthStencilLayoutsFeature) {
    TEST_DESCRIPTION("Attachment reference uses PREINITIALIZED or UNDEFINED layouts");

    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());
    const VkFormat stencil_format = VK_FORMAT_S8_UINT;

    const std::array attachments = {
        VkAttachmentDescription{0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        VkAttachmentDescription{0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        VkAttachmentDescription{0, stencil_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL}};

    VkImageFormatProperties imageFormatProperties;
    VkResult res;
    res = vk::GetPhysicalDeviceImageFormatProperties(gpu_, attachments[0].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0u, &imageFormatProperties);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not supported";
    }
    res = vk::GetPhysicalDeviceImageFormatProperties(gpu_, attachments[1].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0u, &imageFormatProperties);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not supported";
    }
    res = vk::GetPhysicalDeviceImageFormatProperties(gpu_, attachments[2].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0u, &imageFormatProperties);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not supported";
    }

    std::array refs = {
        VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},          // color
        VkAttachmentReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},  // depth stencil
    };
    const std::array subpasses = {
        VkSubpassDescription{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &refs[0], nullptr, &refs[1], 0, nullptr},
    };

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, static_cast<uint32_t>(attachments.size()), attachments.data(),
                                                        static_cast<uint32_t>(subpasses.size()), subpasses.data(), 0u, nullptr);

    // Use UNDEFINED layout
    refs[0].layout = VK_IMAGE_LAYOUT_UNDEFINED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentReference-layout-03077",
                         "VUID-VkAttachmentReference2-layout-03077");

    // Use PREINITIALIZED layout
    refs[0].layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentReference-layout-03077",
                         "VUID-VkAttachmentReference2-layout-03077");

    if (rp2Supported) {
        auto rpci2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);

        // set valid values to start
        rpci2.pSubpasses[0].pColorAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

        // Set a valid VkAttachmentReferenceStencilLayout since the feature bit is set
        VkAttachmentReferenceStencilLayout attachment_reference_stencil_layout = vku::InitStructHelper();
        attachment_reference_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        rpci2.pSubpasses[0].pDepthStencilAttachment->pNext = &attachment_reference_stencil_layout;

        // reset to valid layout
        // The following tests originally were negative tests until it was noticed that the aspectMask only matters for input
        // attachments. These tests were converted into positive tests to catch regression
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        {
            rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            PositiveTestRenderPass2KHRCreate(*m_device, *rpci2.ptr());
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
            PositiveTestRenderPass2KHRCreate(*m_device, *rpci2.ptr());
        }
        {
            rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            const auto original_attachment = rpci2.pSubpasses[0].pDepthStencilAttachment->attachment;
            rpci2.pSubpasses[0].pDepthStencilAttachment->attachment = 2;

            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            PositiveTestRenderPass2KHRCreate(*m_device, *rpci2.ptr());
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            PositiveTestRenderPass2KHRCreate(*m_device, *rpci2.ptr());

            rpci2.pSubpasses[0].pDepthStencilAttachment->attachment = original_attachment;
        }
        {
            rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            PositiveTestRenderPass2KHRCreate(*m_device, *rpci2.ptr());
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
            PositiveTestRenderPass2KHRCreate(*m_device, *rpci2.ptr());
        }

        rpci2.pAttachments[1].format = ds_format;                                                                // reset
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;  // reset

        std::array forbidden_layouts = {
            VK_IMAGE_LAYOUT_PREINITIALIZED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
            // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // Would need VK_KHR_swapchain
        };
        rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        for (size_t i = 0; i < forbidden_layouts.size(); ++i) {
            attachment_reference_stencil_layout.stencilLayout = forbidden_layouts[i];
            TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(),
                                     {"VUID-VkAttachmentReferenceStencilLayout-stencilLayout-03318"});
        }

        attachment_reference_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescriptionStencilLayout attachment_description_stencil_layout = vku::InitStructHelper();
        attachment_description_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

        rpci2.pAttachments[1].pNext = &attachment_description_stencil_layout;

        rpci2.pAttachments[1].initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        rpci2.pAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(), {"VUID-VkAttachmentDescription2-format-06906"});

        rpci2.pAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        rpci2.pAttachments[1].finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(), {"VUID-VkAttachmentDescription2-format-06907"});

        rpci2.pAttachments[1].pNext = nullptr;

        rpci2.pAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        rpci2.pAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(), {"VUID-VkAttachmentDescription2-format-06249"});

        rpci2.pAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        rpci2.pAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *rpci2.ptr(), {"VUID-VkAttachmentDescription2-format-06250"});

        rpci2.pSubpasses[0].pDepthStencilAttachment->pNext = nullptr;
    }
}

TEST_F(NegativeRenderPass, AttachmentReferenceSync2Layout) {
    TEST_DESCRIPTION("Attachment reference uses sync2 and ATTACHMENT_OPTIMAL_KHR or READ_ONLY_OPTIMAL_KHR layouts");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    // synchronization2 not enabled
    RETURN_IF_SKIP(Init());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},          // color
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},  // depth stencil
    };
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &refs[0], nullptr, &refs[1], 0, nullptr},
    };

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 2u, attach, 1u, subpasses, 0u, nullptr);

    // Use READ_ONLY_OPTIMAL_KHR layout
    refs[0].layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    {
        m_errorMonitor->SetDesiredError("VUID-VkAttachmentReference-synchronization2-06910");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription-attachment-06922");
        vkt::RenderPass rp(*m_device, rpci);
        m_errorMonitor->VerifyFound();
    }
    {
        auto rpci2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);
        m_errorMonitor->SetDesiredError("VUID-VkAttachmentReference2-synchronization2-06910");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-attachment-06922");
        vkt::RenderPass rp2_core(*m_device, *rpci2.ptr());
        m_errorMonitor->VerifyFound();
    }

    // Use ATTACHMENT_OPTIMAL_KHR layout
    refs[0].layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, true, "VUID-VkAttachmentReference-synchronization2-06910",
                         "VUID-VkAttachmentReference2-synchronization2-06910");
}

TEST_F(NegativeRenderPass, MixedAttachmentSamplesAMD) {
    TEST_DESCRIPTION("Verify error messages for supported and unsupported sample counts in render pass attachments.");

    AddRequiredExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    std::vector<VkAttachmentDescription> attachments;

    {
        VkAttachmentDescription att = {};
        att.format = VK_FORMAT_R8G8B8A8_UNORM;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments.push_back(att);

        att.format = VK_FORMAT_D16_UNORM;
        att.samples = VK_SAMPLE_COUNT_4_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(att);
    }

    VkAttachmentReference color_ref = {};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_ref = {};
    depth_ref.attachment = 1;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = attachments.size();
    rpci.pAttachments = attachments.data();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    {
        // creating and destroying a RenderPass1 should work;
        vkt::RenderPass(*m_device, rpci);
    }

    // Expect an error message for invalid sample counts
    attachments[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-None-09431",
                         "VUID-VkSubpassDescription2-None-09456");
}

TEST_F(NegativeRenderPass, BeginRenderArea) {
    TEST_DESCRIPTION("Generate INVALID_RENDER_AREA error by beginning renderpass with extent outside of framebuffer");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    InitRenderTarget();

    // Framebuffer for render target is 256x256, exceed that for INVALID_RENDER_AREA
    m_renderPassBeginInfo.renderArea.extent.width = 257;
    m_renderPassBeginInfo.renderArea.extent.height = 256;

    const char *vuid = "VUID-VkRenderPassBeginInfo-pNext-02852";
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &m_renderPassBeginInfo, rp2Supported, vuid, vuid);

    m_renderPassBeginInfo.renderArea.offset.x = 1;
    m_renderPassBeginInfo.renderArea.extent.width = vvl::MaxTypeValue(m_renderPassBeginInfo.renderArea.extent.width) - 1;
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &m_renderPassBeginInfo, rp2Supported, vuid, vuid);

    m_renderPassBeginInfo.renderArea.offset.x = vvl::MaxTypeValue(m_renderPassBeginInfo.renderArea.offset.x);
    m_renderPassBeginInfo.renderArea.extent.width = vvl::MaxTypeValue(m_renderPassBeginInfo.renderArea.extent.width);
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &m_renderPassBeginInfo, rp2Supported, vuid, vuid);

    m_renderPassBeginInfo.renderArea.offset.x = 0;
    m_renderPassBeginInfo.renderArea.extent.width = 256;
    m_renderPassBeginInfo.renderArea.offset.y = 1;
    m_renderPassBeginInfo.renderArea.extent.height = vvl::MaxTypeValue(m_renderPassBeginInfo.renderArea.extent.height) - 1;
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &m_renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-pNext-02853", "VUID-VkRenderPassBeginInfo-pNext-02853");
}

TEST_F(NegativeRenderPass, BeginWithinRenderPass) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    InitRenderTarget();

    // Bind a BeginRenderPass within an active RenderPass
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Just use a dummy Renderpass
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass-renderpass");
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        auto subpassBeginInfo = vku::InitStruct<VkSubpassBeginInfo>(nullptr, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass2-renderpass");
        vk::CmdBeginRenderPass2KHR(m_command_buffer.handle(), &m_renderPassBeginInfo, &subpassBeginInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRenderPass, BeginIncompatibleFramebuffer) {
    TEST_DESCRIPTION("Test that renderpass begin is compatible with the framebuffer renderpass ");

    RETURN_IF_SKIP(Init());

    // Create a depth stencil image view
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView dsv = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D16_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_LOAD,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);
    vkt::RenderPass rp1(*m_device, rpci);

    subpass.pDepthStencilAttachment = nullptr;
    vkt::RenderPass rp2(*m_device, rpci);
    vkt::Framebuffer fb(*m_device, rp1.handle(), 1u, &dsv.handle(), 128, 128);

    auto rp_begin =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp2.handle(), fb.handle(), VkRect2D{{0, 0}, {128u, 128u}}, 0u, nullptr);

    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rp_begin, false,
                        "VUID-VkRenderPassBeginInfo-renderPass-00904", nullptr);
}

TEST_F(NegativeRenderPass, BeginLayoutsFramebufferImageUsageMismatches) {
    TEST_DESCRIPTION(
        "Test that renderpass initial/final layouts match up with the usage bits set for each attachment of the framebuffer");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    const bool feedback_loop_layout = IsExtensionsEnabled(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    const bool maintenance2Supported = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    if (feedback_loop_layout) {
        VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT attachment_feedback_loop_layout_features = vku::InitStructHelper();
        auto features2 = GetPhysicalDeviceFeatures2(attachment_feedback_loop_layout_features);
        attachment_feedback_loop_layout_features.attachmentFeedbackLoopLayout = true;
        RETURN_IF_SKIP(InitState(nullptr, &features2));
    } else {
        RETURN_IF_SKIP(InitState());
    }

    // Create an input attachment view
    vkt::Image iai(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView iav = iai.CreateView();

    // Create an input depth attachment view
    VkFormat dformat = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image iadi(*m_device, 128, 128, 1, dformat, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView iadv = iadi.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    // Create a color attachment view
    vkt::Image cai(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView cav = cai.CreateView();

    // Create a renderPass with those attachments
    VkAttachmentDescription descriptions[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        {1, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL}};

    VkAttachmentReference input_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &input_ref, 1, &color_ref, nullptr, nullptr, 0, nullptr};

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 2u, descriptions, 1u, &subpass, 0u, nullptr);

    // Create a framebuffer

    VkImageView views[] = {iav.handle(), cav.handle()};

    auto fbci = vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, VK_NULL_HANDLE, 2u, views, 128u, 128u, 1u);

    VkClearValue clearValues[2];
    clearValues[0].color = {{0, 0, 0, 0}};
    clearValues[1].color = {{0, 0, 0, 0}};
    auto rp_begin = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, VK_NULL_HANDLE, VK_NULL_HANDLE, VkRect2D{{0, 0}, {128u, 128u}},
                                                           2u, clearValues);

    auto test_layout_helper = [this, &rpci, &rp_begin, rp2Supported, &fbci](const char *rp1_vuid, const char *rp2_vuid) {
        vkt::RenderPass rp_invalid(*m_device, rpci);
        fbci.renderPass = rp_invalid.handle();
        vkt::Framebuffer fb_invalid(*m_device, fbci);
        rp_begin.renderPass = rp_invalid.handle();
        rp_begin.framebuffer = fb_invalid.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rp_begin, rp2Supported, rp1_vuid, rp2_vuid);
    };

    // Initial layout is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but attachment doesn't support IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00895", "VUID-vkCmdBeginRenderPass2-initialLayout-03094");

    // Initial layout is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    // / VK_IMAGE_USAGE_SAMPLED_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00897", "VUID-vkCmdBeginRenderPass2-initialLayout-03097");

    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_GENERAL;

    // Initial layout is VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00898", "VUID-vkCmdBeginRenderPass2-initialLayout-03098");

    // Initial layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00899", "VUID-vkCmdBeginRenderPass2-initialLayout-03099");

    // Change to depth views since we are changing format
    descriptions[0].format = dformat;
    views[0] = iadv;

    // Initial layout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL but attachment doesn't support
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2-initialLayout-03096");

    // Initial layout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL but attachment doesn't support
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2-initialLayout-03096");

    if (maintenance2Supported || rp2Supported) {
        // Initial layout is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL but attachment doesn't support
        // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2-initialLayout-03096");

        // Initial layout is VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL but attachment doesn't support
        // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2-initialLayout-03096");
    }

    if (feedback_loop_layout) {
        // No VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT
        vkt::Image no_fb_loop_attachment(
            *m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        vkt::ImageView image_view_no_fb_loop;
        auto image_view_ci = no_fb_loop_attachment.BasicViewCreatInfo();
        image_view_no_fb_loop.init(*m_device, image_view_ci);
        views[0] = image_view_no_fb_loop.handle();
        descriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
        test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-07001", "VUID-vkCmdBeginRenderPass2-initialLayout-07003");

        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        descriptions[0].format = dformat;
        views[0] = iadv;
        // No VK_IMAGE_USAGE_SAMPLED_BIT_EXT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        vkt::Image no_usage_sampled_attachment(
            *m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);
        image_view_ci.image = no_usage_sampled_attachment.handle();
        vkt::ImageView image_view_no_usage_sampled;
        image_view_no_usage_sampled.init(*m_device, image_view_ci);
        descriptions[1].initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
        views[1] = image_view_no_usage_sampled.handle();
        test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-07000", "VUID-vkCmdBeginRenderPass2-initialLayout-07002");
    }
}

TEST_F(NegativeRenderPass, BeginLayoutsStencilBufferImageUsageMismatches) {
    TEST_DESCRIPTION("Test that separate stencil initial/final layouts match up with the usage bits in framebuffer attachment");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);  // Because TestRenderPassBegin relies on it
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());

    // Closure to create a render pass with just a depth/stencil image used as an input attachment (not a depth attachment!).
    // This image purposely has not VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.
    // The layout of the depth and stencil aspects of this image can be defined separately, allowing to trigger different errors.
    auto test = [this](VkImageLayout depth_initial_layout, VkImageLayout stencil_initial_layout, const char *rp1_vuid,
                       const char *rp2_vuid) {
        // Create an input attachment with a depth stencil format, without VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());
        vkt::Image input_image(*m_device, 128, 128, 1, depth_stencil_format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

        vkt::ImageView input_view = input_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

        VkAttachmentDescriptionStencilLayout stencil_layout = vku::InitStructHelper();
        stencil_layout.stencilInitialLayout = stencil_initial_layout;
        stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkAttachmentReferenceStencilLayout stencil_ref = vku::InitStructHelper();
        stencil_ref.stencilLayout = VK_IMAGE_LAYOUT_GENERAL;

        RenderPass2SingleSubpass rp(*this);
        rp.AddAttachmentDescription(depth_stencil_format, depth_initial_layout, VK_IMAGE_LAYOUT_GENERAL);
        rp.SetAttachmentDescriptionPNext(0, &stencil_layout);
        rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                  &stencil_ref);
        rp.AddInputAttachment(0);
        rp.CreateRenderPass();

        const uint32_t fb_width = input_image.Width();
        const uint32_t fb_height = input_image.Height();
        vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &input_view.handle(), fb_width, fb_height);

        // Begin render pass and trigger errors
        VkRenderPassBeginInfo rp_begin = vku::InitStructHelper();
        rp_begin.renderPass = rp.Handle();
        rp_begin.framebuffer = fb;
        rp_begin.renderArea.extent = {fb_width, fb_height};

        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rp_begin, true, rp1_vuid, rp2_vuid);
    };

    test(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, "VUID-vkCmdBeginRenderPass-initialLayout-02842",
         "VUID-vkCmdBeginRenderPass2-initialLayout-02844");

    test(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, "VUID-vkCmdBeginRenderPass-stencilInitialLayout-02843",
         "VUID-vkCmdBeginRenderPass2-stencilInitialLayout-02845");
}

TEST_F(NegativeRenderPass, BeginStencilFormat) {
    TEST_DESCRIPTION("Test that separate stencil initial/final layouts match up with the usage bits in framebuffer attachment");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());

    // Closure to create a render pass with just a depth/stencil image with specified format.
    // The layout is set to have more or less components than what this format has, triggering an error.
    auto test = [this](VkFormat depth_stencil_format, VkImageLayout depth_stencil_attachment_ref_layout, const char *vuid) {
        VkImageFormatProperties imageFormatProperties;
        VkResult res;
        res = vk::GetPhysicalDeviceImageFormatProperties(gpu_, depth_stencil_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0u, &imageFormatProperties);
        if (res != VK_SUCCESS) {
            return;
        }
        // Create an input attachment with a depth stencil format, without VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        vkt::Image depth_stencil_image(*m_device, 128, 128, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        // Create the render pass attachment...
        VkAttachmentDescription2 depth_stencil_attachment_desc = vku::InitStructHelper();
        depth_stencil_attachment_desc.format = depth_stencil_format;
        depth_stencil_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_stencil_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;  // This will trigger errors
        depth_stencil_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        // ... and a reference to it...
        VkAttachmentReference2 depth_stencil_attachment_ref = vku::InitStructHelper();
        depth_stencil_attachment_ref.attachment = 0;
        depth_stencil_attachment_ref.layout = depth_stencil_attachment_ref_layout;
        depth_stencil_attachment_ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

        // ... which is used in the one subpass.
        VkSubpassDescription2 subpass_desc = vku::InitStructHelper();
        subpass_desc.pDepthStencilAttachment = &depth_stencil_attachment_ref;

        // Create render pass
        VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
        rpci.attachmentCount = 1;
        rpci.pAttachments = &depth_stencil_attachment_desc;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass_desc;
        m_errorMonitor->SetDesiredError(vuid);
        vkt::RenderPass rp(*m_device, rpci);
        m_errorMonitor->VerifyFound();
    };

    test(FindSupportedDepthStencilFormat(Gpu()), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
         "VUID-VkRenderPassCreateInfo2-attachment-06244");
    test(FindSupportedDepthStencilFormat(Gpu()), VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
         "VUID-VkRenderPassCreateInfo2-attachment-06245");
    test(VK_FORMAT_S8_UINT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, "VUID-VkRenderPassCreateInfo2-attachment-06246");
}

TEST_F(NegativeRenderPass, BeginClearOpMismatch) {
    TEST_DESCRIPTION(
        "Begin a renderPass where clearValueCount is less than the number of renderPass attachments that use "
        "loadOp VK_ATTACHMENT_LOAD_OP_CLEAR.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    InitRenderTarget();

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    // Set loadOp to CLEAR
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    vkt::RenderPass rp(*m_device, rpci);

    VkRenderPassBeginInfo rp_begin = vku::InitStructHelper();
    rp_begin.renderPass = RenderPass();
    rp_begin.framebuffer = Framebuffer();
    rp_begin.renderArea.extent = {1, 1};
    rp_begin.clearValueCount = 0;  // Should be 1

    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rp_begin, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-clearValueCount-00902", "VUID-VkRenderPassBeginInfo-clearValueCount-00902");
}

TEST_F(NegativeRenderPass, BeginSampleLocationsIndicesEXT) {
    TEST_DESCRIPTION("Test that attachment indices and subpass indices specifed by sample locations structures are valid");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(sample_locations_props);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        GTEST_SKIP() << "VK_SAMPLE_COUNT_1_BIT sampleLocationSampleCounts is not supported";
    }

    // Create a depth stencil image view
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView dsv = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D16_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_LOAD,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);
    vkt::RenderPass rp(*m_device, rpci);
    vkt::Framebuffer fb(*m_device, rp.handle(), 1u, &dsv.handle(), 128, 128);

    VkSampleLocationEXT sample_location = {0.5, 0.5};

    auto sample_locations_info =
        vku::InitStruct<VkSampleLocationsInfoEXT>(nullptr, VK_SAMPLE_COUNT_1_BIT, VkExtent2D{1u, 1u}, 1u, &sample_location);

    VkAttachmentSampleLocationsEXT attachment_sample_locations = {0, sample_locations_info};
    VkSubpassSampleLocationsEXT subpass_sample_locations = {0, sample_locations_info};

    auto rp_sl_begin = vku::InitStruct<VkRenderPassSampleLocationsBeginInfoEXT>(nullptr, 1u, &attachment_sample_locations, 1u,
                                                                                &subpass_sample_locations);

    auto rp_begin =
        vku::InitStruct<VkRenderPassBeginInfo>(&rp_sl_begin, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {128u, 128u}}, 0u, nullptr);

    attachment_sample_locations.attachmentIndex = 1;
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rp_begin, false,
                        "VUID-VkAttachmentSampleLocationsEXT-attachmentIndex-01531", nullptr);
    attachment_sample_locations.attachmentIndex = 0;

    subpass_sample_locations.subpassIndex = 1;
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rp_begin, false,
                        "VUID-VkSubpassSampleLocationsEXT-subpassIndex-01532", nullptr);
}

TEST_F(NegativeRenderPass, DestroyWhileInUse) {
    TEST_DESCRIPTION("Delete in-use renderPass.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create simple renderpass
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), Framebuffer());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyRenderPass-renderPass-00873");
    vk::DestroyRenderPass(device(), rp.Handle(), nullptr);
    m_errorMonitor->VerifyFound();

    // Wait for queue to complete so we can safely destroy rp
    m_default_queue->Wait();
    m_errorMonitor->SetUnexpectedError("If renderPass is not VK_NULL_HANDLE, renderPass must be a valid VkRenderPass handle");
    m_errorMonitor->SetUnexpectedError("Was it created? Has it already been destroyed?");
}

TEST_F(NegativeRenderPass, FramebufferDepthStencilResolveAttachment) {
    TEST_DESCRIPTION("Create a framebuffer against a render pass using depth stencil resolve, with mismatched information");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = FindSupportedDepthStencilFormat(Gpu());

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormat, VK_SAMPLE_COUNT_4_BIT);  // Depth/stencil
    rp.AddAttachmentDescription(attachmentFormat, VK_SAMPLE_COUNT_1_BIT);  // Depth/stencil resolve
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddDepthStencilAttachment(0);
    rp.AddDepthStencilResolveAttachment(1, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
    rp.CreateRenderPass();

    // Depth resolve attachment, mismatched image usage
    // Try creating Framebuffer with images, but with invalid image create usage flags
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = attachmentFormat;
    image_create_info.extent.width = attachmentWidth;
    image_create_info.extent.height = attachmentHeight;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image ds_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image ds_resolve_image(*m_device, image_create_info, vkt::set_layout);

    vkt::ImageView depth_view = ds_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    vkt::ImageView resolve_view = ds_resolve_image.CreateView();
    VkImageView image_views[2];
    image_views[0] = depth_view;
    image_views[1] = resolve_view;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00880");
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-02634");
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, image_views, attachmentWidth, attachmentHeight);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferIncompatible) {
    TEST_DESCRIPTION(
        "Bind a secondary command buffer with a framebuffer that does not match the framebuffer for the active renderpass.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // A renderpass with one color attachment.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1u, &view.handle());

    VkCommandBufferAllocateInfo cbai = vku::InitStructHelper();
    cbai.commandPool = m_command_pool.handle();
    cbai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cbai.commandBufferCount = 1;

    vkt::CommandBuffer sec_cb(*m_device, cbai);

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = RenderPass();
    cbii.framebuffer = fb.handle();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;
    sec_cb.Begin(&cbbi);
    sec_cb.End();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00099");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &sec_cb.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, FramebufferIncompatibleNoHandle) {
    TEST_DESCRIPTION(
        "Bind a secondary command buffer with a framebuffer that does not match the framebuffer for the active renderpass.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkCommandBufferAllocateInfo cbai = vku::InitStructHelper();
    cbai.commandPool = m_command_pool.handle();
    cbai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cbai.commandBufferCount = 1;

    vkt::CommandBuffer sec_cb(*m_device, cbai);

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = RenderPass();
    cbii.framebuffer = CastFromUint64<VkFramebuffer>(0xFFFFEEEE);
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-00055");
    vk::BeginCommandBuffer(sec_cb.handle(), &cbbi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, NullRenderPass) {
    // Bind a NULL RenderPass
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass-pRenderPassBegin-parameter");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    // Don't care about RenderPass handle b/c error should be flagged before
    // that
    vk::CmdBeginRenderPass(m_command_buffer.handle(), nullptr, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, FramebufferAttachmentPointers) {
    TEST_DESCRIPTION("pAttachments points to valid objects for the Framebuffer creation");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(2);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.CreateRenderPass();

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper();
    fb_ci.width = 100;
    fb_ci.height = 100;
    fb_ci.layers = 1;
    fb_ci.renderPass = rp.Handle();
    fb_ci.attachmentCount = 2;

    fb_ci.pAttachments = nullptr;
    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-02778");
    vk::CreateFramebuffer(device(), &fb_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    vkt::ImageView render_target_view = m_renderTargets[0]->CreateView();
    VkImageView image_views[2] = {render_target_view, CastToHandle<VkImageView, uintptr_t>(0xbaadbeef)};

    fb_ci.pAttachments = image_views;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-02778");
    vk::CreateFramebuffer(device(), &fb_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, NullFramebufferCreateInfo) {
    RETURN_IF_SKIP(Init());
    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-vkCreateFramebuffer-pCreateInfo-parameter");
    vk::CreateFramebuffer(device(), nullptr, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, EndCommandBufferWithinRenderPass) {
    TEST_DESCRIPTION("End a command buffer with an active render pass");

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-commandBuffer-00060");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::EndCommandBuffer(m_command_buffer.handle());

    m_errorMonitor->VerifyFound();

    // End command buffer properly to avoid driver issues. This is safe -- the
    // previous vk::EndCommandBuffer should not have reached the driver.
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // TODO: Add test for VK_COMMAND_BUFFER_LEVEL_SECONDARY
    // TODO: Add test for VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
}

TEST_F(NegativeRenderPass, DrawWithPipelineIncompatibleWithRenderPass) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible cases. Initial case is drawing with an active renderpass that's not compatible with the bound "
        "pipeline state object's creation renderpass");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create a renderpass that will be incompatible with default renderpass
    RenderPassSingleSubpass rp(*this);
    // Format incompatible with PSO RP color attach format B8G8R8A8_UNORM
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = rp.Handle();
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cbbi);
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (the error should trigger on the attempt to draw).
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, DrawWithPipelineIncompatibleWithRenderPassFragmentDensityMap) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible case: drawing with an active renderpass that's not compatible with the bound pipeline state "
        "object's creation renderpass since only the former uses a Fragment Density Map.");

    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = vku::InitStructHelper();
    rpfdmi.fragmentDensityMapAttachment = ref;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpfdmi);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    // Create rp1 with FDM pNext and rp2 without FDM pNext
    vkt::RenderPass rp1(*m_device, rpci);
    ASSERT_TRUE(rp1.initialized());

    rpci.pNext = nullptr;
    rpci.attachmentCount = 1;
    vkt::RenderPass rp2(*m_device, rpci);
    ASSERT_TRUE(rp2.initialized());

    // Create image views
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView iv = image.CreateView();

    // Create a framebuffer with rp1
    vkt::Framebuffer fb(*m_device, rp1.handle(), 1u, &iv.handle(), 128, 128);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp2.handle();
    pipe.CreateGraphicsPipeline();

    // Begin renderpass and bind to pipeline
    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = rp1.handle();
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cbbi);
    m_command_buffer.BeginRenderPass(rp1.handle(), fb.handle(), 128, 128);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (the error should trigger on the attempt to draw).
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, MissingAttachment) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create a renderPass with a single color attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    vkt::ImageView iv = m_renderTargets[0]->CreateView();
    // Create the framebuffer then destory the view it uses.
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &iv.handle(), 100, 100);
    iv.destroy();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-parameter");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    // Don't call vk::CmdEndRenderPass; as the begin has been "skipped" based on the error condition
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

class RenderPassCreatePotentialFormatFeaturesTest : public NegativeRenderPass {
  public:
    void Test(bool const useLinearColorAttachment);
};

void RenderPassCreatePotentialFormatFeaturesTest::Test(bool const useLinearColorAttachment) {
    // Check for VK_KHR_get_physical_device_properties2
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (useLinearColorAttachment) {
        AddRequiredExtensions(VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME);
    }
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    if (useLinearColorAttachment) {
        VkPhysicalDeviceLinearColorAttachmentFeaturesNV linear_color_attachment = vku::InitStructHelper();
        VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(linear_color_attachment);
        if (useLinearColorAttachment && !linear_color_attachment.linearColorAttachment) {
            GTEST_SKIP() << "Test requires linearColorAttachment";
        }
        RETURN_IF_SKIP(InitState(nullptr, &features2));
    } else {
        RETURN_IF_SKIP(InitState());
    }

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Set format features from being found
    const VkFormat validColorFormat = VK_FORMAT_R8G8B8A8_UNORM;  // guaranteed to be valid everywhere
    const VkFormat invalidColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    const VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), invalidColorFormat, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), invalidColorFormat, formatProps);
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), depthFormat, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), depthFormat, formatProps);

    VkAttachmentDescription attachments[4] = {
        {0, validColorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        {0, invalidColorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        {0, validColorFormat, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        {0, depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL}};

    VkAttachmentReference references[4] = {
        {0, VK_IMAGE_LAYOUT_GENERAL},  // valid color
        {1, VK_IMAGE_LAYOUT_GENERAL},  // invalid color
        {2, VK_IMAGE_LAYOUT_GENERAL},  // valid color multisample
        {3, VK_IMAGE_LAYOUT_GENERAL}   // invalid depth stencil
    };

    VkSubpassDescription subpass = {};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &references[0];  // valid
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    VkSubpassDescription originalSubpass = subpass;

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 4u, attachments, 1u, &subpass, 0u, nullptr);

    // Color attachment
    subpass.pColorAttachments = &references[1];
    if (useLinearColorAttachment) {
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-linearColorAttachment-06497",
                             "VUID-VkSubpassDescription2-linearColorAttachment-06500");
    } else {
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pColorAttachments-02648",
                             "VUID-VkSubpassDescription2-pColorAttachments-02898");
    }
    subpass = originalSubpass;

    // Input attachment
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &references[1];
    if (useLinearColorAttachment) {
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-linearColorAttachment-06496",
                             "VUID-VkSubpassDescription2-linearColorAttachment-06499");
    } else {
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pInputAttachments-02647",
                             "VUID-VkSubpassDescription2-pInputAttachments-02897");
    }
    subpass = originalSubpass;

    // Depth Stencil attachment
    subpass.pDepthStencilAttachment = &references[3];
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pDepthStencilAttachment-02650",
                         "VUID-VkSubpassDescription2-pDepthStencilAttachment-02900");
    subpass = originalSubpass;

    // Resolve attachment
    subpass.pResolveAttachments = &references[1];
    subpass.pColorAttachments = &references[2];  // valid
    // Can't use helper function due to need to set unexpected errors
    {
        VkRenderPass render_pass = VK_NULL_HANDLE;

        if (useLinearColorAttachment) {
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription-linearColorAttachment-06498");
        } else {
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription-pResolveAttachments-02649");
        }
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription-pResolveAttachments-00850");
        vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
        m_errorMonitor->VerifyFound();

        if (rp2Supported) {
            auto create_info2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);

            if (useLinearColorAttachment) {
                m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-linearColorAttachment-06501");
            } else {
                m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-pResolveAttachments-09343");
            }
            m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-externalFormatResolve-09339");
            vk::CreateRenderPass2KHR(device(), create_info2.ptr(), nullptr, &render_pass);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(RenderPassCreatePotentialFormatFeaturesTest, Core) {
    TEST_DESCRIPTION("Validate PotentialFormatFeatures in renderpass create");

    Test(false);
}

TEST_F(RenderPassCreatePotentialFormatFeaturesTest, LinearColorAttachment) {
    TEST_DESCRIPTION("Validate PotentialFormatFeatures in renderpass create with linearColorAttachment");

    Test(true);
}

TEST_F(NegativeRenderPass, DepthStencilResolveMode) {
    TEST_DESCRIPTION("Test valid usage of the VkResolveModeFlagBits");

    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormat depthFormat = FindSupportedDepthOnlyFormat(Gpu());
    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(Gpu());
    VkFormat stencilFormat = FindSupportedStencilOnlyFormat(Gpu());
    if (stencilFormat == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    VkPhysicalDeviceDepthStencilResolveProperties ds_resolve_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ds_resolve_props);

    VkRenderPass renderPass;

    VkAttachmentDescription2 attachmentDescriptions[2] = {};
    // Depth/stencil attachment
    attachmentDescriptions[0] = vku::InitStructHelper();
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1] = vku::InitStructHelper();
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    VkAttachmentReference2 depthStencilAttachmentReference = vku::InitStructHelper();
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 0;
    VkAttachmentReference2 depthStencilResolveAttachmentReference = vku::InitStructHelper();
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 1;
    VkSubpassDescriptionDepthStencilResolve subpassDescriptionDSR = vku::InitStructHelper();
    subpassDescriptionDSR.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    VkSubpassDescription2 subpassDescription = vku::InitStructHelper(&subpassDescriptionDSR);
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;

    VkRenderPassCreateInfo2 renderPassCreateInfo = vku::InitStructHelper();
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;

    // Both modes can't be none
    attachmentDescriptions[0].format = depthStencilFormat;
    attachmentDescriptions[1].format = depthStencilFormat;
    subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_NONE;
    subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_NONE;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();

    // Stencil is used but resolve is set to none, depthResolveMode should be ignored
    attachmentDescriptions[0].format = stencilFormat;
    attachmentDescriptions[1].format = stencilFormat;
    subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_NONE;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;

    // Invalid use of UNUSED
    depthStencilAttachmentReference.attachment = VK_ATTACHMENT_UNUSED;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03177");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    depthStencilAttachmentReference.attachment = 0;

    // attachmentCount == 2
    depthStencilResolveAttachmentReference.attachment = 2;
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-pSubpasses-06473");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    depthStencilResolveAttachmentReference.attachment = 1;

    // test invalid sample counts
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03179");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;

    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_4_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03180");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;

    // test resolve and non-resolve formats are not same types
    attachmentDescriptions[0].format = stencilFormat;
    attachmentDescriptions[1].format = depthFormat;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03181");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();

    attachmentDescriptions[0].format = depthFormat;
    attachmentDescriptions[1].format = stencilFormat;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03182");
    vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();

    // test when independentResolve and independentResolve are false
    attachmentDescriptions[0].format = depthStencilFormat;
    attachmentDescriptions[1].format = depthStencilFormat;
    if (ds_resolve_props.independentResolve == VK_FALSE) {
        if (ds_resolve_props.independentResolveNone == VK_FALSE) {
            subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_NONE;
            subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03185");
            vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
            m_errorMonitor->VerifyFound();
        } else {
            if ((ds_resolve_props.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0) {
                subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
                m_errorMonitor->SetDesiredError(
                    "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03186");
                vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
                m_errorMonitor->VerifyFound();
            }
            if ((ds_resolve_props.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0) {
                subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
                subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                m_errorMonitor->SetDesiredError(
                    "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03186");
                vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
                m_errorMonitor->VerifyFound();
            }
        }
    } else {
        // test using unsupported resolve mode, which currently can only be AVERAGE
        // Need independentResolve to make easier to test
        if ((ds_resolve_props.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
            subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-depthResolveMode-03183");
            vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
            m_errorMonitor->VerifyFound();
        }
        if ((ds_resolve_props.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
            subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-stencilResolveMode-03184");
            vk::CreateRenderPass2KHR(device(), &renderPassCreateInfo, nullptr, &renderPass);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeRenderPass, RenderArea) {
    TEST_DESCRIPTION("Begin render pass with render area that is not within the framebuffer.");

    AddOptionalExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkRenderPassBeginInfo rpbinfo = vku::InitStructHelper();
    rpbinfo.renderPass = m_renderPass;
    rpbinfo.framebuffer = Framebuffer();
    rpbinfo.renderArea.extent.width = m_width;
    rpbinfo.renderArea.extent.height = m_height;
    rpbinfo.renderArea.offset.x = -32;
    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = m_renderPassClearValues.data();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-02850");
    m_command_buffer.BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    rpbinfo.renderArea.offset.x = 0;
    rpbinfo.renderArea.offset.y = -128;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-02851");
    m_command_buffer.BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.renderArea.extent.width = m_width + 128;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-02852");
    m_command_buffer.BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    rpbinfo.renderArea.extent.width = m_width;
    rpbinfo.renderArea.extent.height = m_height + 1;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-02853");
    m_command_buffer.BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, DeviceGroupRenderArea) {
    TEST_DESCRIPTION("Begin render pass with device group render area that is not within the framebuffer.");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkRect2D renderArea = {};
    renderArea.offset.x = -1;
    renderArea.offset.y = -1;
    renderArea.extent.width = 64;
    renderArea.extent.height = 64;

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = vku::InitStructHelper();
    device_group_render_pass_begin_info.deviceMask = 0x1;
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &renderArea;

    VkRenderPassBeginInfo rpbinfo = vku::InitStructHelper(&device_group_render_pass_begin_info);
    rpbinfo.renderPass = m_renderPass;
    rpbinfo.framebuffer = Framebuffer();
    rpbinfo.renderArea.extent.width = m_width;
    rpbinfo.renderArea.extent.height = m_height;
    rpbinfo.renderArea.offset.x = -32;
    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = m_renderPassClearValues.data();

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167");
    m_command_buffer.BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    renderArea.offset.x = 0;
    renderArea.offset.y = 1;
    renderArea.extent.width = m_width + 1;
    renderArea.extent.height = m_height;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-02856");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-02857");
    m_command_buffer.BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, RenderPassBeginNullValues) {
    TEST_DESCRIPTION("Test invalid null entries for clear color");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto rpbi = m_renderPassBeginInfo;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = nullptr;  // clearValueCount != 0, but pClearValues = null, leads to 04962
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &rpbi, false,
                        "VUID-VkRenderPassBeginInfo-clearValueCount-04962", nullptr);
}

TEST_F(NegativeRenderPass, DepthStencilResolveAttachmentFormat) {
    TEST_DESCRIPTION("Create subpass with VkSubpassDescriptionDepthStencilResolve that has an ");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UNORM, VK_SAMPLE_COUNT_1_BIT);  // Depth/stencil
    rp.AddAttachmentDescription(ds_format, VK_SAMPLE_COUNT_4_BIT);           // Depth/stencil resolve
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddDepthStencilAttachment(1);
    rp.AddDepthStencilResolveAttachment(0, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);

    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-02651");
    rp.CreateRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, RenderPassAttachmentFormat) {
    TEST_DESCRIPTION("Test creating render pass with attachment format VK_FORMAT_UNDEFINED");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_UNDEFINED;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06698");
    vk::CreateRenderPass(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    VkAttachmentDescription2 attach_desc_2 = vku::InitStructHelper();
    attach_desc_2.format = VK_FORMAT_UNDEFINED;
    attach_desc_2.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc_2.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc_2.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass_2 = vku::InitStructHelper();

    VkRenderPassCreateInfo2 render_pass_ci_2 = vku::InitStructHelper();
    render_pass_ci_2.attachmentCount = 1;
    render_pass_ci_2.pAttachments = &attach_desc_2;
    render_pass_ci_2.subpassCount = 1;
    render_pass_ci_2.pSubpasses = &subpass_2;

    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription2-format-09332");
    vk::CreateRenderPass2(device(), &render_pass_ci_2, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, SamplingFromReadOnlyDepthStencilAttachment) {
    TEST_DESCRIPTION("Use same image as depth stencil attachment in read only layer and as sampler");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = FindSupportedDepthStencilFormat(Gpu());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
    rp.AddDepthStencilAttachment(0);
    rp.CreateRenderPass();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent = {width, height, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    VkImageView image_view_handle = image_view.handle();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view_handle, width, height);

    char const *fsSource = R"glsl(
            #version 450
            layout(set = 0, binding = 0) uniform sampler2D depth;
            void main(){
                vec4 color = texture(depth, ivec2(0));
            }
        )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.ds_ci_.depthTestEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), width, height, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, ColorAttachmentImageViewUsage) {
    TEST_DESCRIPTION("Create image view with missing usage bits.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageViewUsageCreateInfo image_view_usage = vku::InitStructHelper();
    image_view_usage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT, &image_view_usage);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00337");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, StencilLoadOp) {
    TEST_DESCRIPTION("Create render pass with invalid stencil load op.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat stencil_format = FindSupportedStencilOnlyFormat(Gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = stencil_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    VkSubpassDescription2 subpass = vku::InitStructHelper();

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription2-pNext-06704");
    vk::CreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescriptionStencilLayout attach_desc_stencil_layout = vku::InitStructHelper();
    attach_desc_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.pNext = &attach_desc_stencil_layout;

    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription2-pNext-06705");
    vk::CreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, ViewMask) {
    TEST_DESCRIPTION("Create render pass with view mask, but multiview feature disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-multiview-06558");
    vk::CreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, AllViewMasksZero) {
    TEST_DESCRIPTION("Test VkRenderPassMultiviewCreateInfo with all view mask elements being 0.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    dependency.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;
    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkRenderPassMultiviewCreateInfo render_pass_multiview_ci = vku::InitStructHelper();
    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&render_pass_multiview_ci);
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass_description;
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.pDependencies = &dependency;
    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo-pNext-02514");
    vk::CreateRenderPass(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    uint32_t correlation_mask = 0x1;
    render_pass_ci.dependencyCount = 0;
    render_pass_multiview_ci.correlationMaskCount = 1;
    render_pass_multiview_ci.pCorrelationMasks = &correlation_mask;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo-pNext-02515");
    vk::CreateRenderPass(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, AttachmentUndefinedLayout) {
    TEST_DESCRIPTION("Create render pass with invalid attachment undefined layout.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = ds_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkAttachmentDescription-format-06699",
                         "VUID-VkAttachmentDescription2-format-06699");

    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06700");
    VkRenderPass render_pass;
    vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, MultisampledRenderToSingleSampled) {
    TEST_DESCRIPTION("Test VK_EXT_multisampled_render_to_single_sampled");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::multisampledRenderToSingleSampled);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    bool imageless_fb_supported = IsExtensionsEnabled(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    VkPhysicalDeviceVulkan12Properties vulkan_12_features = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(vulkan_12_features);
    InitRenderTarget();

    VkAttachmentReference2 attachmentRef = vku::InitStructHelper();
    attachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentRef.attachment = 0;
    VkAttachmentReference2 depthRef = vku::InitStructHelper();
    depthRef.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthRef.attachment = 1;

    VkMultisampledRenderToSingleSampledInfoEXT ms_render_to_ss = vku::InitStructHelper();
    ms_render_to_ss.multisampledRenderToSingleSampledEnable = VK_TRUE;
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    VkSubpassDescription2 subpass = vku::InitStructHelper(&ms_render_to_ss);
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;

    VkAttachmentDescription2 attach_desc[2] = {};
    attach_desc[0] = vku::InitStructHelper();
    attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1] = vku::InitStructHelper();
    attach_desc[1].format = VK_FORMAT_D32_SFLOAT;
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = attach_desc;

    VkRenderPass rp;
    // attach_desc[0].samples != ms_state.rasterizationSamples
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-pNext-06870");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();

    attach_desc[0].samples = VK_SAMPLE_COUNT_2_BIT;
    subpass.pDepthStencilAttachment = &depthRef;
    // Depth VK_SAMPLE_COUNT_1_BIT, no VkSubpassDescriptionDepthStencilResolve in pNext
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-pNext-06871");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();

    VkSubpassDescriptionDepthStencilResolve depth_stencil_resolve = vku::InitStructHelper();
    ms_render_to_ss.pNext = &depth_stencil_resolve;
    // VkSubpassDescriptionDepthStencilResolve depthResolveMode and stencilResolveMode both VK_RESOLVE_MODE_NONE
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06873");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();

    VkResolveModeFlagBits unsupported_depth = VK_RESOLVE_MODE_NONE;
    VkResolveModeFlagBits supported_depth = VK_RESOLVE_MODE_NONE;
    VkResolveModeFlagBits unsupported_stencil = VK_RESOLVE_MODE_NONE;
    VkResolveModeFlagBits supported_stencil = VK_RESOLVE_MODE_NONE;
    for (VkResolveModeFlagBits i = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT; i <= VK_RESOLVE_MODE_MAX_BIT;
         i = VkResolveModeFlagBits(i << 1)) {
        if ((unsupported_depth == VK_RESOLVE_MODE_NONE) && !(i & vulkan_12_features.supportedDepthResolveModes)) {
            unsupported_depth = i;
        }
        if ((unsupported_stencil == VK_RESOLVE_MODE_NONE) && !(i & vulkan_12_features.supportedStencilResolveModes)) {
            unsupported_stencil = i;
        }
        if (supported_stencil == VK_RESOLVE_MODE_NONE) {
            if (i & vulkan_12_features.supportedDepthResolveModes) {
                supported_stencil = i;
            }
        } else if (supported_depth == VK_RESOLVE_MODE_NONE) {
            // Want supported depth different than supported stencil
            if (i & vulkan_12_features.supportedDepthResolveModes) {
                supported_depth = i;
            }
        }
    }
    if (unsupported_depth != VK_RESOLVE_MODE_NONE) {
        depth_stencil_resolve.depthResolveMode = unsupported_depth;
        // depthResolveMode unsupported
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06874");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
        depth_stencil_resolve.depthResolveMode = VK_RESOLVE_MODE_NONE;
    }

    if (unsupported_stencil != VK_RESOLVE_MODE_NONE) {
        attach_desc[1].format = VK_FORMAT_S8_UINT;
        depth_stencil_resolve.stencilResolveMode = unsupported_stencil;
        // stencilResolveMode unsupported
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06875");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
        depth_stencil_resolve.stencilResolveMode = VK_RESOLVE_MODE_NONE;
        attach_desc[1].format = VK_FORMAT_D32_SFLOAT;
    }

    if (!(vulkan_12_features.independentResolve) && !(vulkan_12_features.independentResolveNone) &&
        (supported_depth != VK_RESOLVE_MODE_NONE) && (supported_stencil != VK_RESOLVE_MODE_NONE)) {
        depth_stencil_resolve.depthResolveMode = supported_depth;
        depth_stencil_resolve.stencilResolveMode = supported_stencil;
        attach_desc[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        // Stencil and depth resolve modes must be the same
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06876");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
    }

    if (!(vulkan_12_features.independentResolve) && vulkan_12_features.independentResolveNone &&
        (supported_depth != VK_RESOLVE_MODE_NONE) && (supported_stencil != VK_RESOLVE_MODE_NONE)) {
        depth_stencil_resolve.depthResolveMode = supported_depth;
        depth_stencil_resolve.stencilResolveMode = supported_stencil;
        attach_desc[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        // Stencil and depth resolve modes must be the same or one of them must be VK_RESOLVE_MODE_NONE
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06877");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
    }

    ms_render_to_ss.pNext = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    // rasterizationSamples can't be VK_SAMPLE_COUNT_1_BIT
    m_errorMonitor->SetDesiredError("VUID-VkMultisampledRenderToSingleSampledInfoEXT-rasterizationSamples-06878");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();
    attach_desc[0].samples = VK_SAMPLE_COUNT_2_BIT;

    ms_render_to_ss.pNext = nullptr;
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    subpass.pDepthStencilAttachment = nullptr;
    rpci.attachmentCount = 1;
    // Create a usable renderpass
    vkt::RenderPass test_rp(*m_device, rpci);

    VkPipelineMultisampleStateCreateInfo ms_state = vku::InitStructHelper();
    ms_state.flags = 0;
    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    ms_state.sampleShadingEnable = VK_FALSE;
    ms_state.minSampleShading = 0.0f;
    ms_state.pSampleMask = nullptr;
    ms_state.alphaToCoverageEnable = VK_FALSE;
    ms_state.alphaToOneEnable = VK_FALSE;

    CreatePipelineHelper pipe_helper(*this);
    pipe_helper.gp_ci_.renderPass = test_rp.handle();
    pipe_helper.ms_ci_ = ms_state;

    // ms_render_to_ss.rasterizationSamples != ms_state.rasterizationSamples
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06854");
    pipe_helper.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Actually create a usable pipeline
    pipe_helper.ms_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe_helper.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    VkRenderingInfo begin_rendering_info = vku::InitStructHelper(&ms_render_to_ss);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    // ms_render_to_ss.rasterizationSamples != ms_state.rasterizationSamples
    // Valid because never hit draw time
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_helper.Handle());
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    VkImageFormatProperties2 image_format_prop = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_format_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
    if ((result != VK_SUCCESS) || !(image_format_prop.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_2_BIT)) {
        GTEST_SKIP() << "Cannot create an image with format VK_FORMAT_B8G8R8A8_UNORM and sample count VK_SAMPLE_COUNT_2_BIT. "
                        "Skipping remainder of the test";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image two_count_image(*m_device, image_create_info, vkt::set_layout);

    auto image_view_ci = two_count_image.BasicViewCreatInfo();
    vkt::ImageView two_count_image_view(*m_device, image_view_ci);

    color_attachment.imageView = two_count_image_view.handle();
    m_command_buffer.Begin();
    // Attachments must have a sample count that is either VK_SAMPLE_COUNT_1_BIT or
    // VkMultisampledRenderToSingleSampledInfoEXT::rasterizationSamples.
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-imageView-06858");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image one_count_image(*m_device, image_create_info, vkt::set_layout);
    auto one_count_image_view_ci = one_count_image.BasicViewCreatInfo();
    vkt::ImageView one_count_image_view(*m_device, one_count_image_view_ci);
    color_attachment.imageView = one_count_image_view.handle();
    // Attachments with a sample count of VK_SAMPLE_COUNT_1_BIT must have been created with
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-imageView-06859");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
    vkt::Image good_one_count_image(*m_device, image_create_info, vkt::set_layout);
    auto good_one_count_image_view_ci = good_one_count_image.BasicViewCreatInfo();
    vkt::ImageView good_one_count_image_view(*m_device, good_one_count_image_view_ci);
    color_attachment.imageView = good_one_count_image_view.handle();
    color_attachment.resolveImageView = good_one_count_image_view.handle();
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    begin_rendering_info.pNext = nullptr;
    color_attachment.imageView = good_one_count_image_view.handle();
    // If resolveMode is not VK_RESOLVE_MODE_NONE, imageView must not have a sample count of VK_SAMPLE_COUNT_1_BIT
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-imageView-06861");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageView = two_count_image_view.handle();
    color_attachment.resolveImageView = VK_NULL_HANDLE;
    // If resolveMode is not VK_RESOLVE_MODE_NONE, resolveImageView must not be VK_NULL_HANDLE
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-imageView-06862");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.pNext = &ms_render_to_ss;
    color_attachment.imageView = good_one_count_image_view.handle();
    color_attachment.resolveImageView = good_one_count_image_view.handle();
    // If imageView has a sample count of VK_SAMPLE_COUNT_1_BIT, resolveImageView must be VK_NULL_HANDLE
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-imageView-06863");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    // Positive Test: Image view with VK_SAMPLE_COUNT_1_BIT should not get error 07285 in pipeline created with attachment with
    // VK_SAMPLE_COUNT_2_BIT
    CreatePipelineHelper dr_pipe_helper(*this);
    dr_pipe_helper.gp_ci_.renderPass = VK_NULL_HANDLE;
    dr_pipe_helper.ms_ci_ = ms_state;
    dr_pipe_helper.cb_ci_.attachmentCount = 0;
    dr_pipe_helper.CreateGraphicsPipeline();
    begin_rendering_info.pNext = nullptr;
    color_attachment.resolveImageView = VK_NULL_HANDLE;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, dr_pipe_helper.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRendering();
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;

    // Positive Test: Same as previous test but using render pass and should not get error 07284
    CreatePipelineHelper test_pipe(*this);
    test_pipe.ms_ci_ = ms_state;
    test_pipe.CreateGraphicsPipeline();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, test_pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    // Find an image format that can't be sampled
    image_format_prop = vku::InitStructHelper();
    image_format_info = vku::InitStructHelper();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_3D;
    image_format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkFormat unsampleable_format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits unsampleable_count = VK_SAMPLE_COUNT_1_BIT;
    for (VkFormat format = VK_FORMAT_UNDEFINED; format <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; format = VkFormat(format + 1)) {
        image_format_info.format = format;
        result = vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
        if (result == VK_SUCCESS) {
            if (image_format_prop.imageFormatProperties.sampleCounts != 0x7f) {
                unsampleable_format = format;
                for (VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT; samples <= VK_SAMPLE_COUNT_64_BIT;
                     samples = VkSampleCountFlagBits(samples << 1)) {
                    if (!(image_format_prop.imageFormatProperties.sampleCounts & samples)) {
                        unsampleable_count = samples;
                        break;
                    }
                }
                break;
            }
        }
    }

    if (unsampleable_format != VK_FORMAT_UNDEFINED) {
        image_create_info.imageType = VK_IMAGE_TYPE_3D;
        image_create_info.format = unsampleable_format;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;  // Can't use unsupported sample count or can't create image view
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.flags =
            VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        vkt::Image unsampleable_image(*m_device, image_create_info, vkt::set_layout);
        auto unsampleable_image_view_ci = unsampleable_image.BasicViewCreatInfo();
        vkt::ImageView unsampleable_image_view(*m_device, unsampleable_image_view_ci);
        begin_rendering_info.pNext = &ms_render_to_ss;
        ms_render_to_ss.rasterizationSamples = unsampleable_count;
        color_attachment.resolveImageView = VK_NULL_HANDLE;
        color_attachment.imageView = unsampleable_image_view.handle();
        // Attachment must have a format that supports the sample count specified in rasterizationSamples
        m_errorMonitor->SetDesiredError("VUID-VkMultisampledRenderToSingleSampledInfoEXT-pNext-06880");
        m_command_buffer.BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();

        attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        subpass.pDepthStencilAttachment = nullptr;
        rpci.attachmentCount = 1;
        attach_desc[0].format = unsampleable_format;

        vkt::RenderPass unsampleable_rp(*m_device, rpci);
        auto unsampleable_fbci = vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, unsampleable_rp.handle(), 1u,
                                                                          &unsampleable_image_view.handle(), 64u, 64u, 1u);

        VkFramebuffer unsampleable_fb;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-samples-07009");
        vk::CreateFramebuffer(device(), &unsampleable_fbci, nullptr, &unsampleable_fb);
        m_errorMonitor->VerifyFound();
        attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;

        if (imageless_fb_supported) {
            VkFormat framebufferAttachmentFormats[1] = {unsampleable_format};
            VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfo = vku::InitStructHelper();
            framebufferAttachmentImageInfo.flags = image_create_info.flags;
            framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            framebufferAttachmentImageInfo.width = 64;
            framebufferAttachmentImageInfo.height = 64;
            framebufferAttachmentImageInfo.layerCount = 1;
            framebufferAttachmentImageInfo.viewFormatCount = 1;
            framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
            VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
            framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
            framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
            rpci.attachmentCount = 1;
            attach_desc[0].format = unsampleable_format;
            attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
            vkt::RenderPass imageless_rp(*m_device, rpci);
            auto imageless_fbci =
                vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, imageless_rp.handle(), 1u, nullptr, 64u, 64u, 1u);
            imageless_fbci.pNext = &framebufferAttachmentsCreateInfo;
            imageless_fbci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
            vkt::Framebuffer imageless_fb(*m_device, imageless_fbci);

            VkRenderPassAttachmentBeginInfo renderPassAttachmentBeginInfo = vku::InitStructHelper();
            renderPassAttachmentBeginInfo.attachmentCount = 1;
            renderPassAttachmentBeginInfo.pAttachments = &unsampleable_image_view.handle();
            VkRenderPassBeginInfo renderPassBeginInfo = vku::InitStructHelper(&renderPassAttachmentBeginInfo);
            renderPassBeginInfo.renderPass = imageless_rp.handle();
            renderPassBeginInfo.renderArea.extent.width = 64;
            renderPassBeginInfo.renderArea.extent.height = 64;
            renderPassBeginInfo.framebuffer = imageless_fb.handle();
            m_errorMonitor->SetDesiredError("VUID-VkRenderPassAttachmentBeginInfo-pAttachments-07010");
            m_command_buffer.BeginRenderPass(renderPassBeginInfo);
            m_errorMonitor->VerifyFound();
            attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
        }
    }

    // Need a renderpass with a COUNT_1 attachment
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    subpass.pDepthStencilAttachment = nullptr;
    rpci.attachmentCount = 1;
    // Create a usable renderpass
    vkt::RenderPass test_rp2(*m_device, rpci);
    auto fbci =
        vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, test_rp2.handle(), 1u, &one_count_image_view.handle(), 64u, 64u, 1u);
    VkFramebuffer fb;
    // Framebuffer attachments with VK_SAMPLE_COUNT_1_BIT must have been created with
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-samples-06881");
    vk::CreateFramebuffer(device(), &fbci, nullptr, &fb);
    m_errorMonitor->VerifyFound();

    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImage bad_flag_image;
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT requires VK_SAMPLE_COUNT_1_BIT
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-06883");
    vk::CreateImage(device(), &image_create_info, nullptr, &bad_flag_image);
    m_errorMonitor->VerifyFound();

    vkt::QueueCreateInfoArray queue_info(m_device->Physical().queue_properties_);
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    device_create_info.queueCreateInfoCount = queue_info.Size();
    device_create_info.pQueueCreateInfos = queue_info.Data();
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice second_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_create_info, nullptr, &second_device));
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    bad_flag_image = VK_NULL_HANDLE;
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT requires multisampledRenderToSingleSampled feature
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-multisampledRenderToSingleSampled-06882");
    vk::CreateImage(second_device, &image_create_info, nullptr, &bad_flag_image);
    m_errorMonitor->VerifyFound();
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(NegativeRenderPass, AttachmentDescriptionUndefinedFormat) {
    TEST_DESCRIPTION("Create a render pass with an attachment description format set to VK_FORMAT_UNDEFINED");
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);

    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06698");
    rp.CreateRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, IncompatibleRenderPass) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    RETURN_IF_SKIP(Init());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    vkt::RenderPass render_pass1(*m_device, rpci);
    rpci.dependencyCount = 0;
    vkt::RenderPass render_pass2(*m_device, rpci);
    rpci.dependencyCount = 1;
    dependency.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vkt::RenderPass render_pass3(*m_device, rpci);
    vkt::Image image(*m_device, width, height, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass1.handle(), 1, &imageView.handle(), width, height);

    VkClearValue clear_values[2] = {};
    clear_values[0].color = {{0, 0, 0, 0}};
    clear_values[1].color = {{0, 0, 0, 0}};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_command_buffer.BeginRenderPass(render_pass2.handle(), framebuffer.handle(), width, height, 2, clear_values);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_command_buffer.BeginRenderPass(render_pass3.handle(), framebuffer.handle(), width, height, 2, clear_values);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, IncompatibleRenderPass2) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference2 attach_ref = vku::InitStructHelper();
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    subpass.viewMask = 0x1;

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDependency2 dependency = {VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                                       nullptr,
                                       0,
                                       0,
                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       VK_DEPENDENCY_BY_REGION_BIT};

    uint32_t correlated_view_mask = 0x1;
    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;
    rpci.correlatedViewMaskCount = 1;
    rpci.pCorrelatedViewMasks = &correlated_view_mask;

    vkt::RenderPass render_pass1(*m_device, rpci);
    rpci.correlatedViewMaskCount = 0;
    vkt::RenderPass render_pass2(*m_device, rpci);
    rpci.correlatedViewMaskCount = 1;
    correlated_view_mask = 0x2;
    vkt::RenderPass render_pass3(*m_device, rpci);

    vkt::Image image(*m_device, width, height, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass1.handle(), 1, &imageView.handle(), width, height);

    VkClearValue clear_values[2] = {};
    clear_values[0].color = {{0, 0, 0, 0}};
    clear_values[1].color = {{0, 0, 0, 0}};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_command_buffer.BeginRenderPass(render_pass2.handle(), framebuffer.handle(), width, height, 2, clear_values);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_command_buffer.BeginRenderPass(render_pass3.handle(), framebuffer.handle(), width, height, 2, clear_values);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, IncompatibleRenderPassSubpassFlags) {
    TEST_DESCRIPTION("Two renderpasses with different VkSubpassDescriptionFlagBits");

    AddRequiredExtensions(VK_EXT_LEGACY_DITHERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::legacyDithering);
    RETURN_IF_SKIP(Init());

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    vkt::RenderPass render_pass1(*m_device, rpci);
    subpass.flags = VK_SUBPASS_DESCRIPTION_ENABLE_LEGACY_DITHERING_BIT_EXT;
    vkt::RenderPass render_pass2(*m_device, rpci);
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass1.handle(), 1, &imageView.handle());

    VkClearValue clear_values[2] = {};
    clear_values[0].color = {{0, 0, 0, 0}};
    clear_values[1].color = {{0, 0, 0, 0}};

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass2.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_command_buffer.BeginRenderPass(render_pass2.handle(), framebuffer.handle(), 32, 32, 2, clear_values);
    m_errorMonitor->VerifyFound();

    m_command_buffer.BeginRenderPass(render_pass1.handle(), framebuffer.handle(), 32, 32, 2, clear_values);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, SubpassAttachmentImageLayout) {
    TEST_DESCRIPTION("Invalid attachment reference layout");

    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    std::array<VkAttachmentDescription, 3> attachments = {{
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        // This attachment is used as a color attachment when using the above one as a resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_2_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        // Depth stencil attachment
        {0, FindSupportedDepthStencilFormat(m_device->Physical()), VK_SAMPLE_COUNT_2_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_GENERAL},
    }};

    VkSubpassDescription subpass{};
    auto reset_subpass = [&]() {
        subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   static_cast<uint32_t>(attachments.size()),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    VkAttachmentReference ref{};
    ref.attachment = 0;

    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06912",
                         "VUID-VkSubpassDescription2-attachment-06912");

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06912",
                         "VUID-VkSubpassDescription2-attachment-06912");

    reset_subpass();

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06913",
                         "VUID-VkSubpassDescription2-attachment-06913");

    ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06913",
                         "VUID-VkSubpassDescription2-attachment-06913");

    reset_subpass();

    {
        VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pResolveAttachments = &ref;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06914",
                             "VUID-VkSubpassDescription2-attachment-06914");

        ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06914",
                             "VUID-VkSubpassDescription2-attachment-06914");
    }

    reset_subpass();

    ref.attachment = 2;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpass.pDepthStencilAttachment = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06915",
                         "VUID-VkSubpassDescription2-attachment-06915");

    ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06915",
                         "VUID-VkSubpassDescription2-attachment-06915");
}

TEST_F(NegativeRenderPass, SubpassAttachmentImageLayoutMaintenance2) {
    TEST_DESCRIPTION("Invalid attachment reference layout, Maintenance2 enabled");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    constexpr std::array<VkAttachmentDescription, 2> attachments = {{
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        // This attachment is used as a color attachment when using the above one as a resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_2_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
    }};

    VkSubpassDescription subpass{};
    auto reset_subpass = [&]() {
        subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   static_cast<uint32_t>(attachments.size()),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    VkAttachmentReference ref{};
    ref.attachment = 0;

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06916",
                         "VUID-VkSubpassDescription2-attachment-06916");

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06916",
                         "VUID-VkSubpassDescription2-attachment-06916");

    reset_subpass();

    {
        VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        subpass.pResolveAttachments = &ref;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06917",
                             "VUID-VkSubpassDescription2-attachment-06917");

        ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06917",
                             "VUID-VkSubpassDescription2-attachment-06917");
    }
}

TEST_F(NegativeRenderPass, SubpassAttachmentImageLayoutSynchronization2) {
    TEST_DESCRIPTION("Invalid attachment reference layout, Synchronization2 enabled");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    constexpr std::array<VkAttachmentDescription, 2> attachments = {{
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        // This attachment is used as a color attachment when using the above one as a resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_2_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
    }};

    VkSubpassDescription subpass{};
    auto reset_subpass = [&]() {
        subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   static_cast<uint32_t>(attachments.size()),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    VkAttachmentReference ref{};
    ref.attachment = 0;

    ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06921",
                         "VUID-VkSubpassDescription2-attachment-06921");

    reset_subpass();

    ref.layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06922",
                         "VUID-VkSubpassDescription2-attachment-06922");

    reset_subpass();

    {
        VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        ref.layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        subpass.pResolveAttachments = &ref;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06923",
                             "VUID-VkSubpassDescription2-attachment-06923");
    }
}

TEST_F(NegativeRenderPass, SubpassAttachmentImageLayoutSeparateDepthStencil) {
    TEST_DESCRIPTION("Invalid attachment reference layout, separate depth stencil enabled");

    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    std::array<VkAttachmentDescription, 3> attachments = {{
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        // This attachment is used as a color attachment when using the above one as a resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_2_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        // Depth stencil attachment
        {0, FindSupportedDepthStencilFormat(m_device->Physical()), VK_SAMPLE_COUNT_2_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_GENERAL},
    }};

    VkSubpassDescription subpass{};
    auto reset_subpass = [&]() {
        subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   static_cast<uint32_t>(attachments.size()),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    VkAttachmentReference ref{};
    ref.attachment = 0;

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06918",
                         "VUID-VkSubpassDescription2-attachment-06918");

    ref.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06918",
                         "VUID-VkSubpassDescription2-attachment-06918");

    reset_subpass();

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06919",
                         "VUID-VkSubpassDescription2-attachment-06919");

    ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06919",
                         "VUID-VkSubpassDescription2-attachment-06919");

    ref.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06919",
                         "VUID-VkSubpassDescription2-attachment-06919");

    ref.layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06919",
                         "VUID-VkSubpassDescription2-attachment-06919");

    reset_subpass();

    {
        VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        subpass.pResolveAttachments = &ref;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06920",
                             "VUID-VkSubpassDescription2-attachment-06920");

        ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06920",
                             "VUID-VkSubpassDescription2-attachment-06920");

        ref.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06920",
                             "VUID-VkSubpassDescription2-attachment-06920");

        ref.layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDescription-attachment-06920",
                             "VUID-VkSubpassDescription2-attachment-06920");
    }

    auto depth_stencil_attachment = vku::InitStruct<VkAttachmentDescription2>(
        nullptr, static_cast<VkAttachmentDescriptionFlags>(0), VK_FORMAT_S8_UINT, VK_SAMPLE_COUNT_2_BIT,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkImageFormatProperties imageFormatProperties;
    VkResult res;
    res =
        vk::GetPhysicalDeviceImageFormatProperties(gpu_, depth_stencil_attachment.format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0u, &imageFormatProperties);
    if (rp2_supported && res == VK_SUCCESS) {
        VkRenderPassCreateInfo2 rpci2 = vku::InitStructHelper();
        rpci2.attachmentCount = 1;
        rpci2.pAttachments = &depth_stencil_attachment;
        VkAttachmentReferenceStencilLayout stencil_ref = vku::InitStructHelper();
        stencil_ref.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference2 depth_stencil_ref = vku::InitStructHelper(&stencil_ref);
        depth_stencil_ref.attachment = 0;
        VkSubpassDescription2 subpass2 = vku::InitStructHelper();
        subpass2.pDepthStencilAttachment = &depth_stencil_ref;
        rpci2.subpassCount = 1;
        rpci2.pSubpasses = &subpass2;

        {
            depth_stencil_ref.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-attachment-06251");
            vkt::RenderPass rp2(*m_device, rpci2);
            m_errorMonitor->VerifyFound();
        }

        {
            depth_stencil_ref.layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
            m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-attachment-06251");
            vkt::RenderPass rp2(*m_device, rpci2);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeRenderPass, BeginInfoWithoutRenderPass) {
    TEST_DESCRIPTION("call VkRenderPassBeginInfo with invalid renderpass");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-parameter");
    m_renderPassBeginInfo.renderPass = CastFromUint64<VkRenderPass>(0xFFFFEEEE);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandle");
    m_renderPassBeginInfo.renderPass = VK_NULL_HANDLE;
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, BeginInfoWithoutFramebuffer) {
    TEST_DESCRIPTION("call VkRenderPassBeginInfo with invalid framebuffer");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-parameter");
    m_renderPassBeginInfo.framebuffer = CastFromUint64<VkFramebuffer>(0xFFFFEEEE);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, EndWithoutRenderPass) {
    TEST_DESCRIPTION("call vkCmdEndRenderPass never starting a renderpass");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndRenderPass-renderpass");
    m_command_buffer.EndRenderPass();
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, RenderPassBegin) {
    TEST_DESCRIPTION("have an invalid pRenderPassBegin");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass-pRenderPassBegin-parameter");
    vk::CmdBeginRenderPass(m_command_buffer.handle(), nullptr, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, IncompatibleFramebuffer) {
    TEST_DESCRIPTION("Incompatible framebuffer in command buffer inheritance info");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);
    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 0, nullptr);

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();
    inheritance_info.renderPass = m_renderPass;
    inheritance_info.subpass = 0u;
    inheritance_info.framebuffer = framebuffer.handle();

    VkCommandBufferBeginInfo cmd_buffer_begin_info = vku::InitStructHelper();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &inheritance_info;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-00055");
    vk::BeginCommandBuffer(secondary_cmd_buffer.handle(), &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, ZeroRenderArea) {
    TEST_DESCRIPTION("renderArea set to zero");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-None-08996");
    m_renderPassBeginInfo.renderArea.extent = {0, 64};
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-None-08997");
    m_renderPassBeginInfo.renderArea.extent = {64, 0};
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, InvalidAttachmentDescriptionDSLayout) {
    TEST_DESCRIPTION("Invalid final layout for ds attachment");
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    VkAttachmentDescription description = {0,
                                           ds_format,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06243");
    vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06242");
    vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, InvalidAttachmentDescriptionColorLayout) {
    TEST_DESCRIPTION("Invalid final layout for color attachment");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription description = {0,
                                           VK_FORMAT_R8G8B8A8_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL};

    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1u, &color_ref, nullptr, nullptr, 0, nullptr};
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06488");
    vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-06487");
    vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, InvalidFramebufferAttachmentImageUsage) {
    TEST_DESCRIPTION("Use image at framebuffer attachment without VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT");

    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    VkAttachmentDescription description = {0,
                                           VK_FORMAT_R8G8B8A8_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, &ref, 1, &ref, nullptr, nullptr, 0, nullptr,
    };
    VkRenderPassCreateInfo rp_ci =
        vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);
    vkt::RenderPass render_pass(*m_device, rp_ci);

    subpass.colorAttachmentCount = 0u;
    subpass.inputAttachmentCount = 1u;
    vkt::RenderPass input_attachment_render_pass(*m_device, rp_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &image_view.handle();
    framebuffer_ci.width = m_width;
    framebuffer_ci.height = m_height;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00877");
    vk::CreateFramebuffer(*m_device, &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.renderPass = input_attachment_render_pass.handle();
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00879");
    vk::CreateFramebuffer(*m_device, &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, ViewMaskWithoutFeature) {
    TEST_DESCRIPTION("Create render pass using view masks with multiview disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0u;

    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1u;
    subpass_description.pColorAttachments = &attachment_reference;

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    uint32_t view_mask = 0x1;

    VkRenderPassMultiviewCreateInfo render_pass_multiview_ci = vku::InitStructHelper();
    render_pass_multiview_ci.subpassCount = 1u;
    render_pass_multiview_ci.pViewMasks = &view_mask;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&render_pass_multiview_ci);
    render_pass_ci.attachmentCount = 1u;
    render_pass_ci.pAttachments = &attachment_description;
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass_description;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassMultiviewCreateInfo-multiview-06555");
    vk::CreateRenderPass(*m_device, &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, AttachmentLayout) {
    TEST_DESCRIPTION("Test attachment descriptions with layouts other than undefined");
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32u, 32u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view.handle());

    VkClearValue clear_value;
    clear_value.color = {{0, 0, 0, 0}};

    VkImageMemoryBarrier ImageMemoryBarrier = vku::InitStructHelper();
    ImageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
    ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    ImageMemoryBarrier.image = image.handle();
    ImageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &ImageMemoryBarrier);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass-initialLayout-00900");
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32, 1, &clear_value);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, ImageSubresourceOverlapBetweenCurrentRenderPassAndDescriptorSets) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_errorMonitor->SetDesiredError("UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-renderPass-00904");

    const uint32_t width = 16;
    const uint32_t height = 16;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentDescription attach_desc2[] = {attach_desc, attach_desc};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attach_desc2;

    vkt::RenderPass render_pass(*m_device, rpci);

    VkClearValue clear_values[2] = {m_renderPassClearValues[0], m_renderPassClearValues[0]};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass.handle(), Framebuffer(), width, height, 2, clear_values);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, BeginRenderPassWithRenderPassStriped) {
    TEST_DESCRIPTION("Various tests to validate begin renderpass begininfo with VK_ARM_render_pass_striped.");
    AddRequiredExtensions(VK_ARM_RENDER_PASS_STRIPED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::renderPassStriped);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceRenderPassStripedPropertiesARM rp_striped_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rp_striped_props);

    const uint32_t stripe_width = rp_striped_props.renderPassStripeGranularity.width;
    const uint32_t stripe_height = rp_striped_props.renderPassStripeGranularity.height;

    uint32_t stripe_count = rp_striped_props.maxRenderPassStripes + 1;
    std::vector<VkRenderPassStripeInfoARM> stripe_infos(rp_striped_props.maxRenderPassStripes + 1);
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i] = vku::InitStructHelper();
        stripe_infos[i].stripeArea.offset.x = stripe_width * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    VkRenderPassStripeBeginInfoARM rp_stripe_info = vku::InitStructHelper();
    rp_stripe_info.stripeInfoCount = stripe_count;
    rp_stripe_info.pStripeInfos = stripe_infos.data();

    m_renderPassBeginInfo.pNext = &rp_stripe_info;
    m_renderPassBeginInfo.renderArea = {{0, 0}, {stripe_width * stripe_count, stripe_height}};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeBeginInfoARM-stripeInfoCount-09450");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    stripe_count = 8;
    stripe_infos.resize(stripe_count);
    rp_stripe_info.pStripeInfos = stripe_infos.data();
    rp_stripe_info.stripeInfoCount = stripe_count;
    m_renderPassBeginInfo.renderArea.extent = {stripe_width * stripe_count, stripe_height};
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i].stripeArea.offset.x = i > 1 && i <= 4 ? (stripe_width * (i - 1) / 2) : stripe_width * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09452");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09452");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeBeginInfoARM-stripeArea-09451");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    const uint32_t half_stripe_width = stripe_width / 2;
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i].stripeArea.offset.x = (i == 1 ? half_stripe_width : stripe_width) * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = i == 2 ? half_stripe_width : stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeBeginInfoARM-stripeArea-09451");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09452");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09453");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    const uint32_t non_align_stripe_width = stripe_width - 12;
    m_renderPassBeginInfo.renderArea.extent.width = (stripe_width * (stripe_count - 1)) + non_align_stripe_width + 4;
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i].stripeArea.offset.x = stripe_width * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = i == 7 ? non_align_stripe_width : stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09453");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-09539");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_renderPassBeginInfo.renderArea.extent = {stripe_width, stripe_height * stripe_count};
    const uint32_t half_stripe_height = stripe_height / 2;
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i].stripeArea.offset.x = 0;
        stripe_infos[i].stripeArea.offset.y = (i == 1 ? half_stripe_height : stripe_height) * i;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = i == 2 ? half_stripe_height : stripe_height;
    }

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeBeginInfoARM-stripeArea-09451");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09454");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09455");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    const uint32_t non_align_stripe_height = stripe_height - 12;
    m_renderPassBeginInfo.renderArea.extent.height = (stripe_height * (stripe_count - 1)) + non_align_stripe_height + 4;
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i].stripeArea.offset.x = 0;
        stripe_infos[i].stripeArea.offset.y = stripe_height * i;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = i == 7 ? non_align_stripe_height : stripe_height;
    }

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeInfoARM-stripeArea-09455");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-pNext-09539");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, RenderPassWithRenderPassStripedQueueSubmit2) {
    TEST_DESCRIPTION("Test to validate VK_ARM_render_pass_striped with QueueSubmit2.");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_ARM_RENDER_PASS_STRIPED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::renderPassStriped);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceRenderPassStripedPropertiesARM rp_striped_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rp_striped_props);

    const uint32_t stripe_width = rp_striped_props.renderPassStripeGranularity.width;
    const uint32_t stripe_height = rp_striped_props.renderPassStripeGranularity.height;

    const uint32_t stripe_count = 4;
    std::vector<VkRenderPassStripeInfoARM> stripe_infos(stripe_count);
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i] = vku::InitStructHelper();
        stripe_infos[i].stripeArea.offset.x = stripe_width * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    VkRenderPassStripeBeginInfoARM rp_stripe_info = vku::InitStructHelper();
    rp_stripe_info.stripeInfoCount = stripe_count;
    rp_stripe_info.pStripeInfos = stripe_infos.data();

    m_renderPassBeginInfo.pNext = &rp_stripe_info;
    m_renderPassBeginInfo.renderArea = {{0, 0}, {stripe_width * stripe_count, stripe_height}};

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer(*m_device, command_pool);

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();

    cmd_buffer.Begin(&cmd_begin);
    cmd_buffer.BeginRenderPass(m_renderPassBeginInfo);
    cmd_buffer.EndRenderPass();
    cmd_buffer.End();

    VkCommandBufferSubmitInfo cb_submit_info = vku::InitStructHelper();
    cb_submit_info.commandBuffer = cmd_buffer.handle();

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferSubmitInfo-commandBuffer-09445");
    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_submit_info;
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper();
    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    VkSemaphoreCreateInfo semaphore_timeline_create_info = vku::InitStructHelper(&semaphore_type_create_info);
    vkt::Semaphore semaphore[stripe_count + 1];
    VkSemaphoreSubmitInfo semaphore_submit_infos[stripe_count + 1];

    for (uint32_t i = 0; i < stripe_count + 1; ++i) {
        VkSemaphoreCreateInfo create_info = i == 4 ? semaphore_timeline_create_info : semaphore_create_info;
        semaphore[i].init(*m_device, create_info);

        semaphore_submit_infos[i] = vku::InitStructHelper();
        semaphore_submit_infos[i].semaphore = semaphore[i].handle();
    }

    VkRenderPassStripeSubmitInfoARM rp_stripe_submit_info = vku::InitStructHelper();
    rp_stripe_submit_info.stripeSemaphoreInfoCount = stripe_count + 1;
    rp_stripe_submit_info.pStripeSemaphoreInfos = semaphore_submit_infos;
    cb_submit_info.pNext = &rp_stripe_submit_info;

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferSubmitInfo-pNext-09446");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassStripeSubmitInfoARM-semaphore-09447");
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, MissingNestedCommandBuffersFeature) {
    TEST_DESCRIPTION("Use VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR when nextedCommandBuffers is not enabled");
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass-contents-09640");
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo,
                           VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, MissingNestedCommandBuffersFeature2) {
    TEST_DESCRIPTION("Use VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR when nextedCommandBuffers is not enabled");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto subpassBeginInfo =
        vku::InitStruct<VkSubpassBeginInfo>(nullptr, VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkSubpassBeginInfo-contents-09382");
    vk::CmdBeginRenderPass2KHR(m_command_buffer.handle(), &m_renderPassBeginInfo, &subpassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeRenderPass, FramebufferMismatchedAttachmentCount) {
    TEST_DESCRIPTION("Create framebuffer with different attachment count than render pass has");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::ImageView rt_view0 = m_renderTargets[0]->CreateView();
    vkt::ImageView rt_view1 = m_renderTargets[0]->CreateView();
    VkImageView ivs[2] = {rt_view0, rt_view1};

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.pAttachments = ivs;
    fb_info.width = 32u;
    fb_info.height = 32u;
    fb_info.layers = 1u;
    fb_info.renderPass = m_renderPass;
    // Set mis-matching attachmentCount
    fb_info.attachmentCount = 2u;

    VkFramebuffer fb;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-attachmentCount-00876");
    vk::CreateFramebuffer(device(), &fb_info, nullptr, &fb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferDepthAttachmentInvalidUsage) {
    TEST_DESCRIPTION("Create framebuffer with depth/stencil attachment that doesn't have depth stencil usage");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image image(*m_device, 32u, 32u, 1u, depth_stencil_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pDepthStencilAttachment = &attach;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = depth_stencil_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1u;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1u;
    rpci.pSubpasses = &subpass;

    vkt::RenderPass rp_ds(*m_device, rpci);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = rp_ds.handle();
    fb_info.attachmentCount = 1u;
    fb_info.pAttachments = &image_view.handle();
    fb_info.width = 32u;
    fb_info.height = 32u;
    fb_info.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-02633");
    vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, Framebuffer2DViewDsFormat) {
    TEST_DESCRIPTION("Create a 2D depth/stencil view from 3D image to use as a framebuffer attachment");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());

    VkImageFormatProperties2 image_format_prop = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_3D;
    image_format_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_format_info.format = depth_format;
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Required image parameters are unsupported";
    }

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = depth_format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference;
    attachment_reference.attachment = 0u;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &attachment_reference;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.attachmentCount = 1u;
    rp_ci.pAttachments = &attachment_description;
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = depth_format;
    image_ci.extent.width = 32u;
    image_ci.extent.height = 32u;
    image_ci.extent.depth = 1u;
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = depth_format;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0u;
    image_view_ci.subresourceRange.levelCount = 1u;
    image_view_ci.subresourceRange.baseArrayLayer = 0u;
    image_view_ci.subresourceRange.layerCount = 1u;
    vkt::ImageView view(*m_device, image_view_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &view.handle();
    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00891");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferMismatchedFormat) {
    TEST_DESCRIPTION("Create a framebuffer where its attachment format doesn't match render pass attachemnt reference format");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageFormatProperties2 image_format_prop = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_format_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
    if (result != VK_SUCCESS || (image_format_prop.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_2_BIT) == 0) {
        GTEST_SKIP() << "Required image parameters are unsupported";
    }

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_B8G8R8A8_UNORM;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference;
    attachment_reference.attachment = 0u;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1u;
    subpass.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.attachmentCount = 1u;
    rp_ci.pAttachments = &attachment_description;
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 32u;
    image_ci.extent.height = 32u;
    image_ci.extent.depth = 1u;
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0u;
    image_view_ci.subresourceRange.levelCount = 1u;
    image_view_ci.subresourceRange.baseArrayLayer = 0u;
    image_view_ci.subresourceRange.layerCount = 1u;
    vkt::ImageView view(*m_device, image_view_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &view.handle();
    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00880");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    image_ci.format = attachment_description.format;
    image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
    vkt::Image image2(*m_device, image_ci);

    image_view_ci.format = attachment_description.format;
    image_view_ci.image = image2.handle();
    vkt::ImageView view2(*m_device, image_view_ci);

    framebuffer_ci.pAttachments = &view2.handle();
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00881");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    image_ci.mipLevels = 2u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image image3(*m_device, image_ci);

    image_view_ci.image = image3.handle();
    image_view_ci.subresourceRange.levelCount = 2u;
    vkt::ImageView view3(*m_device, image_view_ci);

    framebuffer_ci.pAttachments = &view3.handle();
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00883");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferCreateWithInvalidExtent) {
    TEST_DESCRIPTION("Create a framebuffer with extent greater than attachments extent");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::ImageView view = m_renderTargets[0]->CreateView();

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = m_renderPass;
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &view.handle();
    framebuffer_ci.width = m_renderPassBeginInfo.renderArea.extent.width + 1;
    framebuffer_ci.height = m_renderPassBeginInfo.renderArea.extent.height;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04533");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.width = m_renderPassBeginInfo.renderArea.extent.width;
    framebuffer_ci.height = m_renderPassBeginInfo.renderArea.extent.height + 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04534");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.height = m_renderPassBeginInfo.renderArea.extent.height;
    framebuffer_ci.layers = 2u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04535");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferCreateWithInvalidSwizzle) {
    TEST_DESCRIPTION("Create a framebuffer with image view that does not have identity swizzle");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        features2 = GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.imageViewFormatSwizzle) {
            GTEST_SKIP() << "imageViewFormatSwizzle not supported";
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    InitRenderTarget();

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = m_renderTargets[0]->handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = m_render_target_fmt;
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_B;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_R;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0u;
    image_view_ci.subresourceRange.levelCount = 1u;
    image_view_ci.subresourceRange.baseArrayLayer = 0u;
    image_view_ci.subresourceRange.layerCount = 1u;
    vkt::ImageView view(*m_device, image_view_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = m_renderPass;
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &view.handle();
    framebuffer_ci.width = m_renderPassBeginInfo.renderArea.extent.width;
    framebuffer_ci.height = m_renderPassBeginInfo.renderArea.extent.height;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-00884");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferMultiviewWithLayers) {
    TEST_DESCRIPTION("Create a framebuffer with multiple layers when renderpass has non-zero view masks");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference;
    attachment_reference.attachment = 0u;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1u;
    subpass.pColorAttachments = &attachment_reference;

    uint32_t view_mask = 0x3u;
    VkRenderPassMultiviewCreateInfo render_pass_multiview_ci = vku::InitStructHelper();
    render_pass_multiview_ci.subpassCount = 1u;
    render_pass_multiview_ci.pViewMasks = &view_mask;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&render_pass_multiview_ci);
    rp_ci.attachmentCount = 1u;
    rp_ci.pAttachments = &attachment_description;
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rp_ci);

    view_mask = 0x8u;
    vkt::RenderPass render_pass2(*m_device, rp_ci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 32u;
    image_ci.extent.height = 32u;
    image_ci.extent.depth = 1u;
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 2u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0u;
    image_view_ci.subresourceRange.levelCount = 1u;
    image_view_ci.subresourceRange.baseArrayLayer = 0u;
    image_view_ci.subresourceRange.layerCount = 2u;
    vkt::ImageView view(*m_device, image_view_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &view.handle();
    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 2u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-02531");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.renderPass = render_pass2.handle();
    framebuffer_ci.layers = 1u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-04536");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, FramebufferLimits) {
    TEST_DESCRIPTION("Test creating framebuffer outside of limits");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    VkSubpassDescription subpass = {};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.width = m_device->Physical().limits_.maxFramebufferWidth + 1u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-width-00886");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.width = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-width-00885");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.width = 32u;
    framebuffer_ci.height = m_device->Physical().limits_.maxFramebufferHeight + 1u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-height-00888");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.height = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-height-00887");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = m_device->Physical().limits_.maxFramebufferLayers + 1u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-layers-00890");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.layers = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-layers-00889");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRenderPass, CreateFramebufferWithNullHandleView) {
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription description = {0,
                                           VK_FORMAT_R8G8B8A8_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_STORE,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference attachment_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1u, &attachment_ref, nullptr, nullptr, 0,
                                    nullptr};

    auto rp_ci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, &description, 1u, &subpass, 0u, nullptr);
    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkImageView view = VK_NULL_HANDLE;

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1u;
    framebuffer_ci.pAttachments = &view;
    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-02778");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}
