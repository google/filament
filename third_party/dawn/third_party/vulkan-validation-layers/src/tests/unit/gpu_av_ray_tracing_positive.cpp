/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_helper.h"
#include "../framework/gpu_av_helper.h"

class PositiveGpuAVRayTracing : public GpuAVRayTracingTest {};

TEST_F(PositiveGpuAVRayTracing, BasicTraceRays) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
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

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
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

TEST_F(PositiveGpuAVRayTracing, BasicTraceRaysMultipleStages) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray");

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

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
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

TEST_F(PositiveGpuAVRayTracing, DynamicTminTmax) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray, with dynamic t_min and t_max");

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

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            float t_min;
            float t_max;
        } trace_rays_params;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            float t_min;
            float t_max;
        } trace_rays_params;

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            float t_min;
            float t_max;
        } trace_rays_params;

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
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    auto uniform_buffer_ptr = static_cast<float*>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 0.1f;   // t_min
    uniform_buffer_ptr[1] = 42.0f;  // t_max
    uniform_buffer.Memory().Unmap();

    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer.handle(), 0, 4096, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
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

TEST_F(PositiveGpuAVRayTracing, BasicTraceRaysDynamicRayFlags) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray, with dynamic ray flags mask set to gl_RayFlagsCullBackFacingTrianglesEXT");

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

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            uint ray_flags;
        } trace_rays_params;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
        uint ray_flags;
        } trace_rays_params;

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            uint ray_flags;
        } trace_rays_params;

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
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    auto uniform_buffer_ptr = static_cast<uint32_t*>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 16;  // gl_RayFlagsCullBackFacingTrianglesEXT
    uniform_buffer.Memory().Unmap();

    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer.handle(), 0, 4096, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

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

TEST_F(PositiveGpuAVRayTracing, DynamicRayFlagsSkipTriangle) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray, with dynamic ray flags mask set to gl_RayFlagsSkipTrianglesEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTraversalPrimitiveCulling);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_ray_flags_primitive_culling : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            uint ray_flags;
        } trace_rays_params;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_ray_flags_primitive_culling : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            uint ray_flags;
        } trace_rays_params;

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_ray_flags_primitive_culling : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Uniforms {
            uint ray_flags;
        } trace_rays_params;

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
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    auto uniform_buffer_ptr = static_cast<uint32_t*>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = 0x100;  // gl_RayFlagsSkipTrianglesEXT, or RayFlagsSkipTrianglesKHRMask in SPIR-V
    uniform_buffer.Memory().Unmap();

    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer.handle(), 0, 4096, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

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

TEST_F(PositiveGpuAVRayTracing, BasicTraceRaysMultiEntryPoint) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders are in one shader with corresponding entry "
        "points) and acceleration structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Compile with
    // dxc -spirv -T lib_6_4 -fspv-target-env=vulkan1.2 -Fo out.spv in.hlsl
    /*
    struct ColorPayload {
        uint index;
    };

    [shader("miss")] void mainMiss(inout ColorPayload payload) { payload.index = 0; }

    [shader("closesthit")] void mainClosestHit(inout ColorPayload payload, BuiltInTriangleIntersectionAttributes attr) {
        payload.index = 1;
    }

    RWStructuredBuffer<float> OutBuffer : register(u0);  // StorageBuffer (set 0, binding 0)
    Texture2D MyTextures[] : register(t1);               // Texture array (set 0, binding 1)
    RaytracingAccelerationStructure _tlas;               // (set 0, binding 2)

    [shader("raygeneration")] void mainRaygen() {
        float2 uv = float2(0.5, 0.5);

        RayDesc ray;
        ray.Origin = ray.Origin;
        ray.TMin = 0;
        ray.Direction = float3(0.5, 0.5, 0.5);
        ray.TMax = 1e6;

        ColorPayload payload;
        TraceRay(_tlas, -1, RAY_FLAG_NONE, 0, 0, 0, ray, payload);

        // the index will either be 0 or 1, this can be adjusted, but the main thing is to use this to trigger (or not trigger)
        // descriptor indexing OOB
        Texture2D tex = MyTextures[NonUniformResourceIndex(payload.index)];
        float4 val = tex[uv];
        OutBuffer[0] = val.x;
    }
    */
    const char* shader_source = R"(
               OpCapability RayTracingKHR
               OpCapability RuntimeDescriptorArray
               OpCapability ShaderNonUniform
               OpCapability SampledImageArrayNonUniformIndexing
               OpExtension "SPV_KHR_ray_tracing"
               OpExtension "SPV_EXT_descriptor_indexing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MissNV %mainMiss "mainMiss" %payload
               OpEntryPoint ClosestHitNV %mainClosestHit "mainClosestHit" %payload_0
               OpEntryPoint RayGenerationNV %mainRaygen "mainRaygen" %OutBuffer %MyTextures %_tlas %payload_1
               OpSource HLSL 640
               OpName %type_RWStructuredBuffer_float "type.RWStructuredBuffer.float"
               OpName %OutBuffer "OutBuffer"
               OpName %type_2d_image "type.2d.image"
               OpName %MyTextures "MyTextures"
               OpName %accelerationStructureNV "accelerationStructureNV"
               OpName %_tlas "_tlas"
               OpName %ColorPayload "ColorPayload"
               OpMemberName %ColorPayload 0 "index"
               OpName %payload "payload"
               OpName %payload_0 "payload"
               OpName %payload_1 "payload"
               OpName %mainMiss "mainMiss"
               OpName %mainClosestHit "mainClosestHit"
               OpName %mainRaygen "mainRaygen"
               OpDecorate %payload_1 Location 0
               OpDecorate %OutBuffer DescriptorSet 0
               OpDecorate %OutBuffer Binding 0
               OpDecorate %MyTextures DescriptorSet 0
               OpDecorate %MyTextures Binding 1
               OpDecorate %_tlas DescriptorSet 0
               OpDecorate %_tlas Binding 2
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_float 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_float Block
               OpDecorate %15 NonUniform
               OpDecorate %16 NonUniform
               OpDecorate %17 NonUniform
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
      %float = OpTypeFloat 32
  %float_0_5 = OpConstant %float 0.5
    %float_0 = OpConstant %float 0
    %v3float = OpTypeVector %float 3
         %27 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%float_1000000 = OpConstant %float 1000000
%uint_4294967295 = OpConstant %uint 4294967295
%_runtimearr_float = OpTypeRuntimeArray %float
%type_RWStructuredBuffer_float = OpTypeStruct %_runtimearr_float
%_ptr_StorageBuffer_type_RWStructuredBuffer_float = OpTypePointer StorageBuffer %type_RWStructuredBuffer_float
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_runtimearr_type_2d_image = OpTypeRuntimeArray %type_2d_image
%_ptr_UniformConstant__runtimearr_type_2d_image = OpTypePointer UniformConstant %_runtimearr_type_2d_image
%accelerationStructureNV = OpTypeAccelerationStructureKHR
%_ptr_UniformConstant_accelerationStructureNV = OpTypePointer UniformConstant %accelerationStructureNV
%ColorPayload = OpTypeStruct %uint
%_ptr_IncomingRayPayloadNV_ColorPayload = OpTypePointer IncomingRayPayloadNV %ColorPayload
%_ptr_RayPayloadNV_ColorPayload = OpTypePointer RayPayloadNV %ColorPayload
       %void = OpTypeVoid
         %37 = OpTypeFunction %void
    %v4float = OpTypeVector %float 4
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
     %v2uint = OpTypeVector %uint 2
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
  %OutBuffer = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_float StorageBuffer
 %MyTextures = OpVariable %_ptr_UniformConstant__runtimearr_type_2d_image UniformConstant
      %_tlas = OpVariable %_ptr_UniformConstant_accelerationStructureNV UniformConstant
    %payload = OpVariable %_ptr_IncomingRayPayloadNV_ColorPayload IncomingRayPayloadNV
  %payload_0 = OpVariable %_ptr_IncomingRayPayloadNV_ColorPayload IncomingRayPayloadNV
  %payload_1 = OpVariable %_ptr_RayPayloadNV_ColorPayload RayPayloadNV
         %42 = OpUndef %v3float
         %43 = OpUndef %uint
         %44 = OpConstantComposite %ColorPayload %uint_0
         %45 = OpConstantComposite %ColorPayload %uint_1
         %46 = OpConstantComposite %v2uint %uint_0 %uint_0
   %mainMiss = OpFunction %void None %37
         %47 = OpLabel
               OpStore %payload %44
               OpReturn
               OpFunctionEnd
%mainClosestHit = OpFunction %void None %37
         %48 = OpLabel
               OpStore %payload_0 %45
               OpReturn
               OpFunctionEnd
 %mainRaygen = OpFunction %void None %37
         %49 = OpLabel
         %50 = OpCompositeConstruct %ColorPayload %43
               OpStore %payload_1 %50
         %51 = OpLoad %accelerationStructureNV %_tlas
               OpTraceRayKHR %51 %uint_4294967295 %uint_0 %uint_0 %uint_0 %uint_0 %42 %float_0 %27 %float_1000000 %payload_1
         %52 = OpLoad %ColorPayload %payload_1
         %53 = OpCompositeExtract %uint %52 0
         %15 = OpCopyObject %uint %53
         %16 = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTextures %15
         %17 = OpLoad %type_2d_image %16
         %54 = OpImageFetch %v4float %17 %46 Lod %uint_0
         %55 = OpCompositeExtract %float %54 0
         %56 = OpAccessChain %_ptr_StorageBuffer_float %OutBuffer %int_0 %uint_0
               OpStore %56 %55
               OpReturn
               OpFunctionEnd
      )";

    pipeline.AddSpirvRayGenShader(shader_source, "mainRaygen");
    pipeline.AddSpirvMissShader(shader_source, "mainMiss");
    pipeline.AddSpirvClosestHitShader(shader_source, "mainClosestHit");

    // Descriptor set
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, 2);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 2);
    pipeline.CreateDescriptorSet();

    // Buffer binding
    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(0, buffer.handle(), 0, 4096, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    // Texture array binding
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipeline.GetDescriptorSet().WriteDescriptorImageInfo(1, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    pipeline.GetDescriptorSet().WriteDescriptorImageInfo(1, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    // TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(2, 1, &tlas.GetDstAS()->handle());

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

TEST_F(PositiveGpuAVRayTracing, BasicTraceRaysDeferredBuild) {
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

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
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
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

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
