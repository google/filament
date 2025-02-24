/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
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
#include "../framework/pipeline_helper.h"

class NegativeRayTracingPipelineNV : public RayTracingTest {};

TEST_F(NegativeRayTracingPipelineNV, BasicUsage) {
    TEST_DESCRIPTION("Validate vkCreateRayTracingPipelinesNV and CreateInfo parameters during ray-tracing pipeline creation");
    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());

    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT pipleline_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(pipleline_features);
    // Set this to true as it is a required feature
    pipleline_features.pipelineCreationCacheControl = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    VkShaderObj ahit_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
    VkShaderObj chit_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    VkShaderObj miss_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_MISS_BIT_NV);
    VkShaderObj intr_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_INTERSECTION_BIT_NV);
    VkShaderObj call_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_CALLABLE_BIT_NV);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoNV group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
    group_create_info.generalShader = VK_SHADER_UNUSED_NV;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        pipeline_ci.basePipelineIndex = -1;
        constexpr uint64_t fake_pipeline_id = 0xCADECADE;
        VkPipeline fake_pipeline_handle = CastFromUint64<VkPipeline>(fake_pipeline_id);
        pipeline_ci.basePipelineHandle = fake_pipeline_handle;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07984");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredError("VUID-vkCreateRayTracingPipelinesNV-flags-03415");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-07985");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-02957");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-02904");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03456");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03458");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03459");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03460");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03461");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03462");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03463");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-03588");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV;
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-None-09497");
        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-flags-04948");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    // test for vkCreateRayTracingPipelinesNV
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        // appending twice as it is generated twice in auto-validation code
        m_errorMonitor->SetDesiredError("VUID-vkCreateRayTracingPipelinesNV-createInfoCount-arraylength");
        m_errorMonitor->SetDesiredError("VUID-vkCreateRayTracingPipelinesNV-createInfoCount-arraylength");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 0, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingPipelineNV, BindPoint) {
    TEST_DESCRIPTION("Bind a graphics pipeline in the ray-tracing bind point");

    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipelineBindPoint-02392");

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipe.Handle());

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipelineNV, ShaderGroups) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});

    VkShaderObj rgen_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    VkShaderObj ahit_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
    VkShaderObj chit_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    VkShaderObj miss_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_MISS_BIT_NV);
    VkShaderObj intr_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_INTERSECTION_BIT_NV);
    VkShaderObj call_shader(this, kRayTracingNVMinimalGlsl, VK_SHADER_STAGE_CALLABLE_BIT_NV);

    VkPipeline pipeline = VK_NULL_HANDLE;

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = vku::InitStructHelper();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_info.generalShader = VK_SHADER_UNUSED_NV;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingPipelineCreateInfoNV-stage-06232");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Two raygen stages
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[1].module = rgen_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = 1;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        vk::DestroyPipeline(device(), pipeline, NULL);
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = vku::InitStructHelper();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 5;  // Invalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 5;  // Invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredError("VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417");
        vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingPipelineNV, StageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test NV ray tracing pipeline feedback stage count check.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());

    VkPhysicalDeviceRayTracingPropertiesNV rtnv_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rtnv_props);

    if (rtnv_props.maxDescriptorSetAccelerationStructures < 1) {
        GTEST_SKIP() << "maxDescriptorSetAccelerationStructures < 1";
    }

    RETURN_IF_SKIP(InitState());

    VkPipelineCreationFeedbackCreateInfo feedback_info = vku::InitStructHelper();
    VkPipelineCreationFeedback feedbacks[4] = {};

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](nv::rt::RayTracingPipelineHelper &helper) { helper.rp_ci_.pNext = &feedback_info; };

    feedback_info.pipelineStageCreationFeedbackCount = 3;
    nv::rt::RayTracingPipelineHelper::OneshotPositiveTest(*this, set_feedback);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    nv::rt::RayTracingPipelineHelper::OneshotTest(*this, set_feedback,
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651");
}

TEST_F(NegativeRayTracingPipelineNV, MissingEntrypoint) {
    TEST_DESCRIPTION("Test NV ray tracing pipeline with missing entrypoint.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    char const missShaderText[] = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : enable
        layout(location = 0) rayPayloadInEXT float hitValue;
        void main() {
            hitValue = 0.0;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
    VkShaderObj miss_shader(this, missShaderText, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL, nullptr,
                            "foo");
    m_errorMonitor->VerifyFound();
}
