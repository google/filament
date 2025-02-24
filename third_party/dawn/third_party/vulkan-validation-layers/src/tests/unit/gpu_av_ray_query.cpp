/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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
#include "../../layers/gpuav/shaders/gpuav_shaders_constants.h"

class NegativeGpuAVRayQuery : public GpuAVRayQueryTest {};

TEST_F(NegativeGpuAVRayQuery, NegativeTmin) {
    TEST_DESCRIPTION("Ray query with a negative value for Ray TMin");
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
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0),
              trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = -2.0f;  // t_min
    uniform_buffer_ptr[1] = 42.0f;  // t_max
    uniform_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, TMaxLessThenTmin) {
    TEST_DESCRIPTION("Ray query with a Ray TMax less than Ray TMin");
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
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0),
              trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 9.9f;  // t_min
    uniform_buffer_ptr[1] = 9.8f;  // t_max
    uniform_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeRayFlagsBothSkip) {
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

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 0x100 | 0x200;  // SkipTrianglesKHR and SkipAABBsKHR
    uniform_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeRayFlagsOpaque) {
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

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 0x1 | 0x2;  // OpaqueKHR and NoOpaqueKHR
    uniform_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06891");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeRayOriginNaN) {
    TEST_DESCRIPTION("Ray query with a Ray Origin as a NaN");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float x;
          float y;
        } params;

        void main() {
            rayQueryEXT query;
            // fract(1.0 / 0.0) will produce a NaN
            float bad = fract(params.x / params.y);
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,bad,0), 1.0, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 1.0f;  // x
    uniform_buffer_ptr[1] = 0.0f;  // y
    uniform_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeRayOriginNonFinite) {
    TEST_DESCRIPTION("Ray query with a Ray Origin as a non finite value");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float x;
          float y;
        } params;

        void main() {
            rayQueryEXT query;
            // 1.0 / 0.0 will produce positive infinity
            float bad = 1.0 / params.x;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,bad,0), 1.0, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto uniform_buffer_ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 0.0f;  // t_min
    uniform_buffer_ptr[1] = 0.0f;  // t_max
    uniform_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, ComputeUseQueryUninit) {
    TEST_DESCRIPTION("rayQueryInitializeEXT is never called, make sure we don't hang with an uninit query object");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) buffer SSBO {
          float x;
          float y;
        } params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,1,0), params.x, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
            if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
                params.y = rayQueryGetIntersectionTEXT(query, true);
            }
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto buffer_ptr = static_cast<float *>(buffer.Memory().Map());
    buffer_ptr[0] = -4.0f;  // t_min
    buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, RayGenUseQueryUninit) {
    TEST_DESCRIPTION("rayQueryInitializeEXT is never called, make sure we don't hang with an uninit query object");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) buffer SSBO {
            float x;
            float y;
        } params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,1,0), params.x, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
            if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
                params.y = rayQueryGetIntersectionTEXT(query, true);
            }
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer ssbo(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto buffer_ptr = static_cast<float *>(ssbo.Memory().Map());
    buffer_ptr[0] = -16.0f;
    ssbo.Memory().Unmap();
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, ssbo.handle(), 0, 4096, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer.handle(), &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayQuery, FragmentUseQueryUninit) {
    TEST_DESCRIPTION("rayQueryInitializeEXT is never called, make sure we don't hang with an uninit query object");
    RETURN_IF_SKIP(InitGpuAVRayQuery());
    InitRenderTarget();

    char const *fragment_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(location=0) out vec4 color;
        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform SSBO {
            float x;
        } params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0,1,0), params.x, vec3(0,0,1), 100);
            rayQueryProceedEXT(query);
            float x = 0.0;
            if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
                x = rayQueryGetIntersectionTEXT(query, true);
            }
            color = vec4(x);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipeline(*this);
    pipeline.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipeline.CreateGraphicsPipeline();

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_->WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    auto buffer_ptr = static_cast<float *>(buffer.Memory().Map());
    buffer_ptr[0] = -4.0f;  // t_min
    buffer.Memory().Unmap();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349", gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}
