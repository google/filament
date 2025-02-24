/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"

class NegativeFragmentShadingRate : public VkLayerTest {};

TEST_F(NegativeFragmentShadingRate, Values) {
    TEST_DESCRIPTION("Specify invalid fragment shading rate values");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_command_buffer.Begin();
    fragmentSize.width = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;

    fragmentSize.width = 3;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 3;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;

    fragmentSize.width = 8;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 8;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;
    m_command_buffer.End();
}

TEST_F(NegativeFragmentShadingRate, ValuesNoFeatures) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeFragmentShadingRate, CombinerOpsNoFeatures) {
    TEST_DESCRIPTION("Specify combiner operations when only pipeline rate is supported");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineFragmentShadingRate);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_command_buffer.Begin();

    combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    m_command_buffer.End();
}

TEST_F(NegativeFragmentShadingRate, CombinerOpsNoPipelineRate) {
    TEST_DESCRIPTION("Specify pipeline rate when only attachment or primitive rate are supported");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::primitiveFragmentShadingRate);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_command_buffer.Begin();
    fragmentSize.width = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.width = 1;

    fragmentSize.height = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508");
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
    m_errorMonitor->VerifyFound();
    fragmentSize.height = 1;
}

TEST_F(NegativeFragmentShadingRate, CombinerOpsLimit) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::primitiveFragmentShadingRate);
    RETURN_IF_SKIP(Init());
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.fragmentShadingRateNonTrivialCombinerOps) {
        GTEST_SKIP() << "requires fragmentShadingRateNonTrivialCombinerOps to be unsupported.";
    }
    InitRenderTarget();

    VkExtent2D fragmentSize = {1, 1};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    m_command_buffer.Begin();
    {
        // primitiveFragmentShadingRate
        combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512");
        vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
        m_errorMonitor->VerifyFound();
        combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }

    {
        // attachmentFragmentShadingRate
        combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512");
        vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragmentSize, combinerOps);
        m_errorMonitor->VerifyFound();
        combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }
    m_command_buffer.End();
}

TEST_F(NegativeFragmentShadingRate, PrimitiveFragmentShadingRateWriteMultiViewportLimitDynamic) {
    TEST_DESCRIPTION("Test dynamic validation of the primitiveFragmentShadingRateWithMultipleViewports limit");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::primitiveFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        GTEST_SKIP() << "Test requires primitiveFragmentShadingRateWithMultipleViewports to be unsupported.";
    }
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_fragment_shading_rate : enable
        void main() {
            gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkViewport viewports[] = {{0, 0, 16, 16, 0, 1}, {1, 1, 16, 16, 0, 1}};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2, viewports);

    // error produced here.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapReferences) {
    TEST_DESCRIPTION("Create a subpass with the wrong attachment information for a fragment density map ");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) &&
        !IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Extensions not supported";
    }

    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_LOAD,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_PREINITIALIZED,
                                      VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    // Set 1 instead of 0
    VkAttachmentReference ref = {1, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    auto rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-fragmentDensityMapAttachment-06471",
                         nullptr);

    // Set wrong VkImageLayout
    ref = {0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT >(nullptr, ref);
    rpci = vku::InitStruct<VkRenderPassCreateInfo >(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02549", nullptr);

    // Set wrong load operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_CLEAR,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_PREINITIALIZED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT >(nullptr, ref);
    rpci = vku::InitStruct<VkRenderPassCreateInfo >(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02550", nullptr);

    // Set wrong store operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_LOAD,
              VK_ATTACHMENT_STORE_OP_STORE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_PREINITIALIZED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT >(nullptr, ref);
    rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02551", nullptr);
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapDuplicateReferences) {
    /* Note: This test omits testing of depth attachments. The wording of
       VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02548 states that they should be
       covered but currently no depth formats are valid for use as fragment density map attachments.
    */
    TEST_DESCRIPTION("Create a subpass with the attachment used as both fragment density map and another attachment");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) &&
        !IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Extensions not supported";
    }

    // Fragment density map attachment idx referenced in multiple places
    VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());
    VkAttachmentDescription attachments[6] = {
        // 0th: color attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_PREINITIALIZED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // 1st: depth-stencil attachment
        {0, depth_stencil_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_PREINITIALIZED,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // 2nd: fdm
        {0, VK_FORMAT_R8G8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_PREINITIALIZED,
         VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT},
        // 3rd: color preserve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_PREINITIALIZED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // 4th: input attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_PREINITIALIZED,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
    };

    VkAttachmentReference ref_input = {4, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref_color = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference ref_depth = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkAttachmentReference ref_preserve = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference ref_fdm = {2, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    {
        // FDM in input attachments
        VkAttachmentReference ref_fail[2] = {ref_input, ref_fdm};
        VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 2, ref_fail, 1, &ref_color, nullptr, &ref_depth,
                                        1, &ref_preserve.attachment};
        auto rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref_fdm);
        auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 5u, attachments, 1u, &subpass, 0u, nullptr);

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                             "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02548", nullptr);
    }

    {
        // FDM in color attachments
        VkAttachmentReference ref_fail[2] = {ref_color, ref_fdm};
        VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref_input, 2, ref_fail, nullptr, &ref_depth,
                                        1, &ref_preserve.attachment};
        auto rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref_fdm);
        auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 5u, attachments, 1u, &subpass, 0u, nullptr);

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                             "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02548", nullptr);
    }

    {
        // FDM as preserve attachment
        uint32_t ref_fail[2] = {ref_preserve.attachment, ref_fdm.attachment};
        VkSubpassDescription subpass = {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref_input, 1, &ref_color, nullptr, &ref_depth, 2, ref_fail};
        auto rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref_fdm);
        auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpfdmi, 0u, 5u, attachments, 1u, &subpass, 0u, nullptr);

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false,
                             "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02548", nullptr);
    }
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapLayerCount) {
    TEST_DESCRIPTION("Specify a fragment density map attachment with incorrect layerCount");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    auto rpfdmi = vku::InitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>(nullptr, ref);

    // Create a renderPass with viewMask 0
    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.viewMask = 0;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper(&rpfdmi);
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass rp(*m_device, rpci);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView imageView = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, VK_REMAINING_MIP_LEVELS, 0, 2);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView.handle();
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    VkFramebuffer fb;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-02746");
    vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();

    // Set viewMask to non-zero - requires multiview
    subpass.viewMask = 0x10;
    vkt::RenderPass rp_mv(*m_device, rpci);

    fb_info.renderPass = rp_mv.handle();

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-02746");
    vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Fragment Density Maps are used.");

    // VK_EXT_fragment_density_map2 requires VK_EXT_fragment_density_map
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    const bool fdm2Supported = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT density_map_features =
        vku::InitStructHelper();
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT density_map2_features =
        vku::InitStructHelper(&density_map_features);
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(density_map2_features);

    if (density_map_features.fragmentDensityMapDynamic == VK_FALSE) {
        GTEST_SKIP() << "fragmentDensityMapDynamic not supported";
    }

    features2 = vku::InitStructHelper(&density_map2_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT density_map2_properties = vku::InitStructHelper();
    auto properties2 = GetPhysicalDeviceProperties2(density_map2_properties);

    // Test sampler parameters

    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.maxLod = 0.0;
    sampler_info_ref.flags |= VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;

    // min max filters must match
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02574");
    sampler_info.minFilter = sampler_info_ref.minFilter;
    sampler_info.magFilter = sampler_info_ref.magFilter;

    // mipmapMode must be SAMPLER_MIPMAP_MODE_NEAREST
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02575");
    sampler_info.mipmapMode = sampler_info_ref.mipmapMode;

    // minLod and maxLod must be 0.0
    sampler_info.minLod = 1.0;
    sampler_info.maxLod = 1.0;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02576");
    sampler_info.minLod = sampler_info_ref.minLod;
    sampler_info.maxLod = sampler_info_ref.maxLod;

    // addressMode must be CLAMP_TO_EDGE or CLAMP_TO_BORDER
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02577");
    sampler_info.addressModeU = sampler_info_ref.addressModeU;

    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02577");
    sampler_info.addressModeV = sampler_info_ref.addressModeV;

    // some features cannot be enabled for subsampled samplers
    if (features2.features.samplerAnisotropy == VK_TRUE) {
        sampler_info.anisotropyEnable = VK_TRUE;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02578");
        sampler_info.anisotropyEnable = sampler_info_ref.anisotropyEnable;
        sampler_info.anisotropyEnable = VK_FALSE;
    }

    sampler_info.compareEnable = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02579");
    sampler_info.compareEnable = sampler_info_ref.compareEnable;

    sampler_info.unnormalizedCoordinates = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02580");
    sampler_info.unnormalizedCoordinates = sampler_info_ref.unnormalizedCoordinates;

    // Test image parameters

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.flags = 0;

    // only VK_IMAGE_TYPE_2D is supported
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.extent.height = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02557");

    // only VK_SAMPLE_COUNT_1_BIT is supported
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-samples-02558");

    // tiling must be VK_IMAGE_TILING_OPTIMAL
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02565");

    // only 2D
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02566");

    // no cube maps
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 64;
    image_create_info.arrayLayers = 6;
    image_create_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02567");

    // mipLevels must be 1
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 2;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02568");

    // Test image view parameters

    // create a valid density map image
    image_create_info.flags = 0;
    image_create_info.mipLevels = 1;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    vkt::Image densityImage(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = densityImage.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // density maps can't be sparse (or protected)
    if (features2.features.sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
        vkt::Image image(*m_device, image_create_info, vkt::no_mem);

        ivci.image = image.handle();
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-04116");
    }

    if (fdm2Supported) {
        if (!density_map2_features.fragmentDensityMapDeferred) {
            ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT;
            ivci.image = densityImage.handle();
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-03567");
        } else {
            ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT;
            ivci.flags |= VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
            ivci.image = densityImage.handle();
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-03568");
        }
        if (density_map2_properties.maxSubsampledArrayLayers < properties2.properties.limits.maxImageArrayLayers) {
            image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.arrayLayers = density_map2_properties.maxSubsampledArrayLayers + 1;
            vkt::Image image(*m_device, image_create_info, vkt::set_layout);
            ivci.image = image.handle();
            ivci.flags = 0;
            ivci.subresourceRange.layerCount = density_map2_properties.maxSubsampledArrayLayers + 1;
            m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-imageViewType-04973");
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-03569");
        }
    }
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapDisabled) {
    TEST_DESCRIPTION("Checks for when the fragment density map features are not enabled.");

    // VK_EXT_fragment_density_map2 requires VK_EXT_fragment_density_map
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;
    vkt::Image image2D(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Flags must not be set if the feature is not enabled
    ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-02572");
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapReferenceAttachment) {
    TEST_DESCRIPTION(
        "Test creating a framebuffer with fragment density map reference to an attachment with layer count different from 1");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires Vulkan version 1.0";
    }

    VkAttachmentReference ref;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = vku::InitStructHelper();
    rpfdmi.fragmentDensityMapAttachment = ref;

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_R8G8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpfdmi);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rpci);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView imageView = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 4);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = render_pass.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView.handle();
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-02746");
    vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FramebufferUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04548");
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &imageView.handle(),
                                 fsr_properties.minFragmentShadingRateAttachmentTexelSize.width,
                                 fsr_properties.minFragmentShadingRateAttachmentTexelSize.height);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FramebufferDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);
    if (fsr_properties.layeredShadingRateAttachments != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRatePropertiesKHR::layeredShadingRateAttachments not supported.";
    }

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.CreateRenderPass();

    VkImageCreateInfo ici =
        vkt::Image::ImageCreateInfo2D(1, 1, 1, 2, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vkt::Image image(*m_device, ici);
    auto image_view_ci = image.BasicViewCreatInfo();
    image_view_ci.subresourceRange.layerCount = 2;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    const auto imageView = vkt::ImageView(*m_device, image_view_ci);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = rp.Handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView.handle();
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04539");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04540");
    fb.init(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    fb_info.layers = 3;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04538");
    fb.init(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FramebufferDimensionsMultiview) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);
    if (fsr_properties.layeredShadingRateAttachments != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRatePropertiesKHR::layeredShadingRateAttachments not supported.";
    }

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.SetViewMask(0x4);
    rp.CreateRenderPass();

    VkImageCreateInfo ici =
        vkt::Image::ImageCreateInfo2D(1, 1, 1, 2, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vkt::Image image(*m_device, ici);
    auto image_view_ci = image.BasicViewCreatInfo();
    image_view_ci.subresourceRange.layerCount = 2;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    const auto imageView = vkt::ImageView(*m_device, image_view_ci);

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = rp.Handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView.handle();
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04537");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, Attachments) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkAttachmentReference2 attach = vku::InitStructHelper();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = vku::InitStructHelper();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = vku::InitStructHelper(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass rp;

    rpci.flags = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderPassCreateInfo2-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-flags-04521");
    vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    rpci.flags = 0;
    attach_desc.format =
        FindFormatWithoutFeatures(Gpu(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    if (attach_desc.format != VK_FORMAT_UNDEFINED) {
        m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-pAttachments-04586");
        vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
    }
    attach_desc.format = VK_FORMAT_R8_UINT;

    attach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04524");
    vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width + 1;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height + 1;
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04525");
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04528");
    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width ==
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.width) {
        m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04526");
    }
    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height ==
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height) {
        m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04529");
    }
    vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width / 2;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height / 2;
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04527");
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04530");
    vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width * 2;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04526");
    m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04529");
    vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width /
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.height >
        fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
        fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width;
        m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04531");
        vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
        fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    }

    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height /
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.width >
        fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
        fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height;
        m_errorMonitor->SetDesiredError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04532");
        vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
        fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    }
}

TEST_F(NegativeFragmentShadingRate, LoadOpClear) {
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    VkAttachmentReference2 attach = vku::InitStructHelper();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = vku::InitStructHelper();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;
    VkSubpassDescription2 subpass = vku::InitStructHelper(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-pAttachments-09387");
    VkRenderPass rp;
    vk::CreateRenderPass2KHR(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, IncompatibleFragmentRateShadingAttachmentInExecuteCommands) {
    TEST_DESCRIPTION(
        "Test incompatible fragment shading rate attachments "
        "calling CmdExecuteCommands");

    // Enable KHR_fragment_shading_rate
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    // Create a render pass without a Fragment Shading Rate attachment
    RenderPass2SingleSubpass rp_no_fsr(*this);
    rp_no_fsr.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp_no_fsr.CreateRenderPass();

    // Create 2 render passes with fragment shading rate attachments with
    // differing shadingRateAttachmentTexelSize values
    VkExtent2D texel_size_1 = {8, 8};
    VkExtent2D texel_size_2 = {32, 32};

    RenderPass2SingleSubpass rp_fsr_1(*this);
    rp_fsr_1.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp_fsr_1.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp_fsr_1.AddFragmentShadingRateAttachment(0, texel_size_1);
    rp_fsr_1.CreateRenderPass();

    RenderPass2SingleSubpass rp_fsr_2(*this);
    rp_fsr_2.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp_fsr_2.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp_fsr_2.AddFragmentShadingRateAttachment(0, texel_size_2);
    rp_fsr_2.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();

    // Create a frame buffer with a render pass with FSR attachment
    vkt::Framebuffer framebuffer_fsr(*m_device, rp_fsr_1.Handle(), 1, &imageView.handle());

    // Create a frame buffer with a render pass without FSR attachment
    vkt::Framebuffer framebuffer_no_fsr(*m_device, rp_no_fsr.Handle(), 1, &imageView.handle());

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Inheritance info without FSR attachment
    const VkCommandBufferInheritanceInfo cmdbuff_ii_no_fsr = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        rp_no_fsr.Handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi_no_fsr = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                   nullptr,  // pNext
                                                   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii_no_fsr};
    cmdbuff__bi_no_fsr.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    // Render pass begin info for no FSR attachment
    const auto rp_bi_no_fsr = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp_no_fsr.Handle(), framebuffer_no_fsr.handle(),
                                                                     VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    // Inheritance info with FSR attachment
    const VkCommandBufferInheritanceInfo cmdbuff_ii_fsr = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        rp_fsr_2.Handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi_fsr = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                nullptr,  // pNext
                                                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii_fsr};
    cmdbuff__bi_fsr.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    // Render pass begin info with FSR attachment
    const auto rp_bi_fsr = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp_fsr_1.Handle(), framebuffer_fsr.handle(),
                                                                  VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    // Test case where primary command buffer does not have an FSR attachment but
    // secondary command buffer does.
    {
        secondary.Begin(&cmdbuff__bi_fsr);
        secondary.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pBeginInfo-06020");

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp_bi_no_fsr, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
        m_errorMonitor->VerifyFound();
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();
    secondary.Reset();

    // Test case where primary command buffer has FSR attachment but secondary
    // command buffer does not.
    {
        secondary.Begin(&cmdbuff__bi_no_fsr);
        secondary.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pBeginInfo-06020");

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp_bi_fsr, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
        m_errorMonitor->VerifyFound();
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();
    secondary.Reset();

    // Test case where both command buffers have FSR attachments but they are
    // incompatible.
    {
        secondary.Begin(&cmdbuff__bi_fsr);
        secondary.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pBeginInfo-06020");

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp_bi_fsr, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();
    secondary.Reset();
}

TEST_F(NegativeFragmentShadingRate, ShadingRateUsage) {
    TEST_DESCRIPTION("Specify invalid usage of the fragment shading rate image view usage.");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    const VkFormat format =
        FindFormatWithoutFeatures(Gpu(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    if (format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "No format found without shading rate attachment support";
    }

    VkImageFormatProperties imageFormatProperties;
    if (vk::GetPhysicalDeviceImageFormatProperties(Gpu(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                   VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0,
                                                   &imageFormatProperties) == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "Format not supported";
    }
    // Initialize image with transfer source usage
    vkt::Image image(*m_device, 128, 128, 1, format, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageViewCreateInfo createinfo = vku::InitStructHelper();
    createinfo.image = image.handle();
    createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createinfo.format = format;
    createinfo.subresourceRange.layerCount = 1;
    createinfo.subresourceRange.baseMipLevel = 0;
    createinfo.subresourceRange.levelCount = 1;
    if (vkuFormatIsColor(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    } else if (vkuFormatHasDepth(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatHasStencil(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    // Create a view with the fragment shading rate attachment usage, but that doesn't support it
    CreateImageViewTest(*this, &createinfo, "VUID-VkImageViewCreateInfo-usage-04550");

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsrProperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsrProperties);

    if (!fsrProperties.layeredShadingRateAttachments) {
        if (IsPlatformMockICD()) {
            GTEST_SKIP() << "Test not supported by MockICD, doesn't correctly advertise format support for fragment shading "
                            "rate attachments";
        } else {
            auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8_UINT,
                                                          VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
            vkt::Image image2(*m_device, image_ci);

            createinfo.image = image2.handle();
            createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            createinfo.format = VK_FORMAT_R8_UINT;
            createinfo.subresourceRange.layerCount = 2;
            createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            CreateImageViewTest(*this, &createinfo, "VUID-VkImageViewCreateInfo-usage-04551");
        }
    }
}

TEST_F(NegativeFragmentShadingRate, Pipeline) {
    TEST_DESCRIPTION("Specify invalid fragment shading rate values");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineFragmentShadingRate);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = vku::InitStructHelper();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    fsr_ci.fragmentSize.width = 0;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04494");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 0;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04495");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.fragmentSize.width = 3;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04496");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 3;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04497");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.fragmentSize.width = 8;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04498");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 8;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04499");
    fsr_ci.fragmentSize.height = 1;
}

TEST_F(NegativeFragmentShadingRate, PipelineFeatureUsage) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = vku::InitStructHelper();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    fsr_ci.fragmentSize.width = 2;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500");
    fsr_ci.fragmentSize.width = 1;

    fsr_ci.fragmentSize.height = 2;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500");
    fsr_ci.fragmentSize.height = 1;

    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04501");
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04502");
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
}

TEST_F(NegativeFragmentShadingRate, PipelineCombinerOpsLimit) {
    TEST_DESCRIPTION("Specify invalid use of combiner ops when non trivial ops aren't supported");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::pipelineFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::primitiveFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.fragmentShadingRateNonTrivialCombinerOps) {
        GTEST_SKIP() << "requires fragmentShadingRateNonTrivialCombinerOps to be unsupported";
    }

    InitRenderTarget();

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = vku::InitStructHelper();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    // primitiveFragmentShadingRate
    {
        fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
        fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }

    // attachmentFragmentShadingRate
    {
        fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit,
                                          "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
        fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    }
}

TEST_F(NegativeFragmentShadingRate, PrimitiveWriteMultiViewportLimit) {
    TEST_DESCRIPTION("Test static validation of the primitiveFragmentShadingRateWithMultipleViewports limit");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::primitiveFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(InitFramework());

    const bool vil_extension = IsExtensionsEnabled(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    const bool va2_extension = IsExtensionsEnabled(VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        GTEST_SKIP() << "requires primitiveFragmentShadingRateWithMultipleViewports to be unsupported.";
    }

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Test PrimitiveShadingRate writes with multiple viewports
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        VkViewport viewports[2] = {{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}};
        VkRect2D scissors[2] = {};

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.vp_state_ci_.viewportCount = 2;
            info.vp_state_ci_.pViewports = viewports;
            info.vp_state_ci_.scissorCount = 2;
            info.vp_state_ci_.pScissors = scissors;
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, kErrorBit,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503");
    }
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);

    // Test PrimitiveShadingRate writes with ViewportIndex writes in a geometry shader
    if (features2.features.geometryShader) {
        char const *vsSource = R"glsl(
            #version 450
            void main() {}
        )glsl";

        static char const *gsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            layout (points) in;
            layout (points) out;
            layout (max_vertices = 1) out;
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
                gl_ViewportIndex = 0;
                gl_PointSize = 1.0f;
                EmitVertex();
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            info.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, kErrorBit,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504");
    }

    // Test PrimitiveShadingRate writes with ViewportIndex writes in a vertex shader
    if (vil_extension) {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            #extension GL_ARB_shader_viewport_layer_array : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                gl_ViewportIndex = 0;
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(
            *this, info_override, kErrorBit,
            "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504");
    }

    if (va2_extension) {
        // Test PrimitiveShadingRate writes with ViewportIndex writes in a geometry shader
        if (features2.features.geometryShader) {
            char const *vsSource = R"glsl(
                #version 450
                void main() {}
            )glsl";

            static char const *gsSource = R"glsl(
                #version 450
                #extension GL_EXT_fragment_shading_rate : enable
                #extension GL_NV_viewport_array2 : enable
                layout (points) in;
                layout (points) out;
                layout (max_vertices = 1) out;
                void main() {
                   gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                   gl_ViewportMask[0] = 0;
                   gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
                   gl_PointSize = 1.0f;
                   EmitVertex();
                }
            )glsl";

            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
            VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

            auto info_override = [&](CreatePipelineHelper &info) {
                info.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                info.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            };

            CreatePipelineHelper::OneshotTest(
                *this, info_override, kErrorBit,
                "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505");
        }

        // Test PrimitiveShadingRate writes with ViewportIndex writes in a vertex shader
        if (vil_extension) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_fragment_shading_rate : enable
                #extension GL_NV_viewport_array2 : enable
                void main() {
                    gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
                    gl_ViewportMask[0] = 0;
                }
            )glsl";

            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            };

            CreatePipelineHelper::OneshotTest(
                *this, info_override, kErrorBit,
                "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505");
        }
    }
}

TEST_F(NegativeFragmentShadingRate, Ops) {
    TEST_DESCRIPTION("Specify invalid fsr pipeline settings for the enabled features");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::primitiveFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineFragmentShadingRateStateCreateInfoKHR fsr_ci = vku::InitStructHelper();
    fsr_ci.fragmentSize.width = 1;
    fsr_ci.fragmentSize.height = 1;
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    auto set_fsr_ci = [&](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &fsr_ci; };

    // Pass an invalid value for op 0
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_ENUM_KHR;
    // if fragmentShadingRateNonTrivialCombinerOps is not supported
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06567");
    fsr_ci.combinerOps[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;

    // Pass an invalid value for op 1
    fsr_ci.combinerOps[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_ENUM_KHR;
    // if fragmentShadingRateNonTrivialCombinerOps is not supported
    m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506");
    CreatePipelineHelper::OneshotTest(*this, set_fsr_ci, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06568");
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapAttachmentCount) {
    TEST_DESCRIPTION("Test attachmentCount of VkRenderPassFragmentDensityMapCreateInfoEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference ref = {};
    ref.attachment = 1;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = vku::InitStructHelper();
    rpfdmi.fragmentDensityMapAttachment = ref;

    // Create a renderPass with viewMask 0
    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.viewMask = 0;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper(&rpfdmi);
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassCreateInfo2-fragmentDensityMapAttachment-06472");
    vk::CreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapOffsetQCOM) {
    TEST_DESCRIPTION("Ensure RenderPass end meets the requirements for VK_QCOM_fragment_density_map_offset");

    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    std::array<VkAttachmentDescription2, 7> attachments = {
        // FDM attachments
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8_UNORM, VK_SAMPLE_COUNT_1_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT),
        // input attachments
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL),
        // color attachments
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        // depth attachment
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
        // resolve attachment
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        // preserve attachments
        vku::InitStruct<VkAttachmentDescription2>(nullptr, 0u, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
    };

    /* TODO
    std::array<VkAttachmentReference2, 1> fdm = {
        vku::InitStruct<VkAttachmentReference2>(nullptr, 0u, VK_IMAGE_LAYOUT_GENERAL, VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
    };
    */

    std::array<VkAttachmentReference2, 1> input = {
        vku::InitStruct<VkAttachmentReference2>(nullptr, 1u, VK_IMAGE_LAYOUT_GENERAL, VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
    };

    std::array<VkAttachmentReference2, 2> color = {
        vku::InitStruct<VkAttachmentReference2>(nullptr, 2u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
        vku::InitStruct<VkAttachmentReference2>(nullptr, 3u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
    };
    auto depth = vku::InitStruct<VkAttachmentReference2>(nullptr, 4u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                       VkImageAspectFlags{VK_IMAGE_ASPECT_DEPTH_BIT});
    std::vector<VkAttachmentReference2> resolve = {
        vku::InitStruct<VkAttachmentReference2>(nullptr, 5u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VkImageAspectFlags{VK_IMAGE_ASPECT_COLOR_BIT}),
        vku::InitStruct<VkAttachmentReference2>(nullptr, VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0u),
    };
    std::vector<uint32_t> preserve = {6};

    auto subpass = vku::InitStruct<VkSubpassDescription2>(nullptr, 0u, VK_PIPELINE_BIND_POINT_GRAPHICS, 0u, size32(input),
                                                        input.data(), size32(color), color.data(), resolve.data(), &depth,
                                                        size32(preserve), preserve.data());

    // Create a renderPass with a single color attachment for fragment density map
    VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_create_info = vku::InitStructHelper();
    fragment_density_map_create_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo2>(&fragment_density_map_create_info, 0u, size32(attachments),
                                                       attachments.data(), 1u, &subpass, 0u, nullptr, 0u, nullptr);

    // Create rp2[0] without Multiview (zero viewMask), rp2[1] with Multiview
    vkt::RenderPass rp2[2];
    rp2[0].init(*m_device, rpci);

    subpass.viewMask = 0x3u;
    rp2[1].init(*m_device, rpci);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 16;
    image_create_info.extent.height = 16;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.flags = 0;

    vkt::Image fdm_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image input_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image color_image1(*m_device, image_create_info, vkt::set_layout);
    vkt::Image color_image2(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = ds_format;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image depth_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image resolve_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image preserve_image(*m_device, image_create_info, vkt::set_layout);

    // Create view attachment
    VkImageView iv[7];
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = fdm_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.flags = 0;
    ivci.subresourceRange.layerCount = 3;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView iv0(*m_device, ivci);
    ASSERT_TRUE(iv0.initialized());
    iv[0] = iv0.handle();

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.image = input_image.handle();
    vkt::ImageView iv1(*m_device, ivci);
    ASSERT_TRUE(iv1.initialized());
    iv[1] = iv1.handle();

    ivci.image = color_image1.handle();
    vkt::ImageView iv2(*m_device, ivci);
    ASSERT_TRUE(iv2.initialized());
    iv[2] = iv2.handle();

    ivci.image = color_image2.handle();
    vkt::ImageView iv3(*m_device, ivci);
    ASSERT_TRUE(iv3.initialized());
    iv[3] = iv3.handle();

    ivci.format = ds_format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    ivci.image = depth_image.handle();
    vkt::ImageView iv4(*m_device, ivci);
    ASSERT_TRUE(iv4.initialized());
    iv[4] = iv4.handle();

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.image = resolve_image.handle();
    vkt::ImageView iv5(*m_device, ivci);
    ASSERT_TRUE(iv5.initialized());
    iv[5] = iv5.handle();

    ivci.image = preserve_image.handle();
    vkt::ImageView iv6(*m_device, ivci);
    ASSERT_TRUE(iv6.initialized());
    iv[6] = iv6.handle();

    vkt::Framebuffer fb1(*m_device, rp2[0].handle(), 7, iv, 16, 16);
    vkt::Framebuffer fb2(*m_device, rp2[1].handle(), 7, iv, 16, 16);

    // define renderpass begin info
    auto rpbi1 =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp2[0].handle(), fb1.handle(), VkRect2D{{0, 0}, {16u, 16u}}, 0u, nullptr);
    auto rpbi2 =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp2[1].handle(), fb2.handle(), VkRect2D{{0, 0}, {16u, 16u}}, 0u, nullptr);

    {
        VkSubpassFragmentDensityMapOffsetEndInfoQCOM offsetting = vku::InitStructHelper();
        VkSubpassEndInfo subpassEndInfo = vku::InitStructHelper(&offsetting);
        VkOffset2D m_vOffsets[2];
        offsetting.pFragmentDensityOffsets = m_vOffsets;
        offsetting.fragmentDensityOffsetCount = 2;

        VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fdm_offset_properties = vku::InitStructHelper();
        GetPhysicalDeviceProperties2(fdm_offset_properties);

        m_vOffsets[0].x = 1;
        m_vOffsets[0].y = 1;

        m_vOffsets[1].x = 1;
        m_vOffsets[1].y = 1;

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        // begin renderpass that uses rbpi1 renderpass begin info
        vk::CmdBeginRenderPass(m_command_buffer.handle(), &rpbi1, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503");

        if (fdm_offset_properties.fragmentDensityOffsetGranularity.width > 1) {
            m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512", 2);
        }

        if (fdm_offset_properties.fragmentDensityOffsetGranularity.height > 1) {
            m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513", 2);
        }

        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-06502", 7);
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507", 2);
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06511");
        vk::CmdEndRenderPass2KHR(m_command_buffer.handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        // begin renderpass that uses rbpi2 renderpass begin info
        vk::CmdBeginRenderPass(m_command_buffer.handle(), &rpbi2, VK_SUBPASS_CONTENTS_INLINE);
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-06502", 7);
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507", 2);
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509");
        m_errorMonitor->SetDesiredError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06510");
        vk::CmdEndRenderPass2KHR(m_command_buffer.handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeFragmentShadingRate, ShadingRateImageNV) {
    TEST_DESCRIPTION("Test VK_NV_shading_rate_image.");

    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shadingRateImage);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test shading rate image creation
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8_UINT;
    image_create_info.extent.width = 4;
    image_create_info.extent.height = 4;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
    image_create_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    // image type must be 2D
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-02082");

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.arrayLayers = 6;

    // must be single sample
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-samples-02083");

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    // tiling must be optimal
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-shadingRateImage-07727");

    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    // Test image view creation
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // view type must be 2D or 2D_ARRAY
    {
        ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        ivci.subresourceRange.layerCount = 6;
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-02086");
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-01003");
        vkt::ImageView view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange.layerCount = 1;
    }

    // format must be R8_UINT
    {
        ivci.format = VK_FORMAT_R8_UNORM;
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-02087");
        vkt::ImageView view(*m_device, ivci);
        m_errorMonitor->VerifyFound();
        ivci.format = VK_FORMAT_R8_UINT;
    }

    vkt::ImageView view(*m_device, ivci);

    // Test pipeline creation
    VkPipelineViewportShadingRateImageStateCreateInfoNV vsrisci = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV};

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[20] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[20] = {scissor, scissor};
    VkDynamicState dynPalette = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 1, &dynPalette};

    // viewportCount must be 0 or 1 when multiViewport is disabled
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 2;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 2;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 2;
        };
        constexpr std::array vuids = {"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                      "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                      "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"};
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, vuids);
    }

    // pShadingRatePalettes must not be NULL.
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 1;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04057"}));
    }

    // Create an image without the SRI bit
    vkt::Image nonSRIimage(*m_device, 256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView nonSRIview = nonSRIimage.CreateView();

    // Test SRI layout on non-SRI image
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
    img_barrier.image = nonSRIimage.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();

    // Error trying to convert it to SRI layout
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-oldLayout-02088");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // succeed converting it to GENERAL
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // if the view is non-NULL, it must be R8_UINT, USAGE_SRI, image layout must match, layout must be valid
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadingRateImageNV-imageView-02060");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadingRateImageNV-imageView-02061");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadingRateImageNV-imageView-02062");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadingRateImageNV-imageLayout-02063");
    vk::CmdBindShadingRateImageNV(m_command_buffer.handle(), nonSRIview, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_errorMonitor->VerifyFound();

    VkShadingRatePaletteEntryNV paletteEntries[100] = {};
    VkShadingRatePaletteNV palette = {100, paletteEntries};
    VkShadingRatePaletteNV palettes[] = {palette, palette};

    // errors on firstViewport/viewportCount
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067");
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068");
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069");
    vk::CmdSetViewportShadingRatePaletteNV(m_command_buffer.handle(), 20, 2, palettes);
    m_errorMonitor->VerifyFound();

    // shadingRatePaletteEntryCount must be in range
    m_errorMonitor->SetDesiredError("VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071");
    vk::CmdSetViewportShadingRatePaletteNV(m_command_buffer.handle(), 0, 1, palettes);
    m_errorMonitor->VerifyFound();

    VkCoarseSampleLocationNV locations[100] = {
        {0, 0, 0},    {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 1, 1},  // duplicate
        {1000, 0, 0},                                              // pixelX too large
        {0, 1000, 0},                                              // pixelY too large
        {0, 0, 1000},                                              // sample too large
    };

    // Test custom sample orders, both via pipeline state and via dynamic state
    {
        VkCoarseSampleOrderCustomNV sampOrdBadShadingRate = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV, 1, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 3, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleLocationCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV,
                                                                     2, 2, locations};
        VkCoarseSampleOrderCustomNV sampOrdDuplicateLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                 1 * 2 * 2, &locations[1]};
        VkCoarseSampleOrderCustomNV sampOrdOutOfRangeLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                  1 * 2 * 2, &locations[4]};
        VkCoarseSampleOrderCustomNV sampOrdTooLargeSampleLocationCount = {
            VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 64, &locations[8]};
        VkCoarseSampleOrderCustomNV sampOrdGood = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2, 1 * 2 * 2,
                                                   &locations[0]};

        VkPipelineViewportCoarseSampleOrderStateCreateInfoNV csosci = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV};
        csosci.sampleOrderType = VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV;
        csosci.customSampleOrderCount = 1;

        struct TestCase {
            const VkCoarseSampleOrderCustomNV *order;
            std::vector<std::string> vuids;
        };

        std::vector<TestCase> test_cases = {
            {&sampOrdBadShadingRate, {"VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073"}},
            {&sampOrdBadSampleCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074", "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdBadSampleLocationCount, {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdDuplicateLocations, {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdOutOfRangeLocations,
             {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077", "VUID-VkCoarseSampleLocationNV-pixelX-02078",
              "VUID-VkCoarseSampleLocationNV-pixelY-02079", "VUID-VkCoarseSampleLocationNV-sample-02080"}},
            {&sampOrdTooLargeSampleLocationCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076",
              "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdGood, {}},
        };

        for (const auto &test_case : test_cases) {
            const auto break_vp = [&](CreatePipelineHelper &helper) {
                helper.vp_state_ci_.pNext = &csosci;
                csosci.pCustomSampleOrders = test_case.order;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
        }

        for (const auto &test_case : test_cases) {
            for (uint32_t i = 0; i < test_case.vuids.size(); ++i) {
                m_errorMonitor->SetDesiredError(test_case.vuids[i].c_str());
            }
            vk::CmdSetCoarseSampleOrderNV(m_command_buffer.handle(), VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, 1, test_case.order);
            if (test_case.vuids.size()) {
                m_errorMonitor->VerifyFound();
            } else {
            }
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081");
        vk::CmdSetCoarseSampleOrderNV(m_command_buffer.handle(), VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV, 1, &sampOrdGood);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeFragmentShadingRate, ShadingRateImageNVViewportCount) {
    TEST_DESCRIPTION("Test VK_NV_shading_rate_image viewportCount.");
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shadingRateImage);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test shading rate image creation
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8_UINT;
    image_create_info.extent.width = 4;
    image_create_info.extent.height = 4;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
    image_create_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView view = image.CreateView();

    VkPipelineViewportShadingRateImageStateCreateInfoNV vsrisci = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV};

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[20] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[20] = {scissor, scissor};
    VkDynamicState dynPalette = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 1, &dynPalette};

    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.vp_state_ci_.viewportCount = 1;
        helper.vp_state_ci_.pViewports = viewports;
        helper.vp_state_ci_.scissorCount = 1;
        helper.vp_state_ci_.pScissors = scissors;
        helper.vp_state_ci_.pNext = &vsrisci;
        helper.dyn_state_ci_ = dyn;

        vsrisci.shadingRateImageEnable = VK_TRUE;
        vsrisci.viewportCount = 2;
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                      "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056");
}

TEST_F(NegativeFragmentShadingRate, StageUsage) {
    TEST_DESCRIPTION("Specify shading rate pipeline stage with attachmentFragmentShadingRate feature disabled");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);
    const vkt::Event event(*m_device);
    const vkt::Event event2(*m_device);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent2-stageMask-07316");
    vk::CmdResetEvent2KHR(m_command_buffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-07318");
    vk::CmdSetEvent(m_command_buffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-shadingRateImage-07314");
    vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeFragmentShadingRate, StageUsageNV) {
    TEST_DESCRIPTION(
        "Specify shading rate pipeline stage with shading rate features disabled and NV shading rate extension enabled");
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);
    const vkt::Event event(*m_device);
    const vkt::Event event2(*m_device);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent2-stageMask-07316");
    vk::CmdResetEvent2KHR(m_command_buffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-07318");
    vk::CmdSetEvent(m_command_buffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-shadingRateImage-07314");
    vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
TEST_F(NegativeFragmentShadingRate, ImageMaxLimitsQCOM) {
    TEST_DESCRIPTION("Tests physical device limits for VK_QCOM_fragment_density_map_offset.");
    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkPhysicalDeviceLimits &dev_limits = m_device->Physical().limits_;
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8_UNORM;
    image_ci.extent = {1, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageFormatProperties img_limits;
    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(Gpu(), &image_ci, &img_limits));

    image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
    if (dev_limits.maxFramebufferWidth + 1 > img_limits.maxExtent.width) {
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-extent-02252");
    }
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06514");

    image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
    if (dev_limits.maxFramebufferHeight + 1 > img_limits.maxExtent.height) {
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-extent-02253");
    }
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06515");
}

TEST_F(NegativeFragmentShadingRate, Framebuffer) {
    TEST_DESCRIPTION("VUIDs related to framebuffer creation");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceFragmentDensityMapPropertiesEXT fdm_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_properties);

    VkFormat format = VK_FORMAT_R8G8_UNORM;

    VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_ci = vku::InitStructHelper();
    fragment_density_map_ci.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass_description = {};

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&fragment_density_map_ci);
    render_pass_ci.attachmentCount = 1u;
    render_pass_ci.pAttachments = &attachment_description;
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass_description;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Image image2(*m_device, 32u, 32u, 1u, format, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image2.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = format;
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
    framebuffer_ci.width = 33u * fdm_properties.maxFragmentDensityTexelSize.width;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-02555");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 33u * fdm_properties.maxFragmentDensityTexelSize.height;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-02556");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, FragmentDensityMapNonSubsampledImages) {
    TEST_DESCRIPTION("Test creating framebuffer with non subsampled images");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);

    RETURN_IF_SKIP(Init());

    VkFramebuffer fb;

    VkFormat attachment_format = VK_FORMAT_R8G8_UNORM;
    // Just use the same values for both height and width
    uint32_t frame_size = 512;

    // Create a render pass with a color attachment and fragment density map attachment
    VkAttachmentDescription attach[2] = {};
    attach[0].format = attachment_format;
    attach[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attach[0].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attach[1].format = attachment_format;
    attach[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach[1].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach[1].finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    attach[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_create_info = vku::InitStructHelper();
    fragment_density_map_create_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    fragment_density_map_create_info.fragmentDensityMapAttachment.attachment = 1;

    VkSubpassDescription subpass = {};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&fragment_density_map_create_info);
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attach;

    vkt::RenderPass rp(*m_device, rpci);

    // Don't use the VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT flag at the color attachment image creation
    vkt::Image image(*m_device, frame_size, frame_size, 1, attachment_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image_fdm(*m_device, frame_size, frame_size, 1, attachment_format, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = attachment_format;
    ivci.flags = 0;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.image = image.handle();

    VkImageViewCreateInfo ivci_fdm = ivci;
    ivci.image = image_fdm.handle();

    vkt::ImageView image_view(*m_device, ivci);
    vkt::ImageView image_view_fdm(*m_device, ivci_fdm);

    VkImageView views[2];
    views[0] = image_view.handle();
    views[1] = image_view_fdm.handle();

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.flags = 0;
    fbci.width = frame_size;
    fbci.height = frame_size;
    fbci.layers = 1;
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 2;
    fbci.pAttachments = views;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-02553");
    vk::CreateFramebuffer(device(), &fbci, NULL, &fb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, AttachmentFragmentDensityFlags) {
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentDensityMapPropertiesEXT fdm_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_properties);

    VkImageCreateInfo fdm_ici = vku::InitStructHelper();
    fdm_ici.imageType = VK_IMAGE_TYPE_2D;
    fdm_ici.format = VK_FORMAT_R8G8_UNORM;
    fdm_ici.extent.width = 16;
    fdm_ici.extent.height = 16;
    fdm_ici.extent.depth = 1;
    fdm_ici.mipLevels = 1;
    fdm_ici.arrayLayers = 1;
    fdm_ici.samples = VK_SAMPLE_COUNT_1_BIT;
    fdm_ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    fdm_ici.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    // VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT should not be allowed for density map
    fdm_ici.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;

    vkt::Image fdm_image(*m_device, fdm_ici, vkt::set_layout);
    vkt::ImageView fdm_image_view = fdm_image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);

    VkImageCreateInfo base_ici = vku::InitStructHelper();
    base_ici.imageType = VK_IMAGE_TYPE_2D;
    base_ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    base_ici.extent.width = 16;
    base_ici.extent.height = 16;
    base_ici.extent.depth = 1;
    base_ici.mipLevels = 1;
    base_ici.arrayLayers = 1;
    base_ici.samples = VK_SAMPLE_COUNT_1_BIT;
    base_ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    base_ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // for non fragment density map images this is required
    base_ici.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;

    vkt::Image image(*m_device, base_ici, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    VkAttachmentReference fdm_ref;
    fdm_ref.attachment = 0;
    fdm_ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkAttachmentReference color_ref;
    color_ref.attachment = 1;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = vku::InitStructHelper();
    rpfdmi.fragmentDensityMapAttachment = fdm_ref;

    VkAttachmentDescription fdm_attach = {};
    fdm_attach.format = VK_FORMAT_R8G8_UNORM;
    fdm_attach.samples = VK_SAMPLE_COUNT_1_BIT;
    fdm_attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    fdm_attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    fdm_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    fdm_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    fdm_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    fdm_attach.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkAttachmentDescription color_attach = {};
    color_attach.format = VK_FORMAT_R8G8B8A8_UNORM;
    color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription attach[2] = {};
    attach[0] = fdm_attach;
    attach[1] = color_attach;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpfdmi);
    rpci.attachmentCount = 2;
    rpci.pAttachments = attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rpci);

    VkImageView attachments[2] = {};
    attachments[0] = fdm_image_view.handle();
    attachments[1] = image_view.handle();

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.flags = 0;
    fb_info.renderPass = render_pass.handle();
    fb_info.attachmentCount = 2;
    fb_info.pAttachments = attachments;
    fb_info.width = 1;
    fb_info.height = 1;
    fb_info.layers = 1;

    VkFramebuffer framebuffer;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-02552");
    vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeFragmentShadingRate, ImagelessAttachmentFragmentDensity) {
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentDensityMapPropertiesEXT fdm_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fdm_properties);
    const auto &maxFragmentDensityTexelSize = fdm_properties.maxFragmentDensityTexelSize;

    VkFormat image_format = VK_FORMAT_R8G8_UNORM;

    VkAttachmentReference ref;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = vku::InitStructHelper();
    rpfdmi.fragmentDensityMapAttachment = ref;

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_R8G8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpfdmi);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rpci);

    VkFramebufferAttachmentImageInfo fb_fdm = vku::InitStructHelper();
    fb_fdm.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    // FDM width and height must be greater than framebuffer.{width, height} / maxFragmentDensityTexelSize.{width, height}
    fb_fdm.width = 1;
    fb_fdm.height = 1;
    fb_fdm.layerCount = 1;
    fb_fdm.viewFormatCount = 1;
    fb_fdm.pViewFormats = &image_format;

    VkFramebufferAttachmentsCreateInfo fb_aci_fdm = vku::InitStructHelper();
    fb_aci_fdm.attachmentImageInfoCount = 1;
    fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fb_aci_fdm);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = render_pass.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = nullptr;
    fb_info.width = 64;
    fb_info.height = 64;
    fb_info.layers = 1;

    VkFramebuffer framebuffer;
    {
        // fb.width / maxFragmentDensityTexelSize.width > attachment.width
        fb_fdm.width = 1;
        fb_fdm.height = 2048;

        fb_info.width = maxFragmentDensityTexelSize.width * 5;
        fb_info.height = 2;

        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03196");
        vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
        m_errorMonitor->VerifyFound();
    }

    {
        // fb.height / maxFragmentDensityTexelSize.height > attachment.height
        fb_fdm.width = 2048;
        fb_fdm.height = 1;

        fb_info.width = 2;
        fb_info.height = maxFragmentDensityTexelSize.width * 5;

        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03197");
        vk::CreateFramebuffer(device(), &fb_info, nullptr, &framebuffer);
        m_errorMonitor->VerifyFound();
    }
}
