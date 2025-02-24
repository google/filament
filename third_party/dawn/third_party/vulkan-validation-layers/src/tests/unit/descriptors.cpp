/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeDescriptors : public VkLayerTest {};

TEST_F(NegativeDescriptors, DescriptorPoolConsistency) {
    TEST_DESCRIPTION("Allocate descriptor sets from one DS pool and attempt to delete them from another.");

    m_errorMonitor->SetDesiredError("VUID-vkFreeDescriptorSets-pDescriptorSets-parent");

    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool bad_pool(*m_device, ds_pool_ci);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    vk::FreeDescriptorSets(device(), bad_pool.handle(), 1, &descriptor_set.set_);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, AllocDescriptorFromEmptyPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    RETURN_IF_SKIP(Init());

    // This test is valid for Vulkan 1.0 only -- skip if device has an API version greater than 1.0.
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // Create Pool w/ 1 Sampler descriptor, but try to alloc Uniform Buffer
    // descriptor from it
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_samp(*m_device, {dsl_binding_samp});

    // Try to allocate 2 sets when pool only has 1 set
    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout_samp.handle(), ds_layout_samp.handle()};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAllocateInfo-apiVersion-07895");
    vk::AllocateDescriptorSets(device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();

    alloc_info.descriptorSetCount = 1;
    // Create layout w/ descriptor type not available in pool
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_ub(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_set;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layout_ub.handle();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAllocateInfo-apiVersion-07896");
    vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, FreeDescriptorFromOneShotPool) {
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    // Not specifying VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT means
    // app can only call vk::ResetDescriptorPool on this pool.;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    VkResult err = vk::AllocateDescriptorSets(device(), &alloc_info, &descriptorSet);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredError("VUID-vkFreeDescriptorSets-descriptorPool-00312");
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 1, &descriptorSet);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorPool) {
    // Attempt to clear Descriptor Pool with bad object.
    // ObjectTracker should catch this.

    RETURN_IF_SKIP(Init());
    m_errorMonitor->SetDesiredError("VUID-vkResetDescriptorPool-descriptorPool-parameter");
    constexpr uint64_t fake_pool_handle = 0xbaad6001;
    VkDescriptorPool bad_pool = CastFromUint64<VkDescriptorPool>(fake_pool_handle);
    vk::ResetDescriptorPool(device(), bad_pool, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSet) {
    // Attempt to bind an invalid Descriptor Set to a valid Command Buffer
    // ObjectTracker should catch this.
    // Create a valid cmd buffer
    // call vk::CmdBindDescriptorSets w/ false Descriptor Set
    RETURN_IF_SKIP(Init());

    constexpr uint64_t fake_set_handle = 0xbaad6001;
    VkDescriptorSet bad_set = CastFromUint64<VkDescriptorSet>(fake_set_handle);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout descriptor_set_layout(*m_device, {layout_binding});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout});

    m_command_buffer.Begin();
    // Set invalid set
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-parameter");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &bad_set,
                              0, NULL);
    m_errorMonitor->VerifyFound();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                 });
    VkDescriptorSet good_set = descriptor_set.set_;

    // Set out of range firstSet and descriptorSetCount sum
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-firstSet-00360");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 2, 1, &good_set,
                              0, NULL);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DescriptorSetLayout) {
    // Attempt to create a Pipeline Layout with an invalid Descriptor Set Layout.
    // ObjectTracker should catch this.
    constexpr uint64_t fake_layout_handle = 0xbaad6001;
    VkDescriptorSetLayout bad_layout = CastFromUint64<VkDescriptorSetLayout>(fake_layout_handle);
    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-parameter");
    RETURN_IF_SKIP(Init());
    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &bad_layout;
    vk::CreatePipelineLayout(device(), &plci, NULL, &pipeline_layout);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetNullBufferInfo) {
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00324");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetTypeStageMatch) {
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;

    vkt::Buffer uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_infos[5] = {};
    for (int i = 0; i < 5; ++i) {
        buffer_infos[i] = {uniform_buffer, 0, VK_WHOLE_SIZE};
    }
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = buffer_infos;

    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 2;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorCount-00317");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstBinding = 2;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 5;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorCount-00317");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstArrayElement = 1;
    descriptor_write.descriptorCount = 4;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorCount-00317");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetImmutableSamplerMix) {
    RETURN_IF_SKIP(Init());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &sampler.handle()}});

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 2;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorImageInfo image_infos[2] = {
        {sampler, image_view, VK_IMAGE_LAYOUT_GENERAL},
        {VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_GENERAL},
    };
    descriptor_write.pImageInfo = image_infos;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorCount-00318");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetIntegrity) {
    RETURN_IF_SKIP(Init());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()},
                                        {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                        {4, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.descriptorCount = 1;
    descriptor_write.dstArrayElement = 0;

    // Attmept write with incorrect layout for sampled descriptor
    VkDescriptorImageInfo image_info = {VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_UNDEFINED};
    descriptor_write.pImageInfo = &image_info;

    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-04149");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstBinding = 3;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_info.sampler = sampler.handle();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-04150");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstBinding = 4;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-04151");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteImmutableSampler) {
    RETURN_IF_SKIP(Init());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()},
                                       });

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02752");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetIdentitySwizzle) {
    TEST_DESCRIPTION("Test descriptors that need to have identity swizzle set");
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image;
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // G and B are swizzled
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_B;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_G;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;

    vkt::ImageView image_view(*m_device, image_view_ci);
    descriptor_set.WriteDescriptorImageInfo(0, image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00336");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetConsecutiveUpdates) {
    TEST_DESCRIPTION(
        "Verifies that updates rolling over to next descriptor work correctly by destroying buffer from consecutive update known "
        "to be used in descriptor set and verifying that error is flagged.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    vkt::Buffer buffer0(*m_device, 2048, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    CreatePipelineHelper pipe(*this);
    {  // Scope 2nd buffer to cause early destruction
        vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        VkDescriptorBufferInfo buffer_info[3] = {};
        buffer_info[0].buffer = buffer0.handle();
        buffer_info[0].offset = 0;
        buffer_info[0].range = 1024;
        buffer_info[1].buffer = buffer0.handle();
        buffer_info[1].offset = 1024;
        buffer_info[1].range = 1024;
        buffer_info[2].buffer = buffer1.handle();
        buffer_info[2].offset = 0;
        buffer_info[2].range = 1024;

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_set.set_;  // descriptor_set;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 3;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = buffer_info;

        // Update descriptor
        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

        // Create PSO that uses the uniform buffers
        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
            layout(set=0) layout(binding=1) uniform blah { int x; } duh;
            void main(){
               x = vec4(duh.x, bar.y, bar.x, 1);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
        pipe.CreateGraphicsPipeline();

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);

        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
    // buffer2 just went out of scope and was destroyed
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, CmdBufferDescriptorSetBufferDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a bound descriptor set with a buffer dependency being "
        "destroyed.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    {
        // Create a buffer to update the descriptor with
        vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        // Create PSO to be used for draw-time errors below
        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
            void main(){
               x = vec4(bar.y);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.CreateGraphicsPipeline();

        // Correctly update descriptor to avoid "NOT_UPDATED" error
        pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024);
        pipe.descriptor_set_->UpdateDescriptorSets();

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
    // Destroy buffer should invalidate the cmd buffer, causing error on submit

    // Attempt to submit cmd buffer
    // Invalid VkBuffer
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

// This is similar to the CmdBufferDescriptorSetBufferDestroyed test above except that the buffer
// is destroyed before recording the Draw cmd.
TEST_F(NegativeDescriptors, DrawDescriptorSetBufferDestroyed) {
    TEST_DESCRIPTION("Attempt to bind a descriptor set that is invalid at Draw time due to its buffer dependency being destroyed.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    {
        // Create a buffer to update the descriptor with
        vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        // Create PSO to be used for draw-time errors below
        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
            void main(){
               x = vec4(bar.y);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.CreateGraphicsPipeline();

        // Correctly update descriptor to avoid "NOT_UPDATED" error
        pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024);
        pipe.descriptor_set_->UpdateDescriptorSets();
    }

    // The buffer has now been destroyed, but it has been written into the descriptor set.

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    // Invalid VkBuffer - The check is made at Draw time.
    m_errorMonitor->SetDesiredError("that is invalid or has been destroyed");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, CmdBufferDescriptorSetImageSamplerDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a bound descriptor sets with a combined image sampler having "
        "their image, sampler, and descriptor set each respectively destroyed and then attempting to submit associated cmd "
        "buffers. Attempt to destroy a DescriptorSet that is in use.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}},
                                       0, nullptr, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create images to update the descriptor with
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    auto image_create_info = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image tmp_image(*m_device, image_create_info, vkt::no_mem);
    vkt::Image image2(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements memory_reqs;
    bool pass;
    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(device(), tmp_image.handle(), &memory_reqs);
    // Allocate enough memory for both images
    VkDeviceSize align_mod = memory_reqs.size % memory_reqs.alignment;
    VkDeviceSize aligned_size = ((align_mod == 0) ? memory_reqs.size : (memory_reqs.size + memory_reqs.alignment - align_mod));
    memory_info.allocationSize = aligned_size * 2;
    pass = m_device->Physical().SetMemoryType(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory image_memory(*m_device, memory_info);

    tmp_image.BindMemory(image_memory, 0);
    // Bind second image to memory right after first image
    image2.BindMemory(image_memory, aligned_size);

    // First test deletes this view
    vkt::ImageView tmp_view = tmp_image.CreateView();
    vkt::ImageView view = tmp_image.CreateView();

    vkt::ImageView view2 = image2.CreateView();

    vkt::Sampler tmp_sampler(*m_device, SafeSaneSamplerCreateInfo());
    vkt::Sampler sampler2(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = tmp_sampler.handle();
    img_info.imageView = tmp_view.handle();
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    // First error case is destroying sampler prior to cmd buffer submission
    m_command_buffer.Begin();

    // Transit image layout from VK_IMAGE_LAYOUT_UNDEFINED into VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = tmp_image.handle();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // This first submit should be successful
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Now destroy imageview and reset cmdBuffer
    tmp_view.destroy();

    m_command_buffer.Reset(0);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError(" that is invalid or has been destroyed.");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Re-update descriptor with new view
    img_info.imageView = view.handle();
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    // Now test destroying sampler prior to cmd buffer submission
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Destroy sampler invalidates the cmd buffer, causing error on submit
    tmp_sampler.destroy();
    // Attempt to submit cmd buffer
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Now re-update descriptor with valid sampler and delete image
    img_info.sampler = sampler2.handle();
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_command_buffer.Begin(&info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Destroy image invalidates the cmd buffer, causing error on submit
    tmp_image.destroy();

    // Attempt to submit cmd buffer
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    // Now update descriptor to be valid, but then update and free descriptor
    img_info.imageView = view2.handle();
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_command_buffer.Begin(&info);

    // Transit image2 layout from VK_IMAGE_LAYOUT_UNDEFINED into VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    barrier.image = image2.handle();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    // Immediately try to update the descriptor set in the active command buffer - failure expected
    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-None-03047");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Immediately try to destroy the descriptor set in the active command buffer - failure expected
    m_errorMonitor->SetDesiredError("VUID-vkFreeDescriptorSets-pDescriptorSets-00309");
    vk::FreeDescriptorSets(device(), descriptor_set.pool_, 1, &descriptor_set.set_);
    m_errorMonitor->VerifyFound();

    // Try again once the queue is idle - should succeed w/o error
    // TODO - though the particular error above doesn't re-occur, there are other 'unexpecteds' still to clean up
    m_default_queue->Wait();
    m_errorMonitor->SetUnexpectedError(
        "pDescriptorSets must be a valid pointer to an array of descriptorSetCount VkDescriptorSet handles, each element of which "
        "must either be a valid handle or VK_NULL_HANDLE");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorSet obj");
    vk::FreeDescriptorSets(device(), descriptor_set.pool_, 1, &descriptor_set.set_);

    // Attempt to submit cmd buffer containing the freed descriptor set
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, OpArrayLengthStaticallyUsed) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7137");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        layout(set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };

        layout(set = 1, binding = 0) buffer SSBO_1 {
            uint b;
            vec4 c[];
        };

        void main() {
            // length() here is consider static usage of the descriptor
            a = c.length();
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet descriptor_set1(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set0.layout_, &descriptor_set1.layout_});
    descriptor_set0.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();
    descriptor_set1.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 1, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    descriptor_set1.UpdateDescriptorSets();  // command buffer is invalid now

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSetSamplerDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a bound descriptor sets with a combined image sampler where sampler has been deleted.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    // Create images to update the descriptor with
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    vkt::Image image(*m_device, 32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);
    vkt::Sampler sampler1(*m_device, sampler_ci);

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.WriteDescriptorImageInfo(1, view, sampler1);
    descriptor_set.UpdateDescriptorSets();

    // Destroy the sampler before it's bound to the cmd buffer
    sampler1.destroy();

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        layout(set=0, binding=1) uniform sampler2D s1;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
           x = texture(s1, vec2(1));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    // First error case is destroying sampler prior to cmd buffer submission
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, ImageDescriptorLayoutMismatch) {
    TEST_DESCRIPTION("Create an image sampler layout->image layout mismatch within/without a command buffer");

    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool maintenance2 = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    VkDescriptorSet descriptorSet = descriptor_set.set_;

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto image_view_create_info = image.BasicViewCreatInfo();
    vkt::ImageView view(*m_device, image_view_create_info);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    // Setup structure for descriptor update with sampler, for update in do_test below
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler.handle();

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptorSet;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buf(*m_device, m_command_pool);

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    constexpr std::array test_list = {kInternal, kExternal};
    constexpr std::array internal_errors = {"VUID-VkDescriptorImageInfo-imageLayout-00344", "VUID-vkCmdDraw-None-08114"};
    constexpr std::array external_errors = {"VUID-vkCmdDraw-None-09600"};

    // Common steps to create the two classes of errors (or two classes of positives)
    auto do_test = [&](vkt::Image *image, vkt::ImageView *view, VkImageAspectFlags aspect_mask, VkImageLayout image_layout,
                       VkImageLayout descriptor_layout, const bool positive_test) {
        // Set up the descriptor
        img_info.imageView = view->handle();
        img_info.imageLayout = descriptor_layout;
        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

        for (TestType test_type : test_list) {
            cmd_buf.Begin();
            // record layout different than actual descriptor layout.
            const VkFlags read_write = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            auto image_barrier = image->ImageMemoryBarrier(read_write, read_write, VK_IMAGE_LAYOUT_UNDEFINED, image_layout,
                                                           image->SubresourceRange(aspect_mask));
            vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                                   nullptr, 0, nullptr, 1, &image_barrier);
            image->Layout(image_layout);

            if (test_type == kExternal) {
                // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
                cmd_buf.End();
                m_default_queue->Submit(cmd_buf);
                m_default_queue->Wait();
                cmd_buf.Begin();
            }

            cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
            vk::CmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                      &descriptorSet, 0, NULL);

            // At draw time the update layout will mis-match the actual layout
            if (positive_test || (test_type == kExternal)) {
            } else {
                for (const auto &err : internal_errors) {
                    m_errorMonitor->SetDesiredError(err);
                }
            }
            vk::CmdDraw(cmd_buf.handle(), 1, 0, 0, 0);
            if (positive_test || (test_type == kExternal)) {
            } else {
                m_errorMonitor->VerifyFound();
            }

            cmd_buf.EndRenderPass();
            cmd_buf.End();

            // Submit cmd buffer
            if (positive_test || (test_type == kInternal)) {
            } else {
                for (const auto &err : external_errors) {
                    m_errorMonitor->SetDesiredError(err);
                }
            }
            m_default_queue->Submit(cmd_buf);
            m_default_queue->Wait();
            if (positive_test || (test_type == kInternal)) {
            } else {
                m_errorMonitor->VerifyFound();
            }
        }
    };
    do_test(&image, &view, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, /* positive */ false);

    // Create depth stencil image and views
    const VkFormat format_ds = m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    bool ds_test_support = maintenance2 && (format_ds != VK_FORMAT_UNDEFINED);
    const VkImageLayout ds_image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    const VkImageLayout depth_descriptor_layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    const VkImageLayout stencil_descriptor_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    const VkImageAspectFlags depth_stencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    if (ds_test_support) {
        vkt::Image image_ds(*m_device, 32, 32, 1, format_ds,
                            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        auto ds_view_ci = image_ds.BasicViewCreatInfo(VK_IMAGE_ASPECT_DEPTH_BIT);
        vkt::ImageView depth_view(*m_device, ds_view_ci);
        ds_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        vkt::ImageView stencil_view(*m_device, ds_view_ci);
        do_test(&image_ds, &depth_view, depth_stencil, ds_image_layout, depth_descriptor_layout, /* positive */ true);
        do_test(&image_ds, &depth_view, depth_stencil, ds_image_layout, VK_IMAGE_LAYOUT_GENERAL, /* positive */ false);
        do_test(&image_ds, &stencil_view, depth_stencil, ds_image_layout, stencil_descriptor_layout, /* positive */ true);
        do_test(&image_ds, &stencil_view, depth_stencil, ds_image_layout, VK_IMAGE_LAYOUT_GENERAL, /* positive */ false);
    }
}

TEST_F(NegativeDescriptors, DescriptorPoolInUseResetSignaled) {
    TEST_DESCRIPTION("Reset a DescriptorPool with a DescriptorSet that is in use.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create image to update the descriptor with
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    // Update descriptor with image and sampler
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Submit cmd buffer to put pool in-flight
    m_default_queue->Submit(m_command_buffer);
    // Reset pool while in-flight, causing error
    m_errorMonitor->SetDesiredError("VUID-vkResetDescriptorPool-descriptorPool-00313");
    vk::ResetDescriptorPool(device(), descriptor_set.pool_, 0);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeDescriptors, DescriptorImageUpdateNoMemoryBound) {
    TEST_DESCRIPTION("Attempt an image descriptor set update where image's bound memory has been freed.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    // Create images to update the descriptor with
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    auto image_create_info = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    // Create with bound memory to avoid error at bind view time. We'll break binding before update.
    vkt::Image image(*m_device, image_create_info);
    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    // Update descriptor with image and sampler
    descriptor_set.WriteDescriptorImageInfo(0, view.handle(), sampler.handle());
    // Break memory binding and attempt update
    image.Memory().destroy();

    m_errorMonitor->SetDesiredError("UNASSIGNED-VkDescriptorImageInfo-BoundResourceFreedMemoryAccess");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DynamicOffsetCases) {
    // Create a descriptorSet w/ dynamic descriptor and then hit 3 offset error
    // cases:
    // 1. No dynamicOffset supplied
    // 2. Too many dynamicOffsets supplied
    // 3. Dynamic offset oversteps buffer being updated

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create a buffer to update the descriptor with
    vkt::Buffer dynamic_uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer.handle(), 0, 1024,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->VerifyFound();
    uint32_t pDynOff[2] = {0, 756};
    // Now cause error b/c too many dynOffsets in array for # of dyn descriptors
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 2, pDynOff);
    m_errorMonitor->VerifyFound();
    pDynOff[0] = 512;
    // Finally cause error due to dynamicOffset being too big
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentUniformGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // This update should succeed, but offset size of 512 will overstep buffer
    // /w range 1024 & size 1024
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 1, pDynOff);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DynamicOffsetCasesMaintenance6) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer dynamic_uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer.handle(), 0, 1024,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkBindDescriptorSetsInfo bind_ds_info = vku::InitStructHelper();
    bind_ds_info.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bind_ds_info.layout = pipeline_layout.handle();
    bind_ds_info.firstSet = 0;
    bind_ds_info.descriptorSetCount = 1;
    bind_ds_info.pDescriptorSets = &descriptor_set.set_;
    bind_ds_info.dynamicOffsetCount = 0;
    bind_ds_info.pDynamicOffsets = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkBindDescriptorSetsInfo-dynamicOffsetCount-00359");
    vk::CmdBindDescriptorSets2KHR(m_command_buffer.handle(), &bind_ds_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, BindDescriptorSetsInfoPipelineLayout) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::Buffer dynamic_uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer.handle(), 0, 1024,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkBindDescriptorSetsInfo bind_ds_info = vku::InitStructHelper();
    bind_ds_info.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bind_ds_info.layout = CastToHandle<VkPipelineLayout, uintptr_t>(0xbaadb1be);
    bind_ds_info.firstSet = 0;
    bind_ds_info.descriptorSetCount = 1;
    bind_ds_info.pDescriptorSets = &descriptor_set.set_;
    bind_ds_info.dynamicOffsetCount = 0;
    bind_ds_info.pDynamicOffsets = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkBindDescriptorSetsInfo-layout-parameter");
    vk::CmdBindDescriptorSets2KHR(m_command_buffer.handle(), &bind_ds_info);
    m_errorMonitor->VerifyFound();

    bind_ds_info.layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkBindDescriptorSetsInfo-None-09495");
    vk::CmdBindDescriptorSets2KHR(m_command_buffer.handle(), &bind_ds_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorBufferUpdateNoMemoryBound) {
    TEST_DESCRIPTION("Attempt to update a descriptor with a non-sparse buffer that doesn't have memory bound");

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00329");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    // Create a buffer to update the descriptor with
    VkBufferCreateInfo buffCI = vku::InitStructHelper();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer dynamic_uniform_buffer(*m_device, buffCI, vkt::no_mem);

    // Attempt to update descriptor without binding memory to it
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer.handle(), 0, 1024,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DynamicDescriptorSet) {
    RETURN_IF_SKIP(Init());

    const VkDeviceSize partial_size = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    const VkDeviceSize buffer_size = partial_size * 10;  // make sure way more then alignment multiple

    // Create a buffer to update the descriptor with
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // test various uses of offsets and size
    // The non-dynamic binds are there to make sure pDynamicOffsets are matched correctly at bind time
    // clang-format off
    OneOffDescriptorSet descriptor_set_0(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL, nullptr},
        // Gap to ensure looping for binding index is correct
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr}  // pDynamicOffsets[0]
    });
    OneOffDescriptorSet descriptor_set_1(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL, nullptr},
        // This dynamic type has a descriptorCount of 0 which will be skipped
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0, VK_SHADER_STAGE_ALL, nullptr},
    });
    OneOffDescriptorSet descriptor_set_2(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},  // pDynamicOffsets[1]
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL, nullptr},
        // [2] and [3] are same, but tests descriptor arrays
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2, VK_SHADER_STAGE_ALL, nullptr},  // pDynamicOffsets[2]/[3]
        {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr}   // pDynamicOffsets[4]
    });
    // clang-format on
    const vkt::PipelineLayout pipeline_layout(*m_device,
                                              {&descriptor_set_0.layout_, &descriptor_set_1.layout_, &descriptor_set_2.layout_});
    const VkPipelineLayout layout = pipeline_layout.handle();

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);  // non-dynamic
    descriptor_set_1.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);  // non-dynamic
    descriptor_set_2.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE);  // non-dynamic

    // buffer[0, max]
    descriptor_set_0.WriteDescriptorBufferInfo(2, buffer.handle(), 0, VK_WHOLE_SIZE,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);  // pDynamicOffsets[0]
    // buffer[alignment, max]
    descriptor_set_2.WriteDescriptorBufferInfo(0, buffer.handle(), partial_size, VK_WHOLE_SIZE,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);  // pDynamicOffsets[1]
    // buffer[0, max - alignment]
    descriptor_set_2.WriteDescriptorBufferInfo(2, buffer.handle(), 0, buffer_size - partial_size,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);  // pDynamicOffsets[2]s
    // buffer[0, max - alignment]
    descriptor_set_2.WriteDescriptorBufferInfo(2, buffer.handle(), 0, buffer_size - partial_size,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);  // pDynamicOffsets[3]
    // buffer[alignment, max - alignment]
    descriptor_set_2.WriteDescriptorBufferInfo(3, buffer.handle(), partial_size, buffer_size - (partial_size * 2),
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);  // pDynamicOffsets[4]

    descriptor_set_0.UpdateDescriptorSets();
    descriptor_set_1.UpdateDescriptorSets();
    descriptor_set_2.UpdateDescriptorSets();

    m_command_buffer.Begin();

    VkDescriptorSet descriptorSets[3] = {descriptor_set_0.set_, descriptor_set_1.set_, descriptor_set_2.set_};
    uint32_t offsets[5] = {0, 0, 0, 0, 0};

    if (partial_size > 1) {
        // non multiple of alignment
        offsets[4] = 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01971");
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5,
                                  offsets);
        m_errorMonitor->VerifyFound();
        offsets[4] = 0;
    }

    // Larger than buffer
    const uint32_t partial_size32 = static_cast<uint32_t>(partial_size);
    offsets[0] = partial_size32;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-06715");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[0] = 0;

    // Larger than buffer
    offsets[1] = partial_size32;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-06715");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[1] = 0;

    // Makes the range the same size of buffer which is valid
    offsets[2] = partial_size32;
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);

    // Now an extra increment larger than buffer
    offsets[2] = partial_size32 * 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[2] = 0;

    // Same thing but with [3] to test descriptor arrays
    offsets[3] = partial_size32;
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);

    offsets[3] = partial_size32 * 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[3] = 0;

    // range should be at end of buffer (same size)
    offsets[4] = partial_size32;
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);

    // Now an extra increment larger than buffer
    // tests (offset + range + dynamic_offset)
    offsets[4] = partial_size32 * 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DynamicOffsetWithNullBuffer) {
    TEST_DESCRIPTION("Create a descriptorSet w/ dynamic descriptors where 1 binding is inactive, but all have null buffers");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    // Update descriptors
    const uint32_t BINDING_COUNT = 3;
    VkDescriptorBufferInfo buff_info[BINDING_COUNT] = {};
    buff_info[0].buffer = VK_NULL_HANDLE;
    buff_info[0].offset = 0;
    buff_info[0].range = 256;
    buff_info[1].buffer = VK_NULL_HANDLE;
    buff_info[1].offset = 256;
    buff_info[1].range = 512;
    buff_info[2].buffer = VK_NULL_HANDLE;
    buff_info[2].offset = 0;
    buff_info[2].range = 512;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = BINDING_COUNT;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptor_write.pBufferInfo = buff_info;

    // all 3 descriptors produce this error
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-02998");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-02998");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-02998");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0) layout(binding=0) uniform foo1 { int x; int y; } bar1;
        layout(set=0) layout(binding=2) uniform foo2 { int x; int y; } bar2;
        void main(){
           x = vec4(bar1.y) + vec4(bar2.y);
        }
    )glsl";
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    uint32_t dyn_off[BINDING_COUNT] = {0, 1024, 256};
    // The 2 active descriptors produce this error
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, BINDING_COUNT, dyn_off);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DSBufferInfo) {
    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    const VkDeviceSize min_alignment = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    vkt::Buffer buffer(*m_device, min_alignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Cause error due to offset out of range
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, min_alignment, VK_WHOLE_SIZE);
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-offset-00340");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    // Now cause error due to range of 0
    descriptor_set.Clear();
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, 0);
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00341");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    // Now cause error due to range exceeding buffer size - offset
    descriptor_set.Clear();
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, min_alignment + 1);
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00342");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DSBufferInfoTemplate) {
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    const VkDeviceSize min_alignment = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    vkt::Buffer buffer(*m_device, min_alignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Setup for update w/ template tests
    // Create a template of descriptor set updates
    struct SimpleTemplateData {
        uint8_t padding[7];
        VkDescriptorBufferInfo buffer_info;
        uint32_t other_padding[4];
    };
    SimpleTemplateData update_template_data = {};
    update_template_data.buffer_info.buffer = buffer;

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buffer_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();
    vkt::DescriptorUpdateTemplate update_template(*m_device, update_template_ci);

    // Cause error due to offset out of range
    update_template_data.buffer_info.offset = min_alignment;
    update_template_data.buffer_info.range = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-offset-00340");
    vk::UpdateDescriptorSetWithTemplateKHR(device(), descriptor_set.set_, update_template, &update_template_data);
    m_errorMonitor->VerifyFound();

    // Now cause error due to range of 0
    update_template_data.buffer_info.offset = 0;
    update_template_data.buffer_info.range = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00341");
    vk::UpdateDescriptorSetWithTemplateKHR(device(), descriptor_set.set_, update_template, &update_template_data);
    m_errorMonitor->VerifyFound();

    // Now cause error due to range exceeding buffer size - offset
    update_template_data.buffer_info.offset = 0;
    update_template_data.buffer_info.range = min_alignment + 1;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00342");
    vk::UpdateDescriptorSetWithTemplateKHR(device(), descriptor_set.set_, update_template, &update_template_data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, BindInvalidPipelineLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6621");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0, binding=1) uniform foo1 { vec4 y; };
        void main(){
           x = y;
        }
    )glsl";
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    // consume VU so we create a pipeline handle
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGraphicsPipelineCreateInfo-layout-07988");
    pipe.CreateGraphicsPipeline();
    if (pipe.Handle() == VK_NULL_HANDLE) {
        GTEST_SKIP() << "Driver failed to create a invalid pipeline handle";
    }

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, ConstantArrayElementNotBound) {
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    // Don't bind 2nd element which might be accessed so an error will occur
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.UpdateDescriptorSets();

    const char vs_source[] = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform ufoo { uint index; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data; } Data[2];
        void main() {
            Data[index].data = 0xdeadca71;
        }
    )glsl";

    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8918
TEST_F(NegativeDescriptors, DISABLED_MutableBufferUpdate) {
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

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}, 0,
                                       &mdtci);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; };
        void main() {
            x = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, UpdateDescriptorSetMismatchType) {
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, m_device->Physical().limits_.minUniformBufferOffsetAlignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr}});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // wrong type
    descriptor_set.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00319");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSetCompatibility) {
    // Test various desriptorSet errors with bad binding combinations
    VkResult err;

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static const uint32_t NUM_DESCRIPTOR_TYPES = 5;
    VkDescriptorPoolSize ds_type_count[NUM_DESCRIPTOR_TYPES] = {};
    ds_type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count[0].descriptorCount = 10;
    ds_type_count[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    ds_type_count[1].descriptorCount = 2;
    ds_type_count[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    ds_type_count[2].descriptorCount = 2;
    ds_type_count[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count[3].descriptorCount = 5;
    // TODO : LunarG ILO driver currently asserts in desc.c w/ INPUT_ATTACHMENT
    // type
    // ds_type_count[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    ds_type_count[4].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    ds_type_count[4].descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 5;
    ds_pool_ci.poolSizeCount = NUM_DESCRIPTOR_TYPES;
    ds_pool_ci.pPoolSizes = ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    static const uint32_t MAX_DS_TYPES_IN_LAYOUT = 2;
    VkDescriptorSetLayoutBinding dsl_binding[MAX_DS_TYPES_IN_LAYOUT] = {};
    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[0].descriptorCount = 5;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[0].pImmutableSamplers = NULL;

    // Create layout identical to set0 layout but w/ different stageFlags
    VkDescriptorSetLayoutBinding dsl_fs_stage_only = {};
    dsl_fs_stage_only.binding = 0;
    dsl_fs_stage_only.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_fs_stage_only.descriptorCount = 5;
    dsl_fs_stage_only.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // Different stageFlags to cause error at
                                                                  // bind time
    dsl_fs_stage_only.pImmutableSamplers = NULL;

    std::vector<vkt::DescriptorSetLayout> ds_layouts;
    // Create 4 unique layouts for full pipelineLayout, and 1 special fs-only
    // layout for error case
    ds_layouts.emplace_back(*m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    const vkt::DescriptorSetLayout ds_layout_fs_only(*m_device, {dsl_fs_stage_only});

    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dsl_binding[0].descriptorCount = 2;
    dsl_binding[1].binding = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dsl_binding[1].descriptorCount = 2;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[1].pImmutableSamplers = NULL;
    ds_layouts.emplace_back(*m_device, std::vector<VkDescriptorSetLayoutBinding>({dsl_binding[0], dsl_binding[1]}));

    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding[0].descriptorCount = 5;
    ds_layouts.emplace_back(*m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dsl_binding[0].descriptorCount = 2;
    ds_layouts.emplace_back(*m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    const auto &ds_vk_layouts = MakeVkHandles<VkDescriptorSetLayout>(ds_layouts);

    static const uint32_t NUM_SETS = 4;
    VkDescriptorSet descriptorSet[NUM_SETS] = {};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.descriptorSetCount = ds_vk_layouts.size();
    alloc_info.pSetLayouts = ds_vk_layouts.data();
    err = vk::AllocateDescriptorSets(device(), &alloc_info, descriptorSet);
    ASSERT_EQ(VK_SUCCESS, err);
    VkDescriptorSet ds0_fs_only = {};
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layout_fs_only.handle();
    err = vk::AllocateDescriptorSets(device(), &alloc_info, &ds0_fs_only);
    ASSERT_EQ(VK_SUCCESS, err);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layouts[0], &ds_layouts[1]});
    // Create pipelineLayout with only one setLayout
    const vkt::PipelineLayout single_pipe_layout(*m_device, {&ds_layouts[0]});
    // Create pipelineLayout with 2 descriptor setLayout at index 0
    const vkt::PipelineLayout pipe_layout_one_desc(*m_device, {&ds_layouts[3]});
    // Create pipelineLayout with 5 SAMPLER descriptor setLayout at index 0
    const vkt::PipelineLayout pipe_layout_five_samp(*m_device, {&ds_layouts[2]});
    // Create pipelineLayout with UB type, but stageFlags for FS only
    vkt::PipelineLayout pipe_layout_fs_only(*m_device, {&ds_layout_fs_only});
    // Create pipelineLayout w/ incompatible set0 layout, but set1 is fine
    const vkt::PipelineLayout pipe_layout_bad_set0(*m_device, {&ds_layout_fs_only, &ds_layouts[1]});

    // Add buffer binding for UBO
    vkt::Buffer buffer(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptorSet[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentUniformGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipe_layout_fs_only.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // TODO : Want to cause various binding incompatibility issues here to test
    // DrawState
    //  First cause various verify_layout_compatibility() fails
    //  Second disturb early and late sets and verify INFO msgs
    // VerifySetLayoutCompatibility fail cases:
    // 1. invalid VkPipelineLayout (layout) passed into vk::CmdBindDescriptorSets
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-layout-parameter");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                              CastToHandle<VkPipelineLayout, uintptr_t>(0xbaadb1be), 0, 1, &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 2. layoutIndex exceeds # of layouts in layout
    m_errorMonitor->SetDesiredError(" attempting to bind set to index 1");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindDescriptorSets-firstSet-00360");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, single_pipe_layout.handle(), 0, 2,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 3. Pipeline setLayout[0] has 2 descriptors, but set being bound has 5
    // descriptors
    m_errorMonitor->SetDesiredError(" has 2 total descriptors, but ");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_one_desc.handle(), 0, 1,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 4. same # of descriptors but mismatch in type
    m_errorMonitor->SetDesiredError(" is type VK_DESCRIPTOR_TYPE_SAMPLER but binding ");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_five_samp.handle(), 0, 1,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 5. same # of descriptors but mismatch in stageFlags
    m_errorMonitor->SetDesiredError(" has stageFlags VK_SHADER_STAGE_FRAGMENT_BIT but binding 0 for ");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_fs_only.handle(), 0, 1,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // Now that we're done actively using the pipelineLayout that gfx pipeline
    //  was created with, we should be able to delete it. Do that now to verify
    //  that validation obeys pipelineLayout lifetime
    pipe_layout_fs_only.destroy();

    // Cause draw-time errors due to PSO incompatibilities
    // 1. Error due to not binding required set (we actually use same code as
    // above to disturb set0)
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 2,
                              &descriptorSet[0], 0, NULL);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_bad_set0.handle(), 1, 1,
                              &descriptorSet[1], 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08600");

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // 2. Error due to bound set not being compatible with PSO's
    // VkPipelineLayout (diff stageFlags in this case)
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 2,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08600");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Remaining clean-up
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DescriptorSetCompatibilityCompute) {
    RETURN_IF_SKIP(Init());

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Buffer uniform_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set_storage(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                               });
    OneOffDescriptorSet descriptor_set_uniform(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                               });
    descriptor_set_storage.WriteDescriptorBufferInfo(0, storage_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_storage.UpdateDescriptorSets();
    descriptor_set_uniform.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_set_uniform.UpdateDescriptorSets();

    const vkt::PipelineLayout pipeline_layout_a(*m_device, {&descriptor_set_storage.layout_, &descriptor_set_storage.layout_});
    const vkt::PipelineLayout pipeline_layout_b(*m_device, {&descriptor_set_storage.layout_, &descriptor_set_uniform.layout_});

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 1, binding = 0) buffer StorageBuffer_1 {
            uint a;
            uint b;
        };
        void main() {
            a = b;
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipeline.cp_ci_.layout = pipeline_layout_a.handle();
    pipeline.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_a.handle(), 0, 1,
                              &descriptor_set_storage.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_b.handle(), 1, 1,
                              &descriptor_set_uniform.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08600");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DescriptorSetCompatibilityMutableDescriptors) {
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    VkDescriptorType descriptor_types_0[] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};
    VkDescriptorType descriptor_types_1[] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_SAMPLER};

    VkMutableDescriptorTypeListEXT type_list = {};
    type_list.descriptorTypeCount = 2;
    type_list.pDescriptorTypes = descriptor_types_0;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &type_list;

    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         0, &mdtci);
    const vkt::PipelineLayout pipeline_layout_0(*m_device, {&descriptor_set_0.layout_});

    type_list.pDescriptorTypes = descriptor_types_1;
    OneOffDescriptorSet descriptor_set_1(m_device, {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         0, &mdtci);
    const vkt::PipelineLayout pipeline_layout_1(*m_device, {&descriptor_set_1.layout_});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();
    descriptor_set_1.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_1.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO {
            uint a;
        };
        void main() {
            a = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipeline.cp_ci_.layout = pipeline_layout_0.handle();
    pipeline.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_1.handle(), 0, 1,
                              &descriptor_set_1.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08600");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DSUsageBits) {
    TEST_DESCRIPTION("Attempt to update descriptor sets for images and buffers that do not have correct usage bits sets.");

    RETURN_IF_SKIP(Init());

    const VkFormat buffer_format = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), buffer_format, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT for this format";
    }

    constexpr uint32_t kLocalDescriptorTypeRangeSize = (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1);

    std::array<VkDescriptorPoolSize, kLocalDescriptorTypeRangeSize> ds_type_count;
    for (uint32_t i = 0; i < ds_type_count.size(); ++i) {
        ds_type_count[i].type = VkDescriptorType(i);
        ds_type_count[i].descriptorCount = 1;
    }

    vkt::DescriptorPool ds_pool(*m_device, vkt::DescriptorPool::CreateInfo(0, kLocalDescriptorTypeRangeSize, ds_type_count));
    ASSERT_TRUE(ds_pool.initialized());

    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings(1);
    dsl_bindings[0].binding = 0;
    dsl_bindings[0].descriptorType = VkDescriptorType(0);
    dsl_bindings[0].descriptorCount = 1;
    dsl_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_bindings[0].pImmutableSamplers = NULL;

    // Create arrays of layout and descriptor objects
    using UpDescriptorSet = std::unique_ptr<vkt::DescriptorSet>;
    std::vector<UpDescriptorSet> descriptor_sets;
    using UpDescriptorSetLayout = std::unique_ptr<vkt::DescriptorSetLayout>;
    std::vector<UpDescriptorSetLayout> ds_layouts;
    descriptor_sets.reserve(kLocalDescriptorTypeRangeSize);
    ds_layouts.reserve(kLocalDescriptorTypeRangeSize);
    for (uint32_t i = 0; i < kLocalDescriptorTypeRangeSize; ++i) {
        dsl_bindings[0].descriptorType = VkDescriptorType(i);
        ds_layouts.push_back(UpDescriptorSetLayout(new vkt::DescriptorSetLayout(*m_device, dsl_bindings)));
        descriptor_sets.push_back(UpDescriptorSet(ds_pool.AllocateSets(*m_device, *ds_layouts.back())));
        ASSERT_TRUE(descriptor_sets.back()->initialized());
    }

    // Create a buffer & bufferView to be used for invalid updates
    const VkDeviceSize buffer_size = 256;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    vkt::Buffer storage_texel_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    auto buff_view_ci = vkt::BufferView::CreateInfo(buffer.handle(), VK_FORMAT_R8_UNORM);
    vkt::BufferView buffer_view_obj, storage_texel_buffer_view_obj;
    buffer_view_obj.init(*m_device, buff_view_ci);
    buff_view_ci.buffer = storage_texel_buffer.handle();
    storage_texel_buffer_view_obj.init(*m_device, buff_view_ci);
    ASSERT_TRUE(buffer_view_obj.initialized() && storage_texel_buffer_view_obj.initialized());
    VkBufferView buffer_view = buffer_view_obj.handle();
    VkBufferView storage_texel_buffer_view = storage_texel_buffer_view_obj.handle();

    // Create an image to be used for invalid updates
    vkt::Image image_obj(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image_obj.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    buff_info.range = VK_WHOLE_SIZE;
    VkDescriptorImageInfo img_info = {};
    img_info.imageView = image_view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_info.sampler = sampler.handle();
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = &buffer_view;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = &img_info;

    // These error messages align with VkDescriptorType struct
    std::string error_codes[] = {
        "",                                                // placeholder, no error for SAMPLER descriptor
        "VUID-VkWriteDescriptorSet-descriptorType-00337",  // COMBINED_IMAGE_SAMPLER
        "VUID-VkWriteDescriptorSet-descriptorType-00337",  // SAMPLED_IMAGE
        "VUID-VkWriteDescriptorSet-descriptorType-00339",  // STORAGE_IMAGE
        "VUID-VkWriteDescriptorSet-descriptorType-08765",  // UNIFORM_TEXEL_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-08766",  // STORAGE_TEXEL_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00330",  // UNIFORM_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00331",  // STORAGE_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00330",  // UNIFORM_BUFFER_DYNAMIC
        "VUID-VkWriteDescriptorSet-descriptorType-00331",  // STORAGE_BUFFER_DYNAMIC
        "VUID-VkWriteDescriptorSet-descriptorType-00338"   // INPUT_ATTACHMENT
    };

    for (uint32_t i = 1; i < kLocalDescriptorTypeRangeSize; ++i) {
        if (VkDescriptorType(i) == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            // Now check for UNIFORM_TEXEL_BUFFER using storage_texel_buffer_view
            descriptor_write.pTexelBufferView = &storage_texel_buffer_view;
        }
        descriptor_write.descriptorType = VkDescriptorType(i);
        descriptor_write.dstSet = descriptor_sets[i]->handle();
        m_errorMonitor->SetDesiredError(error_codes[i].c_str());

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

        m_errorMonitor->VerifyFound();
        if (VkDescriptorType(i) == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            descriptor_write.pTexelBufferView = &buffer_view;
        }
    }
}

TEST_F(NegativeDescriptors, DSUsageBitsFlags2) {
    TEST_DESCRIPTION(
        "Attempt to update descriptor sets for buffers that do not have correct usage bits sets with VkBufferUsageFlagBits2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    const VkFormat buffer_format = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), buffer_format, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT for this format";
    }

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&buffer_usage_flags);
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info);

    VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = buffer_format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, buff_view_ci);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &buffer_view.handle();
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pBufferInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-08766");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DSBufferLimit) {
    TEST_DESCRIPTION(
        "Attempt to update buffer descriptor set that has VkDescriptorBufferInfo values that violate device limits.\n"
        "Test cases include:\n"
        "1. range of uniform buffer update exceeds maxUniformBufferRange\n"
        "2. offset of uniform buffer update is not a multiple of minUniformBufferOffsetAlignment\n"
        "3. using VK_WHOLE_SIZE with uniform buffer size exceeding maxUniformBufferRange\n"
        "4. range of storage buffer update exceeds maxStorageBufferRange\n"
        "5. offset of storage buffer update is not a multiple of minStorageBufferOffsetAlignment\n"
        "6. using VK_WHOLE_SIZE with storage buffer size exceeding maxStorageBufferRange");

    RETURN_IF_SKIP(Init());

    struct TestCase {
        VkDescriptorType descriptor_type;
        VkBufferUsageFlagBits buffer_usage;
        VkDeviceSize max_range;
        std::string max_range_vu;
        VkDeviceSize min_align;
        std::string min_align_vu;
    };

    for (const auto &test_case : {
             TestCase({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       m_device->Physical().limits_.maxUniformBufferRange, "VUID-VkWriteDescriptorSet-descriptorType-00332",
                       m_device->Physical().limits_.minUniformBufferOffsetAlignment,
                       "VUID-VkWriteDescriptorSet-descriptorType-00327"}),
             TestCase({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       m_device->Physical().limits_.maxStorageBufferRange, "VUID-VkWriteDescriptorSet-descriptorType-00333",
                       m_device->Physical().limits_.minStorageBufferOffsetAlignment,
                       "VUID-VkWriteDescriptorSet-descriptorType-00328"}),
         }) {
        // Create layout with single buffer
        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, test_case.descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        // Create a buffer to be used for invalid updates
        VkBufferCreateInfo bci = vku::InitStructHelper();
        bci.usage = test_case.buffer_usage;
        bci.size = test_case.max_range + test_case.min_align;  // Make buffer bigger than range limit
        vkt::Buffer buffer(*m_device, bci, vkt::no_mem);
        if (buffer.handle() == VK_NULL_HANDLE) {
            std::string msg = "Failed to allocate buffer of size " + std::to_string(bci.size) + " in DSBufferLimitErrors; skipped";
            printf("%s\n", msg.c_str());
            continue;
        }

        // Have to bind memory to buffer before descriptor update
        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

        VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
        mem_alloc.allocationSize = mem_reqs.size;
        bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
        if (!pass) {
            printf("Failed to allocate memory in DSBufferLimitErrors; skipped.\n");
            continue;
        }

        vkt::DeviceMemory mem(*m_device, mem_alloc);
        if (mem.handle() == VK_NULL_HANDLE) {
            printf("Failed to allocate memory in DSBufferLimitErrors; skipped.\n");
            continue;
        }
        VkResult err = vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);
        ASSERT_EQ(VK_SUCCESS, err);

        VkDescriptorBufferInfo buff_info = {};
        buff_info.buffer = buffer.handle();
        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pTexelBufferView = nullptr;
        descriptor_write.pBufferInfo = &buff_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.descriptorType = test_case.descriptor_type;
        descriptor_write.dstSet = descriptor_set.set_;

        // Exceed range limit
        if (test_case.max_range != vvl::kU32Max) {
            buff_info.range = test_case.max_range + 1;
            buff_info.offset = 0;
            m_errorMonitor->SetDesiredError(test_case.max_range_vu.c_str());
            vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
            m_errorMonitor->VerifyFound();
        }

        // Reduce size of range to acceptable limit and cause offset error
        if (test_case.min_align > 1) {
            buff_info.range = test_case.max_range;
            buff_info.offset = test_case.min_align - 1;
            m_errorMonitor->SetDesiredError(test_case.min_align_vu.c_str());
            vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
            m_errorMonitor->VerifyFound();
        }

        // Exceed effective range limit by using VK_WHOLE_SIZE
        buff_info.range = VK_WHOLE_SIZE;
        buff_info.offset = 0;
        m_errorMonitor->SetDesiredError(test_case.max_range_vu.c_str());
        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptors, DSTypeMismatch) {
    // Create DS w/ layout of one type and attempt Update w/ mis-matched type
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00319");

    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DSUpdateOutOfBounds) {
    // For overlapping Update, have arrayIndex exceed that of layout
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    // dstArrayElement of 1 is OOB
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DSUpdateIndex) {
    // Create layout w/ count of 1 and attempt update to that layout w/ binding index 2
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-00315");
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    // This is the wrong type, but out of bounds will be flagged first
    descriptor_set.WriteDescriptorImageInfo(2, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DSUpdateEmptyBinding) {
    TEST_DESCRIPTION("Create layout w/ empty binding and attempt to update it");
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 0 /* !! */, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    // descriptor_write.descriptorCount = 1, Lie here to avoid parameter_validation error
    // This is the wrong type, but empty binding error will be flagged first
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-00316");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, UpdateIndexSmaller) {
    TEST_DESCRIPTION("Only have a binding 2, but try updating binding 1");
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-00316");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DSUpdateStruct) {
    TEST_DESCRIPTION("Call UpdateDS w/ struct type other than valid VK_STRUCTUR_TYPE_UPDATE_* types");
    m_errorMonitor->SetDesiredError(".sType must be VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET");
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo info = {};
    info.sampler = sampler.handle();

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; /* Intentionally broken struct type */
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.descriptorCount = 1;
    // This is the wrong type, but out of bounds will be flagged first
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_write.pImageInfo = &info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, SampleDescriptorUpdate) {
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSampler sampler = CastToHandle<VkSampler, uintptr_t>(0xbaadbeef);  // Sampler with invalid handle

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00325");
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    descriptor_set.Clear();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00325");
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, ImageViewDescriptorUpdate) {
    // Create a single combined Image/Sampler descriptor and send it an invalid
    // imageView

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02996");

    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkImageView view = CastToHandle<VkImageView, uintptr_t>(0xbaadbeef);  // invalid imageView object

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, InputAttachmentDescriptorUpdate) {
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkImageView view = CastToHandle<VkImageView, uintptr_t>(0xbaadbeef);  // invalid imageView object

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-07683");
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, InputAttachmentDepthStencilAspect) {
    TEST_DESCRIPTION("Checks for InputAttachment image view with more than one aspect.");
    RETURN_IF_SKIP(Init());

    VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, ds_format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::Image image2D(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image2D.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-imageView-01976");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, CopyDescriptorUpdate) {
    // Create DS w/ layout of 2 types, write update 1 and attempt to copy-update
    // into the other
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstBinding-02632");

    RETURN_IF_SKIP(Init());

    vkt::Sampler immutable_sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    OneOffDescriptorSet descriptor_set_2(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &immutable_sampler.handle()}});

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    // SAMPLER binding from layout above
    // This write update should succeed
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    // Now perform a copy update that fails due to type mismatch
    VkCopyDescriptorSet copy_ds_update = vku::InitStructHelper();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;  // Copy from SAMPLER binding
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;       // ERROR : copy to UNIFORM binding
    copy_ds_update.descriptorCount = 1;  // copy 1 descriptor
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();
    // Now perform a copy update that fails due to binding out of bounds
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcBinding-00345");
    copy_ds_update = vku::InitStructHelper();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 3;  // ERROR : Invalid binding for matching layout
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 1;  // Copy 1 descriptor
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstBinding-00347");
    copy_ds_update = vku::InitStructHelper();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 0;
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 3;       // ERROR : Invalid binding for matching layout
    copy_ds_update.descriptorCount = 1;  // Copy 1 descriptor
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    // Now perform a copy update that fails due to binding out of bounds
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcArrayElement-00346");
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstArrayElement-00348");
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstBinding-02632");

    copy_ds_update = vku::InitStructHelper();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 5;  // ERROR copy 5 descriptors (out of bounds for layout)
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    // Now perform a copy into an immutable sampler
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstBinding-02753");
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;
    copy_ds_update.dstSet = descriptor_set_2.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 1;
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, Maint1BindingSliceOf3DImage) {
    TEST_DESCRIPTION(
        "Attempt to bind a slice of a 3D texture in a descriptor set. This is explicitly disallowed by KHR_maintenance1 to keep "
        "things simple for drivers.");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    auto ici = vku::InitStruct<VkImageCreateInfo>(
        nullptr, VkImageCreateFlags{VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT}, VK_IMAGE_TYPE_3D, VK_FORMAT_R8G8B8A8_UNORM,
        VkExtent3D{32, 32, 32}, 1u, 1u, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
        VkImageUsageFlags{VK_IMAGE_USAGE_SAMPLED_BIT}, VK_SHARING_MODE_EXCLUSIVE, 0u, nullptr, VK_IMAGE_LAYOUT_UNDEFINED);
    vkt::Image image(*m_device, ici, vkt::set_layout);
    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-imageView-07796");  // missing VK_EXT_image_2d_view_of_3d
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-descriptorType-06714");
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, UpdateDestroyDescriptorSetLayout) {
    TEST_DESCRIPTION("Attempt updates to descriptor sets with destroyed descriptor set layouts");
    RETURN_IF_SKIP(Init());

    // Set up the descriptor (resource) and write/copy operations to use.
    vkt::Buffer buffer(*m_device, sizeof(float) * 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo info = {};
    info.buffer = buffer.handle();
    info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write_descriptor = vku::InitStructHelper();
    write_descriptor.dstSet = VK_NULL_HANDLE;  // must update this
    write_descriptor.dstBinding = 0;
    write_descriptor.descriptorCount = 1;
    write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor.pBufferInfo = &info;

    VkCopyDescriptorSet copy_descriptor = vku::InitStructHelper();
    copy_descriptor.srcSet = VK_NULL_HANDLE;  // must update
    copy_descriptor.srcBinding = 0;
    copy_descriptor.dstSet = VK_NULL_HANDLE;  // must update
    copy_descriptor.dstBinding = 0;
    copy_descriptor.descriptorCount = 1;

    // Create valid and invalid source and destination descriptor sets
    std::vector<VkDescriptorSetLayoutBinding> one_uniform_buffer = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    OneOffDescriptorSet good_dst(m_device, one_uniform_buffer);
    ASSERT_TRUE(good_dst.Initialized());

    OneOffDescriptorSet bad_dst(m_device, one_uniform_buffer);
    // Must assert before invalidating it below
    ASSERT_TRUE(bad_dst.Initialized());
    bad_dst.layout_ = vkt::DescriptorSetLayout();

    OneOffDescriptorSet good_src(m_device, one_uniform_buffer);
    ASSERT_TRUE(good_src.Initialized());

    // Put valid data in the good and bad sources, simultaneously doing a positive test on write and copy operations
    write_descriptor.dstSet = good_src.set_;
    vk::UpdateDescriptorSets(device(), 1, &write_descriptor, 0, NULL);

    OneOffDescriptorSet bad_src(m_device, one_uniform_buffer);
    ASSERT_TRUE(bad_src.Initialized());

    // to complete our positive testing use copy, where above we used write.
    copy_descriptor.srcSet = good_src.set_;
    copy_descriptor.dstSet = bad_src.set_;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_descriptor);
    bad_src.layout_ = vkt::DescriptorSetLayout();

    // Trigger the three invalid use errors
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstSet-00320");
    write_descriptor.dstSet = bad_dst.set_;
    vk::UpdateDescriptorSets(device(), 1, &write_descriptor, 0, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstSet-parameter");
    copy_descriptor.dstSet = bad_dst.set_;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_descriptor);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcSet-parameter");
    copy_descriptor.srcSet = bad_src.set_;
    copy_descriptor.dstSet = good_dst.set_;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_descriptor);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, WriteDescriptorSetNotAllocated) {
    TEST_DESCRIPTION("Try to update a descriptor that has yet to be allocated");
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, sizeof(uint32_t)};

    VkDescriptorSet bad_set = CastFromUint64<VkDescriptorSet>(0xcadecade);
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = bad_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstSet-00320");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    VkDescriptorSet null_set = CastFromUint64<VkDescriptorSet>(0);
    descriptor_write.dstSet = null_set;
    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandle");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, CreateDescriptorPool) {
    TEST_DESCRIPTION("Attempt to create descriptor pool with invalid parameters");

    RETURN_IF_SKIP(Init());

    const uint32_t default_descriptor_count = 1;
    const VkDescriptorPoolSize dp_size_template{VK_DESCRIPTOR_TYPE_SAMPLER, default_descriptor_count};

    const auto dp_ci_template = vku::InitStruct<VkDescriptorPoolCreateInfo>(nullptr, 0u, 1u, 1u, &dp_size_template);
    // try maxSets = 0
    {
        VkDescriptorPoolCreateInfo invalid_dp_ci = dp_ci_template;
        invalid_dp_ci.maxSets = 0;  // invalid maxSets value

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-descriptorPoolOverallocation-09227");
        {
            VkDescriptorPool pool;
            vk::CreateDescriptorPool(device(), &invalid_dp_ci, nullptr, &pool);
        }
        m_errorMonitor->VerifyFound();
    }

    // try descriptorCount = 0
    {
        VkDescriptorPoolSize invalid_dp_size = dp_size_template;
        invalid_dp_size.descriptorCount = 0;  // invalid descriptorCount value

        VkDescriptorPoolCreateInfo dp_ci = dp_ci_template;
        dp_ci.pPoolSizes = &invalid_dp_size;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolSize-descriptorCount-00302");
        {
            VkDescriptorPool pool;
            vk::CreateDescriptorPool(device(), &dp_ci, nullptr, &pool);
        }
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptors, DuplicateDescriptorBinding) {
    TEST_DESCRIPTION("Create a descriptor set layout with a duplicate binding number.");

    RETURN_IF_SKIP(Init());
    // Create layout where two binding #s are "1"
    static const uint32_t NUM_BINDINGS = 3;
    VkDescriptorSetLayoutBinding dsl_binding[NUM_BINDINGS] = {};
    dsl_binding[0].binding = 1;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[0].descriptorCount = 1;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[0].pImmutableSamplers = NULL;
    dsl_binding[1].binding = 0;
    dsl_binding[1].descriptorCount = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[1].descriptorCount = 1;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[1].pImmutableSamplers = NULL;
    dsl_binding[2].binding = 1;  // Duplicate binding should cause error
    dsl_binding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[2].descriptorCount = 1;
    dsl_binding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[2].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = NUM_BINDINGS;
    ds_layout_ci.pBindings = dsl_binding;
    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-binding-00279");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, InlineUniformBlockEXT) {
    TEST_DESCRIPTION("Test VK_EXT_inline_uniform_block.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(inline_uniform_props);

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();

    // Test too many bindings
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    dslb.descriptorCount = 4;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    if (inline_uniform_props.maxInlineUniformBlockSize < dslb.descriptorCount) {
        GTEST_SKIP() << "DescriptorCount exceeds InlineUniformBlockSize limit";
    }

    uint32_t maxBlocks = std::max(inline_uniform_props.maxPerStageDescriptorInlineUniformBlocks,
                                  inline_uniform_props.maxDescriptorSetInlineUniformBlocks);
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of inline uniform blocks";
    }

    for (uint32_t i = 0; i < 1 + maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }
    {
        ds_layout_ci.bindingCount = dslb_vec.size();
        ds_layout_ci.pBindings = dslb_vec.data();
        vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02214");
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02216");
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02215");
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02217");

        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &ds_layout.handle();
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

        vk::CreatePipelineLayout(device(), &pipeline_layout_ci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }
    {
        // Single binding that's too large and is not a multiple of 4
        dslb.binding = 0;
        dslb.descriptorCount = inline_uniform_props.maxInlineUniformBlockSize + 1;
        VkDescriptorSetLayout ds_layout;
        ds_layout_ci.bindingCount = 1;
        ds_layout_ci.pBindings = &dslb;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-02209");
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-08004");
        vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
        m_errorMonitor->VerifyFound();
    }

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 32;

    // Pool size must be a multiple of 4
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    ds_type_count.descriptorCount = 33;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&pool_inline_info);
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    {
        VkDescriptorPool ds_pool = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolSize-type-02218");
        vk::CreateDescriptorPool(device(), &ds_pool_ci, NULL, &ds_pool);
        m_errorMonitor->VerifyFound();
    }

    // Create a valid pool
    ds_type_count.descriptorCount = 32;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    // Create two valid sets with 8 bytes each
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    dslb.descriptorCount = 8;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = &dslb_vec[0];

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout.handle(), ds_layout.handle()};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = pool.handle();
    alloc_info.pSetLayouts = set_layouts;
    VkResult err = vk::AllocateDescriptorSets(device(), &alloc_info, descriptor_sets);
    ASSERT_EQ(VK_SUCCESS, err);

    uint32_t dummyData[8] = {};
    VkWriteDescriptorSetInlineUniformBlock write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 3;
    write_inline_uniform.pData = &dummyData[0];

    // Test invalid VkWriteDescriptorSet parameters (array element and size must be multiple of 4)
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper(&write_inline_uniform);
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 3;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;

    // one for dataSiz and for descriptorCount
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02220");
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSetInlineUniformBlock-dataSize-02222");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstArrayElement = 1;
    descriptor_write.descriptorCount = 4;
    write_inline_uniform.dataSize = 4;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02219");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.pNext = nullptr;
    descriptor_write.dstArrayElement = 0;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02221");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.pNext = &write_inline_uniform;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

    // Test invalid VkCopyDescriptorSet parameters (array element and size must be multiple of 4)
    VkCopyDescriptorSet copy_ds_update = vku::InitStructHelper();
    copy_ds_update.srcSet = descriptor_sets[0];
    copy_ds_update.srcBinding = 0;
    copy_ds_update.srcArrayElement = 0;
    copy_ds_update.dstSet = descriptor_sets[1];
    copy_ds_update.dstBinding = 0;
    copy_ds_update.dstArrayElement = 0;
    copy_ds_update.descriptorCount = 4;

    copy_ds_update.srcArrayElement = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcBinding-02223");
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.srcArrayElement = 0;
    copy_ds_update.dstArrayElement = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstBinding-02224");
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.dstArrayElement = 0;
    copy_ds_update.descriptorCount = 5;
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcBinding-02225");
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.descriptorCount = 4;
    vk::UpdateDescriptorSets(device(), 0, NULL, 1, &copy_ds_update);
}

TEST_F(NegativeDescriptors, InlineUniformBlockEXTFeature) {
    TEST_DESCRIPTION("Test VK_EXT_inline_uniform_block features.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    // Don't enable any features
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding dslb = {};
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = 0;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dslb;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-04604");
    VkDescriptorSetLayout ds_layout = {};
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, MaxInlineUniformBlockBindings) {
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    ds_type_count.descriptorCount = 16;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-09424");
    vk::CreateDescriptorPool(device(), &ds_pool_ci, NULL, &ds_pool);
    m_errorMonitor->VerifyFound();

    // have struct, but with value of zero
    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 0;
    ds_pool_ci.pNext = &pool_inline_info;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-09424");
    vk::CreateDescriptorPool(device(), &ds_pool_ci, NULL, &ds_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DstArrayElement) {
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView view = image.CreateView();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = VK_NULL_HANDLE;
    image_info.imageView = view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor_write.pImageInfo = &image_info;
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    // sum of 3 pointing into array of 2 bindings
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    descriptor_write.dstArrayElement = 2;
    vk::UpdateDescriptorSets(*m_device, 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    OneOffDescriptorSet descriptor_set2(m_device,
                                        {
                                            {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                            {1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                        });
    const VkDescriptorImageInfo image_infos[2] = {image_info, image_info};
    descriptor_write.descriptorCount = 2;
    descriptor_write.pImageInfo = image_infos;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    descriptor_write.dstArrayElement = 3;
    vk::UpdateDescriptorSets(*m_device, 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSetLayoutMisc) {
    TEST_DESCRIPTION("Various invalid ways to create a VkDescriptorSetLayout.");

    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 1;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;

    // Should succeed with shader stage of 0 or fragment
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);
    dsl_binding.stageFlags = 0;
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);

    dsl_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-01510");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSetLayoutStageFlags) {
    TEST_DESCRIPTION("VkDescriptorSetLayout stageFlags are not valid flags");

    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 1;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = 0x3BADFFFF;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorCount-09465");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSetLayoutImmutableSamplers) {
    TEST_DESCRIPTION("VkDescriptorSetLayout with invalid pImmutableSamplers");

    RETURN_IF_SKIP(Init());

    const VkSampler badhandles[2] = {CastFromUint64<VkSampler>(0xFFFFEEEE), CastFromUint64<VkSampler>(0xDDDDAAAA)};
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 1;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 2;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = badhandles;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;
    // One for each descriptor count
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-00282");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-00282");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorSetLayoutNullImmutableSamplers) {
    TEST_DESCRIPTION("VkDescriptorSetLayout with invalid pImmutableSamplers set to null");

    RETURN_IF_SKIP(Init());

    const VkSampler null_samples[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 1;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 2;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = null_samples;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;
    // One for each descriptor count
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-00282");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-00282");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, NullDescriptorsDisabled) {
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02997");
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.Clear();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-02998");
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.Clear();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02995");
    VkBufferView buffer_view = VK_NULL_HANDLE;
    descriptor_set.WriteDescriptorBufferView(2, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.Clear();
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindVertexBuffers-pBuffers-04001");
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer, &offset);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, NullDescriptorsEnabled) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
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

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-02999");
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    offset = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindVertexBuffers-pBuffers-04002");
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer, &offset);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();

    // Make sure sampler with NULL image view doesn't cause a crash or errors
    OneOffDescriptorSet sampler_descriptor_set(m_device,
                                               {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    sampler_descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    sampler_descriptor_set.UpdateDescriptorSets();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&sampler_descriptor_set.layout_});
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D tex;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(tex, vec2(1));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &sampler_descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8095
TEST_F(NegativeDescriptors, DISABLED_ImageSubresourceOverlapBetweenAttachmentsAndDescriptorSets) {
    TEST_DESCRIPTION("Validate if attachments and descriptor set use the same image subresources");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());
    vkt::Image depth_image(
        *m_device, 64, 64, 1, depth_format,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView depth_view = depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 2, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view_input = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 1, 1);
    VkImageView attachments[] = {view_input, depth_view};

    vkt::ImageView view_sampler_overlap = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 1, 1);
    vkt::ImageView view_sampler_not_overlap = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    rp.AddAttachmentDescription(depth_format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.AddDepthStencilAttachment(1);
    rp.CreateRenderPass();

    vkt::Framebuffer fb(*m_device, rp.Handle(), 2u, attachments, 64, 64);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput ia0;
        layout(set=0, binding=1) uniform sampler2D ci1;
        layout(set=0, binding=2) uniform sampler2D ci2;
        layout(set=0, binding=3) uniform sampler2D ci3;
        layout(set=0, binding=4) uniform sampler2D ci4;
        void main() {
           vec4 color = subpassLoad(ia0);
           color = texture(ci1, vec2(0));
           color = texture(ci2, vec2(0));
           color = texture(ci3, vec2(0));
           color = texture(ci4, vec2(0));
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

    VkPipelineDepthStencilStateCreateInfo pipe_ds_state_ci = vku::InitStructHelper();
    pipe_ds_state_ci.depthTestEnable = VK_TRUE;
    pipe_ds_state_ci.stencilTestEnable = VK_FALSE;

    g_pipe.gp_ci_.pDepthStencilState = &pipe_ds_state_ci;
    g_pipe.gp_ci_.renderPass = rp.Handle();
    g_pipe.CreateGraphicsPipeline();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    // input attachment and combined image sampler use the same view to cause DesiredFailure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // image subresource of input attachment and combined image sampler overlap to cause DesiredFailure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_sampler_overlap.handle(), sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // image subresource of input attachment and combined image sampler don't overlap. It should not cause failure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, view_sampler_not_overlap.handle(), sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // Both image subresource and depth stencil attachment are read only. It should not cause failure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(4, depth_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {64, 64}};
    m_renderPassBeginInfo.renderPass = rp.Handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09002");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, CreateDescriptorPoolFlags) {
    TEST_DESCRIPTION("Create descriptor pool with invalid flags.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool bad_pool;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-flags-04607");
    vk::CreateDescriptorPool(device(), &ds_pool_ci, NULL, &bad_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, MissingMutableDescriptorTypeFeature) {
    TEST_DESCRIPTION("Create mutable descriptor pool with feature not enabled.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool bad_pool;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-flags-04609");
    vk::CreateDescriptorPool(device(), &ds_pool_ci, NULL, &bad_pool);
    m_errorMonitor->VerifyFound();

    ds_type_count.type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    ds_pool_ci.flags = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-mutableDescriptorType-04608");
    vk::CreateDescriptorPool(device(), &ds_pool_ci, NULL, &bad_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, MutableDescriptorPoolsWithPartialOverlap) {
    TEST_DESCRIPTION("Create mutable descriptor pools with partial overlap.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    pool_sizes[1].descriptorCount = 1;

    VkDescriptorType first_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkDescriptorType second_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListEXT lists[2] = {};
    lists[0].descriptorTypeCount = 2;
    lists[0].pDescriptorTypes = first_types;
    lists[1].descriptorTypeCount = 2;
    lists[1].pDescriptorTypes = second_types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 2;
    mdtci.pMutableDescriptorTypeLists = lists;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&mdtci);
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 2;
    ds_pool_ci.pPoolSizes = pool_sizes;

    {
        VkDescriptorPool pool;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787");
        vk::CreateDescriptorPool(device(), &ds_pool_ci, nullptr, &pool);
        m_errorMonitor->VerifyFound();

        lists[1].pDescriptorTypes = first_types;
        mdtci.mutableDescriptorTypeListCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787");
        vk::CreateDescriptorPool(device(), &ds_pool_ci, nullptr, &pool);
        m_errorMonitor->VerifyFound();
    }
    {
        mdtci.mutableDescriptorTypeListCount = 2;
        vkt::DescriptorPool pool(*m_device, ds_pool_ci);
    }
    {
        second_types[0] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        lists[1].pDescriptorTypes = second_types;
        vkt::DescriptorPool pool(*m_device, ds_pool_ci);
    }
}

TEST_F(NegativeDescriptors, CreateDescriptorPoolAllocateFlags) {
    TEST_DESCRIPTION("Create descriptor pool with invalid flags.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_samp(*m_device, {dsl_binding_samp},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT);

    VkDescriptorSetLayout set_layout = ds_layout_samp.handle();

    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = pool.handle();
    alloc_info.pSetLayouts = &set_layout;

    VkDescriptorSet descriptor_set;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-04610");
    vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorUpdateOfMultipleBindingWithOneUpdateCall) {
    TEST_DESCRIPTION("Update a descriptor set containing multiple bindings with only one update");

    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceInlineUniformBlockPropertiesEXT inlineUniformProps = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(inlineUniformProps);

    VkResult res;

    float inline_data[] = {1.f, 2.f};

    vkt::DescriptorSetLayout descLayout;
    {
        VkDescriptorSetLayoutBinding layoutBinding[3] = {};
        uint32_t bindingCount[] = {sizeof(inline_data) / 2, 0, sizeof(inline_data) / 2};
        uint32_t bindingPoint[] = {0, 1, 2};
        VkDescriptorType descType[] = {VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                       VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK};
        for (size_t i = 0; i < 3; ++i) {
            layoutBinding[i].binding = bindingPoint[i];
            layoutBinding[i].descriptorCount = bindingCount[i];
            layoutBinding[i].descriptorType = descType[i];
            layoutBinding[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutCreate = vku::InitStructHelper();
        layoutCreate.bindingCount = 3;
        layoutCreate.pBindings = layoutBinding;

        if (inlineUniformProps.maxInlineUniformBlockSize < bindingCount[0] ||
            inlineUniformProps.maxInlineUniformBlockSize < bindingCount[1]) {
            GTEST_SKIP() << "DescriptorCount exceeds InlineUniformBlockSize limit";
        }

        descLayout.init(*m_device, layoutCreate);

        ASSERT_TRUE(descLayout.initialized());
    }

    vkt::DescriptorPool descPool;
    {
        VkDescriptorPoolInlineUniformBlockCreateInfo descPoolInlineInfo = vku::InitStructHelper();
        descPoolInlineInfo.maxInlineUniformBlockBindings = 2;

        VkDescriptorPoolSize poolSize[2];
        poolSize[0].descriptorCount = sizeof(inline_data);
        poolSize[0].type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
        poolSize[1].descriptorCount = 1;
        poolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

        VkDescriptorPoolCreateInfo poolCreate = vku::InitStructHelper(&descPoolInlineInfo);
        poolCreate.poolSizeCount = 2;
        poolCreate.pPoolSizes = poolSize;
        poolCreate.maxSets = 1;

        descPool.init(*m_device, poolCreate);
        ASSERT_TRUE(descPool.initialized());
    }

    VkDescriptorSet descSetHandle = VK_NULL_HANDLE;
    {
        VkDescriptorSetAllocateInfo allocInfo = vku::InitStructHelper();
        allocInfo.pSetLayouts = &descLayout.handle();
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = descPool.handle();

        // The Galaxy S10 device used in LunarG CI fails to allocate this descriptor set
        res = vk::AllocateDescriptorSets(device(), &allocInfo, &descSetHandle);
        if (res != VK_SUCCESS) {
            GTEST_SKIP() << "vkAllocateDescriptorSets failed with error";
        }
    }
    vkt::DescriptorSet descSet(*m_device, &descPool, descSetHandle);

    VkWriteDescriptorSetInlineUniformBlock writeInlineUbDesc = vku::InitStructHelper();
    writeInlineUbDesc.dataSize = sizeof(inline_data);
    writeInlineUbDesc.pData = inline_data;

    VkWriteDescriptorSet writeDesc = vku::InitStructHelper(&writeInlineUbDesc);
    writeDesc.descriptorCount = sizeof(inline_data);
    writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    writeDesc.dstBinding = 0;
    writeDesc.dstArrayElement = 0;
    writeDesc.dstSet = descSet.handle();

    m_errorMonitor->Reset();
    vk::UpdateDescriptorSets(device(), 1, &writeDesc, 0, nullptr);
}

TEST_F(NegativeDescriptors, WriteMutableDescriptorSet) {
    TEST_DESCRIPTION("Write mutable descriptor set with invalid type.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListEXT list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&mdtci);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &ds_layout_handle;

    VkDescriptorSet descriptor_set;
    VkResult err = vk::AllocateDescriptorSets(device(), &allocate_info, &descriptor_set);
    ASSERT_EQ(VK_SUCCESS, err);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0;
    buffer_info.range = 32;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstSet-04611");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}

TEST_F(NegativeDescriptors, MutableDescriptors) {
    TEST_DESCRIPTION("Test mutable descriptors");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER};

    VkMutableDescriptorTypeListEXT mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = 1;
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

    VkMutableDescriptorTypeCreateInfoEXT mutable_descriptor_type_ci = vku::InitStructHelper();
    mutable_descriptor_type_ci.mutableDescriptorTypeListCount = 1;
    mutable_descriptor_type_ci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&mutable_descriptor_type_ci);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;

    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04599");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    mutable_descriptor_type_list.descriptorTypeCount = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04597");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    mutable_descriptor_type_list.descriptorTypeCount = 2;
    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04598");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04600");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04601");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04602");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    m_errorMonitor->SetDesiredError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04603");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorUpdateTemplate) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListEXT list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&mdtci);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = 0;
    update_template_entry.stride = 16;

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = ds_layout_handle;

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-04615");
    vk::CreateDescriptorUpdateTemplate(device(), &update_template_ci, nullptr, &update_template);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, MutableDescriptorSetLayout) {
    TEST_DESCRIPTION("Create mutable descriptor set layout.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkMutableDescriptorTypeListEXT list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    ds_layout_ci.pNext = &mdtci;

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    binding.pImmutableSamplers = &sampler.handle();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    binding.pImmutableSamplers = nullptr;
    sampler.destroy();
}

TEST_F(NegativeDescriptors, MutableDescriptorSetLayoutMissingFeature) {
    TEST_DESCRIPTION("Create mutable descriptor set layout without mutableDescriptorType feature enabled.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT;  // Invalid, feature is not enabled
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04596");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkMutableDescriptorTypeListEXT list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    ds_layout_ci.pNext = &mdtci;
    ds_layout_ci.flags = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-mutableDescriptorType-04595");
    m_errorMonitor->SetUnexpectedError(
        "VUID-VkDescriptorSetLayoutCreateInfo-pNext-pNext");  // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2457
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, ImageSubresourceOverlapBetweenRenderPassAndDescriptorSets) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");
    AddRequiredFeature(vkt::Feature::shaderStorageImageWriteWithoutFormat);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    auto image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    vkt::ImageView image_view = image.CreateView();
    VkImageView image_view_handle = image_view.handle();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view_handle);

    char const *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 x;
            layout(set = 0, binding = 0) writeonly uniform image2D image;
            void main(){
                x = vec4(1.0f);
                imageStore(image, ivec2(0), vec4(0.5f));
            }
        )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    descriptor_set.WriteDescriptorImageInfo(0, image_view.handle(), sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06537");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, ImageSubresourceOverlapBetweenRenderPassAndDescriptorSetsFunction) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    auto image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    vkt::ImageView image_view = image.CreateView();
    VkImageView image_view_handle = image_view.handle();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view_handle);

    // used as a "would be valid" image
    vkt::Image image_2(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view_2 = image_2.CreateView();

    // like the following, but does OpLoad before function call
    // layout(location = 0) out vec4 x;
    // layout(set = 0, binding = 0, rgba8) uniform image2D image_0;
    // layout(set = 0, binding = 1, rgba8) uniform image2D image_1;
    // void foo(image2D bar) {
    //     imageStore(bar, ivec2(0), vec4(0.5f));
    // }
    // void main() {
    //     x = vec4(1.0f);
    //     foo(image_0);
    // }
    char const *fsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color_attach
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color_attach Location 0
               OpDecorate %image_0 DescriptorSet 0
               OpDecorate %image_0 Binding 0
               OpDecorate %image_1 DescriptorSet 0
               OpDecorate %image_1 Binding 1
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%color_attach = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %12 = OpTypeImage %float 2D 0 0 0 2 Rgba8
         %13 = OpTypeFunction %void %12
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
    %image_0 = OpVariable %_ptr_UniformConstant_12 UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %18 = OpConstantComposite %v2int %int_0 %int_0
  %float_0_5 = OpConstant %float 0.5
         %20 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
    %image_1 = OpVariable %_ptr_UniformConstant_12 UniformConstant

         %foo = OpFunction %void None %13
         %bar = OpFunctionParameter %12
         %23 = OpLabel
               OpImageWrite %bar %18 %20
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %6
         %24 = OpLabel
               OpStore %color_attach %11
         %25 = OpLoad %12 %image_0
         %26 = OpFunctionCall %void %foo %25
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    descriptor_set.WriteDescriptorImageInfo(0, image_view.handle(), sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(1, image_view_2.handle(), sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06537");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8095
TEST_F(NegativeDescriptors, DISABLED_DescriptorReadFromWriteAttachment) {
    TEST_DESCRIPTION("Validate reading from a descriptor that uses same image view as framebuffer write attachment");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    auto image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    VkImageView image_view_handle = image_view.handle();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view_handle, width, height);

    char const *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 color;
            layout(set = 0, binding = 0, rgba8) readonly uniform image2D image1;
            void main(){
                color = imageLoad(image1, ivec2(0));
            }
        )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const vkt::DescriptorSetLayout descriptor_set_layout(*m_device, {layout_binding});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout, &descriptor_set_layout});
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09000");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), width, height, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorWriteFromReadAttachment) {
    TEST_DESCRIPTION("Validate writting to a descriptor that uses same image view as framebuffer read attachment");
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    auto image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    VkImageView image_view_handle = image_view.handle();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view_handle, width, height);

    char const *fsSource = R"glsl(
            #version 450
            layout(set = 0, binding = 0, rgba8) writeonly uniform image2D image1;
            layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput inputColor;
            void main(){
                vec4 color = subpassLoad(inputColor);
                imageStore(image1, ivec2(0), color);
            }
        )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding layout_binding1 = {};
    layout_binding1.binding = 0;
    layout_binding1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding1.descriptorCount = 1;
    layout_binding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding1.pImmutableSamplers = nullptr;
    VkDescriptorSetLayoutBinding layout_binding2 = {};
    layout_binding2.binding = 0;
    layout_binding2.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    layout_binding2.descriptorCount = 1;
    layout_binding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding2.pImmutableSamplers = nullptr;
    const vkt::DescriptorSetLayout descriptor_set_layout1(*m_device, {layout_binding1});
    const vkt::DescriptorSetLayout descriptor_set_layout2(*m_device, {layout_binding2});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout1, &descriptor_set_layout2});
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    OneOffDescriptorSet descriptor_set_storage_image(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    OneOffDescriptorSet descriptor_set_input_attachment(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    descriptor_set_storage_image.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                          VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set_storage_image.UpdateDescriptorSets();
    descriptor_set_input_attachment.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                                             VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set_input_attachment.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), width, height, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_storage_image.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &descriptor_set_input_attachment.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06539");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/3297
// This should be checked earlier as causes some driver to crash
TEST_F(NegativeDescriptors, DISABLED_AllocatingVariableDescriptorSets) {
    TEST_DESCRIPTION("Test allocating large variable descriptor sets");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    RETURN_IF_SKIP(Init());

    VkDescriptorBindingFlags flags[2] = {0, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;

    VkDescriptorSetLayoutBinding bindings[2] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, std::numeric_limits<uint32_t>::max() / 64, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.bindingCount = 2;
    ds_layout_ci.pBindings = bindings;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorSetVariableDescriptorCountAllocateInfo count_alloc_info = vku::InitStructHelper();
    count_alloc_info.descriptorSetCount = 1;
    uint32_t variable_count = 2;
    count_alloc_info.pDescriptorCounts = &variable_count;

    VkDescriptorPoolSize pool_sizes[2] = {{bindings[0].descriptorType, bindings[0].descriptorCount},
                                          {bindings[1].descriptorType, bindings[1].descriptorCount}};
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.poolSizeCount = 2;
    dspci.pPoolSizes = pool_sizes;
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper(&count_alloc_info);
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout_handle;

    VkDescriptorSet ds;
    VkResult err = vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(NegativeDescriptors, DescriptorSetLayoutBinding) {
    TEST_DESCRIPTION("Create invalid descriptor set layout.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = &sampler.handle();

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER};

    VkMutableDescriptorTypeListEXT mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = 1;
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&mdtci);
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout setLayout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-04605");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594");
    vk::CreateDescriptorSetLayout(m_device->handle(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, BindingDescriptorSetFromHostOnlyPool) {
    TEST_DESCRIPTION(
        "Try to bind a descriptor set that was allocated from a pool with VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &ds_layout_handle;

    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(device(), &allocate_info, &descriptor_set);

    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-04616");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, CopyMutableDescriptors) {
    TEST_DESCRIPTION("Copy mutable descriptors.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    {
        VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};

        VkMutableDescriptorTypeListEXT mutable_descriptor_type_list = {};
        mutable_descriptor_type_list.descriptorTypeCount = 1;
        mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

        VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
        mdtci.mutableDescriptorTypeListCount = 1;
        mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

        VkDescriptorPoolSize pool_sizes[2] = {};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[0].descriptorCount = 2;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        pool_sizes[1].descriptorCount = 2;

        VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&mdtci);
        ds_pool_ci.maxSets = 2;
        ds_pool_ci.poolSizeCount = 2;
        ds_pool_ci.pPoolSizes = pool_sizes;

        vkt::DescriptorPool pool(*m_device, ds_pool_ci);

        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[0].pImmutableSamplers = nullptr;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&mdtci);
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        vkt::DescriptorSetLayout set_layout(*m_device, create_info);
        VkDescriptorSetLayout set_layout_handle = set_layout.handle();

        VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

        VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
        allocate_info.descriptorPool = pool.handle();
        allocate_info.descriptorSetCount = 2;
        allocate_info.pSetLayouts = layouts;

        VkDescriptorSet descriptor_sets[2];
        vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

        vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = 32;

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_sets[0];
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

        VkCopyDescriptorSet copy_set = vku::InitStructHelper();
        copy_set.srcSet = descriptor_sets[1];
        copy_set.srcBinding = 1;
        copy_set.dstSet = descriptor_sets[0];
        copy_set.dstBinding = 0;
        copy_set.descriptorCount = 1;

        m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstSet-04612");
        vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

        VkMutableDescriptorTypeListEXT mutable_descriptor_type_lists[2] = {};
        mutable_descriptor_type_lists[0].descriptorTypeCount = 1;
        mutable_descriptor_type_lists[0].pDescriptorTypes = descriptor_types;
        mutable_descriptor_type_lists[1].descriptorTypeCount = 2;
        mutable_descriptor_type_lists[1].pDescriptorTypes = descriptor_types;

        VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
        mdtci.mutableDescriptorTypeListCount = 2;
        mdtci.pMutableDescriptorTypeLists = mutable_descriptor_type_lists;

        VkDescriptorPoolSize pool_size = {};
        pool_size.type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        pool_size.descriptorCount = 8;

        VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&mdtci);
        ds_pool_ci.maxSets = 2;
        ds_pool_ci.poolSizeCount = 1;
        ds_pool_ci.pPoolSizes = &pool_size;

        vkt::DescriptorPool pool(*m_device, ds_pool_ci);

        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[0].pImmutableSamplers = nullptr;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&mdtci);
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        vkt::DescriptorSetLayout set_layout(*m_device, create_info);
        VkDescriptorSetLayout set_layout_handle = set_layout.handle();

        VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

        VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
        allocate_info.descriptorPool = pool.handle();
        allocate_info.descriptorSetCount = 2;
        allocate_info.pSetLayouts = layouts;

        VkDescriptorSet descriptor_sets[2];
        vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

        vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = 32;

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_sets[0];
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

        VkCopyDescriptorSet copy_set = vku::InitStructHelper();
        copy_set.srcSet = descriptor_sets[1];
        copy_set.srcBinding = 1;
        copy_set.dstSet = descriptor_sets[0];
        copy_set.dstBinding = 0;
        copy_set.descriptorCount = 1;

        m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-dstSet-04614");
        vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_SAMPLER};

        VkMutableDescriptorTypeListEXT mutable_descriptor_type_lists[2] = {};
        mutable_descriptor_type_lists[0].descriptorTypeCount = 2;
        mutable_descriptor_type_lists[0].pDescriptorTypes = descriptor_types;
        mutable_descriptor_type_lists[1].descriptorTypeCount = 0;
        mutable_descriptor_type_lists[1].pDescriptorTypes = nullptr;

        VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
        mdtci.mutableDescriptorTypeListCount = 2;
        mdtci.pMutableDescriptorTypeLists = mutable_descriptor_type_lists;

        VkDescriptorPoolSize pool_sizes[3] = {};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        pool_sizes[0].descriptorCount = 4;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        pool_sizes[1].descriptorCount = 4;
        pool_sizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[2].descriptorCount = 4;

        VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&mdtci);
        ds_pool_ci.maxSets = 2;
        ds_pool_ci.poolSizeCount = 2;
        ds_pool_ci.pPoolSizes = pool_sizes;

        vkt::DescriptorPool pool(*m_device, ds_pool_ci);

        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        bindings[0].descriptorCount = 2;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[0].pImmutableSamplers = nullptr;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        bindings[1].descriptorCount = 2;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&mdtci);
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        vkt::DescriptorSetLayout set_layout(*m_device, create_info);
        VkDescriptorSetLayout set_layout_handle = set_layout.handle();

        VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

        VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
        allocate_info.descriptorPool = pool.handle();
        allocate_info.descriptorSetCount = 2;
        allocate_info.pSetLayouts = layouts;

        VkDescriptorSet descriptor_sets[2];
        vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

        vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = 32;

        VkDescriptorImageInfo image_info = {};
        image_info.sampler = sampler.handle();

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_sets[0];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptor_write.pImageInfo = &image_info;
        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

        descriptor_write.dstArrayElement = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr;
        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

        VkCopyDescriptorSet copy_set = vku::InitStructHelper();
        copy_set.srcSet = descriptor_sets[0];
        copy_set.srcBinding = 0;
        copy_set.srcArrayElement = 0;
        copy_set.dstSet = descriptor_sets[1];
        copy_set.dstBinding = 1;
        copy_set.dstArrayElement = 0;
        copy_set.descriptorCount = 2;

        // copying both mutables should fail because element 1 is the wrong type
        m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcSet-04613");
        vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();

        // copying element 0 should work
        copy_set.descriptorCount = 1;
        vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);

        // copying element 1 fail because it is the wrong type
        copy_set.srcArrayElement = 1;
        m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcSet-04613");
        vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptors, InvalidDescriptorSetLayoutInlineUniformBlockFlags) {
    TEST_DESCRIPTION("Create descriptor set layout with invalid flags.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);

    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0u;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    binding.descriptorCount = 4u;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci = vku::InitStructHelper();
    binding_flags_ci.bindingCount = 1u;
    binding_flags_ci.pBindingFlags = &binding_flags;

    VkDescriptorSetLayoutCreateInfo layout_ci = vku::InitStructHelper(&binding_flags_ci);
    layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layout_ci.bindingCount = 1u;
    layout_ci.pBindings = &binding;

    VkDescriptorSetLayout set_layout;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingInlineUniformBlockUpdateAfterBind-02211");
    vk::CreateDescriptorSetLayout(*m_device, &layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DispatchWithUnboundSet) {
    TEST_DESCRIPTION("Dispatch with unbound descriptor set");
    RETURN_IF_SKIP(Init());

    char const *cs_source = R"glsl(
        #version 450
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        layout(set = 0, binding = 0) uniform sampler2D InputTexture;
        layout(set = 1, binding = 0, rgba32f) uniform image2D OutputTexture;
        void main() {
            vec4 value = textureGather(InputTexture, vec2(0), 0);
            imageStore(OutputTexture, ivec2(0), value);
        }
    )glsl";

    OneOffDescriptorSet combined_image_set(
        m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    OneOffDescriptorSet storage_image_set(m_device,
                                          {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});

    const VkFormat combined_image_format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 1, 1, 1, combined_image_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&combined_image_set.layout_, &storage_image_set.layout_});
    cs_pipeline.CreateComputePipeline();

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.Handle());

    combined_image_set.WriteDescriptorImageInfo(0, view, sampler.handle());
    combined_image_set.UpdateDescriptorSets();

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(), 0,
                              1, &combined_image_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08600");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, InvalidDescriptorSetLayoutFlags) {
    TEST_DESCRIPTION("Create descriptor set layout with invalid flags.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0u;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    binding.descriptorCount = 1u;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci = vku::InitStructHelper();
    binding_flags_ci.bindingCount = 1u;
    binding_flags_ci.pBindingFlags = &binding_flags;

    VkDescriptorSetLayoutCreateInfo layout_ci = vku::InitStructHelper(&binding_flags_ci);
    layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layout_ci.bindingCount = 1u;
    layout_ci.pBindings = &binding;

    VkDescriptorSetLayout set_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-None-03011");
    vk::CreateDescriptorSetLayout(*m_device, &layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, SampledImageDepthComparisonForFormat) {
    TEST_DESCRIPTION("Verify that OpImage*Dref* operations are supported for given format ");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormat format = VK_FORMAT_UNDEFINED;
    for (uint32_t fmt = VK_FORMAT_R4G4_UNORM_PACK8; fmt < VK_FORMAT_D16_UNORM; fmt++) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2KHR(Gpu(), (VkFormat)fmt, &fmt_props);

        const bool has_sampling = (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT) != 0;
        const bool has_sampling_img_depth_compare =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT) != 0;

        if (has_sampling && !has_sampling_img_depth_compare) {
            format = (VkFormat)fmt;
            break;
        }
    }

    if (format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Cannot find suitable format, skipping.";
    }

    const char vsSource[] = R"glsl(
        #version 450

        void main() {
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 1) uniform sampler2DShadow tex;
        void main() {
           float f = texture(tex, vec3(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_[0] = {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    g_pipe.CreateGraphicsPipeline();

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06479");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, BindDescriptorWithoutPipelineLayout) {
    TEST_DESCRIPTION("Bind a DescriptorSet with a null pipeline layout.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                 });

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandle");
    VkPipelineLayout null_layout = CastFromUint64<VkPipelineLayout>(0);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, null_layout, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, InvalidImageInfoDescriptorType) {
    TEST_DESCRIPTION("Try to copy a descriptor set where the src and dst have different update after bind flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
    void *pNext = nullptr;
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.imageView2DOn3DImage) {
            GTEST_SKIP() << "imageView2DOn3DImage not supported, skipping test";
        }
        pNext = &portability_subset_features;
    }
    RETURN_IF_SKIP(InitState(nullptr, pNext));

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = format;
    image_ci.extent = {32, 32, 2};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-imageView-07795");
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, CopyDescriptorSetMissingSrcFlag) {
    TEST_DESCRIPTION("Try to copy a descriptor set where the src and dst have different update after bind flags.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet src_descriptor_set(m_device,
                                           {
                                               {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, VK_SHADER_STAGE_ALL, nullptr},
                                           },
                                           VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, nullptr,
                                           VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);
    OneOffDescriptorSet dst_descriptor_set(m_device,
                                           {
                                               {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, VK_SHADER_STAGE_ALL, nullptr},
                                           },
                                           0u, nullptr, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);
    OneOffDescriptorSet no_flags_set(m_device,
                                     {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, VK_SHADER_STAGE_ALL, nullptr},
                                     },
                                     0u, nullptr, 0u);

    VkCopyDescriptorSet copy_descriptor_set = vku::InitStructHelper();
    copy_descriptor_set.srcSet = src_descriptor_set.set_;
    copy_descriptor_set.srcBinding = 0u;
    copy_descriptor_set.srcArrayElement = 0u;
    copy_descriptor_set.dstSet = dst_descriptor_set.set_;
    copy_descriptor_set.dstBinding = 0u;
    copy_descriptor_set.dstArrayElement = 0u;
    copy_descriptor_set.descriptorCount = 1u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcSet-01918");
    vk::UpdateDescriptorSets(*m_device, 0u, nullptr, 1u, &copy_descriptor_set);
    m_errorMonitor->VerifyFound();

    copy_descriptor_set.srcSet = dst_descriptor_set.set_;
    copy_descriptor_set.dstSet = no_flags_set.set_;
    m_errorMonitor->SetDesiredError("VUID-VkCopyDescriptorSet-srcSet-01920");
    vk::UpdateDescriptorSets(*m_device, 0u, nullptr, 1u, &copy_descriptor_set);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, InvalidDescriptorWriteImageInfo) {
    TEST_DESCRIPTION("Write descriptor set with invalid image info.");

    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06493");
    vk::UpdateDescriptorSets(*m_device, 1u, &descriptor_write, 0u, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, BindStorageBufferDynamicAlignment) {
    TEST_DESCRIPTION("Bind dynamic storage buffer with invalid alignment.");

    RETURN_IF_SKIP(Init());

    uint32_t alignment = static_cast<uint32_t>(m_device->Physical().limits_.minStorageBufferOffsetAlignment);
    if (alignment < 2) {
        GTEST_SKIP() << "minStorageBufferOffsetAlignment too small";
    }

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    uint32_t dynamic_offset = alignment - 1;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01972");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 1u, &dynamic_offset);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DescriptorIndexingMissingFeatures) {
    TEST_DESCRIPTION("Use partially bound descriptor flag without feature.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    VkDescriptorBindingFlags flag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1u;
    flags_create_info.pBindingFlags = &flag;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout set_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingPartiallyBound-03013");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    flag = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingSampledImageUpdateAfterBind-03006");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageImageUpdateAfterBind-03007");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageBufferUpdateAfterBind-03008");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageTexelBufferUpdateAfterBind-03010");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformTexelBufferUpdateAfterBind-03009");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    flag = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUpdateUnusedWhilePending-03012");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    flag = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

    m_errorMonitor->SetDesiredError(
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingVariableDescriptorCount-03014");
    vk::CreateDescriptorSetLayout(*m_device, &ds_layout_ci, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, IncompatibleDescriptorFlagsWithBindingFlags) {
    TEST_DESCRIPTION("Create descriptor set layout with incompatible flags with binding flags");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding bindings[2];
    bindings[0].binding = 0u;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1u;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[1].binding = 1u;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1u;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags binding_flags[] = {VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, 0};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1u;
    flags_create_info.pBindingFlags = binding_flags;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    create_info.bindingCount = 1u;
    create_info.pBindings = bindings;

    VkDescriptorSetLayout set_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-flags-03003");
    vk::CreateDescriptorSetLayout(*m_device, &create_info, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    create_info.flags = 0u;
    flags_create_info.bindingCount = 2u;
    create_info.bindingCount = 2u;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03004");
    vk::CreateDescriptorSetLayout(*m_device, &create_info, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    flags_create_info.bindingCount = 1u;
    create_info.bindingCount = 1u;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03015");
    vk::CreateDescriptorSetLayout(*m_device, &create_info, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uint32_t descriptorCounts[2] = {1u, 1u};
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_allocate = vku::InitStructHelper();
    variable_allocate.descriptorSetCount = 2u;
    variable_allocate.pDescriptorCounts = descriptorCounts;

    VkDescriptorPoolSize ds_type_count;
    ds_type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ds_type_count.descriptorCount = 1u;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1u;
    ds_pool_ci.poolSizeCount = 1u;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);
    const vkt::DescriptorSetLayout ds_layout(*m_device, {bindings[0]});

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper(&variable_allocate);
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1u;
    allocate_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet descriptor_set;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-descriptorSetCount-03045");
    vk::AllocateDescriptorSets(*m_device, &allocate_info, &descriptor_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, MaxInlineUniformTotalSize) {
    TEST_DESCRIPTION("Test the maxInlineUniformTotalSize limit");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_block_properties = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Properties properties13 = vku::InitStructHelper(&inline_uniform_block_properties);
    GetPhysicalDeviceProperties2(properties13);
    const uint32_t limit = properties13.maxInlineUniformTotalSize;

    if (limit == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "maxInlineUniformTotalSize is too large";
    }

    const uint32_t binding_count = limit / inline_uniform_block_properties.maxInlineUniformBlockSize + 1;
    std::vector<VkDescriptorSetLayoutBinding> bindings(binding_count);
    const VkShaderStageFlags stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                         VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                         VK_SHADER_STAGE_FRAGMENT_BIT};
    for (uint32_t i = 0; i < binding_count; ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
        bindings[i].descriptorCount = inline_uniform_block_properties.maxInlineUniformBlockSize;
        bindings[i].stageFlags = stages[i % 5];
        bindings[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo set_layout_ci = vku::InitStructHelper();
    set_layout_ci.bindingCount = binding_count;
    set_layout_ci.pBindings = bindings.data();
    vkt::DescriptorSetLayout set_layout(*m_device, set_layout_ci);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1u;
    pipeline_layout_ci.pSetLayouts = &set_layout.handle();

    VkPipelineLayout pipeline_layout;
    if (binding_count > inline_uniform_block_properties.maxDescriptorSetUpdateAfterBindInlineUniformBlocks) {
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02217");
    }
    if (binding_count / 5 > inline_uniform_block_properties.maxPerStageDescriptorInlineUniformBlocks) {
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02214");
    }
    if (binding_count / 5 > inline_uniform_block_properties.maxPerStageDescriptorInlineUniformBlocks) {
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02215");
    }
    if (binding_count > inline_uniform_block_properties.maxDescriptorSetInlineUniformBlocks) {
        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02216");
    }
    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-descriptorType-06531");
    vk::CreatePipelineLayout(*m_device, &pipeline_layout_ci, nullptr, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DescriptorTypeNotInPool) {
    TEST_DESCRIPTION("With maintenance1, allocate descriptor with type not in pool");
    SetTargetApiVersion(VK_API_VERSION_1_1);  // Need VK_KHR_maintenance1
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create Pool with 2 Sampler descriptors, but try to alloc an Uniform Buffer
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_sampler = {};
    dsl_binding_sampler.binding = 0;
    dsl_binding_sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_sampler.descriptorCount = 1;
    dsl_binding_sampler.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_sampler.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding dsl_binding_uniform = {};
    dsl_binding_uniform.binding = 1;
    dsl_binding_uniform.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding_uniform.descriptorCount = 1;
    dsl_binding_uniform.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_uniform.pImmutableSamplers = nullptr;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding_sampler, dsl_binding_uniform});

    VkDescriptorSet descriptor_set;
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    VkResult result = vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_set);

    // not all implementations will return this exact error code
    if (result != VK_ERROR_OUT_OF_POOL_MEMORY) {
        GTEST_SKIP() << "Does not return expected VK_ERROR_OUT_OF_POOL_MEMORY";
    }
    m_errorMonitor->SetDesiredWarning("WARNING-CoreValidation-AllocateDescriptorSets-WrongType");
    vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, PushDescriptorWithoutInfo) {
    TEST_DESCRIPTION("Push a descriptor without providing any info structs in VkWriteDescriptorSet");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    m_command_buffer.Begin();
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-pDescriptorWrites-06494");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, GetSupportMutableDescriptorType) {
    TEST_DESCRIPTION("Test vkGetDescriptorSetLayoutSupport with mutable descriptor set layout support");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);

    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo set_layout_ci = vku::InitStructHelper();
    set_layout_ci.bindingCount = 1;
    set_layout_ci.pBindings = &binding;

    VkDescriptorSetLayoutSupport support = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303");
    vk::GetDescriptorSetLayoutSupport(device(), &set_layout_ci, &support);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, EmptyDescriptorSetLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8065");
    RETURN_IF_SKIP(Init());

    // Layout is created with no bindings
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_sizes = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_sizes;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_set);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info = {buffer.handle(), 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = descriptor_set;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-10009");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, DuplicateLayoutDifferentSampler) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8497");
    RETURN_IF_SKIP(Init());
    auto sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler_0(*m_device, sampler_ci);
    sampler_ci.maxLod = 8.0;
    vkt::Sampler sampler_1(*m_device, sampler_ci);

    OneOffDescriptorSet ds_0(m_device,
                             {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, &sampler_0.handle()}});
    const vkt::PipelineLayout pipeline_layout_0(*m_device, {&ds_0.layout_});

    OneOffDescriptorSet ds_1(m_device,
                             {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, &sampler_1.handle()}});

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_0.handle(), 0, 1,
                              &ds_1.set_, 0, nullptr);
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DuplicateLayoutDifferentSamplerArray) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8497");
    RETURN_IF_SKIP(Init());
    auto sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler_0(*m_device, sampler_ci);
    sampler_ci.maxLod = 8.0;
    vkt::Sampler sampler_1(*m_device, sampler_ci);
    VkSampler sampler_array_0[3] = {sampler_0.handle(), sampler_0.handle(), sampler_0.handle()};
    VkSampler sampler_array_1[3] = {sampler_0.handle(), sampler_1.handle(), sampler_0.handle()};

    OneOffDescriptorSet ds_0(m_device,
                             {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_COMPUTE_BIT, sampler_array_0}});
    const vkt::PipelineLayout pipeline_layout_0(*m_device, {&ds_0.layout_});

    OneOffDescriptorSet ds_1(m_device,
                             {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_COMPUTE_BIT, sampler_array_1}});

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_0.handle(), 0, 1,
                              &ds_1.set_, 0, nullptr);
    m_command_buffer.End();
}

TEST_F(NegativeDescriptors, DSBufferLimitWithTemplateUpdate) {
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkDeviceSize storage_buffer_offset_alignment = m_device->Physical().limits_.minStorageBufferOffsetAlignment;

    vkt::Buffer buffer(*m_device, 16u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer;
    buffer_info.offset = storage_buffer_offset_alignment / 2u;
    buffer_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.descriptorCount = 1u;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00328");
    vk::UpdateDescriptorSets(device(), 1u, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    VkDescriptorUpdateTemplateEntry templateEntry;
    templateEntry.dstBinding = 0u;
    templateEntry.dstArrayElement = 0u;
    templateEntry.descriptorCount = 1u;
    templateEntry.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    templateEntry.offset = 0u;
    templateEntry.stride = sizeof(VkDescriptorBufferInfo);

    VkDescriptorUpdateTemplateCreateInfo template_create_info = vku::InitStructHelper();
    template_create_info.descriptorUpdateEntryCount = 1u;
    template_create_info.pDescriptorUpdateEntries = &templateEntry;
    template_create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    template_create_info.descriptorSetLayout = descriptor_set.layout_;

    vkt::DescriptorUpdateTemplate descriptor_update_template(*m_device, template_create_info);

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00328");
    vk::UpdateDescriptorSetWithTemplateKHR(device(), descriptor_set.set_, descriptor_update_template, &buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptors, UpdateDescriptorSetWithAccelerationStructure) {
    TEST_DESCRIPTION("Update descriptor set with invalid acceleration structures.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    pool_size.descriptorCount = 1u;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1u;
    ds_pool_ci.poolSizeCount = 1u;
    ds_pool_ci.pPoolSizes = &pool_size;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0u;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    binding.descriptorCount = 1u;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper();
    create_info.bindingCount = 1u;
    create_info.pBindings = &binding;

    vkt::DescriptorSetLayout set_layout(*m_device, create_info);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1u;
    allocate_info.pSetLayouts = &set_layout.handle();

    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(device(), &allocate_info, &descriptor_set);

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0u;
    descriptor_write.descriptorCount = 1u;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-02382");
    vk::UpdateDescriptorSets(device(), 1u, &descriptor_write, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->Build();

    VkAccelerationStructureKHR null_acceleration_structure = VK_NULL_HANDLE;

    VkWriteDescriptorSetAccelerationStructureKHR blas_descriptor = vku::InitStructHelper();
    blas_descriptor.accelerationStructureCount = 1u;
    blas_descriptor.pAccelerationStructures = &null_acceleration_structure;
    descriptor_write.pNext = &blas_descriptor;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03580");
    vk::UpdateDescriptorSets(device(), 1u, &descriptor_write, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    blas_descriptor.pAccelerationStructures = &blas->handle();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03579");
    vk::UpdateDescriptorSets(device(), 1u, &descriptor_write, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    blas_descriptor.accelerationStructureCount = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSetAccelerationStructureKHR-accelerationStructureCount-arraylength");
    vk::UpdateDescriptorSets(device(), 1u, &descriptor_write, 0u, nullptr);
    m_errorMonitor->VerifyFound();
}
