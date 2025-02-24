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

class PositiveGpuAVIndexBuffer : public GpuAVTest {};

TEST_F(PositiveGpuAVIndexBuffer, BadVertexIndex) {
    TEST_DESCRIPTION("If no vertex buffer is used, all index values are legal");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState(nullptr));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, std::numeric_limits<uint32_t>::max(), 42});

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, VertexIndex) {
    TEST_DESCRIPTION("Validate index buffer values");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    constexpr uint32_t num_vertices = 12;
    std::vector<uint32_t> indicies(num_vertices);
    for (uint32_t i = 0; i < num_vertices; i++) {
        indicies[i] = num_vertices - 1 - i;
    }
    vkt::Buffer index_buffer = vkt::IndexBuffer(*m_device, std::move(indicies));

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, DrawIndexedDynamicStates) {
    TEST_DESCRIPTION("vkCmdDrawIndexed - Set dynamic states");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3PolygonMode);
    // AddRequiredFeature(vkt::Feature::depthBiasClamp);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450

        layout(location=0) in vec3 pos;

        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_CULL_MODE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_FRONT_FACE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 1, 2});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetCullModeEXT(m_command_buffer.handle(), VK_CULL_MODE_NONE);
    vk::CmdSetFrontFaceEXT(m_command_buffer.handle(), VK_FRONT_FACE_CLOCKWISE);
    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetDepthBias(m_command_buffer.handle(), 0.0f, 0.0f, 1.0f);
    vk::CmdSetLineWidth(m_command_buffer.handle(), 1.0f);
    vk::CmdSetPrimitiveRestartEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVIndexBuffer, IndexedIndirectRobustness) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    AddRequiredFeature(vkt::Feature::robustBufferAccess2);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 4;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, std::numeric_limits<uint32_t>::max(), 42});

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, NoShaderInputsVertexIndex16) {
    TEST_DESCRIPTION("Vertex shader defines no vertex attributes - no OOB vertex fetch should be detected");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 2> uv;
        std::array<float, 3> normal;
    };

    char const *vsSource = R"glsl(
        #version 450

        void main() {
            gl_Position = vec4(1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    // "Array of structs" style vertices
    VkVertexInputBindingDescription input_binding = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    std::array<VkVertexInputAttributeDescription, 3> vertex_attributes = {};
    // Position
    vertex_attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    // UV
    vertex_attributes[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float)};
    // Normal
    vertex_attributes[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, (3 + 2) * sizeof(float)};

    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = vertex_attributes.data();
    pipe.vi_ci_.vertexAttributeDescriptionCount = size32(vertex_attributes);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    std::vector<Vertex> vertices;
    for (int i = 0; i < 3; ++i) {
        const Vertex vertex = {{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f, 7.0f}};
        vertices.emplace_back(vertex);
    }
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<Vertex>(*m_device, vertices);
    // Offset vertex buffer so that only first Vertex can correctly be fetched
    VkDeviceSize vertex_buffer_offset = 2 * sizeof(Vertex);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 1, 0});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVIndexBuffer, VertexShaderUnusedLocations) {
    TEST_DESCRIPTION(
        "Vertex shader defines only a position vertex attribute - no OOB vertex fetch should be detected for uv and normal");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450

        layout(location=0) in vec3 pos;
        // Uncommenting will cause OOB vertex attribute fetch
        // layout(location=1) in vec2 uv;

        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    // "Struct of arrays" style vertices
    VkVertexInputBindingDescription position_input_binding_desc = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputBindingDescription uv_input_binding_desc = {1, 2 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputBindingDescription normal_input_binding_desc = {2, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    std::array<VkVertexInputBindingDescription, 3> input_binding_descs = {
        {position_input_binding_desc, uv_input_binding_desc, normal_input_binding_desc}};
    std::array<VkVertexInputAttributeDescription, 3> vertex_attributes = {};
    // Position
    vertex_attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    // UV
    vertex_attributes[1] = {1, 1, VK_FORMAT_R32G32_SFLOAT, 0};
    // Normal
    vertex_attributes[2] = {2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0};

    pipe.vi_ci_.pVertexBindingDescriptions = input_binding_descs.data();
    pipe.vi_ci_.vertexBindingDescriptionCount = size32(input_binding_descs);
    pipe.vi_ci_.pVertexAttributeDescriptions = vertex_attributes.data();
    pipe.vi_ci_.vertexAttributeDescriptionCount = size32(vertex_attributes);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    std::vector<float> positions;
    for (int i = 0; i < 3 * 3; ++i) {
        positions.emplace_back(float(i));
    }
    std::vector<float> uvs;
    for (int i = 0; i < 2; ++i) {
        uvs.emplace_back(float(i));
    }
    std::vector<float> normals;
    for (int i = 0; i < 3; ++i) {
        normals.emplace_back(float(i));
    }

    vkt::Buffer positions_buffer = vkt::VertexBuffer<float>(*m_device, positions);
    vkt::Buffer uvs_buffer = vkt::VertexBuffer<float>(*m_device, uvs);
    vkt::Buffer normals_buffer = vkt::VertexBuffer<float>(*m_device, normals);

    // Only position buffer will not cause OOB vertex attribute fetching, uv/normal would - should be fine, vertex shader does not
    // use those two.
    std::array<VkBuffer, 3> vertex_buffers_handles = {{positions_buffer.handle(), uvs_buffer.handle(), normals_buffer.handle()}};
    std::array<VkDeviceSize, 3> vertex_buffer_offsets = {{0, 0, 0}};
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, size32(vertex_buffers_handles), vertex_buffers_handles.data(),
                             vertex_buffer_offsets.data());

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 1, 2});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVIndexBuffer, InstanceIndex) {
    TEST_DESCRIPTION("No false positive for OOB instance index validation");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 2> uv;
        std::array<float, 3> normal;
    };

    char const *vsSource = R"glsl(
        #version 450

        layout(location=0) in vec3 pos;
        layout(location=1) in vec2 uv;
        layout(location=2) in vec3 normal;

        layout(location=3) in float instance_float;

        void main() {
            gl_Position = vec4(pos + uv.xyx + normal + instance_float, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    // "Array of structs" style vertices
    std::array<VkVertexInputBindingDescription, 2> input_bindings = {
        {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}, {1, sizeof(float), VK_VERTEX_INPUT_RATE_INSTANCE}}};
    std::array<VkVertexInputAttributeDescription, 4> vertex_attributes = {};
    // Position
    vertex_attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    // UV
    vertex_attributes[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float)};
    // Normal
    vertex_attributes[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, (3 + 2) * sizeof(float)};
    // Instance float
    vertex_attributes[3] = {3, 1, VK_FORMAT_R32_SFLOAT, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = size32(input_bindings);
    pipe.vi_ci_.pVertexBindingDescriptions = input_bindings.data();
    pipe.vi_ci_.vertexAttributeDescriptionCount = size32(vertex_attributes);
    pipe.vi_ci_.pVertexAttributeDescriptions = vertex_attributes.data();

    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    std::vector<Vertex> vertices;
    for (int i = 0; i < 3; ++i) {
        const Vertex vertex = {{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f, 7.0f}};
        vertices.emplace_back(vertex);
    }
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<Vertex>(*m_device, vertices);
    // Offset vertex buffer so that only first Vertex can correctly be fetched
    const VkDeviceSize vertex_buffer_offset = 2 * sizeof(Vertex);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    std::vector<float> instance_data = {42.0f, 39.5f, 1233.0f};
    vkt::Buffer instance_buffer = vkt::VertexBuffer<float>(*m_device, instance_data);
    const VkDeviceSize instance_data_offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 1, 1, &instance_buffer.handle(), &instance_data_offset);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 0, 0});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 3, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, CmdSetVertexInputEXT) {
    TEST_DESCRIPTION("Simple graphics pipeline, use vkCmdSetVertexInputEXT");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 2> uv;
        std::array<float, 3> normal;
    };

    char const *vsSource = R"glsl(
        #version 450

        layout(location=0) in vec3 pos;
        layout(location=1) in vec2 uv;
        layout(location=2) in vec3 normal;

        void main() {
            gl_Position = vec4(pos + uv.xyx + normal, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.gp_ci_.pVertexInputState = nullptr;
    // "Array of structs" style vertices
    VkVertexInputBindingDescription2EXT input_binding = vku::InitStructHelper();
    input_binding.binding = 0;
    input_binding.stride = sizeof(Vertex);
    input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    input_binding.divisor = 1;

    std::array<VkVertexInputAttributeDescription2EXT, 3> vertex_attributes = {};
    // Position
    vertex_attributes[0] = vku::InitStructHelper();
    vertex_attributes[0].location = 0;
    vertex_attributes[0].binding = 0;
    vertex_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[0].offset = 0;
    // UV
    vertex_attributes[1] = vku::InitStructHelper();
    vertex_attributes[1].location = 1;
    vertex_attributes[1].binding = 0;
    vertex_attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attributes[1].offset = 3 * sizeof(float);
    // Normal
    vertex_attributes[2] = vku::InitStructHelper();
    vertex_attributes[2].location = 2;
    vertex_attributes[2].binding = 0;
    vertex_attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[2].offset = (3 + 2) * sizeof(float);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &input_binding, size32(vertex_attributes), vertex_attributes.data());

    std::vector<Vertex> vertices;
    for (int i = 0; i < 3; ++i) {
        const Vertex vertex = {{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f, 7.0f}};
        vertices.emplace_back(vertex);
    }
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<Vertex>(*m_device, vertices);
    // Offset vertex buffer so that only first and second vertices can correctly be fetched
    VkDeviceSize vertex_buffer_offset = 1 * sizeof(Vertex);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 1, 0});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, CmdSetVertexInputEXT_CmdBindVertexBuffers2EXT) {
    TEST_DESCRIPTION("Simple graphics pipeline, use vkCmdSetVertexInputEXT and vkCmdBindVertexBuffers2EXT");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 2> uv;
        std::array<float, 3> normal;
    };

    char const *vsSource = R"glsl(
        #version 450

        layout(location=0) in vec3 pos;
        layout(location=1) in vec2 uv;
        layout(location=2) in vec3 normal;

        void main() {
            gl_Position = vec4(pos + uv.xyx + normal, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.gp_ci_.pVertexInputState = nullptr;
    // "Array of structs" style vertices
    VkVertexInputBindingDescription2EXT input_binding = vku::InitStructHelper();
    input_binding.binding = 0;
    input_binding.stride = sizeof(Vertex);
    input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    input_binding.divisor = 1;

    std::array<VkVertexInputAttributeDescription2EXT, 3> vertex_attributes = {};
    // Position
    vertex_attributes[0] = vku::InitStructHelper();
    vertex_attributes[0].location = 0;
    vertex_attributes[0].binding = 0;
    vertex_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[0].offset = 0;
    // UV
    vertex_attributes[1] = vku::InitStructHelper();
    vertex_attributes[1].location = 1;
    vertex_attributes[1].binding = 0;
    vertex_attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attributes[1].offset = 3 * sizeof(float);
    // Normal
    vertex_attributes[2] = vku::InitStructHelper();
    vertex_attributes[2].location = 2;
    vertex_attributes[2].binding = 0;
    vertex_attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[2].offset = (3 + 2) * sizeof(float);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &input_binding, size32(vertex_attributes), vertex_attributes.data());

    std::vector<Vertex> vertices;
    for (int i = 0; i < 3; ++i) {
        const Vertex vertex = {{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f, 7.0f}};
        vertices.emplace_back(vertex);
    }
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<Vertex>(*m_device, vertices);
    // Offset vertex buffer so that only first and second vertices can correctly be fetched
    const VkDeviceSize vertex_buffer_offset = 1 * sizeof(Vertex);
    const VkDeviceSize vertex_buffer_size = 2 * sizeof(Vertex);
    const VkDeviceSize vertex_stride = sizeof(Vertex);
    vk::CmdBindVertexBuffers2EXT(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset,
                                 &vertex_buffer_size, &vertex_stride);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 1, 0});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
