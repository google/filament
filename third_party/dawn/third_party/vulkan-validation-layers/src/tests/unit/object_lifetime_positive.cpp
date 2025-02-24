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
#include "../framework/descriptor_helper.h"
#include "../framework/pipeline_helper.h"

class PositiveObjectLifetime : public VkLayerTest {};

TEST_F(PositiveObjectLifetime, DestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    RETURN_IF_SKIP(Init());
    vk::DestroyBuffer(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyBufferView(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyCommandPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorSetLayout(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDevice(VK_NULL_HANDLE, NULL);
    vk::DestroyEvent(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFence(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFramebuffer(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImage(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImageView(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyInstance(VK_NULL_HANDLE, NULL);
    vk::DestroyPipeline(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineCache(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineLayout(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyQueryPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyRenderPass(device(), VK_NULL_HANDLE, NULL);
    vk::DestroySampler(device(), VK_NULL_HANDLE, NULL);
    vk::DestroySemaphore(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyShaderModule(device(), VK_NULL_HANDLE, NULL);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[3] = {};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 3, descriptor_sets);

    vk::FreeMemory(device(), VK_NULL_HANDLE, NULL);
}

TEST_F(PositiveObjectLifetime, FreeCommandBuffersNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeCommandBuffers");

    RETURN_IF_SKIP(Init());

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &command_buffer);

    VkCommandBuffer free_command_buffers[2] = {command_buffer, VK_NULL_HANDLE};
    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 2, &free_command_buffers[0]);
}

TEST_F(PositiveObjectLifetime, FreeDescriptorSetsNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeDescriptorSets");

    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1};

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                nullptr};

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    // Only set first set, second is still null
    vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_sets[0]);
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 2, descriptor_sets);
}

TEST_F(PositiveObjectLifetime, DescriptorBufferInfoCopy) {
    TEST_DESCRIPTION("Destroy a buffer then try to copy it in the descriptor set");
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet descriptor_set_1(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();
    buffer.destroy();

    VkCopyDescriptorSet copy_ds_update = vku::InitStructHelper();
    copy_ds_update.srcSet = descriptor_set_0.set_;
    copy_ds_update.srcBinding = 0;
    copy_ds_update.dstSet = descriptor_set_1.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 1;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_ds_update);
}

TEST_F(PositiveObjectLifetime, DescriptorSetMutableBufferDestroyed) {
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    VkDescriptorType desc_types[2] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListEXT type_list = {};
    type_list.descriptorTypeCount = 2;
    type_list.pDescriptorTypes = desc_types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &type_list;

    OneOffDescriptorSet descriptor_set0(m_device, {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}, 0,
                                        &mdtci);
    OneOffDescriptorSet descriptor_set1(m_device,
                                        {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set0.layout_, &descriptor_set1.layout_});

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);  // used
    descriptor_set0.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();

    vkt::Buffer uniform_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);  // not used
    descriptor_set1.WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; };
        // layout(set=1, binding=0) uniform UBO { uint y; };
        void main(){
            x = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    // uniform_buffer.destroy();  // Destroy the UNUSED buffer before it's bound to the cmd buffer

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}
