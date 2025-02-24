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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../../layers/gpuav/shaders/gpuav_shaders_constants.h"

class PositiveGpuAV : public GpuAVTest {};

static const std::array gpu_av_enables = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                          VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
static const std::array gpu_av_disables = {VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
                                           VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

// All GpuAVTest should use this for setup as a single access point to more easily toggle which validation features are
// enabled/disabled
VkValidationFeaturesEXT GpuAVTest::GetGpuAvValidationFeatures() {
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = size32(gpu_av_enables);
    features.pEnabledValidationFeatures = gpu_av_enables.data();
    if (m_gpuav_disable_core) {
        features.disabledValidationFeatureCount = size32(gpu_av_disables);
        features.pDisabledValidationFeatures = gpu_av_disables.data();
    }
    return features;
}

// This checks any requirements needed for GPU-AV are met otherwise devices not meeting them will "fail" the tests
void GpuAVTest::InitGpuAvFramework(void *p_next) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    validation_features.pNext = p_next;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
}

TEST_F(PositiveGpuAV, ReserveBinding) {
    TEST_DESCRIPTION(
        "verify that VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT is properly reserving a descriptor slot");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState(nullptr));

    auto ici = GetInstanceCreateInfo();
    VkInstance test_inst;
    vk::CreateInstance(&ici, nullptr, &test_inst);
    uint32_t gpu_count;
    vk::EnumeratePhysicalDevices(test_inst, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> phys_devices(gpu_count);
    vk::EnumeratePhysicalDevices(test_inst, &gpu_count, phys_devices.data());

    VkPhysicalDeviceProperties properties;
    vk::GetPhysicalDeviceProperties(phys_devices[m_gpu_index], &properties);
    if (m_device->Physical().limits_.maxBoundDescriptorSets != properties.limits.maxBoundDescriptorSets - 1) {
        m_errorMonitor->SetError("VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT not functioning as expected");
    }
    vk::DestroyInstance(test_inst, nullptr);
}

TEST_F(PositiveGpuAV, InlineUniformBlock) {
    TEST_DESCRIPTION("Make sure inline uniform blocks don't generate false validation errors");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDescriptorBindingFlags ds_binding_flags[2] = {};
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags[1] = {};
    layout_createinfo_binding_flags[0] = vku::InitStructHelper();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 32;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 32, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, layout_createinfo_binding_flags, 0, nullptr, &pool_inline_info);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    const uint32_t test_data = 0xdeadca7;
    VkWriteDescriptorSetInlineUniformBlock write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &test_data;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;

    descriptor_writes[1] = vku::InitStructHelper(&write_inline_uniform);
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 16;  // Skip first 16 bytes (dummy)
    descriptor_writes[1].descriptorCount = 4;   // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, NULL);

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;
        layout(set = 0, binding = 1) uniform inlineubodef { ivec4 dummy; int val; } inlineubo;

        void main() {
            u_index.index = inlineubo.val;
        }
        )glsl";

    CreateComputePipelineHelper pipe1(*this);
    pipe1.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe1.cp_ci_.layout = pipeline_layout.handle();
    pipe1.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    ASSERT_TRUE(*data = test_data);
    *data = 0;
    buffer.Memory().Unmap();
}

TEST_F(PositiveGpuAV, InlineUniformBlockAndRecovery) {
    TEST_DESCRIPTION(
        "GPU validation: Make sure inline uniform blocks don't generate false validation errors, verify reserved descriptor slot "
        "and verify pipeline recovery");
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDescriptorBindingFlags ds_binding_flags[2] = {};
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags[1] = {};
    layout_createinfo_binding_flags[0] = vku::InitStructHelper();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 32;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 32, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, layout_createinfo_binding_flags, 0, nullptr, &pool_inline_info);

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    const uint32_t test_data = 0xdeadca7;
    VkWriteDescriptorSetInlineUniformBlock write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &test_data;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;

    descriptor_writes[1] = vku::InitStructHelper(&write_inline_uniform);
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 16;  // Skip first 16 bytes (dummy)
    descriptor_writes[1].descriptorCount = 4;   // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    const uint32_t set_count = m_device->Physical().limits_.maxBoundDescriptorSets + 1;  // account for reserved set
    VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(inline_uniform_props);
    if (inline_uniform_props.maxPerStageDescriptorInlineUniformBlocks < set_count) {
        GTEST_SKIP() << "Max per stage inline uniform block limit too small - skipping recovery portion of this test";
    }

    // Now be sure that recovery from an unavailable descriptor set works and that uninstrumented shaders are used
    std::vector<const vkt::DescriptorSetLayout *> layouts(set_count);
    for (uint32_t i = 0; i < set_count; i++) {
        layouts[i] = &descriptor_set.layout_;
    }
    // Expect warning since GPU-AV cannot add debug descriptor to layout
    m_errorMonitor->SetDesiredWarning(
        "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
        "pipelines created with it");
    vkt::PipelineLayout pl_layout(*m_device, layouts);
    m_errorMonitor->VerifyFound();

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;
        layout(set = 0, binding = 1) uniform inlineubodef { ivec4 dummy; int val; } inlineubo;

        void main() {
            u_index.index = inlineubo.val;
        }
    )glsl";

    {
        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
        // We should still be able to use the layout and create a temporary uninstrumented shader module
        pipe.cp_ci_.layout = pl_layout.handle();
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pl_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        pl_layout.destroy();

        uint32_t *data = (uint32_t *)buffer.Memory().Map();
        if (*data != test_data)
            m_errorMonitor->SetError("Pipeline recovery when resources unavailable not functioning as expected");
        *data = 0;
        buffer.Memory().Unmap();
    }

    // Now make sure we can still use the shader with instrumentation
    {
        const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
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
        uint32_t *data = (uint32_t *)buffer.Memory().Map();
        if (*data != test_data) m_errorMonitor->SetError("Using shader after pipeline recovery not functioning as expected");
        *data = 0;
        buffer.Memory().Unmap();
    }
}

// TODO - This test "could" trigger a warning, but we would have to allocate 4k buffer and track each DWORD updated/written to it,
// so currently we just make sure we don't generate false positives.
TEST_F(PositiveGpuAV, InlineUniformBlockUninitialized) {
    TEST_DESCRIPTION(
        "If you don't update an inline uniform block and accessed it, you get undefined values, but it is not invalid and we "
        "should not crash or trigger an error.");
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO { uint out_buffer; };
        layout(set = 0, binding = 1) uniform InlineUBO {
            uint updated_value;
            uint non_updated_value; // will be undefined value
        };
        void main() {
            out_buffer = non_updated_value;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 1;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, nullptr, 0, nullptr, &pool_inline_info);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const uint32_t update_value = 0;
    VkWriteDescriptorSetInlineUniformBlock write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &update_value;
    VkWriteDescriptorSet descriptor_writes = vku::InitStructHelper(&write_inline_uniform);
    descriptor_writes.dstSet = descriptor_set.set_;
    descriptor_writes.dstBinding = 1;
    descriptor_writes.dstArrayElement = 0;
    descriptor_writes.descriptorCount = 4;
    descriptor_writes.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_writes, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
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

TEST_F(PositiveGpuAV, InlineUniformBlockUninitializedUpdateAfterBind) {
    TEST_DESCRIPTION(
        "If you don't update an inline uniform block and accessed it, you get undefined values, but it is not invalid and we "
        "should not crash or trigger an error.");
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    AddRequiredFeature(vkt::Feature::descriptorBindingInlineUniformBlockUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO { uint out_buffer; };
        layout(set = 0, binding = 1) uniform InlineUBO {
            uint updated_value;
            uint non_updated_value; // will be undefined value
        };
        void main() {
            out_buffer = non_updated_value;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 1;

    VkDescriptorBindingFlags ds_binding_flags[2] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = ds_binding_flags;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &flags_create_info,
                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, nullptr, &pool_inline_info);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    const uint32_t update_value = 0;
    VkWriteDescriptorSetInlineUniformBlock write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &update_value;
    VkWriteDescriptorSet descriptor_writes = vku::InitStructHelper(&write_inline_uniform);
    descriptor_writes.dstSet = descriptor_set.set_;
    descriptor_writes.dstBinding = 1;
    descriptor_writes.dstArrayElement = 0;
    descriptor_writes.descriptorCount = 4;
    descriptor_writes.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_writes, 0, nullptr);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, SetSSBOBindDescriptor) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdBindDescriptorSets()");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkPhysicalDeviceProperties properties = {};
    vk::GetPhysicalDeviceProperties(Gpu(), &properties);
    if (properties.limits.maxBoundDescriptorSets < 8) {
        GTEST_SKIP() << "maxBoundDescriptorSets is too low";
    }

    char const *csSource = R"glsl(
        #version 450
        layout(constant_id=0) const uint _const_2_0 = 1;
        layout(constant_id=1) const uint _const_3_0 = 1;
        layout(std430, binding=0) readonly restrict buffer _SrcBuf_0_0 {
            layout(offset=0) uint src[256];
        };
        layout(std430, binding=1) writeonly restrict buffer _DstBuf_1_0 {
            layout(offset=0) uint dst[256];
        };
        layout (local_size_x = 255, local_size_y = 1) in;

        void main() {
            uint word = src[_const_2_0 + gl_GlobalInvocationID.x];
            word = (word & 0xFF00FF00u) >> 8 |
                (word & 0x00FF00FFu) << 8;
            dst[_const_3_0 + gl_GlobalInvocationID.x] = word;
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();

    VkBufferUsageFlags buffer_usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    uint32_t buffer_size = 262144;
    vkt::Buffer buffer_0(*m_device, buffer_size, buffer_usage);
    vkt::Buffer buffer_1(*m_device, buffer_size, buffer_usage);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_0.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, buffer_1.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, SetSSBOPushDescriptor) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdPushDescriptorSetKHR instead of vkUpdateDescriptorSets");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkPhysicalDeviceProperties properties = {};
    vk::GetPhysicalDeviceProperties(Gpu(), &properties);
    if (properties.limits.maxBoundDescriptorSets < 8) {
        GTEST_SKIP() << "maxBoundDescriptorSets is too low";
    }

    char const *csSource = R"glsl(
        #version 450
        layout(constant_id=0) const uint _const_2_0 = 1;
        layout(constant_id=1) const uint _const_3_0 = 1;
        layout(std430, binding=0) readonly restrict buffer _SrcBuf_0_0 {
            layout(offset=0) uint src[256];
        };
        layout(std430, binding=1) writeonly restrict buffer _DstBuf_1_0 {
            layout(offset=0) uint dst[256];
        };
        layout (local_size_x = 255, local_size_y = 1) in;

        void main() {
            uint word = src[_const_2_0 + gl_GlobalInvocationID.x];
            word = (word & 0xFF00FF00u) >> 8 |
                (word & 0x00FF00FFu) << 8;
            dst[_const_3_0 + gl_GlobalInvocationID.x] = word;
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});

    VkComputePipelineCreateInfo pipeline_info = vku::InitStructHelper();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = cs.GetStageCreateInfo();
    vkt::Pipeline pipeline(*m_device, pipeline_info);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_ci.size = 262144;
    vkt::Buffer buffer_0(*m_device, buffer_ci);
    vkt::Buffer buffer_1(*m_device, buffer_ci);

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = 0;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VkDescriptorBufferInfo buffer_info_0 = {buffer_0.handle(), 0, 1024};
    descriptor_writes[0].pBufferInfo = &buffer_info_0;

    descriptor_writes[1] = descriptor_writes[0];
    descriptor_writes[1].dstBinding = 1;
    VkDescriptorBufferInfo buffer_info_1 = {buffer_1.handle(), 0, 1024};
    descriptor_writes[1].pBufferInfo = &buffer_info_1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 2,
                                descriptor_writes);

    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

// Regression test for semaphore timeout with GPU-AV enabled:
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveGpuAV, GetCounterFromSignaledSemaphoreAfterSubmit) {
    TEST_DESCRIPTION("Get counter value from the semaphore signaled by queue submit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore;
    signal_info.value = 1;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE));

    std::uint64_t counter = 0;
    ASSERT_EQ(VK_SUCCESS, vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
    m_device->Wait();  // so vkDestroySemaphore doesn't call while semaphore is active
}

TEST_F(PositiveGpuAV, MutableBuffer) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdBindDescriptorSets()");
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkPhysicalDeviceProperties properties = {};
    vk::GetPhysicalDeviceProperties(Gpu(), &properties);
    if (properties.limits.maxBoundDescriptorSets < 8) {
        GTEST_SKIP() << "maxBoundDescriptorSets is too low";
    }

    char const *csSource = R"glsl(
        #version 450
        layout(constant_id=0) const uint _const_2_0 = 1;
        layout(constant_id=1) const uint _const_3_0 = 1;
        layout(std430, binding=0) readonly restrict buffer _SrcBuf_0_0 {
            layout(offset=0) uint src[256];
        };
        layout(std430, binding=1) writeonly restrict buffer _DstBuf_1_0 {
            layout(offset=0) uint dst[2][256];
        };
        layout (local_size_x = 255, local_size_y = 1) in;

        void main() {
            uint word = src[_const_2_0 + gl_GlobalInvocationID.x];
            word = (word & 0xFF00FF00u) >> 8 |
                (word & 0x00FF00FFu) << 8;
            dst[0][_const_3_0 + gl_GlobalInvocationID.x] = word;
            dst[1][_const_3_0 + gl_GlobalInvocationID.x] = word;
        }
    )glsl";

    VkDescriptorType desc_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListEXT lists[3] = {};
    lists[1].descriptorTypeCount = 2;
    lists[1].pDescriptorTypes = desc_types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 3;
    mdtci.pMutableDescriptorTypeLists = lists;

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                          {1, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 2, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         0, &mdtci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});

    VkComputePipelineCreateInfo pipeline_info = vku::InitStructHelper();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = cs.GetStageCreateInfo();
    vkt::Pipeline pipeline(*m_device, pipeline_info);

    VkBufferUsageFlags buffer_usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    uint32_t buffer_size = 262144;
    vkt::Buffer buffer_0(*m_device, buffer_size, buffer_usage);
    vkt::Buffer buffer_1(*m_device, buffer_size, buffer_usage);

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set_0.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VkDescriptorBufferInfo buffer_info_0 = {buffer_0.handle(), 0, 1024};
    descriptor_write.pBufferInfo = &buffer_info_0;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    VkCopyDescriptorSet descriptor_copy = vku::InitStructHelper();
    // copy the storage descriptor to the first mutable descriptor
    descriptor_copy.srcSet = descriptor_set_0.set_;
    descriptor_copy.srcBinding = 0;
    descriptor_copy.dstSet = descriptor_set_0.set_;
    descriptor_copy.dstBinding = 1;
    descriptor_copy.dstArrayElement = 1;
    descriptor_copy.descriptorCount = 1;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &descriptor_copy);

    // copy the first mutable descriptor to the second storage desc
    descriptor_copy.srcBinding = 1;
    descriptor_copy.srcArrayElement = 1;
    descriptor_copy.dstBinding = 1;
    descriptor_copy.dstArrayElement = 0;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &descriptor_copy);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);

    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, MaxDescriptorsClamp) {
    TEST_DESCRIPTION("Make sure maxUpdateAfterBindDescriptorsInAllPools is clamped");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    auto desc_indexing_props = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingProperties>();
    auto props2 = vku::InitStruct<VkPhysicalDeviceProperties2>(&desc_indexing_props);

    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    ASSERT_GE(gpuav::glsl::kDebugInputBindlessMaxDescriptors, desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools);
}

TEST_F(PositiveGpuAV, MaxDescriptorsClamp13) {
    TEST_DESCRIPTION("Make sure maxUpdateAfterBindDescriptorsInAllPools is clamped");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    auto vk12_props = vku::InitStruct<VkPhysicalDeviceVulkan12Properties>();
    auto props2 = vku::InitStruct<VkPhysicalDeviceProperties2>(&vk12_props);

    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    ASSERT_GE(gpuav::glsl::kDebugInputBindlessMaxDescriptors, vk12_props.maxUpdateAfterBindDescriptorsInAllPools);
}

TEST_F(PositiveGpuAV, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("Use a bad vertex shader, but don't select it for validation and make sure we don't get a buffer oob warning");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                       &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &setting};
    RETURN_IF_SKIP(InitGpuAvFramework(&layer_settings_create_info));

    // Robust buffer access will be on by default
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    InitState(nullptr, nullptr, pool_flags);
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
    )glsl";
    // Don't instrument buggy vertex shader
    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    // Instrument non-buggy fragment shader
    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    VkShaderObj fs(this, vertshader, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", &features);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_.clear();
    pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    // Should not get a warning since buggy vertex shader wasn't instrumented
    m_errorMonitor->ExpectSuccess(kWarningBit | kErrorBit);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, DrawingWithUnboundUnusedSet) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with first descriptor set begin unused and "
        "unbound.");
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests requires Vulkan 1.1 exactly";
    }

    char const *fs_source = R"glsl(
        #version 450
        layout (set = 1, binding = 0) uniform sampler2D samplerColor;
        layout(location = 0) out vec4 color;
        void main() {
           color = texture(samplerColor, gl_FragCoord.xy);
           color += texture(samplerColor, gl_FragCoord.wz);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 1, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveGpuAV, FirstInstance) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectFirstInstance);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 4 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            kHostVisibleMemProps);
    auto draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.Memory().Map());
    for (uint32_t i = 0; i < 4; i++) {
        draw_ptr->vertexCount = 3;
        draw_ptr->instanceCount = 1;
        draw_ptr->firstVertex = 0;
        draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        draw_ptr++;
    }
    draw_buffer.Memory().Unmap();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirect(m_command_buffer.handle(), draw_buffer.handle(), 0, 4, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Now with an offset and indexed draw
    vkt::Buffer indexed_draw_buffer(*m_device, 4 * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    kHostVisibleMemProps);
    auto indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.Memory().Map();
    for (uint32_t i = 0; i < 4; i++) {
        indexed_draw_ptr->indexCount = 3;
        indexed_draw_ptr->instanceCount = 1;
        indexed_draw_ptr->firstIndex = 0;
        indexed_draw_ptr->vertexOffset = 0;
        indexed_draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        indexed_draw_ptr++;
    }
    indexed_draw_buffer.Memory().Unmap();

    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), indexed_draw_buffer.handle(), sizeof(VkDrawIndexedIndirectCommand), 3,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, CopyBufferToImageD32) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with all depth values in the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT.");
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, sizeof(float) * 64 * 64,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, kHostVisibleMemProps);

    float *ptr = static_cast<float *>(copy_src_buffer.Memory().Map());
    for (size_t i = 0; i < 64 * 64; ++i) {
        if (i % 2) {
            ptr[i] = 1.0f;
        } else {
            ptr[i] = 0.0f;
        }
    }
    copy_src_buffer.Memory().Unmap();

    vkt::Image copy_dst_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkBufferImageCopy buffer_image_copy_1;
    buffer_image_copy_1.bufferOffset = 0;
    buffer_image_copy_1.bufferRowLength = 0;
    buffer_image_copy_1.bufferImageHeight = 0;
    buffer_image_copy_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    buffer_image_copy_1.imageOffset = {0, 0, 0};
    buffer_image_copy_1.imageExtent = {64, 64, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy_1);

    VkBufferImageCopy buffer_image_copy_2 = buffer_image_copy_1;
    buffer_image_copy_2.imageOffset = {32, 32, 0};
    buffer_image_copy_2.imageExtent = {32, 32, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy_2);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAV, CopyBufferToImageD32U8) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with all depth values in the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT_S8_UINT.");
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, 5 * 64 * 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                kHostVisibleMemProps);

    auto ptr = static_cast<uint8_t *>(copy_src_buffer.Memory().Map());
    std::memset(ptr, 0, static_cast<size_t>(copy_src_buffer.CreateInfo().size));
    for (size_t i = 0; i < 64 * 64; ++i) {
        auto ptr_float = reinterpret_cast<float *>(ptr + 5 * i);
        if (i % 2) {
            *ptr_float = 1.0f;
        } else {
            *ptr_float = 0.0f;
        }
    }

    copy_src_buffer.Memory().Unmap();

    vkt::Image copy_dst_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    buffer_image_copy.imageOffset = {33, 33, 0};
    buffer_image_copy.imageExtent = {31, 31, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAV, CopyBufferToImageTwoSubmit) {
    TEST_DESCRIPTION("Make sure resources are managed correctly afer a CopyBufferToImage call.");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::CommandBuffer cb_0(*m_device, m_command_pool);
    vkt::CommandBuffer cb_1(*m_device, m_command_pool);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::Buffer buffer(*m_device, 4096, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32, 32, 1};
    region.bufferOffset = 0;

    cb_0.Begin();
    vk::CmdCopyBufferToImage(cb_0.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    cb_0.End();
    m_default_queue->Submit(cb_0);
    m_default_queue->Wait();

    cb_1.Begin();
    cb_1.End();
    m_default_queue->Submit(cb_1);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, SwapchainImage) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8091");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    const vkt::Fence fence(*m_device);
    uint32_t image_index = 0;
    {
        VkResult result{};
        image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout, &result);
        ASSERT_TRUE(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        fence.Wait(vvl::kU32Max);
    }

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = swapchain_images[image_index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = m_surface_formats[0].format;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view(*m_device, ivci);
}

class PositiveGpuAVParameterized : public GpuAVTest,
                                   public ::testing::WithParamInterface<std::tuple<std::vector<const char *>, uint32_t>> {};

TEST_P(PositiveGpuAVParameterized, SettingsCombinations) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectFirstInstance);

    std::vector<const char *> setting_names = std::get<0>(GetParam());
    const uint32_t setting_values = std::get<1>(GetParam());

    std::vector<VkLayerSettingEXT> layer_settings(setting_names.size());
    std::vector<VkBool32> layer_settings_values(setting_names.size());
    for (const auto [setting_name_i, setting_name] : vvl::enumerate(setting_names)) {
        VkLayerSettingEXT &layer_setting = layer_settings[setting_name_i];
        VkBool32 &layer_setting_value = layer_settings_values[setting_name_i];

        layer_setting_value = (setting_values & (1u << setting_name_i)) ? VK_TRUE : VK_FALSE;
        layer_setting = {OBJECT_LAYER_NAME, setting_name, VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &layer_setting_value};
    }

    VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
    layer_settings_create_info.settingCount = static_cast<uint32_t>(layer_settings.size());
    layer_settings_create_info.pSettings = layer_settings.data();
    RETURN_IF_SKIP(InitGpuAvFramework(&layer_settings_create_info));

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 4 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            kHostVisibleMemProps);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.Memory().Map());
    for (uint32_t i = 0; i < 4; i++) {
        draw_ptr->vertexCount = 3;
        draw_ptr->instanceCount = 1;
        draw_ptr->firstVertex = 0;
        draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        draw_ptr++;
    }
    draw_buffer.Memory().Unmap();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.lineWidth = 1.0f;
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirect(m_command_buffer.handle(), draw_buffer.handle(), 0, 4, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Now with an offset and indexed draw
    vkt::Buffer indexed_draw_buffer(*m_device, 4 * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    kHostVisibleMemProps);
    VkDrawIndexedIndirectCommand *indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.Memory().Map();
    for (uint32_t i = 0; i < 4; i++) {
        indexed_draw_ptr->indexCount = 3;
        indexed_draw_ptr->instanceCount = 1;
        indexed_draw_ptr->firstIndex = 0;
        indexed_draw_ptr->vertexOffset = 0;
        indexed_draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        indexed_draw_ptr++;
    }
    indexed_draw_buffer.Memory().Unmap();

    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), indexed_draw_buffer.handle(), sizeof(VkDrawIndexedIndirectCommand), 3,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

static std::string GetGpuAvSettingsCombinationTestName(const testing::TestParamInfo<PositiveGpuAVParameterized::ParamType> &info) {
    std::vector<const char *> setting_names = std::get<0>(info.param);
    const uint32_t setting_values = std::get<1>(info.param);
    std::stringstream test_name;
    for (auto [setting_name_i, setting_name] : vvl::enumerate(setting_names)) {
        const char *enabled_str = (setting_values & (1u << setting_name_i)) ? "_1" : "_0";
        if (setting_name_i != 0) {
            test_name << "_";
        }
        test_name << *setting_name << enabled_str;
    }

    return test_name.str();
}

// /!\ Note when copy pasting this:
// Be mindful that the constant number specified as the end range parameter in ::testing::Range
// is based on the number of settings in the settings list. If you have N settings, you want your range end to be uint32_t(1) << N
INSTANTIATE_TEST_SUITE_P(GpuAvShaderInstrumentationMainSettings, PositiveGpuAVParameterized,

                         ::testing::Combine(::testing::Values(std::vector<const char *>(
                                                {"gpuav_descriptor_checks", "gpuav_buffer_address_oob", "gpuav_vma_linear_output",
                                                 "gpuav_validate_ray_query", "gpuav_select_instrumented_shaders"})),
                                            ::testing::Range(uint32_t(0), uint32_t(1) << 5)),

                         [](const testing::TestParamInfo<PositiveGpuAVParameterized::ParamType> &info) {
                             return GetGpuAvSettingsCombinationTestName(info);
                         });

INSTANTIATE_TEST_SUITE_P(GpuAvMainSettings, PositiveGpuAVParameterized,

                         ::testing::Combine(::testing::Values(std::vector<const char *>({"gpuav_shader_instrumentation",
                                                                                         "gpuav_buffers_validation",
                                                                                         "gpuav_vma_linear_output"})),
                                            ::testing::Range(uint32_t(0), uint32_t(1) << 3)),

                         [](const testing::TestParamInfo<PositiveGpuAVParameterized::ParamType> &info) {
                             return GetGpuAvSettingsCombinationTestName(info);
                         });

INSTANTIATE_TEST_SUITE_P(GpuAvBufferContentValidationSettings, PositiveGpuAVParameterized,
                         ::testing::Combine(::testing::Values(std::vector<const char *>(
                                                {"gpuav_indirect_draws_buffers", "gpuav_indirect_dispatches_buffers",
                                                 "gpuav_indirect_trace_rays_buffers", "gpuav_buffer_copies"})),
                                            ::testing::Range(uint32_t(0), uint32_t(1) << 4)),

                         [](const testing::TestParamInfo<PositiveGpuAVParameterized::ParamType> &info) {
                             return GetGpuAvSettingsCombinationTestName(info);
                         });

TEST_F(PositiveGpuAV, RestoreUserPushConstants) {
    TEST_DESCRIPTION("Test that user supplied push constants are correctly restored. One graphics pipeline, indirect draw.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;

    indirect_draw_parameters_buffer.Memory().Unmap();

    constexpr int32_t int_count = 16;
    vkt::Buffer storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    // Use different push constant ranges for vertex and fragment shader.
    // The underlying storage buffer is the same.
    // Vertex shader will fill the first 8 integers, fragment shader the other 8
    struct PushConstants {
        // Vertex shader
        VkDeviceAddress storage_buffer_ptr_1;
        int32_t integers_1[int_count / 2];
        // Fragment shader
        VkDeviceAddress storage_buffer__ptr_2;
        int32_t integers_2[int_count / 2];
    } push_constants;

    push_constants.storage_buffer_ptr_1 = storage_buffer.Address();
    push_constants.storage_buffer__ptr_2 = storage_buffer.Address() + sizeof(int32_t) * (int_count / 2);
    for (int32_t i = 0; i < int_count / 2; ++i) {
        push_constants.integers_1[i] = i;
        push_constants.integers_2[i] = (int_count / 2) + i;
    }

    constexpr uint32_t shader_pcr_byte_size = uint32_t(sizeof(VkDeviceAddress)) + uint32_t(sizeof(int32_t)) * (int_count / 2);
    std::array<VkPushConstantRange, 2> push_constant_ranges = {{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size},
        {VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size, shader_pcr_byte_size},
    }};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.pushConstantRangeCount = size32(push_constant_ranges);
    plci.pPushConstantRanges = push_constant_ranges.data();
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    char const *vs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            vec2 vertices[3];

            void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);

              for (int i = 0; i < 8; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                layout(offset = 40) MyPtrType ptr;
                int in_array[8];
            } pc;

            layout(location = 0) out vec4 uFragColor;

            void main() {
                for (int i = 0; i < 8; ++i) {
                      pc.ptr.out_array[i] = pc.in_array[i];
                }
            }
        )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size,
                         &push_constants);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size,
                         shader_pcr_byte_size, &push_constants.storage_buffer__ptr_2);
    // Make sure pushing the same push constants twice does not break internal management
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size,
                         &push_constants);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size,
                         shader_pcr_byte_size, &push_constants.storage_buffer__ptr_2);
    // Vertex shader will write 8 values to storage buffer, fragment shader another 8
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    auto storage_buffer_ptr = static_cast<int32_t *>(storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(storage_buffer_ptr[i], i);
    }
    storage_buffer.Memory().Unmap();
}

TEST_F(PositiveGpuAV, RestoreUserPushConstants2) {
    TEST_DESCRIPTION(
        "Test that user supplied push constants are correctly restored. One graphics pipeline, one compute pipeline, indirect draw "
        "and dispatch.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    constexpr int32_t int_count = 8;

    struct PushConstants {
        VkDeviceAddress storage_buffer;
        int32_t integers[int_count];
    };

    // Graphics pipeline
    // ---

    char const *vs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            vec2 vertices[3];

            void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);

              for (int i = 0; i < 4; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            layout(location = 0) out vec4 uFragColor;

            void main() {
                for (int i = 4; i < 8; ++i) {
                      pc.ptr.out_array[i] = pc.in_array[i];
                }
            }
        )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange graphics_push_constant_ranges = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                         sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo graphics_plci = vku::InitStructHelper();
    graphics_plci.pushConstantRangeCount = 1;
    graphics_plci.pPushConstantRanges = &graphics_push_constant_ranges;
    vkt::PipelineLayout graphics_pipeline_layout(*m_device, graphics_plci);

    vkt::Buffer graphics_storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                        vkt::device_address);

    PushConstants graphics_push_constants;
    graphics_push_constants.storage_buffer = graphics_storage_buffer.Address();
    for (int32_t i = 0; i < int_count; ++i) {
        graphics_push_constants.integers[i] = i;
    }

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;
    indirect_draw_parameters_buffer.Memory().Unmap();

    CreatePipelineHelper graphics_pipe(*this);
    graphics_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    graphics_pipe.gp_ci_.layout = graphics_pipeline_layout.handle();
    graphics_pipe.CreateGraphicsPipeline();

    // Compute pipeline
    // ---

    char const *compute_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

            void main() {
              for (int i = 0; i < 8; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";
    VkShaderObj compute(this, compute_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPushConstantRange compute_push_constant_ranges = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo compute_plci = vku::InitStructHelper();
    compute_plci.pushConstantRangeCount = 1;
    compute_plci.pPushConstantRanges = &compute_push_constant_ranges;
    vkt::PipelineLayout compute_pipeline_layout(*m_device, compute_plci);

    vkt::Buffer compute_storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                       vkt::device_address);

    PushConstants compute_push_constants;
    compute_push_constants.storage_buffer = compute_storage_buffer.Address();
    for (int32_t i = 0; i < int_count; ++i) {
        compute_push_constants.integers[i] = int_count + i;
    }

    CreateComputePipelineHelper compute_pipe(*this);
    compute_pipe.cs_ = std::make_unique<VkShaderObj>(this, compute_source, VK_SHADER_STAGE_COMPUTE_BIT);
    compute_pipe.cp_ci_.layout = compute_pipeline_layout.handle();
    compute_pipe.CreateComputePipeline();

    vkt::Buffer indirect_dispatch_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                    kHostVisibleMemProps);
    auto &indirect_dispatch_parameters =
        *static_cast<VkDispatchIndirectCommand *>(indirect_dispatch_parameters_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1;
    indirect_dispatch_parameters.y = 1;
    indirect_dispatch_parameters.z = 1;
    indirect_dispatch_parameters_buffer.Memory().Unmap();

    // Submit commands
    // ---

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipe.Handle());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe.Handle());

    vk::CmdPushConstants(m_command_buffer.handle(), graphics_pipeline_layout.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(graphics_push_constants),
                         &graphics_push_constants);

    // Vertex shader will write 4 values to graphics storage buffer, fragment shader another 4
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();

    vk::CmdPushConstants(m_command_buffer.handle(), compute_pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
                         sizeof(compute_push_constants), &compute_push_constants);
    // Compute shaders will write 8 values to compute storage buffer
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_dispatch_parameters_buffer.handle(), 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    auto compute_storage_buffer_ptr = static_cast<int32_t *>(compute_storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(compute_storage_buffer_ptr[i], int_count + i);
    }
    compute_storage_buffer.Memory().Unmap();

    auto graphics_storage_buffer_ptr = static_cast<int32_t *>(graphics_storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(graphics_storage_buffer_ptr[i], i);
    }
    graphics_storage_buffer.Memory().Unmap();
}

TEST_F(PositiveGpuAV, PipelineLayoutMixing) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;

    indirect_draw_parameters_buffer.Memory().Unmap();

    VkPushConstantRange push_constant_ranges = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int)};
    VkPipelineLayoutCreateInfo pipe_layout_ci_1 = vku::InitStructHelper();
    pipe_layout_ci_1.pushConstantRangeCount = 1;
    pipe_layout_ci_1.pPushConstantRanges = &push_constant_ranges;
    vkt::PipelineLayout pipe_layout_1(*m_device, pipe_layout_ci_1);

    CreatePipelineHelper pipe_1(*this);
    pipe_1.gp_ci_.layout = pipe_layout_1.handle();
    pipe_1.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_2(*this);
    pipe_2.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    pipe_2.CreateGraphicsPipeline();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipe_2.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    pipe_2.descriptor_set_->UpdateDescriptorSets();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // CmdPushConstants with layout 1 (is associated to pipeline 1, only has a push constant range)
    int dummy = 0;
    vk::CmdPushConstants(m_command_buffer.handle(), pipe_layout_1.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &dummy);
    // CmdBindDescriptorSets with layout 2 (is associated to pipeline 2, only has a descriptor set, so is incompatible with layout
    // 1)
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.pipeline_layout_.handle(), 0, 1,
                              &pipe_2.descriptor_set_->set_, 0, nullptr);

    // Bind pipeline 1, draw
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.Handle());
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    // Bind pipeline 2, draw
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, SharedPipelineLayoutSubset) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout dsl1(*m_device, binding);
    vkt::DescriptorSetLayout dsl2(*m_device, binding);
    VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pSetLayouts = set_layouts;

    pipeline_layout_ci.setLayoutCount = 1;
    const vkt::PipelineLayout pipeline_layout_1(*m_device, pipeline_layout_ci);
    pipeline_layout_ci.setLayoutCount = 2;
    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; int b;};
        void main() {
            a = b;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout_1.handle();
    pipe.CreateComputePipeline();

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = set_layouts;

    VkDescriptorSet descriptor_sets[2];
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_sets[0];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_sets[1];
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2.handle(), 0, 2,
                              descriptor_sets, 0, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, SharedPipelineLayoutSubsetWithUnboundDescriptorSet) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout dsl1(*m_device, binding);
    VkDescriptorSetLayout set_layouts[3] = {dsl1.handle(), dsl1.handle(), dsl1.handle()};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pSetLayouts = set_layouts;

    pipeline_layout_ci.setLayoutCount = 1;
    const vkt::PipelineLayout pipeline_layout_1(*m_device, pipeline_layout_ci);
    // pipeline_layout_2 has 3 descriptor sets, but the one at index 1 will not be bound
    pipeline_layout_ci.setLayoutCount = 3;
    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 {
            int a;
            int b;
        };
        void main() {
            a = b;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout_1.handle();
    pipe.CreateComputePipeline();

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = set_layouts;

    VkDescriptorSet descriptor_sets[2];
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_sets[0];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_sets[1];
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    // Only bind descriptor sets at indices 0 and 2
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2.handle(), 0, 1,
                              &descriptor_sets[0], 0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2.handle(), 2, 1,
                              &descriptor_sets[1], 0, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAV, DestroyedPipelineLayout) {
    TEST_DESCRIPTION("Check if can catch pipeline layout not being bound");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Destroy pipeline layout after creating pipeline
    CreatePipelineHelper pipe(*this);
    {
        const vkt::PipelineLayout doomed_pipeline_layout(*m_device);
        pipe.gp_ci_.layout = doomed_pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();
    }

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveGpuAV, DestroyedPipelineLayout2) {
    TEST_DESCRIPTION("Have a descriptor set that needs to be bound as well so GPU-AV can use that");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint x; };
        void main() {
            x = 0;
        }
    )glsl";
    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Destroy pipeline layout after creating pipeline
    CreatePipelineHelper pipe(*this);
    {
        const vkt::PipelineLayout doomed_pipeline_layout(*m_device, {&descriptor_set.layout_});
        pipe.shader_stages_[0] = vs.GetStageCreateInfo();
        pipe.gp_ci_.layout = doomed_pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();
    }

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// TODO - Currently can't use IES with GPU-AV due to us creating invalid Pipeline Layouts
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/8806
TEST_F(PositiveGpuAV, DISABLED_DeviceGeneratedCommandsIES) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is not supported.";
    }

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_COMPUTE_BIT};
    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;
    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    tokens[1].offset = sizeof(uint32_t);

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    char const *shader_source_1 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo {
            uint x[];
        };
        void main() {
            x[48] = 0; // invalid!
        }
    )glsl";
    char const *shader_source_2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo {
            uint x[];
        };
        void main() {
            x[24] = 0; // invalid!
        }
    )glsl";
    char const *shader_source_3 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo {
            uint x[];
        };
        void main() {
            x[0] = 0; // valid
        }
    )glsl";

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreateComputePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    init_pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source_1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    init_pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_1(*this, &pipe_flags2);
    pipe_1.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe_1.cs_ = std::make_unique<VkShaderObj>(this, shader_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe_1.CreateComputePipeline();

    CreateComputePipelineHelper pipe_2(*this, &pipe_flags2);
    pipe_2.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, shader_source_3, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe_2.CreateComputePipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 3);
    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets[2];
    write_exe_sets[0] = vku::InitStructHelper();
    write_exe_sets[0].index = 1;
    write_exe_sets[0].pipeline = pipe_1.Handle();
    write_exe_sets[1] = vku::InitStructHelper();
    write_exe_sets[1].index = 2;
    write_exe_sets[1].pipeline = pipe_2.Handle();
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 2, write_exe_sets);

    vkt::Buffer ssbo_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    init_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, ssbo_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    init_pipe.descriptor_set_->UpdateDescriptorSets();

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkDeviceSize pre_process_size = 0;
    {
        VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper();
        dgc_mem_reqs.indirectCommandsLayout = command_layout.handle();
        dgc_mem_reqs.indirectExecutionSet = exe_set.handle();
        dgc_mem_reqs.maxSequenceCount = 1;
        VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
        vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &dgc_mem_reqs, &mem_reqs2);
        pre_process_size = mem_reqs2.memoryRequirements.size;
    }

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = pre_process_size;
    vkt::Buffer pre_process_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

    uint32_t *block_buffer_ptr = (uint32_t *)block_buffer.Memory().Map();
    block_buffer_ptr[0] = 2;  // pick pipeline 2
    VkDispatchIndirectCommand *indirect_command_ptr = (VkDispatchIndirectCommand *)(block_buffer_ptr + 1);
    indirect_command_ptr->x = 1;
    indirect_command_ptr->y = 1;
    indirect_command_ptr->z = 1;
    block_buffer.Memory().Unmap();

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    generated_commands_info.indirectExecutionSet = exe_set.handle();
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = sizeof(uint32_t) + sizeof(VkDispatchIndirectCommand);
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.preprocessAddress = pre_process_buffer.Address();
    generated_commands_info.preprocessSize = pre_process_size;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, init_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, init_pipe.pipeline_layout_.handle(), 0, 1,
                              &init_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
