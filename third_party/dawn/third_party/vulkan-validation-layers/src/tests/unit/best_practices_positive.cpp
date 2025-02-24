/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class VkPositiveBestPracticesLayerTest : public VkBestPracticesLayerTest {};

TEST_F(VkPositiveBestPracticesLayerTest, TestDestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vk::DestroyBuffer(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyBufferView(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyCommandPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorSetLayout(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDevice(VK_NULL_HANDLE, NULL);
    vk::DestroyEvent(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFence(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFramebuffer(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImage(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImageView(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyInstance(VK_NULL_HANDLE, NULL);
    vk::DestroyPipeline(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineCache(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineLayout(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyQueryPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyRenderPass(device(), VK_NULL_HANDLE, NULL);
    vk::DestroySampler(device(), VK_NULL_HANDLE, NULL);
    vk::DestroySemaphore(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyShaderModule(device(), VK_NULL_HANDLE, NULL);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[3] = {};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 3, descriptor_sets);

    vk::FreeMemory(device(), VK_NULL_HANDLE, NULL);
}

TEST_F(VkPositiveBestPracticesLayerTest, DrawingWithUnboundUnusedSet) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with first descriptor set begin unused and "
        "unbound. Its purpose is to catch regression of this bug: "
        "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4597.");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    OneOffDescriptorSet empty_ds(m_device, {});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&empty_ds.layout_, &empty_ds.layout_});

    m_command_buffer.Begin();

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &empty_ds.set_, 0, nullptr);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);

    // The draw command will most likely produce a crash in case of a regression.
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, DynStateIgnoreAttachments) {
    TEST_DESCRIPTION("Make sure pAttachments is ignored if dynamic state is enabled");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendAdvanced);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorWriteMask);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    if (!IsPlatformMockICD()) {
        // Several drivers have been observed to crash on the legal null pAttachments - restrict to MockICD for now
        GTEST_SKIP() << "This test only runs on MockICD";
    }
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    // pAttachments should be ignored with these four states set
    VkDynamicState dynamic_states[4] = {VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                                        VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = vku::InitStructHelper();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 4;

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.pAttachments = nullptr;
    pipe.gp_ci_.pDynamicState = &dynamic_create_info;
    pipe.CreateGraphicsPipeline();
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, PipelineLibraryNoRendering) {
    TEST_DESCRIPTION("Create a pipeline library without a render pass or rendering info");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    CreatePipelineHelper pre_raster_lib(*this);
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    pre_raster_lib.CreateGraphicsPipeline();
}

TEST_F(VkPositiveBestPracticesLayerTest, PushConstantSet) {
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { uint x[4]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0] * constants.x[1]);
        }
    )glsl";

    const char fsSource[] = R"glsl(
        #version 460
        layout(push_constant, std430) uniform foo {
            layout(offset = 16) float x;
        } constants;
        layout(location = 0) out vec4 uFragColor;
        void main(){
            uFragColor = vec4(0,1,0,constants.x);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    uint32_t data[5];
    std::vector<VkPushConstantRange> push_constant_ranges = {{VK_SHADER_STAGE_VERTEX_BIT, 0, 16},
                                                             {VK_SHADER_STAGE_FRAGMENT_BIT, 16, 4}};

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, push_constant_ranges);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 16, data);
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 16, 4, data);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, VertexBufferNotForAllDraws) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7636");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 32, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    input_attrib.location = 4;

    CreatePipelineHelper pipe0(*this);
    pipe0.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe0.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe0.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe0.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe0.CreateGraphicsPipeline();

    CreatePipelineHelper pipe1(*this);
    pipe1.CreateGraphicsPipeline();

    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kPerformanceWarningBit);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &kZeroDeviceSize);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe0.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, SetDifferentEvents) {
    TEST_DESCRIPTION("Signal different events");
    RETURN_IF_SKIP(InitBestPractices());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);  // TODO: should be part of BP config

    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.SetEvent(event2, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, ResetEventBeforeSet) {
    TEST_DESCRIPTION("Set event two times with reset in between");
    RETURN_IF_SKIP(InitBestPractices());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);  // TODO: should be part of BP config

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, ResetEventBeforeSetMultipleSubmits) {
    TEST_DESCRIPTION("Set event two times with reset in between from multiple submits");
    RETURN_IF_SKIP(InitBestPractices());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);  // TODO: should be part of BP config

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb2.End();
    m_default_queue->Submit(cb2);
    m_default_queue->Wait();
}

TEST_F(VkPositiveBestPracticesLayerTest, ResetEventBeforeSetMultipleSubmits2) {
    TEST_DESCRIPTION("Set event two times with reset in between using single submit with two batches");
    RETURN_IF_SKIP(InitBestPractices());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);  // TODO: should be part of BP config

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.ResetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb2.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb2.End();

    VkSubmitInfo submits[2];
    submits[0] = vku::InitStructHelper();
    submits[0].commandBufferCount = 1;
    submits[0].pCommandBuffers = &m_command_buffer.handle();
    submits[1] = vku::InitStructHelper();
    submits[1].commandBufferCount = 1;
    submits[1].pCommandBuffers = &cb2.handle();

    vk::QueueSubmit(m_default_queue->handle(), 2, submits, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(VkPositiveBestPracticesLayerTest, ResetEventFromSecondary) {
    TEST_DESCRIPTION("Set event two times with reset in between executed from a secondary command buffer");
    RETURN_IF_SKIP(InitBestPractices());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);  // TODO: should be part of BP config

    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.Begin();
    secondary_cb.ResetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    secondary_cb.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.ExecuteCommands(secondary_cb);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
}

TEST_F(VkPositiveBestPracticesLayerTest, CreateFifoRelaxedSwapchain) {
    TEST_DESCRIPTION("Test creating fifo relaxed swapchain");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    bool fifo_relaxed = false;
    for (const auto &present_mode : m_surface_present_modes) {
        if (present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            fifo_relaxed = true;
            break;
        }
    }
    if (!fifo_relaxed) {
        GTEST_SKIP() << "fifo relaxed present mode not supported";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 2;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(VkPositiveBestPracticesLayerTest, ResetCommandPool) {
    TEST_DESCRIPTION("Destroy event that was set in a command buffer");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    void *pNext = nullptr;
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.events) {
            GTEST_SKIP() << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events not supported";
        }
        pNext = &portability_subset_features;
    }
    RETURN_IF_SKIP(InitState(nullptr, pNext));

    {
        vkt::Event event1(*m_device);
        m_command_buffer.Begin();
        event1.CmdSet(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    vkt::Event event2(*m_device);
    m_command_buffer.Begin();
    event2.CmdSet(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(VkPositiveBestPracticesLayerTest, ShaderObjectDraw) {
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char kVertexGlsl[] = R"glsl(
        #version 460
        layout(location = 0) in vec4 pos;
        void main() {
           gl_Position = pos;
        }
    )glsl";

    std::vector<uint32_t> vert_spirv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexGlsl);
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = vert_spirv.size() * sizeof(uint32_t);
    create_info.pCode = vert_spirv.data();
    create_info.pName = "main";
    vkt::Shader vert_shader(*m_device, create_info);

    std::vector<uint32_t> frag_spirv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info.codeSize = frag_spirv.size() * sizeof(uint32_t);
    create_info.pCode = frag_spirv.data();
    vkt::Shader frag_shader(*m_device, create_info);

    m_vertex_buffer = new vkt::Buffer(*m_device, sizeof(float) * 12u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkVertexInputBindingDescription2EXT binding_desc = vku::InitStructHelper();
    binding_desc.binding = 0u;
    binding_desc.stride = sizeof(float);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_desc.divisor = 1u;

    VkVertexInputAttributeDescription2EXT attr_desc = vku::InitStructHelper();
    attr_desc.location = 0u;
    attr_desc.binding = 0u;
    attr_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    attr_desc.offset = 0u;

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kPerformanceWarningBit);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    VkDeviceSize offset = 0u;
    m_command_buffer.BindVertFragShader(vert_shader, frag_shader);
    SetDefaultDynamicStatesExclude();
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0u, 1u, &m_vertex_buffer->handle(), &offset);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1u, &binding_desc, 1u, &attr_desc);
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}
