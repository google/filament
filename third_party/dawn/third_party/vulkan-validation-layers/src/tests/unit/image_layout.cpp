/*
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class NegativeImageLayout : public ImageTest {};

TEST_F(NegativeImageLayout, Blit) {
    TEST_DESCRIPTION("Incorrect vkCmdBlitImage layouts");
    RETURN_IF_SKIP(Init());

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;

    vkt::Image img_src_transfer(*m_device, 64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img_dst_transfer(*m_device, 64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img_general(*m_device, 64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img_color(*m_device, 64, 64, 1, fmt,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    img_src_transfer.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    img_dst_transfer.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    img_general.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    img_color.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkImageBlit blit_region = {};
    blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {32, 32, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};

    m_command_buffer.Begin();

    vk::CmdBlitImage(m_command_buffer.handle(), img_general.handle(), img_general.Layout(), img_general.handle(),
                     img_general.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    // Illegal srcImageLayout
    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImageLayout-01398");
    vk::CmdBlitImage(m_command_buffer.handle(), img_src_transfer.handle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     img_dst_transfer.handle(), img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();

    // Illegal destImageLayout
    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-dstImageLayout-01399");
    vk::CmdBlitImage(m_command_buffer.handle(), img_src_transfer.handle(), img_src_transfer.Layout(), img_dst_transfer.handle(),
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();

    m_command_buffer.Reset(0);
    m_command_buffer.Begin();

    // Source image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    vk::CmdBlitImage(m_command_buffer.handle(), img_src_transfer.handle(), img_src_transfer.Layout(), img_color.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    m_command_buffer.Reset(0);
    m_command_buffer.Begin();

    // Destination image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    vk::CmdBlitImage(m_command_buffer.handle(), img_color.handle(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.handle(),
                     img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    // Source image in invalid layout in the middle of CB
    m_command_buffer.Reset(0);
    m_command_buffer.Begin();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = img_general.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImageLayout-00221");
    vk::CmdBlitImage(m_command_buffer.handle(), img_general.handle(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.handle(),
                     img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    // Destination image in invalid layout in the middle of CB
    m_command_buffer.Reset(0);
    m_command_buffer.Begin();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.image = img_dst_transfer.handle();

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-dstImageLayout-00226");
    vk::CmdBlitImage(m_command_buffer.handle(), img_src_transfer.handle(), img_src_transfer.Layout(), img_dst_transfer.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeImageLayout, Compute) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    {  // Verify invalid image layout with CmdDispatch
        vkt::CommandBuffer cmd(*m_device, m_command_pool);
        cmd.Begin();
        vk::CmdBindDescriptorSets(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);
        vk::CmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(cmd.handle(), 1, 1, 1);
        cmd.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
        m_default_queue->Submit(cmd);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    {  // Verify invalid image layout with CmdDispatchBaseKHR
        vkt::CommandBuffer cmd(*m_device, m_command_pool);
        cmd.Begin();
        vk::CmdBindDescriptorSets(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);
        vk::CmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatchBaseKHR(cmd.handle(), 0, 0, 0, 1, 1, 1);
        cmd.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
        m_default_queue->Submit(cmd);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImageLayout, Compute11) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader using vkCmdDispatchBase");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatchBase(m_command_buffer.handle(), 0, 0, 0, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// inspired by https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5846
TEST_F(NegativeImageLayout, MultipleCommandDispatches) {
    TEST_DESCRIPTION("Make sure we can detect the exact dispatch command that caused the error later");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        void main(){
            vec4 v = texture(s, vec2(0.0));
        }
    )glsl";

    OneOffDescriptorSet descriptor_set0(m_device,
                                        {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet descriptor_set1(m_device,
                                        {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set0.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Image good_image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView good_image_view = good_image.CreateView();
    descriptor_set0.WriteDescriptorImageInfo(0, good_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set0.UpdateDescriptorSets();

    vkt::Image bad_image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    bad_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView bad_image_view = bad_image.CreateView();
    descriptor_set1.WriteDescriptorImageInfo(0, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set1.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    // contains good image
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // contains bad image
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // contains good image again
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8918
// This fails to create descriptor on RADV, might need to adjust OneOffDescriptorSet
TEST_F(NegativeImageLayout, DISABLED_Mutable) {
    TEST_DESCRIPTION("Invalid image layout with mutable descriptors");
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        void main(){
            vec4 v = 2.0 * texture(s, vec2(0.0));
        }
    )glsl";

    VkDescriptorType desc_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 64, 64, 1, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, PushDescriptor) {
    TEST_DESCRIPTION("Use a push descriptor with a mismatched image layout.");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    auto pipeline_layout = vkt::PipelineLayout(*m_device, {&ds_layout});

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D tex;
        layout(location=0) out vec4 color;
        void main(){
           color = textureLod(tex, vec2(0.5, 0.5), 0.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler.handle();
    img_info.imageView = image_view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.dstBinding = 0;

    for (uint32_t i = 0; i < 2; i++) {
        m_command_buffer.Begin();
        if (i == 1) {
            // Test path where image layout in command buffer is known at draw time
            image.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                     VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        }
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                    &descriptor_write);

        if (i == 1) {
            // Test path where image layout in command buffer is known at draw time
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
            m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-imageLayout-00344");
            vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
            m_errorMonitor->VerifyFound();
            break;
        }
        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// INVALID_IMAGE_LAYOUT tests (one other case is hit by MapMemWithoutHostVisibleBit and not here)
TEST_F(NegativeImageLayout, Basic) {
    TEST_DESCRIPTION("Generally these involve having images in the wrong layout when they're copied or transitioned.");
    // 3 in ValidateCmdBufImageLayouts
    // *  -1 Attempt to submit cmd buf w/ deleted image
    // *  -2 Cmd buf submit of image w/ layout not matching first use w/ subresource
    // *  -3 Cmd buf submit of image w/ layout not matching first use w/o subresource

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;
    vkt::Image src_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_D16_UNORM;
    image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image depth_image(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent.width = 1;
    copy_region.extent.height = 1;
    copy_region.extent.depth = 1;

    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImageLayout-01917");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImageLayout-00128");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImageLayout-00133");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImageLayout-01395");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_UNDEFINED, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent tests using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageCopy2 copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2,
                                           NULL,
                                           copy_region.srcSubresource,
                                           copy_region.srcOffset,
                                           copy_region.dstSubresource,
                                           copy_region.dstOffset,
                                           copy_region.extent};
        VkCopyImageInfo2 copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
                                             NULL,
                                             src_image.handle(),
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             dst_image.handle(),
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             1,
                                             &copy_region2};

        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);

        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImageLayout-00128");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImageLayout-01917");
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();

        // Now verify same checks for dst
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);

        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImageLayout-00133");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImageLayout-01395");
        copy_image_info2.dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Convert dst and depth images to TRANSFER_DST for subsequent tests
    VkImageMemoryBarrier transfer_dst_image_barrier[1] = {};
    transfer_dst_image_barrier[0] = vku::InitStructHelper();
    transfer_dst_image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transfer_dst_image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transfer_dst_image_barrier[0].srcAccessMask = 0;
    transfer_dst_image_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transfer_dst_image_barrier[0].image = dst_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    transfer_dst_image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, transfer_dst_image_barrier);
    transfer_dst_image_barrier[0].image = depth_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, transfer_dst_image_barrier);

    // Cause errors due to clearing with invalid image layouts
    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail due to explicitly prohibited layout for color clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-01394");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-00004");
    vk::CmdClearColorImage(m_command_buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for color clear.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-00004");
    vk::CmdClearColorImage(m_command_buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();

    VkClearDepthStencilValue depth_clear_value = {};
    clear_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Fail due to explicitly prohibited layout for depth clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-imageLayout-00012");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), depth_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &depth_clear_value, 1,
                                  &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for depth clear.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &depth_clear_value, 1,
                                  &clear_range);
    m_errorMonitor->VerifyFound();

    VkImageMemoryBarrier image_barrier[1] = {};
    // In synchronization2, if oldLayout == newLayout, we're not doing an ILT and these fields don't need to match
    // the image's layout.
    image_barrier[0] = vku::InitStructHelper();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].image = src_image.handle();
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, image_barrier);

    // Now cause error due to bad image layout transition in PipelineBarrier
    image_barrier[0] = vku::InitStructHelper();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_barrier[0].image = src_image.handle();
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-oldLayout-01197");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-oldLayout-01210");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, image_barrier);
    m_errorMonitor->VerifyFound();

    // Finally some layout errors at RenderPass create time
    // Just hacking in specific state to get to the errors we want so don't copy this unless you know what you're doing.
    VkAttachmentReference attach = {};
    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 0;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &attach;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkRenderPass rp;

    // For this error we need a valid renderpass so create default one
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach.attachment = 0;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = depth_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Can't do a CLEAR load on READ_ONLY initialLayout
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rpci.pAttachments = &attach_desc;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-03283");
    vk::CreateRenderPass(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, StorageImage) {
    TEST_DESCRIPTION("Attempt to update a STORAGE_IMAGE descriptor w/o GENERAL layout.");

    RETURN_IF_SKIP(Init());

    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    if ((m_device->FormatFeaturesOptimal(tex_format) & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT; skipped.";
    }

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    vkt::Image image(*m_device, 32, 32, 1, tex_format, VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::ImageView view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-04152");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, ArrayLayers) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);

    // layer 0 now VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // layer 1 is still VK_IMAGE_LAYOUT_UNDEFINED.
    m_command_buffer.Begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.image = image;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Get layer with undefined layout
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 1, 1);

    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, MultiArrayLayers) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);

    // layer 0 now VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // layer 1 is still VK_IMAGE_LAYOUT_UNDEFINED.
    m_command_buffer.Begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.image = image;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Bind view to both layers
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    const char *fs_source = R"glsl(
        #version 460
        layout(set=0, binding=0) uniform sampler2DArray s;
        layout(location=0) out vec4 x;
        void main(){
            x = texture(s, vec3(1, 1, 1)); // accesses invalid layer
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DescriptorArrayStaticIndex) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        // [0] is good layout
        // [1] is bad layout
        layout(set = 0, binding = 0) uniform sampler2D tex[2];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[1], vec2(0, 0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Image bad_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image good_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView bad_image_view = bad_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::ImageView good_image_view = good_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    descriptor_set.WriteDescriptorImageInfo(0, good_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
