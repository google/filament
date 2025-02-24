/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "ray_tracing_helper_nv.h"

namespace nv {
namespace rt {

RayTracingPipelineHelper::RayTracingPipelineHelper(VkLayerTest &test) : layer_test_(test) { InitInfo(); }
RayTracingPipelineHelper::~RayTracingPipelineHelper() {
    VkDevice device = layer_test_.device();
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device, pipeline_cache_, nullptr);
        pipeline_cache_ = VK_NULL_HANDLE;
    }
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

void RayTracingPipelineHelper::InitShaderGroups() {
    {
        VkRayTracingShaderGroupCreateInfoNV group = vku::InitStructHelper();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group.generalShader = 0;
        group.closestHitShader = VK_SHADER_UNUSED_NV;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoNV group = vku::InitStructHelper();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group.generalShader = VK_SHADER_UNUSED_NV;
        group.closestHitShader = 1;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoNV group = vku::InitStructHelper();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group.generalShader = 2;
        group.closestHitShader = VK_SHADER_UNUSED_NV;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
}

void RayTracingPipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
        {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
    };
}

void RayTracingPipelineHelper::InitDescriptorSetInfoKHR() {
    dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
        {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
    };
}

void RayTracingPipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_ = vku::InitStructHelper();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void RayTracingPipelineHelper::InitShaderInfo() {  // DONE
    static const char rayGenShaderText[] = R"glsl(
        #version 460 core
        #extension GL_NV_ray_tracing : require
        layout(set = 0, binding = 0, rgba8) uniform image2D image;
        layout(set = 0, binding = 1) uniform accelerationStructureNV as;

        layout(location = 0) rayPayloadNV float payload;

        void main()
        {
           vec4 col = vec4(0, 0, 0, 1);

           vec3 origin = vec3(float(gl_LaunchIDNV.x)/float(gl_LaunchSizeNV.x), float(gl_LaunchIDNV.y)/float(gl_LaunchSizeNV.y), 1.0);
           vec3 dir = vec3(0.0, 0.0, -1.0);

           payload = 0.5;
           traceNV(as, gl_RayFlagsCullBackFacingTrianglesNV, 0xff, 0, 1, 0, origin, 0.0, dir, 1000.0, 0);

           col.y = payload;

           imageStore(image, ivec2(gl_LaunchIDNV.xy), col);
        }
    )glsl";

    static char const closestHitShaderText[] = R"glsl(
        #version 460 core
        #extension GL_NV_ray_tracing : require
        layout(location = 0) rayPayloadInNV float hitValue;

        void main() {
            hitValue = 1.0;
        }
    )glsl";

    static char const missShaderText[] = R"glsl(
        #version 460 core
        #extension GL_NV_ray_tracing : require
        layout(location = 0) rayPayloadInNV float hitValue;

        void main() {
            hitValue = 0.0;
        }
    )glsl";

    rgs_ = std::make_unique<VkShaderObj>(&layer_test_, rayGenShaderText, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    chs_ = std::make_unique<VkShaderObj>(&layer_test_, closestHitShaderText, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    mis_ = std::make_unique<VkShaderObj>(&layer_test_, missShaderText, VK_SHADER_STAGE_MISS_BIT_NV);

    shader_stages_ = {rgs_->GetStageCreateInfo(), chs_->GetStageCreateInfo(), mis_->GetStageCreateInfo()};
}

void RayTracingPipelineHelper::InitNVRayTracingPipelineInfo() {
    rp_ci_ = vku::InitStructHelper();
    rp_ci_.maxRecursionDepth = 0;
    rp_ci_.stageCount = shader_stages_.size();
    rp_ci_.pStages = shader_stages_.data();
    rp_ci_.groupCount = groups_.size();
    rp_ci_.pGroups = groups_.data();
}

void RayTracingPipelineHelper::AddLibrary(const RayTracingPipelineHelper &library) {
    libraries_.emplace_back(library.Handle());
    rp_library_ci_ = vku::InitStructHelper();
    rp_library_ci_.libraryCount = size32(libraries_);
    rp_library_ci_.pLibraries = libraries_.data();
    rp_ci_KHR_.pLibraryInfo = &rp_library_ci_;
}

void RayTracingPipelineHelper::InitPipelineCacheInfo() {
    pc_ci_ = vku::InitStructHelper();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void RayTracingPipelineHelper::InitInfo() {
    InitShaderGroups();
    InitDescriptorSetInfo();
    InitPipelineLayoutInfo();
    InitShaderInfo();
    InitNVRayTracingPipelineInfo();
    InitPipelineCacheInfo();
}

void RayTracingPipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_EQ(VK_SUCCESS, err);
}

void RayTracingPipelineHelper::LateBindPipelineInfo(bool isKHR) {
    // By value or dynamically located items must be late bound
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    pipeline_layout_ = vkt::PipelineLayout(*layer_test_.DeviceObj(), {&descriptor_set_->layout_});

    if (isKHR) {
        rp_ci_KHR_.layout = pipeline_layout_.handle();
        rp_ci_KHR_.stageCount = shader_stages_.size();
        rp_ci_KHR_.pStages = shader_stages_.data();
    } else {
        rp_ci_.layout = pipeline_layout_.handle();
        rp_ci_.stageCount = shader_stages_.size();
        rp_ci_.pStages = shader_stages_.data();
    }
}

VkResult RayTracingPipelineHelper::CreateNVRayTracingPipeline(bool do_late_bind) {
    InitPipelineCache();
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        (PFN_vkCreateRayTracingPipelinesNV)vk::GetInstanceProcAddr(layer_test_.instance(), "vkCreateRayTracingPipelinesNV");
    return vkCreateRayTracingPipelinesNV(layer_test_.device(), pipeline_cache_, 1, &rp_ci_, nullptr, &pipeline_);
}

void GetSimpleGeometryForAccelerationStructureTests(const vkt::Device &device, vkt::Buffer *vbo, vkt::Buffer *ibo,
                                                    VkGeometryNV *geometry, VkDeviceSize offset, bool buffer_device_address) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    void *alloc_pnext = nullptr;
    if (buffer_device_address) {
        usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        alloc_pnext = &alloc_flags;
    }
    vbo->init(device, 1024, usage, kHostVisibleMemProps, alloc_pnext);
    ibo->init(device, 1024, usage, kHostVisibleMemProps, alloc_pnext);

    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indicies = {{0, 1, 2}};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo->Memory().Map();
    std::memcpy(mapped_vbo_buffer_data + offset, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo->Memory().Unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo->Memory().Map();
    std::memcpy(mapped_ibo_buffer_data + offset, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo->Memory().Unmap();

    *geometry = {};
    geometry->sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    geometry->geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry->geometry.triangles.vertexData = vbo->handle();
    geometry->geometry.triangles.vertexOffset = 0;
    geometry->geometry.triangles.vertexCount = 3;
    geometry->geometry.triangles.vertexStride = 12;
    geometry->geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry->geometry.triangles.indexData = ibo->handle();
    geometry->geometry.triangles.indexOffset = 0;
    geometry->geometry.triangles.indexCount = 3;
    geometry->geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry->geometry.triangles.transformData = VK_NULL_HANDLE;
    geometry->geometry.triangles.transformOffset = 0;
    geometry->geometry.aabbs = {};
    geometry->geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
}
}  // namespace rt
}  // namespace nv
