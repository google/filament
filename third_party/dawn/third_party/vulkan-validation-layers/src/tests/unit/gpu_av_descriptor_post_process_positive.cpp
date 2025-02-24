/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class PositiveGpuAVDescriptorPostProcess : public GpuAVDescriptorIndexingTest {};

TEST_F(PositiveGpuAVDescriptorPostProcess, MixingProtectedResources) {
    TEST_DESCRIPTION("Have protected resources, but don't actually access it so no VU is triggered");
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(protected_memory_properties);
    if (protected_memory_properties.protectedNoFault) {
        GTEST_SKIP() << "protectedNoFault is supported";
    }

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vkt::Image image_protected(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs_image_protected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_image_protected);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs_image_protected.size;
    if (!m_device->Physical().SetMemoryType(mem_reqs_image_protected.memoryTypeBits, &alloc_info,
                                            VK_MEMORY_PROPERTY_PROTECTED_BIT)) {
        GTEST_SKIP() << "Memory type not found";
    }
    vkt::DeviceMemory memory_image_protected(*m_device, alloc_info);
    vk::BindImageMemory(device(), image_protected.handle(), memory_image_protected.handle(), 0);
    image_protected.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_protected = image_protected.CreateView();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view_protected, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index;
        } in_buffer;

        // [0] non-protected
        // [1] protected
        layout(set = 0, binding = 1) uniform sampler2D tex[];

        void main() {
           vec4 result = texture(tex[in_buffer.index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageBindingPartiallyBound) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D float_textures[2];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[2];
        layout(set = 0, binding = 1) buffer output_buffer {
            uint index;
            vec4 data;
        };

        void main() {
            const vec4 value = texelFetch(float_textures[index], ivec2(0), 0);
            const uint mask = texelFetch(uint_textures[index + 1], ivec2(0), 0).x;
            data = mask > 0 ? value : vec4(0.0);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
         {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    *data = 0;
    buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(0, float_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, uint_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageBindingRuntimeArray) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D float_textures[];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[];
        layout(set = 0, binding = 1) buffer output_buffer {
            uint index;
            vec4 data;
        };

        void main() {
            const vec4 value = texelFetch(float_textures[nonuniformEXT(index)], ivec2(0), 0);
            const uint mask = texelFetch(uint_textures[nonuniformEXT(index + 1)], ivec2(0), 0).x;
            data = mask > 0 ? value : vec4(0.0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci);
    float_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci);
    uint_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    *data = 0;
    buffer.Memory().Unmap();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, float_image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL, 0);
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, uint_image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL, 1);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisample) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        layout (constant_id = 0) const int path = 0; // always zero, but prevents dead code elimination

        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
            if (path > 10) {
                dummy += texelFetch(BaseTextureMS, ivec2(0), 0);
            }
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisampleDescriptorSets) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across two descriptor sets");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";

    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorSet descriptor_set0(m_device, {
                                                      {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                      {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                      {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  });
    const vkt::PipelineLayout pipeline_layout0(*m_device, {&descriptor_set0.layout_});
    descriptor_set0.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set0.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set0.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();

    OneOffDescriptorSet descriptor_set1(m_device, {
                                                      {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                      {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  });
    const vkt::PipelineLayout pipeline_layout1(*m_device, {&descriptor_set1.layout_});
    descriptor_set1.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set1.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout0.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout1.handle();
    pipe_ms.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout1.handle(), 0, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisampleDescriptorSetsPartiallyBound) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across two descriptor sets");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";
    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";
    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorIndexingSet descriptor_set0(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout0(*m_device, {&descriptor_set0.layout_});
    descriptor_set0.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set0.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set0.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();

    OneOffDescriptorIndexingSet descriptor_set1(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout1(*m_device, {&descriptor_set1.layout_});
    descriptor_set1.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set1.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout0.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout1.handle();
    pipe_ms.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout1.handle(), 0, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisampleDispatches) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across dispatches");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };

        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";

    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };

        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout.handle();
    pipe_ms.CreateComputePipeline();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    descriptor_set.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, NonMultisampleMismatchWithPipeline) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image bad_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView bad_image_view = bad_image.CreateView();

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image good_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView good_image_view = good_image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                   {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                               });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, bad_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, good_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        // mySampler[0] is bad
        // mySampler[1] is good
        layout(set=0, binding=0) uniform sampler2D mySampler[2];
        layout(set=0, binding=1) buffer SSBO {
            int index;
            vec4 out_value;
        };
        void main() {
           out_value = texelFetch(mySampler[index], ivec2(0), 0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, SharedDescriptorDifferentOpVariableId) {
    TEST_DESCRIPTION("Same descriptor set is accessed by 2 different shaders, so the Variable IDs will not match other pipeline");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Buffer buffer(*m_device, 60, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;
    buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source0 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) uniform sampler2D sample_array[];
        layout(set=0, binding=1) buffer SSBO {
            uint index;
            vec4 out_value;
        };
        void main() {
           out_value = texture(sample_array[index], vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe0(*this);
    pipe0.cs_ = std::make_unique<VkShaderObj>(this, cs_source0, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe0.cp_ci_.layout = pipeline_layout.handle();
    pipe0.CreateComputePipeline();

    // This is the same shader as above, but the goal is to make sure the OpVariable ID are different as the descriptor is being
    // aliased
    char const *cs_source1 = R"(
               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_ %sample_array
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 16
               OpDecorate %_ Binding 1
               OpDecorate %_ DescriptorSet 0
               OpDecorate %sample_array Binding 0
               OpDecorate %sample_array DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %SSBO = OpTypeStruct %uint %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
         %15 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %16 = OpTypeSampledImage %15
%runtimearr = OpTypeRuntimeArray %16
      %int_0 = OpConstant %int 0
%_ptr_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_v4float = OpTypePointer StorageBuffer %v4float
            ;; Variables moved here
  %_ptr_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_SSBO StorageBuffer
%_ptr_runtimearr = OpTypePointer UniformConstant %runtimearr
%sample_array = OpVariable %_ptr_runtimearr UniformConstant
       %main = OpFunction %void None %4
          %6 = OpLabel
         %22 = OpAccessChain %_ptr_uint %_ %int_0
         %23 = OpLoad %uint %22
         %25 = OpAccessChain %_ptr_UniformConstant_16 %sample_array %23
         %26 = OpLoad %16 %25
         %30 = OpImageSampleExplicitLod %v4float %26 %29 Lod %float_0
         %32 = OpAccessChain %_ptr_v4float %_ %int_1
               OpStore %32 %30
               OpReturn
               OpFunctionEnd
    )";
    CreateComputePipelineHelper pipe1(*this);
    pipe1.cs_ = std::make_unique<VkShaderObj>(this, cs_source1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe1.cp_ci_.layout = pipeline_layout.handle();
    pipe1.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe0.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe1.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, DescriptorIndexingSlang) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // [[vk::binding(0, 0)]]
    // Texture2D texture[];
    // [[vk::binding(1, 0)]]
    // SamplerState sampler;
    //
    // struct StorageBuffer {
    //     uint data; // Will be zero
    //     float4 color;
    // };
    //
    // [[vk::binding(2, 0)]]
    // RWStructuredBuffer<StorageBuffer> storageBuffer;
    //
    // [shader("compute")]
    // void main() {
    //     uint dataIndex = storageBuffer[0].data;
    //     float4 sampledColor = texture[dataIndex].Sample(sampler, float2(0));
    //     storageBuffer[0].color = sampledColor;
    // }
    char const *cs_source = R"(
               OpCapability RuntimeDescriptorArray
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %storageBuffer %texture %sampler
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %StorageBuffer_std430 0 Offset 0
               OpMemberDecorate %StorageBuffer_std430 1 Offset 16
               OpDecorate %_runtimearr_StorageBuffer_std430 ArrayStride 32
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %storageBuffer Binding 2
               OpDecorate %storageBuffer DescriptorSet 0
               OpDecorate %_runtimearr_21 ArrayStride 8
               OpDecorate %texture Binding 0
               OpDecorate %texture DescriptorSet 0
               OpDecorate %sampler Binding 1
               OpDecorate %sampler DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%StorageBuffer_std430 = OpTypeStruct %uint %v4float
%_ptr_StorageBuffer_StorageBuffer_std430 = OpTypePointer StorageBuffer %StorageBuffer_std430
%_runtimearr_StorageBuffer_std430 = OpTypeRuntimeArray %StorageBuffer_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_StorageBuffer_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
         %21 = OpTypeImage %float 2D 2 0 0 1 Unknown
%_runtimearr_21 = OpTypeRuntimeArray %21
%_ptr_UniformConstant__runtimearr_21 = OpTypePointer UniformConstant %_runtimearr_21
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
         %28 = OpTypeSampler
%_ptr_UniformConstant_28 = OpTypePointer UniformConstant %28
         %32 = OpTypeSampledImage %21
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %36 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
%storageBuffer = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
    %texture = OpVariable %_ptr_UniformConstant__runtimearr_21 UniformConstant
    %sampler = OpVariable %_ptr_UniformConstant_28 UniformConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
         %12 = OpAccessChain %_ptr_StorageBuffer_StorageBuffer_std430 %storageBuffer %int_0 %int_0
         %18 = OpAccessChain %_ptr_StorageBuffer_uint %12 %int_0
  %dataIndex = OpLoad %uint %18
         %25 = OpAccessChain %_ptr_UniformConstant_21 %texture %dataIndex
         %26 = OpLoad %21 %25
         %27 = OpLoad %21 %25
         %29 = OpLoad %28 %sampler
%sampledImage = OpSampledImage %32 %27 %29
    %sampled = OpImageSampleExplicitLod %v4float %sampledImage %36 None
         %38 = OpCopyObject %v4float %sampled
         %39 = OpAccessChain %_ptr_StorageBuffer_StorageBuffer_std430 %storageBuffer %int_0 %int_0
         %42 = OpAccessChain %_ptr_StorageBuffer_v4float %39 %int_1
               OpStore %42 %38
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    *buffer_ptr = 0;
    buffer.Memory().Unmap();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, ZeroBindingDescriptor) {
    TEST_DESCRIPTION(
        "Have an unused descriptor slot actually be backed by zero bindings (and therefor having no post process buffer)");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : enable
        layout(set = 1, binding = 0) uniform texture2D textures[2];
        layout(set = 1, binding = 1) buffer output_buffer {
            uint index;
            vec4 data;
        };

        void main() {
            data = texelFetch(textures[index], ivec2(0), 0);
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    OneOffDescriptorSet descriptor_set_unused(m_device, {{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    // Create a VkDescriptorSet backed by zero bindings
    VkDescriptorSetLayoutCreateInfo empty_set_layout_ci = vku::InitStructHelper();
    empty_set_layout_ci.bindingCount = 0;
    empty_set_layout_ci.pBindings = nullptr;
    vkt::DescriptorSetLayout empty_set_layout(*m_device, empty_set_layout_ci);

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &empty_set_layout.handle();

    VkDescriptorSet descriptor_set_empty = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(device(), &allocate_info, &descriptor_set_empty);

    const vkt::PipelineLayout pipeline_layout1(*m_device, {&descriptor_set_unused.layout_, &descriptor_set.layout_});
    const vkt::PipelineLayout pipeline_layout2(*m_device, {&empty_set_layout, &descriptor_set.layout_});

    CreateComputePipelineHelper pipe1(*this);
    pipe1.cp_ci_.layout = pipeline_layout1.handle();
    pipe1.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe1.CreateComputePipeline();

    CreateComputePipelineHelper pipe2(*this);
    pipe2.cp_ci_.layout = pipeline_layout2.handle();
    pipe2.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe2.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;
    buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    VkDescriptorSet sets[2] = {descriptor_set_unused.set_, descriptor_set.set_};
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout1, 0, 2, sets, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    sets[0] = descriptor_set_empty;
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout2, 0, 2, sets, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
