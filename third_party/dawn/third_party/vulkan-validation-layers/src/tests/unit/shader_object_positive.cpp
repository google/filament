/*
 * Copyright (c) 2023-2025 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/shader_object_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_templates.h"

void ShaderObjectTest::InitBasicShaderObject() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());
}

void ShaderObjectTest::InitBasicMeshShaderObject(APIVersion target_api_version) {
    SetTargetApiVersion(target_api_version);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(Init());
}

class PositiveShaderObject : public ShaderObjectTest {};

void ShaderObjectTest::CreateMinimalShaders() {
    std::vector<uint32_t> vert_spirv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = vert_spirv.size() * sizeof(uint32_t);
    create_info.pCode = vert_spirv.data();
    create_info.pName = "main";
    m_vert_shader.init(*m_device, create_info);

    std::vector<uint32_t> frag_spirv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info.codeSize = frag_spirv.size() * sizeof(uint32_t);
    create_info.pCode = frag_spirv.data();
    m_frag_shader.init(*m_device, create_info);
}

TEST_F(PositiveShaderObject, CreateAndDestroyShaderObject) {
    TEST_DESCRIPTION("Create and destroy shader object.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    vk::DestroyShaderEXT(m_device->handle(), shader, nullptr);
}

TEST_F(PositiveShaderObject, BindShaderObject) {
    TEST_DESCRIPTION("Use graphics shaders with unsupported command pool.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    const vkt::Shader vertShader(*m_device, stage, GLSLToSPV(stage, kVertexMinimalGlsl));

    m_command_buffer.Begin();
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &vertShader.handle());
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawWithVertAndFragShaderObjects) {
    TEST_DESCRIPTION("Draw with only vertex and fragment shader objects bound.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawWithVertAndFragBinaryShaderObjects) {
    TEST_DESCRIPTION("Draw with binary vertex and fragment shader objects bound.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GetShaderBinaryDataEXT not implemented";
    }

    InitDynamicRenderTarget();
    CreateMinimalShaders();

    size_t vertDataSize;
    vk::GetShaderBinaryDataEXT(*m_device, m_vert_shader.handle(), &vertDataSize, nullptr);
    std::vector<uint8_t> vertData(vertDataSize);
    vk::GetShaderBinaryDataEXT(*m_device, m_vert_shader.handle(), &vertDataSize, vertData.data());

    size_t fragDataSize;
    vk::GetShaderBinaryDataEXT(*m_device, m_frag_shader.handle(), &fragDataSize, nullptr);
    std::vector<uint8_t> fragData(fragDataSize);
    vk::GetShaderBinaryDataEXT(*m_device, m_frag_shader.handle(), &fragDataSize, fragData.data());

    vkt::Shader binaryVertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vertData);
    vkt::Shader binaryFragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, fragData);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(binaryVertShader, binaryFragShader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, LinkedVertexAndFragmentShaders) {
    TEST_DESCRIPTION("Create linked vertex and fragment shaders.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    createInfos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, LinkedGraphicsShaders) {
    TEST_DESCRIPTION("Create linked vertex and fragment shaders.");
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto tesc_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    const auto tese_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    const auto geom_spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[5];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    createInfos[1] =
        ShaderCreateInfoLink(tesc_spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    createInfos[2] = ShaderCreateInfoLink(tese_spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT);
    createInfos[3] = ShaderCreateInfoLink(geom_spv, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    createInfos[4] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shaders[5];
    vk::CreateShadersEXT(m_device->handle(), 5u, createInfos, nullptr, shaders);

    for (uint32_t i = 0; i < 5; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, MissingCmdSetDepthBiasEnable) {
    TEST_DESCRIPTION("Draw with shaders without setting depth bias enable.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE});
    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, VertFragShaderDraw) {
    TEST_DESCRIPTION("Test drawing with a vertex and fragment shader");

    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    VkShaderStageFlagBits unusedShaderStages[] = {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT};

    const vkt::Shader vertShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], vert_src));
    const vkt::Shader fragShader(*m_device, shaderStages[1], GLSLToSPV(shaderStages[1], frag_src));

    VkShaderEXT shaders[] = {vertShader.handle(), fragShader.handle()};

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, kHostVisibleMemProps, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 2u, shaderStages, shaders);
    for (const auto &unusedShader : unusedShaderStages) {
        VkShaderEXT null_shader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &unusedShader, &null_shader);
    }
    SetDefaultDynamicStatesExclude();
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    vk::CmdEndRenderingKHR(m_command_buffer.handle());

    {
        VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);
    }

    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.imageOffset.x = static_cast<int32_t>(m_width / 2) + 1;
    copyRegion.imageOffset.y = static_cast<int32_t>(m_height / 2) + 1;
    copyRegion.imageExtent = {1, 1, 1};

    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1u, &copyRegion);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveShaderObject, DrawWithAllGraphicsShaderStagesUsed) {
    TEST_DESCRIPTION("Test drawing using all graphics shader");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (!features.tessellationShader || !features.geometryShader) {
        GTEST_SKIP() << "Required shaders not supported.";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char tesc_src[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char tese_src[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<uint32_t> spv[5];
    spv[0] = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    spv[1] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tesc_src);
    spv[2] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tese_src);
    spv[3] = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);
    spv[4] = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    const vkt::Shader vertShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], vert_src));
    const vkt::Shader tescShader(*m_device, shaderStages[1], GLSLToSPV(shaderStages[1], tesc_src));
    const vkt::Shader teseShader(*m_device, shaderStages[2], GLSLToSPV(shaderStages[2], tese_src));
    const vkt::Shader geomShader(*m_device, shaderStages[3], GLSLToSPV(shaderStages[3], geom_src));
    const vkt::Shader fragShader(*m_device, shaderStages[4], GLSLToSPV(shaderStages[4], frag_src));

    VkShaderEXT shaders[5] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), geomShader.handle(),
                              fragShader.handle()};

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, shaderStages, shaders);
    SetDefaultDynamicStatesExclude({}, true);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    vk::CmdEndRenderingKHR(m_command_buffer.handle());

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveShaderObject, ComputeShader) {
    TEST_DESCRIPTION("Test dispatching with compute shader");

    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char comp_src[] = R"glsl(
        #version 450
        layout(local_size_x=16, local_size_x=1, local_size_x=1) in;
        layout(binding = 0) buffer Output {
            uint values[16];
        } buffer_out;

        void main() {
            buffer_out.values[gl_LocalInvocationID.x] = gl_LocalInvocationID.x;
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_COMPUTE_BIT};

    vkt::Buffer storageBuffer(*m_device, 16u * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, storageBuffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    VkDescriptorSetLayout descriptorSetLayout = descriptor_set.layout_.handle();

    const vkt::Shader compShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], comp_src), &descriptorSetLayout);

    m_command_buffer.Begin();

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, shaderStages, &compShader.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveShaderObject, TaskMeshShadersDraw) {
    TEST_DESCRIPTION("Test drawing using task and mesh shaders");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);

    static const char task_src[] = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    static const char mesh_src[] = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3, 1);
            gl_MeshVerticesEXT[0].gl_Position = vec4(-1.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[1].gl_Position = vec4( 3.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[2].gl_Position = vec4(-1.0,  3.0, 0.0f, 1.0f);
            gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    const vkt::Shader taskShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], task_src, SPV_ENV_VULKAN_1_3));
    const vkt::Shader meshShader(*m_device, shaderStages[1], GLSLToSPV(shaderStages[1], mesh_src, SPV_ENV_VULKAN_1_3));
    const vkt::Shader fragShader(*m_device, shaderStages[2], GLSLToSPV(shaderStages[2], frag_src));

    VkShaderEXT shaders[3] = {taskShader.handle(), meshShader.handle(), fragShader.handle()};

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &begin_rendering_info);
    std::vector<VkShaderStageFlagBits> nullStages = {VK_SHADER_STAGE_VERTEX_BIT};
    if (features.tessellationShader) {
        nullStages.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        nullStages.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }
    if (features.geometryShader) {
        nullStages.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    for (const auto stage : nullStages) {
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &nullShader);
    }

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 3u, shaderStages, shaders);
    SetDefaultDynamicStatesExclude();
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdEndRenderingKHR(m_command_buffer.handle());

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveShaderObject, FailCreateShaders) {
    TEST_DESCRIPTION("Test failing to create shaders");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD because shader needs to fail";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char tesc_src[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char tese_src[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    constexpr uint32_t stages_count = 5;
    constexpr uint32_t shaders_count = 20;
    constexpr uint32_t fail_index = 15;

    VkShaderStageFlagBits shaderStages[stages_count] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                        VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<uint32_t> spv[stages_count];
    spv[0] = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    spv[1] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tesc_src);
    spv[2] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tese_src);
    spv[3] = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);
    spv[4] = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkShaderEXT shaders[shaders_count];

    VkShaderCreateInfoEXT createInfos[shaders_count];
    for (uint32_t i = 0; i < shaders_count; ++i) {
        createInfos[i] = vku::InitStructHelper();
        createInfos[i].stage = shaderStages[i % stages_count];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i % stages_count].size() * sizeof(uint32_t);
        createInfos[i].pCode = spv[i % stages_count].data();
        createInfos[i].pName = "main";
    }

    // Binary code must be aligned to 16 bytes
    std::vector<uint8_t> garbage(createInfos[fail_index].codeSize + 16);
    auto pCode = reinterpret_cast<std::uintptr_t>(garbage.data());
    while (pCode % 16 != 0) {
        pCode += 1;
    }
    std::memcpy(reinterpret_cast<void *>(pCode), createInfos[fail_index].pCode, createInfos[fail_index].codeSize);
    createInfos[fail_index].codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    createInfos[fail_index].pCode = reinterpret_cast<const void *>(pCode);

    VkResult res = vk::CreateShadersEXT(m_device->handle(), 20u, createInfos, nullptr, shaders);
    ASSERT_EQ(res, VK_INCOMPATIBLE_SHADER_BINARY_EXT);

    for (uint32_t i = 0; i < shaders_count; ++i) {
        if (i < fail_index) {
            vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
        }
    }
}

TEST_F(PositiveShaderObject, DrawMinimalDynamicStates) {
    TEST_DESCRIPTION("Draw with only required dynamic states set.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkViewport viewport = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {m_width, m_height}};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 1u, &viewport);
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 1u, &scissor);
    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetStencilTestEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetPolygonModeEXT(m_command_buffer.handle(), VK_POLYGON_MODE_FILL);
    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 1u;
    vk::CmdSetSampleMaskEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vk::CmdSetAlphaToCoverageEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetCullModeEXT(m_command_buffer.handle(), VK_CULL_MODE_NONE);
    vk::CmdSetDepthTestEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetDepthBoundsTestEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 0u, nullptr, 0u, nullptr);
    vk::CmdSetPrimitiveRestartEnableEXT(m_command_buffer.handle(), VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEnable);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(m_command_buffer.handle(), 0u, 1u, &colorWriteMask);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEquation);

    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawMinimalDynamicStatesRasterizationDisabled) {
    TEST_DESCRIPTION("Draw with only required dynamic states set.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkViewport viewport = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {m_width, m_height}};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 1u, &viewport);
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 1u, &scissor);
    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetStencilTestEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 0u, nullptr, 0u, nullptr);
    vk::CmdSetPrimitiveRestartEnableEXT(m_command_buffer.handle(), VK_FALSE);

    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, ShadersDescriptorSets) {
    TEST_DESCRIPTION("Draw with shaders using multiple descriptor sets.");

    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    OneOffDescriptorSet vert_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });
    OneOffDescriptorSet frag_descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });

    vkt::PipelineLayout pipeline_layout(*m_device, {&vert_descriptor_set.layout_, &frag_descriptor_set.layout_});

    static const char vert_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec2 uv;
        layout(set = 0, binding = 0) buffer Buffer {
            vec4 pos;
        } buf;
        void main() {
            uv = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
            gl_Position = vec4(buf.pos);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(set = 1, binding = 0) uniform sampler2D s;
        layout(location = 0) in vec2 uv;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(s, uv);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkDescriptorSetLayout descriptor_set_layouts[] = {vert_descriptor_set.layout_.handle(), frag_descriptor_set.layout_.handle()};

    const vkt::Shader vertShader(*m_device, ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 2, descriptor_set_layouts));
    const vkt::Shader fragShader(*m_device, ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 2, descriptor_set_layouts));

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 1, 1);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    frag_descriptor_set.WriteDescriptorImageInfo(0, view, sampler.handle());
    frag_descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1u, 1u,
                              &frag_descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DescriptorBuffer) {
    TEST_DESCRIPTION("use VK_EXT_descriptor_buffer and do a basic draw.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    const char frag_spv[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        layout(set=0, binding=0) uniform foo { vec4 x; } bar;
        void main(){
            uFragColor = bar.x;
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl),
                                 &set_layout.handle());

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_spv),
                                 &set_layout.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &buffer_binding_info);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, MultiplePushConstants) {
    TEST_DESCRIPTION("Draw with shaders using multiple push constants.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        layout (push_constant) uniform constants {
            int pos;
        } pushConst;
        void main() {
            gl_Position = vec4(pushConst.pos);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout (push_constant) uniform constants {
            layout(offset = 4) float c;
        } pushConst;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(pushConst.c);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkPushConstantRange push_constant_ranges[2];
    push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_ranges[0].offset = 0u;
    push_constant_ranges[0].size = sizeof(int);
    push_constant_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_ranges[1].offset = sizeof(int);
    push_constant_ranges[1].size = sizeof(float);
    vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_ranges[0], push_constant_ranges[1]});

    const vkt::Shader vertShader(*m_device,
                                 ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr, 2, push_constant_ranges));
    const vkt::Shader fragShader(*m_device,
                                 ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 0, nullptr, 2, push_constant_ranges));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    int pos = 1;
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(int), &pos);
    float color = 1.0f;
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(int),
                         sizeof(float), &color);

    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, MultipleSpecializationConstants) {
    TEST_DESCRIPTION("Draw with shaders using multiple specialization constants.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int pos = 1;
        void main() {
            gl_Position = vec4(pos);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout (constant_id = 1) const float c = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(c);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkSpecializationMapEntry map_entries[2];
    map_entries[0].constantID = 0u;
    map_entries[0].offset = 0u;
    map_entries[0].size = sizeof(int);
    map_entries[1].constantID = 1u;
    map_entries[1].offset = sizeof(int);
    map_entries[1].size = sizeof(float);

    struct Data {
        int pos = 0u;
        float color = 1.0f;
    } data;

    VkSpecializationInfo specialization_info;
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = map_entries;
    specialization_info.dataSize = sizeof(int) + sizeof(float);
    specialization_info.pData = &data;

    const vkt::Shader vertShader(
        *m_device, ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr, 0, nullptr, &specialization_info));
    const vkt::Shader fragShader(
        *m_device, ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 0, nullptr, 0, nullptr, &specialization_info));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, IndirectDraw) {
    TEST_DESCRIPTION("Draw with all 5 shaders stages using indirect draw and seconary command buffers.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char tesc_src[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char tese_src[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src));
    const vkt::Shader tescShader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tesc_src));
    const vkt::Shader teseShader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tese_src));
    const vkt::Shader geomShader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src));

    vkt::Buffer indirect_buffer(*m_device, 32, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    SetDefaultDynamicStatesExclude({}, true);

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), geomShader.handle(),
                                   fragShader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);

    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0u, 1u, 0u);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawInSecondaryCommandBuffers) {
    TEST_DESCRIPTION("Draw in secondary command buffers.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const std::optional<uint32_t> graphics_queue_family_index = m_device->QueueFamily(VK_QUEUE_GRAPHICS_BIT);

    vkt::CommandPool command_pool(*m_device, graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    command_buffer.Begin();
    command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {m_vert_shader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, m_frag_shader.handle()};
    vk::CmdBindShadersEXT(command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStatesExclude({}, true, command_buffer.handle());
    vk::CmdDraw(command_buffer.handle(), 4, 1, 0, 0);
    command_buffer.EndRendering();
    command_buffer.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &command_buffer.handle());
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, OutputToMultipleAttachments) {
    TEST_DESCRIPTION("Draw with fragment shader writing to multiple attachments.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    InitDynamicRenderTarget();

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor1;
        layout(location = 1) out vec4 uFragColor2;
        void main(){
           uFragColor1 = vec4(0,1,0,1);
           uFragColor2 = vec4(1,0,1,0);
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src));

    vkt::Image img1(*m_device, m_width, m_height, 1, m_render_target_fmt,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img2(*m_device, m_width, m_height, 1, m_render_target_fmt,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::ImageView view1 = img1.CreateView();
    vkt::ImageView view2 = img2.CreateView();

    VkRenderingAttachmentInfo attachments[2];
    attachments[0] = vku::InitStructHelper();
    attachments[0].imageView = view1;
    attachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1] = vku::InitStructHelper();
    attachments[1].imageView = view2;
    attachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 2u;
    renderingInfo.pColorAttachments = attachments;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawWithNonBlendableFormat) {
    TEST_DESCRIPTION("Draw with shader objects to an attachment format that does not support blending.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_R32_UINT, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0 ||
        (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) != 0) {
        GTEST_SKIP() << "color attachment format not suitable.";
    }

    InitDynamicRenderTarget(VK_FORMAT_R32_UINT);
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    VkBool32 enabled = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0, 1, &enabled);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawInSecondaryCommandBuffersWithRenderPassContinue) {
    TEST_DESCRIPTION("Draw in secondary command buffers with render pass continue flag.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitBasicShaderObject());

    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const std::optional<uint32_t> graphics_queue_family_index = m_device->QueueFamily(VK_QUEUE_GRAPHICS_BIT);

    vkt::CommandPool command_pool(*m_device, graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferInheritanceRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &m_render_target_fmt;
    rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper(&rendering_info);
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &hinfo;
    command_buffer.Begin(&begin_info);
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {m_vert_shader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, m_frag_shader.handle()};
    vk::CmdBindShadersEXT(command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStatesExclude({}, true, command_buffer.handle());
    vk::CmdDraw(command_buffer.handle(), 4, 1, 0, 0);
    command_buffer.End();

    m_command_buffer.Begin();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = GetDynamicRenderTarget();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.BeginRendering(renderingInfo);

    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &command_buffer.handle());
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawRebindingShaders) {
    TEST_DESCRIPTION("Draw after rebinding only some shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    InitDynamicRenderTarget();

    const VkShaderStageFlagBits vertStage = VK_SHADER_STAGE_VERTEX_BIT;
    const VkShaderStageFlagBits tescStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    const VkShaderStageFlagBits teseStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    const VkShaderStageFlagBits geomStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    const VkShaderStageFlagBits fragStage = VK_SHADER_STAGE_FRAGMENT_BIT;

    const vkt::Shader vertShader(*m_device, vertStage, GLSLToSPV(vertStage, kVertexMinimalGlsl));
    const vkt::Shader tescShader(*m_device, tescStage, GLSLToSPV(tescStage, kTessellationControlMinimalGlsl));
    const vkt::Shader teseShader(*m_device, teseStage, GLSLToSPV(teseStage, kTessellationEvalMinimalGlsl));
    const vkt::Shader geomShader(*m_device, geomStage, GLSLToSPV(geomStage, kGeometryMinimalGlsl));
    const vkt::Shader fragShader(*m_device, fragStage, GLSLToSPV(fragStage, kFragmentMinimalGlsl));

    const VkShaderEXT nullShader = VK_NULL_HANDLE;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    SetDefaultDynamicStatesExclude({}, true);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &vertStage, &vertShader.handle());
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &tescStage, &nullShader);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &teseStage, &nullShader);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &geomStage, &nullShader);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &fragStage, &fragShader.handle());
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &geomStage, &geomShader.handle());
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &tescStage, &tescShader.handle());
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &teseStage, &teseShader.handle());
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &geomStage, &nullShader);
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &fragStage, &nullShader);
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawWithBinaryShaders) {
    TEST_DESCRIPTION("Draw using binary shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GetShaderBinaryDataEXT not implemented";
    }
    InitDynamicRenderTarget();

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<uint32_t> spv[5];
    spv[0] = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    spv[1] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    spv[2] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    spv[3] = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    spv[4] = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderEXT shaders[5];
    VkShaderEXT binaryShaders[5];
    for (uint32_t i = 0; i < 5u; ++i) {
        VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
        createInfo.stage = shaderStages[i];
        createInfo.nextStage = 0u;
        if (i < 4) {
            createInfo.nextStage = shaderStages[i + 1];
        }
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv[i].size() * sizeof(spv[i][0]);
        createInfo.pCode = spv[i].data();
        createInfo.pName = "main";

        vk::CreateShadersEXT(*m_device, 1u, &createInfo, nullptr, &shaders[i]);
        size_t dataSize;
        vk::GetShaderBinaryDataEXT(*m_device, shaders[i], &dataSize, nullptr);
        std::vector<uint8_t> data(dataSize);
        vk::GetShaderBinaryDataEXT(*m_device, shaders[i], &dataSize, data.data());

        createInfo.codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
        createInfo.codeSize = dataSize;
        createInfo.pCode = data.data();
        vk::CreateShadersEXT(*m_device, 1u, &createInfo, nullptr, &binaryShaders[i]);
    }

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({}, true);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, shaderStages, binaryShaders);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    for (uint32_t i = 0; i < 5; ++i) {
        vk::DestroyShaderEXT(*m_device, shaders[i], nullptr);
        vk::DestroyShaderEXT(*m_device, binaryShaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, NotSettingDepthBounds) {
    TEST_DESCRIPTION("Draw without setting depth bounds.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, VK_DYNAMIC_STATE_DEPTH_BOUNDS});
    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, CreateAndDrawLinkedAndUnlinkedShaders) {
    TEST_DESCRIPTION("Create and draw with some linked and some unlinked shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    const auto vert_spv = GLSLToSPV(stages[0], kVertexMinimalGlsl);
    const auto tesc_spv = GLSLToSPV(stages[1], kTessellationControlMinimalGlsl);
    const auto tese_spv = GLSLToSPV(stages[2], kTessellationEvalMinimalGlsl);
    const auto geom_spv = GLSLToSPV(stages[3], kGeometryMinimalGlsl);
    const auto frag_spv = GLSLToSPV(stages[4], kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[5];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, stages[0], stages[1]);
    createInfos[1] = ShaderCreateInfoLink(tesc_spv, stages[1], stages[2]);
    createInfos[2] = ShaderCreateInfoLink(tese_spv, stages[2], stages[3] | stages[4]);
    // unlinked shader
    createInfos[3] = ShaderCreateInfoLink(geom_spv, stages[3], stages[4]);
    createInfos[3].flags = 0;
    createInfos[4] = ShaderCreateInfoLink(frag_spv, stages[4]);
    createInfos[4].flags = 0;

    VkShaderEXT shaders[5];
    vk::CreateShadersEXT(*m_device, 3u, createInfos, nullptr, shaders);
    for (uint32_t i = 3u; i < 5u; ++i) {
        vk::CreateShadersEXT(*m_device, 1u, &createInfos[i], nullptr, &shaders[i]);
    }

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({}, true);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);

    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    for (uint32_t i = 0; i < 5u; ++i) {
        vk::DestroyShaderEXT(*m_device, shaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, IgnoredColorAttachmentCount) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6523/diffs");
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(renderingInfo);

    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DisabledColorBlend) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEquationEXT when color blend is disabled.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT});
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEnable);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DrawWithVertGeomFragShaderObjects) {
    TEST_DESCRIPTION("Draw with vertex, geometry and fragment shader objects bound.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 450

        void main(void) {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        layout(location = 0) out vec4 color;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 450

        layout(location = 0) in vec4 in_color;
        layout(location = 0) out vec4 out_color;

        void main(void) {
            out_color = in_color;
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src));
    const vkt::Shader geomShader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    VkShaderStageFlagBits geomStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &geomStage, &geomShader.handle());
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, DiscardRectangleModeEXT) {
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (!DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitDynamicRenderTarget();
    CreateMinimalShaders();
    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rectangle_properties);
    std::vector<VkRect2D> discard_rectangles(discard_rectangle_properties.maxDiscardRectangles);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetDiscardRectangleEXT(m_command_buffer.handle(), 0u, discard_rectangles.size(), discard_rectangles.data());
    vk::CmdSetDiscardRectangleModeEXT(m_command_buffer.handle(), VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT);
    m_command_buffer.BindVertFragShader(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveShaderObject, SetPatchControlPointsEXT) {
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader tescShader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl));
    const vkt::Shader teseShader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY});
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), VK_NULL_HANDLE,
                                   fragShader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    vk::CmdSetPatchControlPointsEXT(m_command_buffer.handle(), 3);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}