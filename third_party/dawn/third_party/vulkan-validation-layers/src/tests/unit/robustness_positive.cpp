/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class PositiveRobustness : public VkLayerTest {};

TEST_F(PositiveRobustness, WriteDescriptorSetAccelerationStructureNVNullDescriptor) {
    TEST_DESCRIPTION("Validate using NV acceleration structure descriptor writing with null descriptor.");

    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_MISS_BIT_NV, nullptr},
                                     });

    VkAccelerationStructureNV top_level_as = VK_NULL_HANDLE;

    VkWriteDescriptorSetAccelerationStructureNV acc = vku::InitStructHelper();
    acc.accelerationStructureCount = 1;
    acc.pAccelerationStructures = &top_level_as;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper(&acc);
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}

TEST_F(PositiveRobustness, BindVertexBuffers2EXTNullDescriptors) {
    TEST_DESCRIPTION("Test nullDescriptor works wih CmdBindVertexBuffers variants");

    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkBufferView buffer_view = VK_NULL_HANDLE;
    descriptor_set.WriteDescriptorBufferView(2, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.Clear();

    m_command_buffer.Begin();
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer, &offset);
    vk::CmdBindVertexBuffers2EXT(m_command_buffer.handle(), 0, 1, &buffer, &offset, nullptr, nullptr);
    m_command_buffer.End();
}

TEST_F(PositiveRobustness, PipelineRobustnessRobustImageAccessExposed) {
    TEST_DESCRIPTION("Check if VK_EXT_image_robustness is exposed feature doesn't need to be enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineRobustness);
    RETURN_IF_SKIP(Init());

    VkPipelineRobustnessCreateInfo pipeline_robustness_info = vku::InitStructHelper();
    CreateComputePipelineHelper pipe(*this, &pipeline_robustness_info);
    pipeline_robustness_info.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS;
    pipe.CreateComputePipeline();
}

TEST_F(PositiveRobustness, PipelineRobustnessRobustBufferAccess2Supported) {
    TEST_DESCRIPTION("Create a pipeline using VK_EXT_pipeline_robustness with robustBufferAccess2 being supported");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineRobustness);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(robustness2_features);

    if (!robustness2_features.robustBufferAccess2) {
        GTEST_SKIP() << "robustBufferAccess2 is not supported";
    }

    {
        VkPipelineRobustnessCreateInfo pipeline_robustness_info = vku::InitStructHelper();
        CreateComputePipelineHelper pipe(*this, &pipeline_robustness_info);
        pipeline_robustness_info.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2;
        pipe.CreateComputePipeline();
    }

    {
        VkPipelineRobustnessCreateInfo pipeline_robustness_info = vku::InitStructHelper();
        CreateComputePipelineHelper pipe(*this, &pipeline_robustness_info);
        pipeline_robustness_info.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2;
        pipe.CreateComputePipeline();
    }

    {
        VkPipelineRobustnessCreateInfo pipeline_robustness_info = vku::InitStructHelper();
        CreateComputePipelineHelper pipe(*this, &pipeline_robustness_info);
        pipeline_robustness_info.vertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2;
        pipe.CreateComputePipeline();
    }
}

TEST_F(PositiveRobustness, PipelineRobustnessRobustImageAccess2Supported) {
    TEST_DESCRIPTION("Create a pipeline using VK_EXT_pipeline_robustness with robustImageAccess2 being supported");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineRobustness);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(robustness2_features);

    if (!robustness2_features.robustBufferAccess2) {
        GTEST_SKIP() << "robustBufferAccess2 is not supported";
    }

    VkPipelineRobustnessCreateInfo pipeline_robustness_info = vku::InitStructHelper();
    CreateComputePipelineHelper pipe(*this, &pipeline_robustness_info);
    pipeline_robustness_info.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2;
    pipe.CreateComputePipeline();
}
