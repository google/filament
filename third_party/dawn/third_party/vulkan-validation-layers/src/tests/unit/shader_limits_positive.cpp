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

class PositiveShaderLimits : public VkLayerTest {};

TEST_F(PositiveShaderLimits, MaxSampleMaskWords) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Valid input of sample mask
    char const *fs_source = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main(){
           int y = gl_SampleMaskIn[0];
           uFragColor = vec4(0,1,0,1) * y;
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto validPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, validPipeline, kErrorBit);
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    // More background: When workgroupMemoryExplicitLayout is enabled and there are 2 or more structs, the
    // maxComputeSharedMemorySize is the MAX of the structs since they share the same WorkGroup memory. Test makes sure validation
    // is not doing an ADD and correctly doing a MAX operation in this case.

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);
    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_vec4 = max_shared_memory_size / 16;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        // Both structs by themselves are 16 bytes less than the max
        shared X {
            vec4 x1[)glsl";
    csSource << (max_shared_vec4 - 1);
    csSource << R"glsl(];
            vec4 x2;
        };

        void main() {
            x2.x = 0.0f; // prevent dead-code elimination
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryBooleanAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize using Booleans");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    // "Boolean values considered as 32-bit integer values for the purpose of this calculation."
    const uint32_t max_shared_bools = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared bool a[)glsl";
    csSource << (max_shared_bools);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, MeshSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate mesh shader shared memory is valid at the exact maxMeshSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        shared int a[)glsl";
    mesh_source << (max_shared_ints);
    mesh_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderLimits, TaskSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate Task shader shared memory is valid at the exact maxTaskSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream task_source;
    task_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        shared int a[)glsl";
    task_source << (max_shared_ints);
    task_source << R"glsl(];
        void main(){
            EmitMeshTasksEXT(32u, 1u, 1u);
        }
    )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj mesh(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderLimits, MaxFragmentDualSrcAttachments) {
    TEST_DESCRIPTION("Test maxFragmentDualSrcAttachments when blend is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::independentBlend);
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    RETURN_IF_SKIP(Init());

    const uint32_t count = m_device->Physical().limits_.maxFragmentDualSrcAttachments + 1;
    if (count != 2) {
        GTEST_SKIP() << "Test is designed for a maxFragmentDualSrcAttachments of 1";
    }
    InitRenderTarget(count);

    const char *fs_src = R"glsl(
        #version 460
        layout(location = 0) out vec4 c0;
        layout(location = 1) out vec4 c1;
        void main() {
            c0 = vec4(0.0f);
            c1 = vec4(0.0f);
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState color_blend[2] = {};
    color_blend[0] = DefaultColorBlendAttachmentState();
    color_blend[1] = DefaultColorBlendAttachmentState();
    color_blend[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!
    color_blend[0].blendEnable = false;                               // but ignored

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = color_blend;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveShaderLimits, MaxFragmentDualSrcAttachmentsDynamicEnabled) {
    TEST_DESCRIPTION("Test maxFragmentDualSrcAttachments when blend is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::independentBlend);
    RETURN_IF_SKIP(Init());

    const uint32_t count = m_device->Physical().limits_.maxFragmentDualSrcAttachments + 1;
    if (count != 2) {
        GTEST_SKIP() << "Test is designed for a maxFragmentDualSrcAttachments of 1";
    }
    InitRenderTarget(count);

    const char *fs_src = R"glsl(
        #version 460
        layout(location = 0) out vec4 c0;
        layout(location = 1) out vec4 c1;
        void main() {
            c0 = vec4(0.0f);
            c1 = vec4(0.0f);
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState color_blend[2] = {};
    color_blend[0] = DefaultColorBlendAttachmentState();
    color_blend[1] = DefaultColorBlendAttachmentState();
    color_blend[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = color_blend;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    VkBool32 color_blend_enabled[2] = {VK_FALSE, VK_FALSE};  // disable any blending
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0, 2, color_blend_enabled);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveShaderLimits, MaxFragmentOutputAttachments) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxFragmentOutputAttachments != 4) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is not 4";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 c0;
        layout(location=1) out vec4 c1;
        layout(location=2) out vec4 c2;
        layout(location=3) out vec4 c3; // at limit
        void main(){
           c0 = vec4(1.0);
           c1 = vec4(1.0);
           c2 = vec4(1.0);
           c3 = vec4(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderLimits, MaxFragmentOutputAttachmentsArray) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxFragmentOutputAttachments != 4) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is not 4";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 c[4];
        void main(){
           c[3] = vec4(1.0);
           c[0] = vec4(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}