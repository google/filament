/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/buffer_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeGpuAVIndirectBuffer : public GpuAVTest {};

TEST_F(NegativeGpuAVIndirectBuffer, DrawCountDeviceLimit) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);  // instead of enabling feature
    AddOptionalExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper(&mesh_shader_features);
    bool mesh_shader_enabled = false;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        GetPhysicalDeviceFeatures2(mesh_shader_features);
        mesh_shader_enabled = IsExtensionsEnabled(VK_EXT_MESH_SHADER_EXTENSION_NAME) && features13.maintenance4;
        if (mesh_shader_enabled) {
            mesh_shader_features.multiviewMeshShader = VK_FALSE;
            mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        }
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState(nullptr, (features13.dynamicRendering || mesh_shader_enabled) ? (void *)&features13 : nullptr));
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 2 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            kHostVisibleMemProps);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.Memory().Map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndirectCommand));
    draw_buffer.Memory().Unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.Memory().Unmap();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-countBuffer-02717");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                sizeof(VkDrawIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    if (features13.dynamicRendering) {
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-countBuffer-02717");
        vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                    sizeof(VkDrawIndirectCommand));

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    if (mesh_shader_enabled) {
        char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
          uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {})glsl";
        VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
        CreatePipelineHelper mesh_pipe(*this);
        mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
        mesh_pipe.CreateGraphicsPipeline();
        vkt::Buffer mesh_draw_buffer(*m_device, 2 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                     kHostVisibleMemProps);

        VkDrawMeshTasksIndirectCommandEXT *mesh_draw_ptr =
            static_cast<VkDrawMeshTasksIndirectCommandEXT *>(mesh_draw_buffer.Memory().Map());
        mesh_draw_ptr->groupCountX = 0;
        mesh_draw_ptr->groupCountY = 0;
        mesh_draw_ptr->groupCountZ = 0;
        mesh_draw_buffer.Memory().Unmap();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02717");
        count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
        *count_ptr = 2;
        count_buffer.Memory().Unmap();
        m_command_buffer.Begin(&begin_info);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), mesh_draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                             sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGpuAVIndirectBuffer, DrawCountDeviceLimitSubmit2) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit using vkQueueSubmit2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 2 * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            kHostVisibleMemProps);
    VkDrawIndexedIndirectCommand *draw_ptr = static_cast<VkDrawIndexedIndirectCommand *>(draw_buffer.Memory().Map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndexedIndirectCommand));
    draw_buffer.Memory().Unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.Memory().Unmap();

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717");
    vk::CmdDrawIndexedIndirectCount(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                    sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // use vkQueueSumit2
    m_default_queue->Submit2(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, DrawCount) {
    TEST_DESCRIPTION("GPU validation: Validate Draw*IndirectCount countBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper(&mesh_shader_features);
    bool mesh_shader_enabled = false;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        GetPhysicalDeviceFeatures2(mesh_shader_features);
        mesh_shader_enabled = IsExtensionsEnabled(VK_EXT_MESH_SHADER_EXTENSION_NAME) && features13.maintenance4;
        if (mesh_shader_enabled) {
            mesh_shader_features.multiviewMeshShader = VK_FALSE;
            mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, mesh_shader_enabled ? &features13 : nullptr));
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.Memory().Map());
    draw_ptr->firstInstance = 0;
    draw_ptr->firstVertex = 0;
    draw_ptr->instanceCount = 1;
    draw_ptr->vertexCount = 3;
    draw_buffer.Memory().Unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 2;
    count_buffer.Memory().Unmap();
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredWarningRegex(
        "WARNING-GPU-AV-drawCount",
        "Indirect draw count of 2 would exceed size \\(16\\) of buffer .* stride = 16 offset = 0 "
        ".* = 32");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 1;
    count_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Offset of 4 should error
    m_errorMonitor->SetDesiredWarningRegex(
        "WARNING-GPU-AV-drawCount",
        "Indirect draw count of 1 would exceed size \\(16\\) of buffer .* stride = 16 offset = 4 "
        ".* = 20");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vkt::Buffer indexed_draw_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    kHostVisibleMemProps);
    VkDrawIndexedIndirectCommand *indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.Memory().Map();
    indexed_draw_ptr->indexCount = 3;
    indexed_draw_ptr->firstIndex = 0;
    indexed_draw_ptr->instanceCount = 1;
    indexed_draw_ptr->firstInstance = 0;
    indexed_draw_ptr->vertexOffset = 0;
    indexed_draw_buffer.Memory().Unmap();

    count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 2;
    count_buffer.Memory().Unmap();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->SetDesiredWarningRegex(
        "WARNING-GPU-AV-drawCount",
        "Indirect draw count of 2 would exceed size \\(20\\) of buffer .* stride = 20 offset = 0 "
        ".* = 40");

    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), indexed_draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 1;
    count_buffer.Memory().Unmap();

    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    // Offset of 4 should error
    m_errorMonitor->SetDesiredWarningRegex(
        "WARNING-GPU-AV-drawCount",
        "Indirect draw count of 1 would exceed size \\(20\\) of buffer .* stride = 20 offset = 4 "
        ".* = 24");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), indexed_draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
    if (mesh_shader_enabled) {
        char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
          uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {})glsl";
        VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
        CreatePipelineHelper mesh_pipe(*this);
        mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
        mesh_pipe.CreateGraphicsPipeline();
        vkt::Buffer mesh_draw_buffer(*m_device, sizeof(VkDrawMeshTasksIndirectCommandEXT), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDrawMeshTasksIndirectCommandEXT *mesh_draw_ptr =
            +static_cast<VkDrawMeshTasksIndirectCommandEXT *>(mesh_draw_buffer.Memory().Map());
        mesh_draw_ptr->groupCountX = 0;
        mesh_draw_ptr->groupCountY = 0;
        mesh_draw_ptr->groupCountZ = 0;
        mesh_draw_buffer.Memory().Unmap();
        m_errorMonitor->SetDesiredWarningRegex(
            "WARNING-GPU-AV-drawCount",
            "Indirect draw count of 1 would exceed size \\(12\\) of buffer .* stride = 12 offset = 8 "
            ".* = 20");
        count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
        *count_ptr = 1;
        count_buffer.Memory().Unmap();
        m_command_buffer.Begin(&begin_info);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), mesh_draw_buffer.handle(), 8, count_buffer.handle(), 0, 1,
                                             sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredWarningRegex(
            "WARNING-GPU-AV-drawCount",
            "Indirect draw count of 2 would exceed size \\(12\\) of buffer .* stride = 12 offset = 4 "
            ".* = 28");
        count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
        *count_ptr = 2;
        count_buffer.Memory().Unmap();
        m_command_buffer.Begin(&begin_info);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), mesh_draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                             sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGpuAVIndirectBuffer, Mesh) {
    TEST_DESCRIPTION("GPU validation: Validate DrawMeshTasksIndirect* DrawBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_props);

    if (mesh_shader_props.maxMeshWorkGroupTotalCount > 0xfffffffe) {
        GTEST_SKIP() << "MeshWorkGroupTotalCount too high for this test";
    }
    const uint32_t num_commands = 3;
    uint32_t buffer_size = num_commands * (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4);  // 4 byte pad between commands

    vkt::Buffer draw_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *draw_ptr = static_cast<uint32_t *>(draw_buffer.Memory().Map());
    // Set all mesh group counts to 1
    for (uint32_t i = 0; i < num_commands * 4; ++i) {
        draw_ptr[i] = 1;
    }

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 3;
    count_buffer.Memory().Unmap();
    char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
          uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {}
    )glsl";
    VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
    CreatePipelineHelper mesh_pipe(*this);
    mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    mesh_pipe.CreateGraphicsPipeline();

    // Set x in third draw
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());

    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), draw_buffer.handle(), 0, 3,
                                    (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    if (mesh_shader_props.maxMeshWorkGroupCount[0] < std::numeric_limits<uint32_t>::max()) {
        draw_ptr[8] = mesh_shader_props.maxMeshWorkGroupCount[0] + 1;
        m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326");
        if (mesh_shader_props.maxMeshWorkGroupCount[0] + 1 >= mesh_shader_props.maxMeshWorkGroupTotalCount) {
            m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329");
        }
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
        draw_ptr[8] = 1;
    }

    // Set y in second draw
    if (mesh_shader_props.maxMeshWorkGroupCount[1] < std::numeric_limits<uint32_t>::max()) {
        draw_ptr[5] = mesh_shader_props.maxMeshWorkGroupCount[1] + 1;
        m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327");
        if (mesh_shader_props.maxMeshWorkGroupCount[1] + 1 >= mesh_shader_props.maxMeshWorkGroupTotalCount) {
            m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329");
        }
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
        draw_ptr[5] = 1;
    }

    // Set z in first draw
    if (mesh_shader_props.maxMeshWorkGroupCount[2] < std::numeric_limits<uint32_t>::max()) {
        draw_ptr[2] = mesh_shader_props.maxMeshWorkGroupCount[2] + 1;
        m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328");
        if (mesh_shader_props.maxMeshWorkGroupCount[2] + 1 >= mesh_shader_props.maxMeshWorkGroupTotalCount) {
            m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329");
        }
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
        draw_ptr[2] = 1;
    }
// total count can end up being really high, draw takes too long and times out
#if 0
    if (mesh_shader_props.maxMeshWorkGroupTotalCount < std::numeric_limits<uint32_t>::max()) {
        const uint32_t half_total = (mesh_shader_props.maxMeshWorkGroupTotalCount + 2) / 2;
        if (half_total < mesh_shader_props.maxMeshWorkGroupCount[0]) {
            draw_ptr[2] = 1;
            draw_ptr[1] = 2;
            draw_ptr[0] = (mesh_shader_props.maxMeshWorkGroupTotalCount + 2) / 2;

            m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329");
            m_default_queue->Submit(m_command_buffer);
            m_default_queue->Wait();
            m_errorMonitor->VerifyFound();

            draw_ptr[2] = 1;
            draw_ptr[1] = 1;
            draw_ptr[0] = 1;
        }
    }
#endif
    draw_buffer.Memory().Unmap();
}

TEST_F(NegativeGpuAVIndirectBuffer, DISABLED_MeshTask) {
    TEST_DESCRIPTION("GPU validation: Validate DrawMeshTasksIndirect* DrawBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_props);

    if (mesh_shader_props.maxMeshWorkGroupTotalCount > 0xfffffffe) {
        GTEST_SKIP() << "MeshWorkGroupTotalCount too high for this test";
    }
    const uint32_t num_commands = 3;
    uint32_t buffer_size = num_commands * (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4);  // 4 byte pad between commands

    vkt::Buffer draw_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *draw_ptr = static_cast<uint32_t *>(draw_buffer.Memory().Map());
    // Set all mesh group counts to 1
    for (uint32_t i = 0; i < num_commands * 4; ++i) {
        draw_ptr[i] = 1;
    }

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 3;
    count_buffer.Memory().Unmap();
    char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
        uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {}
        )glsl";
    VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);

    char const *task_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
        }
        )glsl";
    VkShaderObj task_shader(this, task_shader_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3);
    CreatePipelineHelper task_pipe(*this);
    task_pipe.shader_stages_[0] = task_shader.GetStageCreateInfo();
    task_pipe.shader_stages_[1] = mesh_shader.GetStageCreateInfo();
    task_pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, task_pipe.Handle());
    vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 3,
                                         (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Set x in second draw
    if (mesh_shader_props.maxTaskWorkGroupCount[0] < std::numeric_limits<uint32_t>::max()) {
        draw_ptr[4] = mesh_shader_props.maxTaskWorkGroupCount[0] + 1;
        m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
        draw_ptr[4] = 1;
    }

    // Set y in first draw
    if (mesh_shader_props.maxTaskWorkGroupCount[1] < std::numeric_limits<uint32_t>::max()) {
        draw_ptr[1] = mesh_shader_props.maxTaskWorkGroupCount[1] + 1;
        m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
        draw_ptr[1] = 1;
    }

    // Set z in third draw
    if (mesh_shader_props.maxTaskWorkGroupCount[2] < std::numeric_limits<uint32_t>::max()) {
        draw_ptr[10] = mesh_shader_props.maxTaskWorkGroupCount[2] + 1;
        m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
        draw_ptr[10] = 1;
    }

    if (mesh_shader_props.maxTaskWorkGroupTotalCount < std::numeric_limits<uint32_t>::max()) {
        const uint32_t half_total = (mesh_shader_props.maxTaskWorkGroupTotalCount + 2) / 2;
        if (half_total < mesh_shader_props.maxTaskWorkGroupCount[0]) {
            draw_ptr[2] = 1;
            draw_ptr[1] = 2;
            draw_ptr[0] = (mesh_shader_props.maxTaskWorkGroupTotalCount + 2) / 2;

            m_errorMonitor->SetDesiredError("VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325");
            m_default_queue->Submit(m_command_buffer);
            m_default_queue->Wait();
            m_errorMonitor->VerifyFound();

            draw_ptr[2] = 1;
            draw_ptr[1] = 1;
            draw_ptr[0] = 1;
        }
    }

    draw_buffer.Memory().Unmap();
}

TEST_F(NegativeGpuAVIndirectBuffer, FirstInstance) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    // silence MacOS issue
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.lineWidth = 1.0f;
    pipe.CreateGraphicsPipeline();

    VkDrawIndirectCommand draw_params{};
    draw_params.vertexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstVertex = 0;
    draw_params.firstInstance = 0;
    VkDrawIndirectCommand draw_params_invalid_first_instance_1 = draw_params;
    draw_params_invalid_first_instance_1.firstInstance = 1;
    VkDrawIndirectCommand draw_params_invalid_first_instance_42 = draw_params;
    draw_params_invalid_first_instance_42.firstInstance = 42;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndirectCommand>(
        *m_device, {draw_params, draw_params_invalid_first_instance_1, draw_params, draw_params_invalid_first_instance_42});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredErrorRegex("VUID-VkDrawIndirectCommand-firstInstance-00501", "at index 1 is 1");
    m_errorMonitor->SetDesiredErrorRegex("VUID-VkDrawIndirectCommand-firstInstance-00501", "at index 3 is 42");
    vk::CmdDrawIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 4, sizeof(VkDrawIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, FirstInstanceIndexed) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    VkDrawIndexedIndirectCommand draw_params_invalid_first_instance = draw_params;
    draw_params_invalid_first_instance.firstInstance = 1;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(
        *m_device, {draw_params, draw_params, draw_params, draw_params_invalid_first_instance});

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {1, 2, 3});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->SetDesiredErrorRegex("VUID-VkDrawIndexedIndirectCommand-firstInstance-00554", "at index 2 is 1");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), sizeof(VkDrawIndexedIndirectCommand), 3,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, DispatchWorkgroupSize) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand");
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());

    vkt::Buffer indirect_buffer(*m_device, 5 * sizeof(VkDispatchIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.Memory().Map());
    // VkDispatchIndirectCommand[0]
    ptr->x = 4;  // over
    ptr->y = 2;
    ptr->z = 1;
    // VkDispatchIndirectCommand[1]
    ptr++;
    ptr->x = 2;
    ptr->y = 3;  // over
    ptr->z = 1;
    // VkDispatchIndirectCommand[2] - valid in between
    ptr++;
    ptr->x = 1;
    ptr->y = 1;
    ptr->z = 1;
    // VkDispatchIndirectCommand[3]
    ptr++;
    ptr->x = 0;  // allowed
    ptr->y = 2;
    ptr->z = 3;  // over
    // VkDispatchIndirectCommand[4]
    ptr++;
    ptr->x = 3;  // over
    ptr->y = 2;
    ptr->z = 3;  // over
    indirect_buffer.Memory().Unmap();

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 4 * sizeof(VkDispatchIndirectCommand));

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Check again in a 2nd submitted command buffer
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);

    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, DispatchWorkgroupSizeShaderObjects) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitState());

    vkt::Buffer indirect_buffer(*m_device, 5 * sizeof(VkDispatchIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.Memory().Map());
    // VkDispatchIndirectCommand[0]
    ptr->x = 4;  // over
    ptr->y = 2;
    ptr->z = 1;
    // VkDispatchIndirectCommand[1]
    ptr++;
    ptr->x = 2;
    ptr->y = 3;  // over
    ptr->z = 1;
    // VkDispatchIndirectCommand[2] - valid in between
    ptr++;
    ptr->x = 1;
    ptr->y = 1;
    ptr->z = 1;
    // VkDispatchIndirectCommand[3]
    ptr++;
    ptr->x = 0;  // allowed
    ptr->y = 2;
    ptr->z = 3;  // over
    // VkDispatchIndirectCommand[4]
    ptr++;
    ptr->x = 3;  // over
    ptr->y = 2;
    ptr->z = 3;  // over
    indirect_buffer.Memory().Unmap();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
    vkt::Shader shader(*m_device, stage, GLSLToSPV(stage, kMinimalShaderGlsl));

    m_command_buffer.Begin();
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &shader.handle());

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 4 * sizeof(VkDispatchIndirectCommand));

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Check again in a 2nd submitted command buffer
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &shader.handle());

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);

    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, BasicTraceRaysMultipleStages) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    // TODO: Refacto ray tracing extensions listing.
    // Originally from RayTracingTest::InitFrameworkForRayTracingTest
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
        traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
        )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char *miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
        hit = vec3(0.1, 0.2, 0.3);
        }
        )glsl";
    pipeline.AddGlslMissShader(miss);

    const char *closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
        const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
        hit = barycentricCoords;
        }
        )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    // Descriptor set
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}
