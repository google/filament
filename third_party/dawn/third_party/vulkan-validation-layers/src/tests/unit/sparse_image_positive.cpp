/*
 * Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 * Copyright (c) 2023-2024 Collabora, Inc.
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

class PositiveSparseImage : public VkLayerTest {};

TEST_F(PositiveSparseImage, MultipleBinds) {
    TEST_DESCRIPTION("Bind 2 memory ranges to one image using vkQueueBindSparse, destroy the image and then free the memory");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements memory_reqs;
    vk::GetImageMemoryRequirements(device(), image, &memory_reqs);
    // Find an image big enough to allow sparse mapping of 2 memory regions
    // Increase the image size until it is at least twice the
    // size of the required alignment, to ensure we can bind both
    // allocated memory blocks to the image on aligned offsets.
    while (memory_reqs.size < (memory_reqs.alignment * 2)) {
        image.destroy();
        image_create_info.extent.width *= 2;
        image_create_info.extent.height *= 2;
        image.InitNoMemory(*m_device, image_create_info);
        vk::GetImageMemoryRequirements(device(), image, &memory_reqs);
    }
    // Allocate 2 memory regions of minimum alignment size, bind one at 0, the other
    // at the end of the first
    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = memory_reqs.alignment;
    bool pass = m_device->Physical().SetMemoryType(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory_one(*m_device, memory_info);
    vkt::DeviceMemory memory_two(*m_device, memory_info);

    std::array<VkSparseMemoryBind, 2> binds = {};
    binds[0].memory = memory_one;
    binds[0].memoryOffset = 0;
    binds[0].resourceOffset = 0;
    binds[0].size = memory_info.allocationSize;
    binds[1].memory = memory_two;
    binds[1].memoryOffset = 0;
    binds[1].resourceOffset = memory_info.allocationSize;
    binds[1].size = memory_info.allocationSize;

    VkSparseImageOpaqueMemoryBindInfo opaqueBindInfo;
    opaqueBindInfo.image = image;
    opaqueBindInfo.bindCount = size32(binds);
    opaqueBindInfo.pBinds = binds.data();

    VkBindSparseInfo bindSparseInfo = vku::InitStructHelper();
    bindSparseInfo.imageOpaqueBindCount = 1;
    bindSparseInfo.pImageOpaqueBinds = &opaqueBindInfo;

    vk::QueueBindSparse(m_default_queue->handle(), 1, &bindSparseInfo, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseImage, BindFreeMemory) {
    TEST_DESCRIPTION("Test using a sparse image after freeing memory that was bound to it.");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 512;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements memory_reqs;

    vk::GetImageMemoryRequirements(device(), image, &memory_reqs);
    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = memory_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory memory(*m_device, memory_info);

    VkSparseMemoryBind bind;
    bind.flags = 0;
    bind.memory = memory;
    bind.memoryOffset = 0;
    bind.resourceOffset = 0;
    bind.size = memory_info.allocationSize;

    VkSparseImageOpaqueMemoryBindInfo opaqueBindInfo;
    opaqueBindInfo.image = image;
    opaqueBindInfo.bindCount = 1;
    opaqueBindInfo.pBinds = &bind;

    VkBindSparseInfo bindSparseInfo = vku::InitStructHelper();
    bindSparseInfo.imageOpaqueBindCount = 1;
    bindSparseInfo.pImageOpaqueBinds = &opaqueBindInfo;

    // Bind to the memory
    vk::QueueBindSparse(m_default_queue->handle(), 1, &bindSparseInfo, VK_NULL_HANDLE);

    // Bind back to NULL
    bind.memory = VK_NULL_HANDLE;
    vk::QueueBindSparse(m_default_queue->handle(), 1, &bindSparseInfo, VK_NULL_HANDLE);

    m_default_queue->Wait();

    // Free the memory, then use the image in a new command buffer
    memory.destroy();

    m_command_buffer.Begin();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.image = image;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdClearColorImage(m_command_buffer.handle(), image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseImage, BindMetadata) {
    TEST_DESCRIPTION("Bind memory for the metadata aspect of a sparse image");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    // Create a sparse image
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    // Query image memory requirements
    VkMemoryRequirements memory_reqs;
    vk::GetImageMemoryRequirements(device(), image, &memory_reqs);

    // Query sparse memory requirements
    uint32_t sparse_reqs_count = 0;
    vk::GetImageSparseMemoryRequirements(device(), image, &sparse_reqs_count, nullptr);
    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(sparse_reqs_count);
    vk::GetImageSparseMemoryRequirements(device(), image, &sparse_reqs_count, sparse_reqs.data());

    // Find requirements for metadata aspect
    const VkSparseImageMemoryRequirements *metadata_reqs = nullptr;
    for (auto const &aspect_sparse_reqs : sparse_reqs) {
        if ((aspect_sparse_reqs.formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) != 0) {
            metadata_reqs = &aspect_sparse_reqs;
        }
    }

    if (!metadata_reqs) {
        GTEST_SKIP() << "Sparse image does not require memory for metadata";
    }

    // Allocate memory for the metadata
    VkMemoryAllocateInfo metadata_memory_info = vku::InitStructHelper();
    metadata_memory_info.allocationSize = metadata_reqs->imageMipTailSize;
    m_device->Physical().SetMemoryType(memory_reqs.memoryTypeBits, &metadata_memory_info, 0);
    vkt::DeviceMemory metadata_memory(*m_device, metadata_memory_info);

    // Bind metadata
    VkSparseMemoryBind sparse_bind = {};
    sparse_bind.resourceOffset = metadata_reqs->imageMipTailOffset;
    sparse_bind.size = metadata_reqs->imageMipTailSize;
    sparse_bind.memory = metadata_memory;
    sparse_bind.memoryOffset = 0;
    sparse_bind.flags = VK_SPARSE_MEMORY_BIND_METADATA_BIT;

    VkSparseImageOpaqueMemoryBindInfo opaque_bind_info = {};
    opaque_bind_info.image = image;
    opaque_bind_info.bindCount = 1;
    opaque_bind_info.pBinds = &sparse_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageOpaqueBindCount = 1;
    bind_info.pImageOpaqueBinds = &opaque_bind_info;

    vk::QueueBindSparse(m_default_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseImage, OpImageSparse) {
    TEST_DESCRIPTION("Use OpImageSparse* operations at draw time");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    AddRequiredFeature(vkt::Feature::shaderResourceResidency);
    RETURN_IF_SKIP(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    ds.UpdateDescriptorSets();

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_ARB_sparse_texture2 : enable

        layout(set = 0, binding = 0) uniform sampler2D s2D;

        layout(location = 0) out vec4 outColor;

        void main() {
            vec4 texel = vec4(1.0);
            int resident = 0;
            resident |= sparseTextureARB(s2D, vec2(0.0), texel);
            resident |= sparseTextureLodARB(s2D, vec2(0.0), 2.0, texel);
            resident |= sparseTexelFetchARB(s2D, ivec2(0), 2, texel);

            outColor = sparseTexelsResidentARB(resident) ? vec4(0.0) : vec4(1.0);
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1, &ds.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSparseImage, BindImage) {
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {512, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkSparseImageMemoryBind image_memory_bind = {};
    image_memory_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_bind.extent = image_create_info.extent;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image.handle();
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageBindCount = 1;
    bind_info.pImageBinds = &image_memory_bind_info;

    vkt::Queue *sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    sparse_queue->Wait();
}