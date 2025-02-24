/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/shader_helper.h"
#include "../framework/gpu_av_helper.h"

class NegativeGpuAVRayTracing : public GpuAVRayTracingTest {};

TEST_F(NegativeGpuAVRayTracing, CmdTraceRaysIndirect) {
    TEST_DESCRIPTION("Test debug printf in raygen shader.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect);
    RETURN_IF_SKIP(InitGpuAvFramework());
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }

    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
          traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    // Create and fill buffers storing indirect data (ray query dimensions)
    vkt::Buffer trace_rays_big_width(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkTraceRaysIndirectCommandKHR trace_rays_dim{rt_pipeline_props.maxRayDispatchInvocationCount + 1, 1, 1};

    uint8_t *ray_query_dimensions_buffer_1_ptr = (uint8_t *)trace_rays_big_width.Memory().Map();
    std::memcpy(ray_query_dimensions_buffer_1_ptr, &trace_rays_dim, sizeof(trace_rays_dim));
    trace_rays_big_width.Memory().Unmap();

    trace_rays_dim = {1, rt_pipeline_props.maxRayDispatchInvocationCount + 1, 1};

    vkt::Buffer trace_rays_big_height(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    uint8_t *ray_query_dimensions_buffer_2_ptr = (uint8_t *)trace_rays_big_height.Memory().Map();
    std::memcpy(ray_query_dimensions_buffer_2_ptr, &trace_rays_dim, sizeof(trace_rays_dim));
    trace_rays_big_height.Memory().Unmap();

    trace_rays_dim = {1, 1, rt_pipeline_props.maxRayDispatchInvocationCount + 1};

    vkt::Buffer trace_ray_big_depth(*m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                    vkt::device_address);

    uint8_t *ray_query_dimensions_buffer_3_ptr = (uint8_t *)trace_ray_big_depth.Memory().Map();
    std::memcpy(ray_query_dimensions_buffer_3_ptr, &trace_rays_dim, sizeof(trace_rays_dim));
    trace_ray_big_depth.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());

    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

    if (uint64_t(PhysicalDeviceProps().limits.maxComputeWorkGroupCount[0]) *
            uint64_t(PhysicalDeviceProps().limits.maxComputeWorkGroupSize[0]) <
        uint64_t(rt_pipeline_props.maxRayDispatchInvocationCount + 1)) {
        m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-width-03638");
    }
    m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-width-03641");
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, trace_rays_big_width.Address());

    if (uint64_t(PhysicalDeviceProps().limits.maxComputeWorkGroupCount[1]) *
            uint64_t(PhysicalDeviceProps().limits.maxComputeWorkGroupSize[1]) <
        uint64_t(rt_pipeline_props.maxRayDispatchInvocationCount + 1)) {
        m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-height-03639");
    }
    m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-width-03641");
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, trace_rays_big_height.Address());

    if (uint64_t(PhysicalDeviceProps().limits.maxComputeWorkGroupCount[2]) *
            uint64_t(PhysicalDeviceProps().limits.maxComputeWorkGroupSize[2]) <
        uint64_t(rt_pipeline_props.maxRayDispatchInvocationCount + 1)) {
        m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-depth-03640");
    }
    m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-width-03641");
    vk::CmdTraceRaysIndirectKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                                &trace_rays_sbt.callable_sbt, trace_ray_big_depth.Address());

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8545
TEST_F(NegativeGpuAVRayTracing, DISABLED_BasicTraceRaysDeferredBuild) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders, and deferred build) and acceleration "
        "structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) readonly buffer RayTracingParams {
            vec4 nothing;
            float Tmin;
            float Tmax;
        };

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform uniform_buffer {
            RayTracingParams rt_params;
        };

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), rt_params.Tmin, vec3(0,0,1), rt_params.Tmax, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char *miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char *closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    // Create TLAS
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Create uniform_buffer
    vkt::Buffer rt_params_buffer(*m_device, 4 * sizeof(float), 0, vkt::device_address);  // missing space for Tmin and Tmax
    vkt::Buffer uniform_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    data[0] = rt_params_buffer.Address();
    uniform_buffer.Memory().Unmap();
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Add one to use the descriptor slot GPU-AV tried to reserve
    const uint32_t max_bound_desc_sets = m_device->Physical().limits_.maxBoundDescriptorSets + 1;

    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> desc_set_layouts(max_bound_desc_sets);
        for (uint32_t i = 0; i < max_bound_desc_sets; i++) {
            desc_set_layouts[i] = &pipeline.GetDescriptorSet().layout_;
        }
        vkt::PipelineLayout bad_pipe_layout(*m_device, desc_set_layouts);
        m_errorMonitor->VerifyFound();
    }

    // Then use the maximum allowed number of sets
    std::vector<const vkt::DescriptorSetLayout *> des_set_layouts(max_bound_desc_sets - 1);
    for (uint32_t i = 0; i < max_bound_desc_sets - 1; i++) {
        des_set_layouts[i] = &pipeline.GetDescriptorSet().layout_;
    }
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();

    pipeline.GetPipelineLayout().init(*m_device, pipe_layout_ci, des_set_layouts);

    // Deferred pipeline build
    RETURN_IF_SKIP(pipeline.DeferBuild());
    RETURN_IF_SKIP(pipeline.Build());

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
