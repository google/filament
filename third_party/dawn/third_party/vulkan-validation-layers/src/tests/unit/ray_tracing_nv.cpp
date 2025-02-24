/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_helper_nv.h"
#include "../framework/descriptor_helper.h"

void RayTracingTest::NvInitFrameworkForRayTracingTest(VkPhysicalDeviceFeatures2KHR *features2 /*= nullptr*/,
                                                      VkValidationFeaturesEXT *enabled_features /*= nullptr*/) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework(enabled_features));

    if (features2) {
        // extension enabled as dependency of RT extension
        vk::GetPhysicalDeviceFeatures2KHR(Gpu(), features2);
    }
}

class NegativeRayTracingNV : public RayTracingTest {
  public:
    void OOBRayTracingShadersTestBodyNV(bool gpu_assisted);
};

void NegativeRayTracingNV::OOBRayTracingShadersTestBodyNV(bool gpu_assisted) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    VkValidationFeatureEnableEXT validation_feature_enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeatureDisableEXT validation_feature_disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT validation_features = vku::InitStructHelper(&kDisableMessageLimit);
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures = validation_feature_enables;
    validation_features.disabledValidationFeatureCount = 4;
    validation_features.pDisabledValidationFeatures = validation_feature_disables;
    RETURN_IF_SKIP(InitFramework(gpu_assisted ? (void *)&validation_features : (void *)&kDisableMessageLimit));
    bool descriptor_indexing = IsExtensionsEnabled(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    if (gpu_assisted && IsPlatformMockICD()) {
        GTEST_SKIP() << "GPU-AV can't run on MockICD";
    }

    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper();
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    if (descriptor_indexing) {
        features2 = GetPhysicalDeviceFeatures2(indexing_features);
        if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingPartiallyBound ||
            !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
            !indexing_features.descriptorBindingVariableDescriptorCount) {
            printf("Not all descriptor indexing features supported, skipping descriptor indexing tests\n");
            descriptor_indexing = false;
        }
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);
    if (ray_tracing_properties.maxTriangleCount == 0) {
        GTEST_SKIP() << "Did not find required ray tracing properties";
    }

    vkt::Queue *ray_tracing_queue = m_default_queue;
    uint32_t ray_tracing_queue_family_index = 0;

    // If supported, run on the compute only queue.
    vkt::Queue *compute_only_queue = m_device->ComputeOnlyQueue();
    if (compute_only_queue) {
        ray_tracing_queue = compute_only_queue;
        ray_tracing_queue_family_index = compute_only_queue->family_index;
    }

    vkt::CommandPool ray_tracing_command_pool(*m_device, ray_tracing_queue_family_index,
                                              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer ray_tracing_command_buffer(*m_device, ray_tracing_command_pool);

    constexpr std::array<VkAabbPositionsKHR, 1> aabbs = {{{-1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f}}};

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    const VkDeviceSize aabb_buffer_size = sizeof(VkAabbPositionsKHR) * aabbs.size();
    vkt::Buffer aabb_buffer;
    aabb_buffer.init(*m_device, aabb_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, kHostVisibleMemProps, nullptr,
                     vvl::make_span(&ray_tracing_queue_family_index, 1));

    uint8_t *mapped_aabb_buffer_data = (uint8_t *)aabb_buffer.Memory().Map();
    std::memcpy(mapped_aabb_buffer_data, (uint8_t *)aabbs.data(), static_cast<std::size_t>(aabb_buffer_size));
    aabb_buffer.Memory().Unmap();

    VkGeometryNV geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    geometry.geometry.triangles = {};
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry.geometry.aabbs = {};
    geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    geometry.geometry.aabbs.aabbData = aabb_buffer.handle();
    geometry.geometry.aabbs.numAABBs = static_cast<uint32_t>(aabbs.size());
    geometry.geometry.aabbs.offset = 0;
    geometry.geometry.aabbs.stride = static_cast<VkDeviceSize>(sizeof(VkAabbPositionsKHR));
    geometry.flags = 0;

    VkAccelerationStructureInfoNV bot_level_as_info = vku::InitStructHelper();
    bot_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_info.instanceCount = 0;
    bot_level_as_info.geometryCount = 1;
    bot_level_as_info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = bot_level_as_info;

    vkt::AccelerationStructureNV bot_level_as(*m_device, bot_level_as_create_info);

    const std::array instances = {
        VkGeometryInstanceNV{
            {
                // clang-format off
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                // clang-format on
            },
            0,
            0xFF,
            0,
            VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
            bot_level_as.OpaqueHandle(),
        },
    };

    VkDeviceSize instance_buffer_size = sizeof(instances[0]) * instances.size();
    vkt::Buffer instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, kHostVisibleMemProps, nullptr,
                         vvl::make_span(&ray_tracing_queue_family_index, 1));

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.Memory().Map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)instances.data(), static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.Memory().Unmap();

    VkAccelerationStructureInfoNV top_level_as_info = vku::InitStructHelper();
    top_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_info.instanceCount = 1;
    top_level_as_info.geometryCount = 0;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = top_level_as_info;

    vkt::AccelerationStructureNV top_level_as(*m_device, top_level_as_create_info);

    VkDeviceSize scratch_buffer_size = std::max(bot_level_as.BuildScratchMemoryRequirements().memoryRequirements.size,
                                                top_level_as.BuildScratchMemoryRequirements().memoryRequirements.size);
    vkt::Buffer scratch_buffer(*m_device, scratch_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    ray_tracing_command_buffer.Begin();

    // Build bot level acceleration structure
    vk::CmdBuildAccelerationStructureNV(ray_tracing_command_buffer.handle(), &bot_level_as.Info(), VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, scratch_buffer.handle(), 0);

    // Barrier to prevent using scratch buffer for top level build before bottom level build finishes
    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    vk::CmdPipelineBarrier(ray_tracing_command_buffer.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
                           VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);

    // Build top level acceleration structure
    vk::CmdBuildAccelerationStructureNV(ray_tracing_command_buffer.handle(), &top_level_as.Info(), instance_buffer.handle(), 0,
                                        VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, scratch_buffer.handle(), 0);

    ray_tracing_command_buffer.End();

    ray_tracing_queue->Submit(ray_tracing_command_buffer);
    ray_tracing_queue->Wait();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDeviceSize storage_buffer_size = 1024;
    vkt::Buffer storage_buffer;
    storage_buffer.init(*m_device, storage_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps, nullptr,
                        vvl::make_span(&ray_tracing_queue_family_index, 1));

    VkDeviceSize shader_binding_table_buffer_size = ray_tracing_properties.shaderGroupBaseAlignment * 4ull;
    vkt::Buffer shader_binding_table_buffer;
    shader_binding_table_buffer.init(*m_device, shader_binding_table_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                                     kHostVisibleMemProps, nullptr, vvl::make_span(&ray_tracing_queue_family_index, 1));

    // Setup descriptors!
    const VkShaderStageFlags kAllRayTracingStages = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV |
                                                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV |
                                                    VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV;

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    VkDescriptorPoolCreateFlags pool_create_flags = 0;
    VkDescriptorSetLayoutCreateFlags layout_create_flags = 0;
    VkDescriptorBindingFlags ds_binding_flags[3] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags[1] = {};
    if (descriptor_indexing) {
        ds_binding_flags[0] = 0;
        ds_binding_flags[1] = 0;
        ds_binding_flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

        layout_createinfo_binding_flags[0] = vku::InitStructHelper();
        layout_createinfo_binding_flags[0].bindingCount = 3;
        layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
        layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        pool_create_flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        layout_pnext = layout_createinfo_binding_flags;
    }

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, kAllRayTracingStages, nullptr},
                           },
                           layout_create_flags, layout_pnext, pool_create_flags);

    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    uint32_t desc_counts;
    if (descriptor_indexing) {
        layout_create_flags = 0;
        pool_create_flags = 0;
        ds_binding_flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
        desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
        variable_count.descriptorSetCount = 1;
        variable_count.pDescriptorCounts = &desc_counts;
        allocate_pnext = &variable_count;
    }

    OneOffDescriptorSet ds_variable(m_device,
                                    {
                                        {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                                        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, kAllRayTracingStages, nullptr},
                                    },
                                    layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    VkAccelerationStructureNV top_level_as_handle = top_level_as.handle();
    VkWriteDescriptorSetAccelerationStructureNV write_descript_set_as = vku::InitStructHelper();
    write_descript_set_as.accelerationStructureCount = 1;
    write_descript_set_as.pAccelerationStructures = &top_level_as_handle;

    VkDescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.buffer = storage_buffer.handle();
    descriptor_buffer_info.offset = 0;
    descriptor_buffer_info.range = storage_buffer_size;

    VkDescriptorImageInfo descriptor_image_infos[6] = {};
    for (int i = 0; i < 6; i++) {
        descriptor_image_infos[i] = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[3] = {};
    descriptor_writes[0] = vku::InitStructHelper(&write_descript_set_as);
    descriptor_writes[0].dstSet = ds.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = ds.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &descriptor_buffer_info;

    descriptor_writes[2] = vku::InitStructHelper();
    descriptor_writes[2].dstSet = ds.set_;
    descriptor_writes[2].dstBinding = 2;
    if (descriptor_indexing) {
        descriptor_writes[2].descriptorCount = 5;  // Intentionally don't write index 5
    } else {
        descriptor_writes[2].descriptorCount = 6;
    }
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[2].pImageInfo = descriptor_image_infos;
    vk::UpdateDescriptorSets(device(), 3, descriptor_writes, 0, NULL);
    if (descriptor_indexing) {
        descriptor_writes[0].dstSet = ds_variable.set_;
        descriptor_writes[1].dstSet = ds_variable.set_;
        descriptor_writes[2].dstSet = ds_variable.set_;
        vk::UpdateDescriptorSets(device(), 3, descriptor_writes, 0, NULL);
    }

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});
    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&ds_variable.layout_});

    const auto SetImagesArrayLength = [](const std::string &shader_template, const std::string &length_str) {
        const std::string to_replace = "IMAGES_ARRAY_LENGTH";

        std::string result = shader_template;
        auto position = result.find(to_replace);
        assert(position != std::string::npos);
        result.replace(position, to_replace.length(), length_str);
        return result;
    };

    const std::string rgen_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 0) uniform accelerationStructureNV topLevelAS;
        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
            uint rgen_index;
            uint ahit_index;
            uint chit_index;
            uint miss_index;
            uint intr_index;
            uint call_index;

            uint rgen_ran;
            uint ahit_ran;
            uint chit_ran;
            uint miss_ran;
            uint intr_ran;
            uint call_ran;

            float result1;
            float result2;
            float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadNV vec3 payload;
        layout(location = 3) callableDataNV vec3 callableData;

        void main() {
            sbo.rgen_ran = 1;

            executeCallableNV(0, 3);
            sbo.result1 = callableData.x;

            vec3 origin = vec3(0.0f, 0.0f, -2.0f);
            vec3 direction = vec3(0.0f, 0.0f, 1.0f);

            traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, direction, 10000.0, 0);
            sbo.result2 = payload.x;

            traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, -direction, 10000.0, 0);
            sbo.result3 = payload.x;

            if (sbo.rgen_index > 0) {
                // OOB here:
                sbo.result3 = texelFetch(textures[sbo.rgen_index], ivec2(0, 0), 0).x;
            }
        }
        )";

    const std::string rgen_source = SetImagesArrayLength(rgen_source_template, "6");
    const std::string rgen_source_runtime = SetImagesArrayLength(rgen_source_template, "");

    const std::string ahit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
            uint rgen_index;
            uint ahit_index;
            uint chit_index;
            uint miss_index;
            uint intr_index;
            uint call_index;

            uint rgen_ran;
            uint ahit_ran;
            uint chit_ran;
            uint miss_ran;
            uint intr_ran;
            uint call_ran;

            float result1;
            float result2;
            float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
            sbo.ahit_ran = 2;

            payload = vec3(0.1234f);

            if (sbo.ahit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.ahit_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string ahit_source = SetImagesArrayLength(ahit_source_template, "6");
    const std::string ahit_source_runtime = SetImagesArrayLength(ahit_source_template, "");

    const std::string chit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
            uint rgen_index;
            uint ahit_index;
            uint chit_index;
            uint miss_index;
            uint intr_index;
            uint call_index;

            uint rgen_ran;
            uint ahit_ran;
            uint chit_ran;
            uint miss_ran;
            uint intr_ran;
            uint call_ran;

            float result1;
            float result2;
            float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        hitAttributeNV vec3 attribs;

        void main() {
            sbo.chit_ran = 3;

            payload = attribs;
            if (sbo.chit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.chit_index], ivec2(0, 0), 0).x;
            }
        }
        )";
    const std::string chit_source = SetImagesArrayLength(chit_source_template, "6");
    const std::string chit_source_runtime = SetImagesArrayLength(chit_source_template, "");

    const std::string miss_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
            uint rgen_index;
            uint ahit_index;
            uint chit_index;
            uint miss_index;
            uint intr_index;
            uint call_index;

            uint rgen_ran;
            uint ahit_ran;
            uint chit_ran;
            uint miss_ran;
            uint intr_ran;
            uint call_ran;

            float result1;
            float result2;
            float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
            sbo.miss_ran = 4;

            payload = vec3(1.0, 0.0, 0.0);

            if (sbo.miss_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.miss_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string miss_source = SetImagesArrayLength(miss_source_template, "6");
    const std::string miss_source_runtime = SetImagesArrayLength(miss_source_template, "");

    const std::string intr_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
            uint rgen_index;
            uint ahit_index;
            uint chit_index;
            uint miss_index;
            uint intr_index;
            uint call_index;

            uint rgen_ran;
            uint ahit_ran;
            uint chit_ran;
            uint miss_ran;
            uint intr_ran;
            uint call_ran;

            float result1;
            float result2;
            float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        void main() {
            sbo.intr_ran = 5;

            hitValue = vec3(0.0f, 0.5f, 0.0f);

            reportIntersectionNV(1.0f, 0);

            if (sbo.intr_index > 0) {
                // OOB here:
                hitValue.x = texelFetch(textures[sbo.intr_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string intr_source = SetImagesArrayLength(intr_source_template, "6");
    const std::string intr_source_runtime = SetImagesArrayLength(intr_source_template, "");

    const std::string call_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
            uint rgen_index;
            uint ahit_index;
            uint chit_index;
            uint miss_index;
            uint intr_index;
            uint call_index;

            uint rgen_ran;
            uint ahit_ran;
            uint chit_ran;
            uint miss_ran;
            uint intr_ran;
            uint call_ran;

            float result1;
            float result2;
            float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 3) callableDataInNV vec3 callableData;

        void main() {
            sbo.call_ran = 6;

            callableData = vec3(0.1234f);

            if (sbo.call_index > 0) {
                // OOB here:
                callableData.x = texelFetch(textures[sbo.call_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string call_source = SetImagesArrayLength(call_source_template, "6");
    const std::string call_source_runtime = SetImagesArrayLength(call_source_template, "");

    struct TestCase {
        const std::string &rgen_shader_source;
        const std::string &ahit_shader_source;
        const std::string &chit_shader_source;
        const std::string &miss_shader_source;
        const std::string &intr_shader_source;
        const std::string &call_shader_source;
        bool variable_length;
        uint32_t rgen_index;
        uint32_t ahit_index;
        uint32_t chit_index;
        uint32_t miss_index;
        uint32_t intr_index;
        uint32_t call_index;
        const char *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 25, 0, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 25, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 25, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 25, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 25, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 0, 25,
                     "Index of 25 used to index descriptor array of length 6."});

    if (descriptor_indexing) {
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 25, 0, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 25, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 25, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 25, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 25, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 25, "Index of 25 used to index descriptor array of length 6."});

        // For this group, 6 is less than max specified (max specified is 8) but more than actual specified (actual specified is 5)
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 6, 0, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 6, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 6, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 6, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 6, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 6, "Index of 6 used to index descriptor array of length 6."});

        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 5, 0, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 5, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 5, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 5, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 5, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 5, "Descriptor index 5 is uninitialized."});
    }

    // Iteration 0 tests with no descriptor set bound (to sanity test "draw" validation). Iteration 1
    // tests what's in the test case vector.
    for (const auto &test : tests) {
        if (gpu_assisted) {
            m_errorMonitor->SetDesiredError(test.expected_error);
        }

        VkShaderObj rgen_shader(this, test.rgen_shader_source.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_NV);
        VkShaderObj ahit_shader(this, test.ahit_shader_source.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_NV);
        VkShaderObj chit_shader(this, test.chit_shader_source.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
        VkShaderObj miss_shader(this, test.miss_shader_source.c_str(), VK_SHADER_STAGE_MISS_BIT_NV);
        VkShaderObj intr_shader(this, test.intr_shader_source.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_NV);
        VkShaderObj call_shader(this, test.call_shader_source.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_NV);

        VkPipelineShaderStageCreateInfo stage_create_infos[6] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        stage_create_infos[2] = vku::InitStructHelper();
        stage_create_infos[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[2].module = chit_shader.handle();
        stage_create_infos[2].pName = "main";

        stage_create_infos[3] = vku::InitStructHelper();
        stage_create_infos[3].stage = VK_SHADER_STAGE_MISS_BIT_NV;
        stage_create_infos[3].module = miss_shader.handle();
        stage_create_infos[3].pName = "main";

        stage_create_infos[4] = vku::InitStructHelper();
        stage_create_infos[4].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[4].module = intr_shader.handle();
        stage_create_infos[4].pName = "main";

        stage_create_infos[5] = vku::InitStructHelper();
        stage_create_infos[5].stage = VK_SHADER_STAGE_CALLABLE_BIT_NV;
        stage_create_infos[5].module = call_shader.handle();
        stage_create_infos[5].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[4] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;  // rgen
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 3;  // miss
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[2] = vku::InitStructHelper();
        group_create_infos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[2].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[2].closestHitShader = 2;
        group_create_infos[2].anyHitShader = 1;
        group_create_infos[2].intersectionShader = 4;

        group_create_infos[3] = vku::InitStructHelper();
        group_create_infos[3].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[3].generalShader = 5;  // call
        group_create_infos[3].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 6;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 4;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.maxRecursionDepth = 2;
        pipeline_ci.layout = test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle();

        VkPipeline pipeline = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS,
                  vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline));

        std::vector<uint8_t> shader_binding_table_data;
        shader_binding_table_data.resize(static_cast<std::size_t>(shader_binding_table_buffer_size), 0);
        ASSERT_EQ(VK_SUCCESS, vk::GetRayTracingShaderGroupHandlesNV(m_device->handle(), pipeline, 0, 4,
                                                                    static_cast<std::size_t>(shader_binding_table_buffer_size),
                                                                    shader_binding_table_data.data()));

        uint8_t *mapped_shader_binding_table_data = (uint8_t *)shader_binding_table_buffer.Memory().Map();
        std::memcpy(mapped_shader_binding_table_data, shader_binding_table_data.data(), shader_binding_table_data.size());
        shader_binding_table_buffer.Memory().Unmap();

        ray_tracing_command_buffer.Begin();

        vk::CmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);

        if (gpu_assisted) {
            vk::CmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                      test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                      test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);
        } else {
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-None-08600");
        }

        if (gpu_assisted) {
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458");
            // Need these values to pass mapped storage buffer checks
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupHandleSize * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupHandleSize * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupHandleSize * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupHandleSize * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
        } else {
            // offset shall be multiple of shaderGroupBaseAlignment and stride of shaderGroupHandleSize
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
        }

        ray_tracing_command_buffer.End();
        // Update the index of the texture that the shaders should read
        uint32_t *mapped_storage_buffer_data = (uint32_t *)storage_buffer.Memory().Map();
        mapped_storage_buffer_data[0] = test.rgen_index;
        mapped_storage_buffer_data[1] = test.ahit_index;
        mapped_storage_buffer_data[2] = test.chit_index;
        mapped_storage_buffer_data[3] = test.miss_index;
        mapped_storage_buffer_data[4] = test.intr_index;
        mapped_storage_buffer_data[5] = test.call_index;
        mapped_storage_buffer_data[6] = 0;
        mapped_storage_buffer_data[7] = 0;
        mapped_storage_buffer_data[8] = 0;
        mapped_storage_buffer_data[9] = 0;
        mapped_storage_buffer_data[10] = 0;
        mapped_storage_buffer_data[11] = 0;
        storage_buffer.Memory().Unmap();

        ray_tracing_queue->Submit(ray_tracing_command_buffer);
        ray_tracing_queue->Wait();
        m_errorMonitor->VerifyFound();

        if (gpu_assisted) {
            mapped_storage_buffer_data = (uint32_t *)storage_buffer.Memory().Map();
            ASSERT_TRUE(mapped_storage_buffer_data[6] == 1);
            ASSERT_TRUE(mapped_storage_buffer_data[7] == 2);
            ASSERT_TRUE(mapped_storage_buffer_data[8] == 3);
            ASSERT_TRUE(mapped_storage_buffer_data[9] == 4);
            ASSERT_TRUE(mapped_storage_buffer_data[10] == 5);
            ASSERT_TRUE(mapped_storage_buffer_data[11] == 6);
            storage_buffer.Memory().Unmap();
        } else {
            ray_tracing_command_buffer.Begin();
            vk::CmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);
            vk::CmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                      test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                      test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462");
            VkDeviceSize stride_align = ray_tracing_properties.shaderGroupHandleSize;
            VkDeviceSize invalid_max_stride = ray_tracing_properties.maxShaderGroupStride +
                                              (stride_align - (ray_tracing_properties.maxShaderGroupStride %
                                                               stride_align));  // should be less than maxShaderGroupStride
            VkDeviceSize invalid_stride =
                ray_tracing_properties.shaderGroupHandleSize >> 1;  // should  be multiple of shaderGroupHandleSize
            VkDeviceSize invalid_offset =
                ray_tracing_properties.shaderGroupBaseAlignment >> 1;  // should be multiple of shaderGroupBaseAlignment

            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(), invalid_offset,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, invalid_stride,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, invalid_max_stride,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // hit shader
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), invalid_offset, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               invalid_stride, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               invalid_max_stride, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // miss shader
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               invalid_offset, ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 2ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, invalid_stride,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, invalid_max_stride,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // raygenshader
            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(), invalid_offset,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 1ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 2ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);

            m_errorMonitor->VerifyFound();
            const auto &limits = m_device->Physical().limits_;

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-width-02469");
            uint32_t invalid_width = limits.maxComputeWorkGroupCount[0] + 1;
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/invalid_width, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-height-02470");
            uint32_t invalid_height = limits.maxComputeWorkGroupCount[1] + 1;
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/invalid_height, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredError("VUID-vkCmdTraceRaysNV-depth-02471");
            uint32_t invalid_depth = limits.maxComputeWorkGroupCount[2] + 1;
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/invalid_depth);
            m_errorMonitor->VerifyFound();

            ray_tracing_command_buffer.End();
        }
        vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    }
}

TEST_F(NegativeRayTracingNV, ArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "Core validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders.");

    OOBRayTracingShadersTestBodyNV(false);
}

TEST_F(NegativeRayTracingNV, AccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());

    VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_props);

    RETURN_IF_SKIP(InitState());

    uint32_t maxBlocks = ray_tracing_props.maxDescriptorSetAccelerationStructures;
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of descriptor set acceleration structures, skipping tests";
    }
    if (maxBlocks < 1) {
        GTEST_SKIP() << "Test requires maxDescriptorSetAccelerationStructures >= 1";
    }

    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutBinding dslb = {};
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    for (uint32_t i = 0; i <= maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02381");
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, ValidateGeometry) {
    TEST_DESCRIPTION("Validate acceleration structure geometries.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, kHostVisibleMemProps);

    vkt::Buffer ibo;
    ibo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, kHostVisibleMemProps);

    vkt::Buffer tbo;
    tbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, kHostVisibleMemProps);

    vkt::Buffer aabbbo;
    aabbbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, kHostVisibleMemProps);

    VkBufferCreateInfo unbound_buffer_ci = vku::InitStructHelper();
    unbound_buffer_ci.size = 1024;
    unbound_buffer_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    vkt::Buffer unbound_buffer(*m_device, unbound_buffer_ci, vkt::no_mem);

    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indicies = {0, 1, 2};
    constexpr std::array aabbs = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    constexpr std::array transforms = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo.Memory().Map();
    std::memcpy(mapped_vbo_buffer_data, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo.Memory().Unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo.Memory().Map();
    std::memcpy(mapped_ibo_buffer_data, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo.Memory().Unmap();

    uint8_t *mapped_tbo_buffer_data = (uint8_t *)tbo.Memory().Map();
    std::memcpy(mapped_tbo_buffer_data, (uint8_t *)transforms.data(), sizeof(float) * transforms.size());
    tbo.Memory().Unmap();

    uint8_t *mapped_aabbbo_buffer_data = (uint8_t *)aabbbo.Memory().Map();
    std::memcpy(mapped_aabbbo_buffer_data, (uint8_t *)aabbs.data(), sizeof(float) * aabbs.size());
    aabbbo.Memory().Unmap();

    VkGeometryNV valid_geometry_triangles = vku::InitStructHelper();
    valid_geometry_triangles.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    valid_geometry_triangles.geometry.triangles = vku::InitStructHelper();
    valid_geometry_triangles.geometry.triangles.vertexData = vbo.handle();
    valid_geometry_triangles.geometry.triangles.vertexOffset = 0;
    valid_geometry_triangles.geometry.triangles.vertexCount = 3;
    valid_geometry_triangles.geometry.triangles.vertexStride = 12;
    valid_geometry_triangles.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    valid_geometry_triangles.geometry.triangles.indexData = ibo.handle();
    valid_geometry_triangles.geometry.triangles.indexOffset = 0;
    valid_geometry_triangles.geometry.triangles.indexCount = 3;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.transformData = tbo.handle();
    valid_geometry_triangles.geometry.triangles.transformOffset = 0;
    valid_geometry_triangles.geometry.aabbs = vku::InitStructHelper();

    VkGeometryNV valid_geometry_aabbs = vku::InitStructHelper();
    valid_geometry_aabbs.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    valid_geometry_aabbs.geometry.triangles = vku::InitStructHelper();
    valid_geometry_aabbs.geometry.aabbs = vku::InitStructHelper();
    valid_geometry_aabbs.geometry.aabbs.aabbData = aabbbo.handle();
    valid_geometry_aabbs.geometry.aabbs.numAABBs = 1;
    valid_geometry_aabbs.geometry.aabbs.offset = 0;
    valid_geometry_aabbs.geometry.aabbs.stride = 24;

    const auto GetCreateInfo = [](const VkGeometryNV &geometry) {
        VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
        as_create_info.info = vku::InitStructHelper();
        as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        as_create_info.info.instanceCount = 0;
        as_create_info.info.geometryCount = 1;
        as_create_info.info.pGeometries = &geometry;
        return as_create_info;
    };

    VkAccelerationStructureNV as;

    // Invalid vertex format.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R64_UINT;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-vertexFormat-02430");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - not a multiple of component size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-vertexOffset-02429");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 12 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex buffer - no such buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = VkBuffer(123456789);

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-vertexData-parameter");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index offset - not a multiple of index size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-indexOffset-02432");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-indexOffset-02431");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index count - must be 0 if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = VK_NULL_HANDLE;
        geometry.geometry.triangles.indexCount = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-indexCount-02436");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index data - must be VK_NULL_HANDLE if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = ibo.handle();
        geometry.geometry.triangles.indexCount = 0;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-indexData-02434");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid transform offset - not a multiple of 16.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-transformOffset-02438");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid transform offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryTrianglesNV-transformOffset-02437");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb offset - not a multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryAABBNV-offset-02440");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 8 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryAABBNV-offset-02439");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb stride - not a multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryAABBNV-stride-02441");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV or VK_GEOMETRY_TYPE_AABBS_NV
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredError("VUID-VkGeometryNV-geometryType-03503");
        m_errorMonitor->SetUnexpectedError("VUID-VkGeometryNV-geometryType-parameter");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateCreateAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();

    VkAccelerationStructureNV as = VK_NULL_HANDLE;

    // Top level can not have geometry
    {
        VkAccelerationStructureCreateInfoNV bad_top_level_create_info = as_create_info;
        bad_top_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
        bad_top_level_create_info.info.instanceCount = 0;
        bad_top_level_create_info.info.geometryCount = 1;
        bad_top_level_create_info.info.pGeometries = &geometry;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-type-02425");
        vk::CreateAccelerationStructureNV(device(), &bad_top_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Bot level can not have instances
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_bot_level_create_info.info.instanceCount = 1;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-type-02426");
        vk::CreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Type must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR;
        bad_bot_level_create_info.info.instanceCount = 0;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-type-04623");
        vk::CreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not prefer both fast trace and fast build
    {
        VkAccelerationStructureCreateInfoNV bad_flags_level_create_info = as_create_info;
        bad_flags_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_flags_level_create_info.info.instanceCount = 0;
        bad_flags_level_create_info.info.geometryCount = 1;
        bad_flags_level_create_info.info.pGeometries = &geometry;
        bad_flags_level_create_info.info.flags =
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-flags-02592");
        vk::CreateAccelerationStructureNV(device(), &bad_flags_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not have geometry or instance for compacting
    {
        VkAccelerationStructureCreateInfoNV bad_compacting_as_create_info = as_create_info;
        bad_compacting_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_compacting_as_create_info.info.instanceCount = 0;
        bad_compacting_as_create_info.info.geometryCount = 1;
        bad_compacting_as_create_info.info.pGeometries = &geometry;
        bad_compacting_as_create_info.info.flags = 0;
        bad_compacting_as_create_info.compactedSize = 1024;
        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421");
        vk::CreateAccelerationStructureNV(device(), &bad_compacting_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not mix different geometry types into single bottom level acceleration structure
    {
        VkGeometryNV aabb_geometry = vku::InitStructHelper();
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        aabb_geometry.geometry.triangles = vku::InitStructHelper();
        aabb_geometry.geometry.aabbs = vku::InitStructHelper();
        // Buffer contents do not matter for this test.
        aabb_geometry.geometry.aabbs.aabbData = geometry.geometry.triangles.vertexData;
        aabb_geometry.geometry.aabbs.numAABBs = 1;
        aabb_geometry.geometry.aabbs.offset = 0;
        aabb_geometry.geometry.aabbs.stride = 24;

        std::vector<VkGeometryNV> geometries = {geometry, aabb_geometry};

        VkAccelerationStructureCreateInfoNV mix_geometry_types_as_create_info = as_create_info;
        mix_geometry_types_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        mix_geometry_types_as_create_info.info.instanceCount = 0;
        mix_geometry_types_as_create_info.info.geometryCount = static_cast<uint32_t>(geometries.size());
        mix_geometry_types_as_create_info.info.pGeometries = geometries.data();
        mix_geometry_types_as_create_info.info.flags = 0;

        m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-type-02786");
        vk::CreateAccelerationStructureNV(device(), &mix_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateBindAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;
    as_create_info.info.instanceCount = 0;

    vkt::AccelerationStructureNV as(*m_device, as_create_info, false);

    VkMemoryRequirements as_memory_requirements = as.MemoryRequirements().memoryRequirements;

    VkBindAccelerationStructureMemoryInfoNV as_bind_info = vku::InitStructHelper();
    as_bind_info.accelerationStructure = as.handle();

    VkMemoryAllocateInfo as_memory_alloc = vku::InitStructHelper();
    as_memory_alloc.allocationSize = as_memory_requirements.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(as_memory_requirements.memoryTypeBits, &as_memory_alloc, 0));

    // Can not bind already freed memory
    {
        VkDeviceMemory as_memory_freed = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_freed));
        vk::FreeMemory(device(), as_memory_freed, NULL);

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_freed = as_bind_info;
        as_bind_info_freed.memory = as_memory_freed;
        as_bind_info_freed.memoryOffset = 0;
        m_errorMonitor->SetDesiredError("VUID-VkBindAccelerationStructureMemoryInfoNV-memory-parameter");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_freed);
        m_errorMonitor->VerifyFound();
    }

    // Can not bind with bad alignment
    if (as_memory_requirements.alignment > 1) {
        VkMemoryAllocateInfo as_memory_alloc_bad_alignment = as_memory_alloc;
        as_memory_alloc_bad_alignment.allocationSize += 1;

        VkDeviceMemory as_memory_bad_alignment = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc_bad_alignment, NULL, &as_memory_bad_alignment));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_alignment = as_bind_info;
        as_bind_info_bad_alignment.memory = as_memory_bad_alignment;
        as_bind_info_bad_alignment.memoryOffset = 1;

        m_errorMonitor->SetDesiredError("VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_alignment);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_alignment, NULL);
    }

    // Can not bind with offset outside the allocation
    {
        VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
        as_bind_info_bad_offset.memory = as_memory_bad_offset;
        as_bind_info_bad_offset.memoryOffset =
            (as_memory_alloc.allocationSize + as_memory_requirements.alignment) & ~(as_memory_requirements.alignment - 1);

        m_errorMonitor->SetDesiredError("VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03621");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_offset, NULL);
    }

    // Can not bind with offset that doesn't leave enough size
    {
        VkDeviceSize offset = (as_memory_requirements.size - 1) & ~(as_memory_requirements.alignment - 1);
        if (offset > 0 && (as_memory_requirements.size < (as_memory_alloc.allocationSize - as_memory_requirements.alignment))) {
            VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
            as_bind_info_bad_offset.memory = as_memory_bad_offset;
            as_bind_info_bad_offset.memoryOffset = offset;

            m_errorMonitor->SetDesiredError("VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624");
            vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_offset, NULL);
        }
    }

    // Can not bind with memory that has unsupported memory type
    {
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vk::GetPhysicalDeviceMemoryProperties(m_device->Physical().handle(), &memory_properties);

        uint32_t supported_memory_type_bits = as_memory_requirements.memoryTypeBits;
        uint32_t unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~supported_memory_type_bits;
        if (unsupported_mem_type_bits != 0) {
            VkMemoryAllocateInfo as_memory_alloc_bad_type = as_memory_alloc;
            ASSERT_TRUE(m_device->Physical().SetMemoryType(unsupported_mem_type_bits, &as_memory_alloc_bad_type, 0));

            VkDeviceMemory as_memory_bad_type = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc_bad_type, NULL, &as_memory_bad_type));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_type = as_bind_info;
            as_bind_info_bad_type.memory = as_memory_bad_type;

            m_errorMonitor->SetDesiredError("VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
            vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_type);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_type, NULL);
        }
    }

    // Can not bind memory twice
    {
        vkt::AccelerationStructureNV as_twice(*m_device, as_create_info, false);

        VkDeviceMemory as_memory_twice_1 = VK_NULL_HANDLE;
        VkDeviceMemory as_memory_twice_2 = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_1));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_2));
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_1 = as_bind_info;
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_2 = as_bind_info;
        as_bind_info_twice_1.accelerationStructure = as_twice.handle();
        as_bind_info_twice_2.accelerationStructure = as_twice.handle();
        as_bind_info_twice_1.memory = as_memory_twice_1;
        as_bind_info_twice_2.memory = as_memory_twice_2;

        ASSERT_EQ(VK_SUCCESS, vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_1));
        m_errorMonitor->SetDesiredError("VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(NegativeRayTracingNV, ValidateWriteDescriptorSetAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
                           });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkWriteDescriptorSetAccelerationStructureNV acc = vku::InitStructHelper();
    acc.accelerationStructureCount = 1;
    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::AccelerationStructureNV top_level_as(*m_device, top_level_as_create_info);

    acc.pAccelerationStructures = &top_level_as.handle();
    descriptor_write.pNext = &acc;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(NegativeRayTracingNV, ValidateCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructureNV bot_level_as(*m_device, bot_level_as_create_info);

    const vkt::Buffer bot_level_as_scratch = bot_level_as.CreateScratchBuffer(*m_device);

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-recording");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();

    // Incompatible type
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_type = bot_level_as_create_info.info;
    as_build_info_with_incompatible_type.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    as_build_info_with_incompatible_type.instanceCount = 1;
    as_build_info_with_incompatible_type.geometryCount = 0;

    // This is duplicated since it triggers one error for different types and one error for lower instance count - the
    // build info is incompatible but still needs to be valid to get past the stateless checks.
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &as_build_info_with_incompatible_type, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible flags
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_flags = bot_level_as_create_info.info;
    as_build_info_with_incompatible_flags.flags = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &as_build_info_with_incompatible_flags, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible build size
    VkGeometryNV geometry_with_more_vertices = geometry;
    geometry_with_more_vertices.geometry.triangles.vertexCount += 1;

    VkAccelerationStructureInfoNV as_build_info_with_incompatible_geometry = bot_level_as_create_info.info;
    as_build_info_with_incompatible_geometry.pGeometries = &geometry_with_more_vertices;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &as_build_info_with_incompatible_geometry, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer too small
    VkBufferCreateInfo too_small_scratch_buffer_info = vku::InitStructHelper();
    too_small_scratch_buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    too_small_scratch_buffer_info.size = 1;
    vkt::Buffer too_small_scratch_buffer(*m_device, too_small_scratch_buffer_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, too_small_scratch_buffer.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer with offset too small
    VkDeviceSize scratch_buffer_offset = 5;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(),
                                        scratch_buffer_offset);
    m_errorMonitor->VerifyFound();

    // Src must have been built before
    vkt::AccelerationStructureNV bot_level_as_updated(*m_device, bot_level_as_create_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-update-02489");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                        bot_level_as_updated.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Src must have been built before with the VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV flag
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-update-02490");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                        bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid scratch buffer (invalid usage)
    VkBufferCreateInfo create_info = vku::InitStructHelper();
    create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const vkt::Buffer bot_level_as_invalid_scratch = bot_level_as.CreateScratchBuffer(*m_device, &create_info);
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-scratch-02781");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_invalid_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid instance data.
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureInfoNV-instanceData-02782");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info,
                                        bot_level_as_invalid_scratch.handle(), 0, VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE,
                                        bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // must be called outside renderpass
    InitRenderTarget();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBuildAccelerationStructureNV-renderpass");
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_command_buffer.EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, ObjInUseCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructureNV bot_level_as(*m_device, bot_level_as_create_info);

    const vkt::Buffer bot_level_as_scratch = bot_level_as.CreateScratchBuffer(*m_device);

    m_command_buffer.Begin();
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), ibo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), vbo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), bot_level_as_scratch.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    vk::DestroyAccelerationStructureNV(device(), bot_level_as.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeRayTracingNV, ValidateGetAccelerationStructureHandle) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    // Not enough space for the handle
    {
        vkt::AccelerationStructureNV bot_level_as(*m_device, bot_level_as_create_info);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureHandleNV-dataSize-02240");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint8_t), &handle);
        m_errorMonitor->VerifyFound();
    }

    // No memory bound to acceleration structure
    {
        vkt::AccelerationStructureNV bot_level_as(*m_device, bot_level_as_create_info, /*init_memory=*/false);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint64_t), &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateCmdCopyAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure copying.");

    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.instanceCount = 0;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructureNV src_as(*m_device, as_create_info);
    vkt::AccelerationStructureNV dst_as(*m_device, as_create_info);
    vkt::AccelerationStructureNV dst_as_without_mem(*m_device, as_create_info, false);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    const vkt::Buffer bot_level_as_scratch = src_as.CreateScratchBuffer(*m_device);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-src-04963");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        src_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-recording");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();

    // Src must have been created with allow compaction flag
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-src-03411");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV);
    m_errorMonitor->VerifyFound();

    // Dst must have been bound with memory
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-dst-07792");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as_without_mem.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);

    m_errorMonitor->VerifyFound();

    // mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-mode-03410");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR);
    m_errorMonitor->VerifyFound();

    // mode must be a valid VkCopyAccelerationStructureModeKHR value
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-mode-parameter");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR);
    m_errorMonitor->VerifyFound();

    // This command must only be called outside of a render pass instance
    InitRenderTarget();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-renderpass");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_command_buffer.EndRenderPass();
    m_errorMonitor->VerifyFound();

    vkt::DeviceMemory host_memory;
    host_memory.init(*m_device,
                     vkt::DeviceMemory::GetResourceAllocInfo(*m_device, dst_as_without_mem.MemoryRequirements().memoryRequirements,
                                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    VkBindAccelerationStructureMemoryInfoNV bind_info = vku::InitStructHelper();
    bind_info.accelerationStructure = dst_as_without_mem.handle();
    bind_info.memory = host_memory.handle();
    vk::BindAccelerationStructureMemoryNV(*m_device, 1, &bind_info);

    uint64_t handle;
    vk::GetAccelerationStructureHandleNV(*m_device, dst_as_without_mem.handle(), sizeof(uint64_t), &handle);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-buffer-03719");
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), dst_as_without_mem.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyAccelerationStructureNV-buffer-03718");
    const vkt::Buffer bot_level_as_scratch2 = dst_as_without_mem.CreateScratchBuffer(*m_device);
    vk::CmdBuildAccelerationStructureNV(m_command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        dst_as_without_mem.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    vk::CmdCopyAccelerationStructureNV(m_command_buffer.handle(), src_as.handle(), dst_as_without_mem.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, DescriptorBuffers) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    VkDeviceAddress invalid_buffer = CastToHandle<VkDeviceAddress, uintptr_t>(0xbaadbeef);

    uint8_t buffer[128];
    VkDescriptorAddressInfoEXT dai = vku::InitStructHelper();
    dai.address = invalid_buffer;
    dai.range = 64;
    dai.format = VK_FORMAT_R8_UINT;

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dgi.data.pStorageTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08029");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, DescriptorGetInfo) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    uint8_t buffer[128];
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dgi.data.accelerationStructure = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08042");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.accelerationStructureDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}
