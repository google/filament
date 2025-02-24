/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2023 Google, Inc.
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

class NegativeGpuAVImageLayout : public GpuAVImageLayout {};

TEST_F(NegativeGpuAVImageLayout, ImageArrayDynamicIndexing) {
    TEST_DESCRIPTION("GPU validation: test that only dynamically used indices are validated");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    RETURN_IF_SKIP(InitGpuAVImageLayout());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    const uint32_t kDescCount = 40;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[kDescCount] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[kDescCount] = {};
    for (int i = 0; i < kDescCount; i++) {
        image_info[i] = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorCount = kDescCount;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;

    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";

    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 35;
    buffer0.Memory().Unmap();

    // VUID-vkCmdDraw-None-09600
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.");

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - cb_state.image_layout_map is empty after updating a mutable descriptor
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8918
TEST_F(NegativeGpuAVImageLayout, DISABLED_Mutable) {
    TEST_DESCRIPTION("Invalid image layout with mutable descriptors");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(InitGpuAVImageLayout());

    const char *cs = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s[2];
        layout(set=0, binding=1) buffer SSBO { uint index; };
        void main(){
            vec4 v = 2.0 * texture(s[index], vec2(0.0));
        }
    )glsl";

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *storage_buffer_ptr = (uint32_t *)storage_buffer.Memory().Map();
    storage_buffer_ptr[0] = 1;
    storage_buffer.Memory().Unmap();

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

    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 2, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                       0, &mdtci);
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

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
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

TEST_F(NegativeGpuAVImageLayout, DescriptorArrayLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAVImageLayout());
    RETURN_IF_SKIP(InitRenderTarget());

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform UBO { uint index; };
        // [0] is good layout
        // [1] is bad layout
        layout(set = 0, binding = 1) uniform sampler2D tex[2];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                                   {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                               });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *in_buffer_ptr = (uint32_t *)in_buffer.Memory().Map();
    in_buffer_ptr[0] = 1;
    in_buffer.Memory().Unmap();

    vkt::Image bad_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image good_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView bad_image_view = bad_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::ImageView good_image_view = good_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorImageInfo(1, good_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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

TEST_F(NegativeGpuAVImageLayout, MultiArrayLayers) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(InitGpuAVImageLayout());
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

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    const char *fs_source = R"glsl(
        #version 460
        layout(set=0, binding=0) uniform sampler2DArray s;
        layout(set=0, binding=1) uniform UBO { uint index; };
        layout(location=0) out vec4 x;
        void main(){
            x = texture(s, vec3(1, 1, index));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *in_buffer_ptr = (uint32_t *)in_buffer.Memory().Map();
    in_buffer_ptr[0] = 1;
    in_buffer.Memory().Unmap();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(1, in_buffer, 0, VK_WHOLE_SIZE);
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

TEST_F(NegativeGpuAVImageLayout, MultipleCommandBuffersSameDescriptorSet) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAVImageLayout());

    vkt::CommandBuffer cb_0(*m_device, m_command_pool);
    vkt::CommandBuffer cb_1(*m_device, m_command_pool);

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image bad_image(*m_device, image_ci);
    vkt::Image good_image(*m_device, image_ci);
    bad_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView bad_view = bad_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::ImageView good_view = good_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) buffer SSBO {
            uint index;
            vec4 out_value;
        };
        layout(set=0, binding=1) uniform sampler2D sample_array[2];
        void main() {
           out_value = texture(sample_array[index], vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    cb_0.Begin();
    vk::CmdBindPipeline(cb_0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb_0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb_0.handle(), 1, 1, 1);
    cb_0.End();

    cb_1.Begin();
    vk::CmdBindPipeline(cb_1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb_1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb_1.handle(), 1, 1, 1);
    cb_1.End();

    uint32_t *in_buffer_ptr = (uint32_t *)buffer.Memory().Map();
    in_buffer_ptr[0] = 0;
    buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(1, good_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, good_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();
    m_default_queue->Submit(cb_0);
    m_default_queue->Wait();

    descriptor_set.WriteDescriptorImageInfo(1, bad_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(cb_1);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure if we fix it afterwards, the VU goes away
    bad_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_default_queue->Submit(cb_0);
    m_default_queue->Submit(cb_1);
    m_default_queue->Wait();
}
