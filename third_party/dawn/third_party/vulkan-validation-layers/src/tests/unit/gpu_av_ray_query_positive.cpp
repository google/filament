/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/ray_tracing_objects.h"

void GpuAVRayQueryTest::InitGpuAVRayQuery() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVRayQuery : public GpuAVRayQueryTest {};

TEST_F(PositiveGpuAVRayQuery, ComputeBasic) {
    TEST_DESCRIPTION("Ray query in a compute shader");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, ComputeDynamicTminTmax) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set t_min and t_max");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float t_min;
          float t_max;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(100000),
              trace_rays_params.t_min, vec3(0,1,0), trace_rays_params.t_max);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Add uniform buffer binding
    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);

    pipeline.descriptor_set_->UpdateDescriptorSets();

    // Ray query with t_min dynamically set to 0
    {
        auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.Memory().Map());
        uniform_buffer_ptr[0] = 0.0f;   // t_min
        uniform_buffer_ptr[1] = 42.0f;  // t_max
        uniform_buffer.Memory().Unmap();
    }
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Ray query with both t_min and t_max dynamically set to 42
    {
        auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.Memory().Map());
        uniform_buffer_ptr[0] = 42.0f;  // t_min
        uniform_buffer_ptr[1] = 42.0f;  // t_max
        uniform_buffer.Memory().Unmap();
    }
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, ComputeDynamicRayFlags) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set ray flags");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          uint ray_query_flags;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, trace_rays_params.ray_query_flags, 0xff, vec3(0), 0.1, vec3(0,0,1), 42.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Add uniform buffer binding
    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);

    pipeline.descriptor_set_->UpdateDescriptorSets();

    // Ray query with t_min dynamically set to 0
    {
        auto uniform_buffer_ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
        uniform_buffer_ptr[0] = 4u | 16u;  // gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsCullBackFacingTrianglesEXT
        uniform_buffer.Memory().Unmap();
    }
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, ComputeDynamicRayFlagsSkipTriangles) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set ray flags containing gl_RayFlagsSkipTrianglesEXT");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTraversalPrimitiveCulling);
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          uint ray_query_flags;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, trace_rays_params.ray_query_flags, 0xff, vec3(0), 0.1, vec3(0,0,1), 42.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Add uniform buffer binding
    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);

    pipeline.descriptor_set_->UpdateDescriptorSets();

    {
        auto uniform_buffer_ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
        uniform_buffer_ptr[0] = 4u | 0x100;  // gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipTrianglesEXT
        uniform_buffer.Memory().Unmap();
    }
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, GraphicsBasic) {
    TEST_DESCRIPTION("Ray query in a vertex shader");
    RETURN_IF_SKIP(InitGpuAVRayQuery());
    InitRenderTarget();

    char const *vertex_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
            rayQueryProceedEXT(query);
            gl_Position = vec4(1);
        }
    )glsl";
    VkShaderObj vs(this, vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipeline(*this);
    pipeline.shader_stages_ = {vs.GetStageCreateInfo(), pipeline.fs_->GetStageCreateInfo()};
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    pipeline.CreateGraphicsPipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, RayTracingBasic) {
    TEST_DESCRIPTION("Ray query in a ray generation shader");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set ray gen shader

    const char *ray_gen = R"glsl(
    #version 460

    #extension GL_EXT_ray_query : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    void main() {
      rayQueryEXT query;
      rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
      rayQueryProceedEXT(query);
    }
)glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Build pipeline
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
