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

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/external_memory_sync.h"

class NegativeObjectLifetime : public VkLayerTest {};

TEST_F(NegativeObjectLifetime, CmdBufferBufferDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a buffer dependency being destroyed.");
    RETURN_IF_SKIP(Init());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_EQ(VK_SUCCESS, err);

    err = vk::BindBufferMemory(device(), buffer, mem, 0);
    ASSERT_EQ(VK_SUCCESS, err);

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    // Destroy buffer dependency prior to submit to cause ERROR
    vk::DestroyBuffer(device(), buffer, NULL);

    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
    vk::FreeMemory(m_device->handle(), mem, NULL);
}

TEST_F(NegativeObjectLifetime, CmdBarrierBufferDestroyed) {
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    vkt::Buffer buffer(*m_device, buf_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs.size;

    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory buffer_mem(*m_device, alloc_info);
    ASSERT_TRUE(buffer_mem.initialized());

    ASSERT_EQ(VK_SUCCESS, vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), 0));

    m_command_buffer.Begin();
    VkBufferMemoryBarrier buf_barrier = vku::InitStructHelper();
    buf_barrier.buffer = buffer.handle();
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 1, &buf_barrier, 0, NULL);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkFreeMemory-memory-00677");
    vk::FreeMemory(m_device->handle(), buffer_mem.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeObjectLifetime, CmdBarrierImageDestroyed) {
    RETURN_IF_SKIP(Init());

    vkt::DeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    image_mem.init(*m_device, alloc_info);

    auto err = vk::BindImageMemory(device(), image.handle(), image_mem.handle(), 0);
    ASSERT_EQ(VK_SUCCESS, err);

    m_command_buffer.Begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, &img_barrier);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkFreeMemory-memory-00677");
    vk::FreeMemory(m_device->handle(), image_mem.handle(), NULL);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeObjectLifetime, Sync2CmdBarrierBufferDestroyed) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_EQ(VK_SUCCESS, err);

    err = vk::BindBufferMemory(device(), buffer, mem, 0);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-commandBuffer-00059");
    m_command_buffer.Begin();
    VkBufferMemoryBarrier2 buf_barrier = vku::InitStructHelper();
    buf_barrier.buffer = buffer;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buf_barrier;

    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dep_info);

    vk::FreeMemory(m_device->handle(), mem, NULL);

    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vk::DestroyBuffer(m_device->handle(), buffer, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, Sync2CmdBarrierImageDestroyed) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkImage image;
    VkDeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    auto err = vk::CreateImage(device(), &image_ci, nullptr, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    err = vk::AllocateMemory(device(), &alloc_info, NULL, &image_mem);
    ASSERT_EQ(VK_SUCCESS, err);

    err = vk::BindImageMemory(device(), image, image_mem, 0);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-commandBuffer-00059");
    m_command_buffer.Begin();
    VkImageMemoryBarrier2 img_barrier = vku::InitStructHelper();
    img_barrier.image = image;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &img_barrier;

    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dep_info);

    vk::FreeMemory(m_device->handle(), image_mem, NULL);

    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vk::DestroyImage(m_device->handle(), image, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, CmdBufferBufferViewDestroyed) {
    TEST_DESCRIPTION("Delete bufferView bound to cmd buffer, then attempt to submit cmd buffer.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    CreatePipelineHelper pipe(*this);
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    VkBufferView view;

    {
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        vkt::Buffer buffer(*m_device, buffer_create_info);

        bvci.buffer = buffer.handle();
        bvci.format = VK_FORMAT_R32_SFLOAT;
        bvci.range = VK_WHOLE_SIZE;

        VkResult err = vk::CreateBufferView(device(), &bvci, NULL, &view);
        ASSERT_EQ(VK_SUCCESS, err);

        descriptor_set.WriteDescriptorBufferView(0, view);
        descriptor_set.UpdateDescriptorSets();

        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;
            layout(location=0) out vec4 x;
            void main(){
               x = imageLoad(s, 0);
            }
        )glsl";
        VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
        err = pipe.CreateGraphicsPipeline();
        if (err != VK_SUCCESS) {
            GTEST_SKIP() << "Unable to compile shader";
        }

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

        // Bind pipeline to cmd buffer - This causes crash on Mali
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
    }
    // buffer is released.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");  // buffer
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyBufferView(device(), view, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");  // bufferView
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, buffer_create_info);

    bvci.buffer = buffer.handle();
    VkResult err = vk::CreateBufferView(device(), &bvci, NULL, &view);
    ASSERT_EQ(VK_SUCCESS, err);
    descriptor_set.Clear();
    descriptor_set.WriteDescriptorBufferView(0, view);
    descriptor_set.UpdateDescriptorSets();

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Delete BufferView in order to invalidate cmd buffer
    vk::DestroyBufferView(device(), view, NULL);
    // Now attempt submit of cmd buffer
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, DescriptorSetStorageBufferDestroyed) {
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; };
        void main(){
            x = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    // Destroy the buffer before it's bound to the cmd buffer
    buffer.destroy();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8918
TEST_F(NegativeObjectLifetime, DISABLED_DescriptorSetMutableBufferDestroyed) {
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
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; };
        void main(){
            x = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    buffer.destroy();  // Destroy the buffer before it's bound to the cmd buffer

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8918
TEST_F(NegativeObjectLifetime, DISABLED_DescriptorSetMutableBufferArrayDestroyed) {
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

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 2, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}, 0,
                                       &mdtci);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);  // not used
    vkt::Buffer uniform_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);  // used
    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; } ssbo[2];
        void main(){
            ssbo[1].x = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    uniform_buffer.destroy();  // Destroy the buffer before it's bound to the cmd buffer

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeObjectLifetime, CmdBufferImageDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an image dependency being destroyed.");
    RETURN_IF_SKIP(Init()) {
        const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = tex_format;
        image_create_info.extent = {32, 32, 1};
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_create_info.flags = 0;
        vkt::Image image(*m_device, image_create_info, vkt::set_layout);

        m_command_buffer.Begin();
        VkClearColorValue ccv;
        ccv.float32[0] = 1.0f;
        ccv.float32[1] = 1.0f;
        ccv.float32[2] = 1.0f;
        ccv.float32[3] = 1.0f;
        VkImageSubresourceRange isr = {};
        isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isr.baseArrayLayer = 0;
        isr.baseMipLevel = 0;
        isr.layerCount = 1;
        isr.levelCount = 1;
        vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
        m_command_buffer.End();
    }
    // Destroy image dependency prior to submit to cause ERROR
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, CmdBufferFramebufferImageDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a framebuffer image dependency being destroyed.");
    RETURN_IF_SKIP(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Image format doesn't support required features";
    }
    VkFramebuffer fb;
    VkImageView view;

    InitRenderTarget();
    {
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_ci.extent = {32, 32, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.flags = 0;
        vkt::Image image(*m_device, image_ci, vkt::set_layout);

        VkImageViewCreateInfo ivci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            image.handle(),
            VK_IMAGE_VIEW_TYPE_2D,
            VK_FORMAT_B8G8R8A8_UNORM,
            {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        err = vk::CreateImageView(device(), &ivci, nullptr, &view);
        ASSERT_EQ(VK_SUCCESS, err);

        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 32, 32, 1};
        err = vk::CreateFramebuffer(device(), &fci, nullptr, &fb);
        ASSERT_EQ(VK_SUCCESS, err);

        // Just use default renderpass with our framebuffer
        m_renderPassBeginInfo.framebuffer = fb;
        m_renderPassBeginInfo.renderArea.extent.width = 32;
        m_renderPassBeginInfo.renderArea.extent.height = 32;
        // Create Null cmd buffer for submit
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
    // Destroy image attached to framebuffer to invalidate cmd buffer
    // Now attempt to submit cmd buffer and verify error
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vk::DestroyFramebuffer(device(), fb, nullptr);
    vk::DestroyImageView(device(), view, nullptr);
}

TEST_F(NegativeObjectLifetime, FramebufferAttachmentMemoryFreed) {
    TEST_DESCRIPTION("Attempt to create framebuffer with attachment which memory was freed.");
    RETURN_IF_SKIP(Init());
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Image format doesn't support required features";
    }
    VkFramebuffer fb;

    InitRenderTarget();
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.flags = 0;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    vkt::DeviceMemory *image_memory = new vkt::DeviceMemory;
    image_memory->init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0));
    image.BindMemory(*image_memory, 0);

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    vkt::ImageView view(*m_device, ivci);

    VkFramebufferCreateInfo fci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view.handle(), 32, 32, 1};

    // Introduce error:
    // Free the attachment image memory, then create framebuffer.
    delete image_memory;
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkFramebufferCreateInfo-BoundResourceFreedMemoryAccess");
    vk::CreateFramebuffer(device(), &fci, nullptr, &fb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, DescriptorPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete a DescriptorPool with a DescriptorSet that is in use.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create image to update the descriptor with
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.CreateGraphicsPipeline();

    // Update descriptor with image and sampler
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Submit cmd buffer to put pool in-flight
    m_default_queue->Submit(m_command_buffer);
    // Destroy pool while in-flight, causing error
    m_errorMonitor->SetDesiredError("VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    vk::DestroyDescriptorPool(device(), pipe.descriptor_set_->pool_, NULL);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    m_errorMonitor->SetUnexpectedError(
        "If descriptorPool is not VK_NULL_HANDLE, descriptorPool must be a valid VkDescriptorPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorPool obj");
    // TODO : It seems Validation layers think ds_pool was already destroyed, even though it wasn't?
}

TEST_F(NegativeObjectLifetime, FramebufferInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use framebuffer.");
    RETURN_IF_SKIP(Init());
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    InitRenderTarget();

    vkt::Image image(*m_device, 256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView view = image.CreateView();

    VkFramebufferCreateInfo fci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view.handle(), 256, 256, 1};
    VkFramebuffer fb;
    vk::CreateFramebuffer(device(), &fci, nullptr, &fb);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Submit cmd buffer to put it in-flight
    m_default_queue->Submit(m_command_buffer);
    // Destroy framebuffer while in-flight
    m_errorMonitor->SetDesiredError("VUID-vkDestroyFramebuffer-framebuffer-00892");
    vk::DestroyFramebuffer(device(), fb, NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy everything
    m_default_queue->Wait();
    m_errorMonitor->SetUnexpectedError("If framebuffer is not VK_NULL_HANDLE, framebuffer must be a valid VkFramebuffer handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Framebuffer obj");
    vk::DestroyFramebuffer(device(), fb, nullptr);
}

TEST_F(NegativeObjectLifetime, PushDescriptorUniformDestroySignaled) {
    TEST_DESCRIPTION("Destroy a uniform buffer in use by a push descriptor set");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});
    // Create push descriptor set layout
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&push_ds_layout, &ds_layout});
    helper.CreateGraphicsPipeline();

    const uint32_t data_size = sizeof(float) * 3;
    vkt::Buffer vbo(*m_device, data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo.handle();
    buff_info.offset = 0;
    buff_info.range = data_size;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    m_command_buffer.Begin();

    // In Intel GPU, it needs to bind pipeline before push descriptor set.
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.Handle());
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_layout_.handle(), 0, 1,
                                &descriptor_write);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), vbo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeObjectLifetime, FramebufferImageInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use image that's child of framebuffer.");
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView view = image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle(), 256, 256);

    // Create Null cmd buffer for submit
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Submit cmd buffer to put framebuffer and children in-flight
    m_default_queue->Submit(m_command_buffer);
    // Destroy image attached to framebuffer while in-flight
    m_errorMonitor->SetDesiredError("VUID-vkDestroyImage-image-01000");
    vk::DestroyImage(device(), image.handle(), NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy image and other objects
    m_default_queue->Wait();
    m_errorMonitor->SetUnexpectedError("If image is not VK_NULL_HANDLE, image must be a valid VkImage handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Image obj");
    fb.destroy();
}

TEST_F(NegativeObjectLifetime, EventInUseDestroyedSignaled) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();

    VkEvent event;
    VkEventCreateInfo event_create_info = vku::InitStructHelper();
    vk::CreateEvent(device(), &event_create_info, nullptr, &event);
    vk::CmdSetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    m_command_buffer.End();
    vk::DestroyEvent(device(), event, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, InUseDestroyedSignaled) {
    TEST_DESCRIPTION(
        "Use vkCmdExecuteCommands with invalid state in primary and secondary command buffers. Delete objects that are in use. "
        "Call VkQueueSubmit with an event that has been deleted.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper();
    VkSemaphore semaphore;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore));
    VkFenceCreateInfo fence_create_info = vku::InitStructHelper();
    VkFence fence;
    ASSERT_EQ(VK_SUCCESS, vk::CreateFence(device(), &fence_create_info, nullptr, &fence));

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkEvent event;
    VkEventCreateInfo event_create_info = vku::InitStructHelper();
    vk::CreateEvent(device(), &event_create_info, nullptr, &event);

    m_command_buffer.Begin();

    vk::CmdSetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    m_command_buffer.End();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, fence);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyEvent-event-01145");
    vk::DestroyEvent(device(), event, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkDestroySemaphore-semaphore-05149");
    vk::DestroySemaphore(device(), semaphore, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyFence-fence-01120");
    vk::DestroyFence(device(), fence, nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
    m_errorMonitor->SetUnexpectedError("If semaphore is not VK_NULL_HANDLE, semaphore must be a valid VkSemaphore handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Semaphore obj");
    vk::DestroySemaphore(device(), semaphore, nullptr);
    m_errorMonitor->SetUnexpectedError("If fence is not VK_NULL_HANDLE, fence must be a valid VkFence handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Fence obj");
    vk::DestroyFence(device(), fence, nullptr);
    m_errorMonitor->SetUnexpectedError("If event is not VK_NULL_HANDLE, event must be a valid VkEvent handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Event obj");
    vk::DestroyEvent(device(), event, nullptr);
}

TEST_F(NegativeObjectLifetime, PipelineInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use pipeline.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const vkt::PipelineLayout pipeline_layout(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyPipeline-pipeline-00765");
    // Create PSO to be used for draw-time errors below

    // Store pipeline handle so we can actually delete it before test finishes
    VkPipeline delete_this_pipeline;
    {  // Scope pipeline so it will be auto-deleted
        CreatePipelineHelper pipe(*this);
        pipe.CreateGraphicsPipeline();

        delete_this_pipeline = pipe.Handle();

        m_command_buffer.Begin();
        // Bind pipeline to cmd buffer
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_command_buffer.End();

        // Submit cmd buffer and then pipeline destroyed while in-flight
        m_default_queue->Submit(m_command_buffer);
    }  // Pipeline deletion triggered here
    m_errorMonitor->VerifyFound();
    // Make sure queue finished and then actually delete pipeline
    m_default_queue->Wait();
    m_errorMonitor->SetUnexpectedError("If pipeline is not VK_NULL_HANDLE, pipeline must be a valid VkPipeline handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Pipeline obj");
    vk::DestroyPipeline(m_device->handle(), delete_this_pipeline, nullptr);
}

TEST_F(NegativeObjectLifetime, ImageViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use imageView.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vk::CreateSampler(device(), &sampler_ci, NULL, &sampler);
    ASSERT_EQ(VK_SUCCESS, err);

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView view = image.CreateView();

    // Create PSO to use the sampler
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyImageView-imageView-01026");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Submit cmd buffer and then destroy imageView while in-flight
    m_default_queue->Submit(m_command_buffer);

    vk::DestroyImageView(device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
    // Now we can actually destroy imageView
    m_errorMonitor->SetUnexpectedError("If imageView is not VK_NULL_HANDLE, imageView must be a valid VkImageView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove ImageView obj");
    vk::DestroySampler(device(), sampler, nullptr);
}

TEST_F(NegativeObjectLifetime, BufferViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use bufferView.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferView view;
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    VkResult err = vk::CreateBufferView(device(), &bvci, NULL, &view);
    ASSERT_EQ(VK_SUCCESS, err);

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;
        layout(location=0) out vec4 x;
        void main(){
           x = imageLoad(s, 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    err = pipe.CreateGraphicsPipeline();
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Unable to compile shader";
    }

    pipe.descriptor_set_->WriteDescriptorBufferView(0, view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyBufferView-bufferView-00936");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Submit cmd buffer and then destroy bufferView while in-flight
    m_default_queue->Submit(m_command_buffer);

    vk::DestroyBufferView(device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
    // Now we can actually destroy bufferView
    m_errorMonitor->SetUnexpectedError("If bufferView is not VK_NULL_HANDLE, bufferView must be a valid VkBufferView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove BufferView obj");
    vk::DestroyBufferView(device(), view, NULL);
}

TEST_F(NegativeObjectLifetime, SamplerInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use sampler.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(device(), &sampler_ci, NULL, &sampler);

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView view = image.CreateView();

    // Create PSO to use the sampler
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkDestroySampler-sampler-01082");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Submit cmd buffer and then destroy sampler while in-flight
    m_default_queue->Submit(m_command_buffer);

    vk::DestroySampler(device(), sampler, nullptr);  // Destroyed too soon
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    // Now we can actually destroy sampler
    m_errorMonitor->SetUnexpectedError("If sampler is not VK_NULL_HANDLE, sampler must be a valid VkSampler handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Sampler obj");
    vk::DestroySampler(device(), sampler, NULL);  // Destroyed for real
}

TEST_F(NegativeObjectLifetime, CmdBufferEventDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an event dependency being destroyed.");
    RETURN_IF_SKIP(Init());

    VkEvent event;
    VkEventCreateInfo evci = vku::InitStructHelper();
    VkResult result = vk::CreateEvent(device(), &evci, NULL, &event);
    ASSERT_EQ(VK_SUCCESS, result);

    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    // Destroy event dependency prior to submit to cause ERROR
    vk::DestroyEvent(device(), event, NULL);

    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, ImportFdSemaphoreInUse) {
    TEST_DESCRIPTION("Import semaphore when semaphore is in use.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    if (!SemaphoreExportImportSupported(Gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through fd handle";
    }

    // Create semaphore and export its fd handle
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;
    VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore export_semaphore(*m_device, create_info);
    int fd = -1;
    ASSERT_EQ(VK_SUCCESS, export_semaphore.ExportHandle(fd, handle_type));

    // Create a new semaphore and put it to work
    vkt::Semaphore semaphore(*m_device);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE));

    // Try to import fd handle while semaphore is still in use
    m_errorMonitor->SetDesiredError("VUID-vkImportSemaphoreFdKHR-semaphore-01142");
    semaphore.ImportHandle(fd, handle_type);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeObjectLifetime, ImportWin32SemaphoreInUse) {
    TEST_DESCRIPTION("Import semaphore when semaphore is in use.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (!SemaphoreExportImportSupported(Gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    // Create semaphore and export its Win32 handle
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;
    VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore export_semaphore(*m_device, create_info);
    HANDLE handle = NULL;
    ASSERT_EQ(VK_SUCCESS, export_semaphore.ExportHandle(handle, handle_type));

    // Create a new semaphore and put it to work
    vkt::Semaphore semaphore(*m_device);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE));

    // Try to import Win32 handle while semaphore is still in use
    // Waiting for: https://gitlab.khronos.org/vulkan/vulkan/-/issues/3507
    m_errorMonitor->SetDesiredError("VUID_Undefined");
    semaphore.ImportHandle(handle, handle_type);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}
#endif

TEST_F(NegativeObjectLifetime, LeakAnObject) {
    TEST_DESCRIPTION("Create a fence and destroy its device without first destroying the fence.");

    RETURN_IF_SKIP(InitFramework());
    if (!IsPlatformMockICD()) {
        // This test leaks a fence (on purpose) and should not be run on a real driver
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vkt::PhysicalDevice(Gpu()).queue_properties_;
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice leaky_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_ci, nullptr, &leaky_device));

    const VkFenceCreateInfo fence_ci = vku::InitStructHelper();
    VkFence leaked_fence;
    ASSERT_EQ(VK_SUCCESS, vk::CreateFence(leaky_device, &fence_ci, nullptr, &leaked_fence));

    m_errorMonitor->SetDesiredError("VUID-vkDestroyDevice-device-05137");
    vk::DestroyDevice(leaky_device, nullptr);
    m_errorMonitor->VerifyFound();

    // There's no way we can destroy the fence at this point. Even though DestroyDevice failed, the loader has already removed
    // references to the device
    m_errorMonitor->SetUnexpectedError("VUID-vkDestroyDevice-device-05137");
    m_errorMonitor->SetUnexpectedError("VUID-vkDestroyInstance-instance-00629");
}

TEST_F(NegativeObjectLifetime, LeakABuffer) {
    TEST_DESCRIPTION("Create a fence and destroy its device without first destroying the buffer.");

    RETURN_IF_SKIP(InitFramework());
    if (!IsPlatformMockICD()) {
        // This test leaks a buffer (on purpose) and should not be run on a real driver
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vkt::PhysicalDevice(Gpu()).queue_properties_;
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    auto features = vkt::PhysicalDevice(Gpu()).Features();
    if (!features.sparseBinding) {
        GTEST_SKIP() << "Test requires unsupported sparseBinding feature";
    }

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;
    device_ci.pEnabledFeatures = &features;

    VkDevice leaky_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_ci, nullptr, &leaky_device));

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.size = 1;

    VkBuffer buffer{};
    ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(leaky_device, &buffer_create_info, nullptr, &buffer));

    m_errorMonitor->SetDesiredError("VUID-vkDestroyDevice-device-05137");
    vk::DestroyDevice(leaky_device, nullptr);
    m_errorMonitor->VerifyFound();

    // There's no way we can destroy the buffer at this point.
    // Even though DestroyDevice failed, the loader has already removed references to the device
    m_errorMonitor->SetUnexpectedError("VUID-vkDestroyDevice-device-05137");
    m_errorMonitor->SetUnexpectedError("VUID-vkDestroyInstance-instance-00629");
}

TEST_F(NegativeObjectLifetime, FreeCommandBuffersNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeCommandBuffers");
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 2, nullptr);
    m_errorMonitor->VerifyFound();

    VkCommandBuffer invalid_cb = CastToHandle<VkCommandBuffer, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredError("VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 1, &invalid_cb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, FreeDescriptorSetsNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeDescriptorSets");
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    m_errorMonitor->SetDesiredError("VUID-vkFreeDescriptorSets-pDescriptorSets-00310");
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 2, nullptr);
    m_errorMonitor->VerifyFound();

    VkDescriptorSet invalid_set = CastToHandle<VkDescriptorSet, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredError("VUID-vkFreeDescriptorSets-pDescriptorSets-00310");
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 1, &invalid_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, DescriptorBufferInfoUpdate) {
    TEST_DESCRIPTION("Destroy a buffer then try to update it in the descriptor set");
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBuffer invalid_buffer = CastToHandle<VkBuffer, uintptr_t>(0xbaadbeef);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, invalid_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    buffer.destroy();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-parameter");  // destroyed
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-buffer-parameter");  // invalid
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, DestroyFenceInUseErrorMessage) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8745
    TEST_DESCRIPTION("Test that error message identifies a queue where the fence is used");
    RETURN_IF_SKIP(Init());
    vkt::Fence fence(*m_device);
    m_default_queue->Submit(vkt::no_cmd, fence);
    // The issue was that error message reported that fence was used not by the queue but by the fence itself
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkDestroyFence-fence-01120", "in use by VkQueue");
    vk::DestroyFence(*m_device, fence, nullptr);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeObjectLifetime, DestroySemaphoreInUseErrorMessage) {
    TEST_DESCRIPTION("Test that error message identifies a queue where the semaphore is used");
    RETURN_IF_SKIP(Init());
    vkt::Semaphore semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkDestroySemaphore-semaphore-05149", "in use by VkQueue");
    vk::DestroySemaphore(*m_device, semaphore, nullptr);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}
