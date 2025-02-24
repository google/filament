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

class PositiveRayTracingPipeline : public RayTracingTest {};

TEST_F(PositiveRayTracingPipeline, ShaderGroupsKHR) {
    TEST_DESCRIPTION("Test that no warning is produced when a library is referenced in the raytracing shader groups.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipeline pipeline = VK_NULL_HANDLE;

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = vku::InitStructHelper();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
    stage_create_infos[0] = vku::InitStructHelper();
    stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_infos[0].module = rgen_shader.handle();
    stage_create_infos[0].pName = "main";

    stage_create_infos[1] = vku::InitStructHelper();
    stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_infos[1].module = chit_shader.handle();
    stage_create_infos[1].pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
    group_create_infos[0] = vku::InitStructHelper();
    group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_infos[0].generalShader = 0;
    group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    group_create_infos[1] = vku::InitStructHelper();
    group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to the closest hit shader from the library
    group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 2;
    pipeline_ci.pStages = stage_create_infos;
    pipeline_ci.groupCount = 2;
    pipeline_ci.pGroups = group_create_infos;
    pipeline_ci.layout = empty_pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkResult err =
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_NE(pipeline, VK_NULL_HANDLE);

    vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    vk::DestroyPipeline(m_device->handle(), library, nullptr);
}

TEST_F(PositiveRayTracingPipeline, CacheControl) {
    TEST_DESCRIPTION("Create ray tracing pipeline with VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::pipelineCreationCacheControl);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);
    vk::DestroyPipeline(device(), library, nullptr);
}

TEST_F(PositiveRayTracingPipeline, GetCaptureReplayShaderGroupHandlesKHR) {
    TEST_DESCRIPTION(
        "Regression test for issue 6282: make sure that when validating vkGetRayTracingCaptureReplayShaderGroupHandlesKHR on a "
        "pipeline created using pipeline libraries, the total shader group count is computed using info from the libraries.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::pipelineLibraryGroupHandles);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineShaderGroupHandleCaptureReplay);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipe_lib(*this, m_device);
    rt_pipe_lib.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe_lib.InitLibraryInfo();
    rt_pipe_lib.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.AddGlslMissShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.Build();

    vkt::rt::Pipeline rt_pipe(*this, m_device);
    rt_pipe.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe.InitLibraryInfo();
    rt_pipe.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipe.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipe.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipe.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipe.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe.AddLibrary(rt_pipe_lib);
    rt_pipe.Build();

    // dataSize must be at least groupCount * VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleCaptureReplaySize
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);
    const size_t buffer_size = (3 * ray_tracing_properties.shaderGroupHandleCaptureReplaySize);
    void* out_buffer = malloc(buffer_size);
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.Handle(), 0, 3, buffer_size, out_buffer);
    free(out_buffer);
}
