/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativePipelineBinary : public VkLayerTest {};

TEST_F(NegativePipelineBinary, GetPipelineKey) {
    TEST_DESCRIPTION("Try getting the pipeline key with invalid input structures");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    {
        VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper();
        VkDescriptorSetLayoutCreateInfo layout_create_info = vku::InitStructHelper();
        VkPipelineBinaryKeyKHR pipeline_key = vku::InitStructHelper();

        pipeline_create_info.pNext = &layout_create_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineCreateInfoKHR-pNext-09604");
        vk::GetPipelineKeyKHR(device(), &pipeline_create_info, &pipeline_key);
        m_errorMonitor->VerifyFound();
    }

    {
        VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

        std::vector<VkDescriptorSetLayoutBinding> bindings(0);
        const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
        const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

        VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
        compute_create_info.stage = cs.GetStageCreateInfo();
        compute_create_info.layout = pipeline_layout.handle();

        VkPipelineBinaryInfoKHR pipeline_binary_info = vku::InitStructHelper();
        pipeline_binary_info.binaryCount = 1;

        compute_create_info.pNext = &pipeline_binary_info;

        VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&compute_create_info);

        VkPipelineBinaryKeyKHR pipeline_key = vku::InitStructHelper();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelineKeyKHR-pNext-09605");
        vk::GetPipelineKeyKHR(device(), &pipeline_create_info, &pipeline_key);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativePipelineBinary, ReleaseCapturedDataAllocator) {
    TEST_DESCRIPTION("Test using ReleaseCapturedData with/without an allocator");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    struct Alloc {
        static VKAPI_ATTR void *VKAPI_CALL alloc(void *, size_t size, size_t, VkSystemAllocationScope) { return malloc(size); };
        static VKAPI_ATTR void *VKAPI_CALL reallocFunc(void *, void *original, size_t size, size_t, VkSystemAllocationScope) {
            return realloc(original, size);
        };
        static VKAPI_ATTR void VKAPI_CALL freeFunc(void *, void *ptr) { free(ptr); };
        static VKAPI_ATTR void VKAPI_CALL internalAlloc(void *, size_t, VkInternalAllocationType, VkSystemAllocationScope){};
        static VKAPI_ATTR void VKAPI_CALL internalFree(void *, size_t, VkInternalAllocationType, VkSystemAllocationScope){};
    };
    const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::reallocFunc, Alloc::freeFunc, nullptr, nullptr};

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
    compute_create_info.pNext = &flags2;

    VkPipeline test_pipeline_with_allocator;
    VkResult err =
        vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, &allocator, &test_pipeline_with_allocator);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipeline test_pipeline_no_allocator;
    err = vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &test_pipeline_no_allocator);
    ASSERT_EQ(VK_SUCCESS, err);

    VkReleaseCapturedPipelineDataInfoKHR data_info = vku::InitStructHelper();

    data_info.pipeline = test_pipeline_with_allocator;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkReleaseCapturedPipelineDataKHR-pipeline-09611");
    vk::ReleaseCapturedPipelineDataKHR(device(), &data_info, nullptr);
    m_errorMonitor->VerifyFound();

    data_info.pipeline = test_pipeline_no_allocator;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkReleaseCapturedPipelineDataKHR-pipeline-09612");
    vk::ReleaseCapturedPipelineDataKHR(device(), &data_info, &allocator);
    m_errorMonitor->VerifyFound();

    vk::DestroyPipeline(device(), test_pipeline_with_allocator, &allocator);
    vk::DestroyPipeline(device(), test_pipeline_no_allocator, nullptr);
}

TEST_F(NegativePipelineBinary, ReleaseCapturedData) {
    TEST_DESCRIPTION("Test using ReleaseCapturedPipelineDataKHR in a bad state");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    {
        VkReleaseCapturedPipelineDataInfoKHR data_info = vku::InitStructHelper();
        data_info.pipeline = pipe.Handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkReleaseCapturedPipelineDataInfoKHR-pipeline-09613");
        vk::ReleaseCapturedPipelineDataKHR(device(), &data_info, nullptr);
        m_errorMonitor->VerifyFound();
    }

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    CreateComputePipelineHelper pipe2(*this, &flags2);
    pipe2.CreateComputePipeline(true, true);

    {
        VkReleaseCapturedPipelineDataInfoKHR data_info = vku::InitStructHelper();
        data_info.pipeline = pipe2.Handle();

        vk::ReleaseCapturedPipelineDataKHR(device(), &data_info, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkReleaseCapturedPipelineDataInfoKHR-pipeline-09618");
        vk::ReleaseCapturedPipelineDataKHR(device(), &data_info, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativePipelineBinary, Destroy) {
    TEST_DESCRIPTION("Test using DestroyPipelineBinary with/without an allocator");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    CreateComputePipelineHelper pipe(*this, &flags2);
    pipe.CreateComputePipeline(true, true);

    struct Alloc {
        static VKAPI_ATTR void *VKAPI_CALL alloc(void *, size_t size, size_t, VkSystemAllocationScope) { return malloc(size); };
        static VKAPI_ATTR void *VKAPI_CALL reallocFunc(void *, void *original, size_t size, size_t, VkSystemAllocationScope) {
            return realloc(original, size);
        };
        static VKAPI_ATTR void VKAPI_CALL freeFunc(void *, void *ptr) { free(ptr); };
        static VKAPI_ATTR void VKAPI_CALL internalAlloc(void *, size_t, VkInternalAllocationType, VkSystemAllocationScope){};
        static VKAPI_ATTR void VKAPI_CALL internalFree(void *, size_t, VkInternalAllocationType, VkSystemAllocationScope){};
    };
    const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::reallocFunc, Alloc::freeFunc, nullptr, nullptr};

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pipeline = pipe.Handle();

    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;

    VkPipelineBinaryKHR pipeline_binary_alloc;
    handles_info.pPipelineBinaries = &pipeline_binary_alloc;
    VkResult err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, &allocator, &handles_info);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyPipelineBinaryKHR-pipelineBinary-09614");
    vk::DestroyPipelineBinaryKHR(device(), pipeline_binary_alloc, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineBinaryKHR(device(), pipeline_binary_alloc, &allocator);

    VkPipelineBinaryKHR pipeline_binary_no_alloc;
    handles_info.pPipelineBinaries = &pipeline_binary_no_alloc;
    err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyPipelineBinaryKHR-pipelineBinary-09615");
    vk::DestroyPipelineBinaryKHR(device(), pipeline_binary_no_alloc, &allocator);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineBinaryKHR(device(), pipeline_binary_no_alloc, nullptr);
}

TEST_F(NegativePipelineBinary, ComputePipeline) {
    TEST_DESCRIPTION("Test creating a compute pipeline with bad pipeline binary settings");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPipelineCache pipeline_cache;
    VkPipelineCacheCreateInfo cache_create_info = vku::InitStructHelper();
    cache_create_info.initialDataSize = 0;
    VkResult err = vk::CreatePipelineCache(device(), &cache_create_info, nullptr, &pipeline_cache);
    ASSERT_EQ(VK_SUCCESS, err);

    {
        VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
        flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateComputePipelines-pNext-09617");
        CreateComputePipelineHelper pipe(*this, &flags2);
        pipe.CreateComputePipeline(true, false);
        m_errorMonitor->VerifyFound();

        pipe.CreateComputePipeline(true, true);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = pipe.Handle();

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pipelineBinaryCount = 1;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        std::vector<VkPipelineBinaryKHR> binaries(handles_info.pipelineBinaryCount);
        handles_info.pPipelineBinaries = binaries.data();

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();
        binary_info.binaryCount = handles_info.pipelineBinaryCount;
        binary_info.pPipelineBinaries = handles_info.pPipelineBinaries;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateComputePipelines-pNext-09616");
        CreateComputePipelineHelper pipe2(*this, &binary_info);
        pipe2.CreateComputePipeline(true, false);
        m_errorMonitor->VerifyFound();

        for (uint32_t i = 0; i < binaries.size(); i++) {
            vk::DestroyPipelineBinaryKHR(device(), binaries[i], nullptr);
        }
    }

    {
        VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
        flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

        CreateComputePipelineHelper pipe(*this, &flags2);
        pipe.CreateComputePipeline(true, true);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = pipe.Handle();

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pipelineBinaryCount = 1;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        std::vector<VkPipelineBinaryKHR> binaries(handles_info.pipelineBinaryCount);
        handles_info.pPipelineBinaries = binaries.data();

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();
        binary_info.binaryCount = handles_info.pipelineBinaryCount;
        binary_info.pPipelineBinaries = handles_info.pPipelineBinaries;

        VkPipelineCreationFeedbackCreateInfo feedback_create_info = vku::InitStructHelper();
        VkPipelineCreationFeedback feedback = {};

        feedback_create_info.pPipelineCreationFeedback = &feedback;
        feedback.flags = VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT |
                         VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT;

        flags2.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
        flags2.pNext = &binary_info;
        binary_info.pNext = &feedback_create_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateComputePipelines-binaryCount-09620");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateComputePipelines-binaryCount-09621");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateComputePipelines-binaryCount-09622");
        CreateComputePipelineHelper pipe2(*this, &flags2);
        pipe2.CreateComputePipeline(true, true);
        m_errorMonitor->VerifyFound();

        for (uint32_t i = 0; i < binaries.size(); i++) {
            vk::DestroyPipelineBinaryKHR(device(), binaries[i], nullptr);
        }
    }

    vk::DestroyPipelineCache(device(), pipeline_cache, nullptr);
}

TEST_F(NegativePipelineBinary, GraphicsPipeline) {
    TEST_DESCRIPTION("Test creating a graphics pipeline with bad pipeline binary settings");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPipelineCache pipeline_cache;
    VkPipelineCacheCreateInfo cache_create_info = vku::InitStructHelper();
    cache_create_info.initialDataSize = 0;
    VkResult err = vk::CreatePipelineCache(device(), &cache_create_info, nullptr, &pipeline_cache);
    ASSERT_EQ(VK_SUCCESS, err);

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());

    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr};

    const VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};

    const VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info_template{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f};

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = vku::InitStructHelper();
    err = vk::CreatePipelineLayout(device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
        pipeline_rasterization_state_create_info_template;
    pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_TRUE;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                               nullptr,
                                                               0,
                                                               1,
                                                               &vs.GetStageCreateInfo(),
                                                               &pipeline_vertex_input_state_create_info,
                                                               &pipeline_input_assembly_state_create_info,
                                                               nullptr,
                                                               nullptr,
                                                               &pipeline_rasterization_state_create_info,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               pipeline_layout,
                                                               m_renderPass,
                                                               0,
                                                               0,
                                                               0};

    {
        VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
        flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
        graphics_pipeline_create_info.pNext = &flags2;

        VkPipeline test_pipeline;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateGraphicsPipelines-pNext-09617");
        vk::CreateGraphicsPipelines(device(), pipeline_cache, 1, &graphics_pipeline_create_info, nullptr, &test_pipeline);
        m_errorMonitor->VerifyFound();

        err = vk::CreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &test_pipeline);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = test_pipeline;

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pipelineBinaryCount = 1;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        std::vector<VkPipelineBinaryKHR> binaries(handles_info.pipelineBinaryCount);
        handles_info.pPipelineBinaries = binaries.data();

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();
        binary_info.binaryCount = handles_info.pipelineBinaryCount;
        binary_info.pPipelineBinaries = handles_info.pPipelineBinaries;

        graphics_pipeline_create_info.pNext = &binary_info;

        VkPipeline test_pipeline2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateGraphicsPipelines-pNext-09616");
        vk::CreateGraphicsPipelines(device(), pipeline_cache, 1, &graphics_pipeline_create_info, nullptr, &test_pipeline2);
        m_errorMonitor->VerifyFound();

        for (uint32_t i = 0; i < binaries.size(); i++) {
            vk::DestroyPipelineBinaryKHR(device(), binaries[i], nullptr);
        }

        vk::DestroyPipeline(device(), test_pipeline, nullptr);
    }

    {
        VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
        flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
        graphics_pipeline_create_info.pNext = &flags2;

        VkPipeline test_pipeline;

        err = vk::CreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &test_pipeline);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = test_pipeline;

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pipelineBinaryCount = 1;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        std::vector<VkPipelineBinaryKHR> binaries(handles_info.pipelineBinaryCount);
        handles_info.pPipelineBinaries = binaries.data();

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();
        binary_info.binaryCount = handles_info.pipelineBinaryCount;
        binary_info.pPipelineBinaries = handles_info.pPipelineBinaries;

        VkPipelineCreationFeedbackCreateInfo feedback_create_info = vku::InitStructHelper();
        VkPipelineCreationFeedback feedback = {};

        feedback_create_info.pPipelineCreationFeedback = &feedback;
        feedback.flags = VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT |
                         VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT;

        flags2.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
        flags2.pNext = &binary_info;
        binary_info.pNext = &feedback_create_info;

        VkPipeline test_pipeline2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateGraphicsPipelines-binaryCount-09620");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateGraphicsPipelines-binaryCount-09621");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateGraphicsPipelines-binaryCount-09622");

        vk::CreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &test_pipeline2);
        m_errorMonitor->VerifyFound();

        for (uint32_t i = 0; i < binaries.size(); i++) {
            vk::DestroyPipelineBinaryKHR(device(), binaries[i], nullptr);
        }

        vk::DestroyPipeline(device(), test_pipeline, nullptr);
    }

    vk::DestroyPipelineLayout(device(), pipeline_layout, nullptr);
    vk::DestroyPipelineCache(device(), pipeline_cache, nullptr);
}

TEST_F(NegativePipelineBinary, Creation1) {
    TEST_DESCRIPTION("Test creating pipeline binaries with invalid parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline(true, true);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pipeline = pipe.Handle();

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;
    handles_info.pPipelineBinaries = &pipeline_binary;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-pipeline-09607");
    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipelineBinary, Creation2) {
    TEST_DESCRIPTION("Test creating pipeline binaries with invalid parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    CreateComputePipelineHelper pipe(*this, &flags2);
    pipe.CreateComputePipeline(true, true);

    VkReleaseCapturedPipelineDataInfoKHR release_info = vku::InitStructHelper();
    release_info.pipeline = pipe.Handle();

    VkResult err = vk::ReleaseCapturedPipelineDataKHR(device(), &release_info, nullptr);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pipeline = pipe.Handle();

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;
    handles_info.pPipelineBinaries = &pipeline_binary;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-pipeline-09608");
    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipelineBinary, Creation3) {
    TEST_DESCRIPTION("Test creating pipeline binaries with invalid parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPhysicalDevicePipelineBinaryPropertiesKHR pipeline_binary_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pipeline_binary_properties);

    if (pipeline_binary_properties.pipelineBinaryInternalCache) {
        GTEST_SKIP() << "pipelineBinaryInternalCache is VK_TRUE";
    }

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
    compute_create_info.pNext = &flags2;

    VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&compute_create_info);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pPipelineCreateInfo = &pipeline_create_info;

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;
    handles_info.pPipelineBinaries = &pipeline_binary;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-pipelineBinaryInternalCache-09609");
    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipelineBinary, Creation4) {
    TEST_DESCRIPTION("Test creating pipeline binaries with invalid parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    CreateComputePipelineHelper pipe(*this, &flags2);
    pipe.CreateComputePipeline(true, true);

    VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&pipe.cp_ci_);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;
    handles_info.pPipelineBinaries = &pipeline_binary;

    // test 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-pKeysAndDataInfo-09619");
    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();

    // test > 0
    binary_create_info.pipeline = pipe.Handle();
    binary_create_info.pPipelineCreateInfo = &pipeline_create_info;

    VkPhysicalDevicePipelineBinaryPropertiesKHR pipeline_binary_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pipeline_binary_properties);
    if (!pipeline_binary_properties.pipelineBinaryInternalCache) {
        m_errorMonitor->SetUnexpectedError("VUID-VkPipelineBinaryCreateInfoKHR-pipelineBinaryInternalCache-09609");
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-pKeysAndDataInfo-09619");
    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipelineBinary, Creation5) {
    TEST_DESCRIPTION("Test creating pipeline binaries with invalid parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkPhysicalDevicePipelineBinaryPropertiesKHR pipeline_binary_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pipeline_binary_properties);
    if (!pipeline_binary_properties.pipelineBinaryInternalCache) {
        GTEST_SKIP() << "pipelineBinaryInternalCache is VK_FALSE";
    }

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    CreateComputePipelineHelper pipe(*this, &flags2);
    pipe.CreateComputePipeline(true, true);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pipeline = pipe.Handle();

    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();

    VkResult err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    ASSERT_EQ(VK_SUCCESS, err);

    std::vector<VkPipelineBinaryKHR> binaries(handles_info.pipelineBinaryCount);
    handles_info.pPipelineBinaries = binaries.data();

    err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&pipe.cp_ci_);

    VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();
    binary_info.binaryCount = binaries.size();
    binary_info.pPipelineBinaries = binaries.data();
    flags2.pNext = &binary_info;

    handles_info.pPipelineBinaries = nullptr;
    binary_create_info.pipeline = VK_NULL_HANDLE;
    binary_create_info.pPipelineCreateInfo = &pipeline_create_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-pPipelineCreateInfo-09606");
    err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();

    for (uint32_t i = 0; i < binaries.size(); i++) {
        vk::DestroyPipelineBinaryKHR(device(), binaries[i], nullptr);
    }
}

TEST_F(NegativePipelineBinary, CreateCacheControl) {
    TEST_DESCRIPTION("Creating pipeline binaries with internal cache disabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePipelineBinaryPropertiesKHR pipeline_binary_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pipeline_binary_properties);

    if (!pipeline_binary_properties.pipelineBinaryInternalCacheControl) {
        GTEST_SKIP() << "pipelineBinaryInternalCacheControl is VK_FALSE";
    }

    VkDevicePipelineBinaryInternalCacheControlKHR cache_control = vku::InitStructHelper();
    cache_control.disableInternalCache = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &cache_control));

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
    compute_create_info.pNext = &flags2;

    VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&compute_create_info);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pPipelineCreateInfo = &pipeline_create_info;

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;
    handles_info.pPipelineBinaries = &pipeline_binary;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryCreateInfoKHR-device-09610");
    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePipelineBinary, InvalidPNext) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    VkPipelineBinaryInfoKHR pipeline_binary_info = vku::InitStructHelper();
    pipeline_binary_info.binaryCount = 0;

    compute_create_info.pNext = &pipeline_binary_info;

    VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&compute_create_info);

    VkPipelineBinaryKeyKHR pipeline_key = vku::InitStructHelper(&pipeline_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineBinaryKeyKHR-pNext-pNext");
    vk::GetPipelineKeyKHR(device(), &pipeline_create_info, &pipeline_key);
    m_errorMonitor->VerifyFound();
}
