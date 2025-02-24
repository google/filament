/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2024 Google, Inc.
 * Copyright (c) 2023-2025 NVIDIA Corporation
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

class PositiveGpuAVCooperativeVector : public GpuAVTest {};

TEST_F(PositiveGpuAVCooperativeVector, CooperativeVector) {
    TEST_DESCRIPTION("GPU AV positive test for CooperativeVector");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_REPLICATED_COMPOSITES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddRequiredFeature(vkt::Feature::cooperativeVectorTraining);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderReplicatedComposites);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_NV_cooperative_vector : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable

        layout(set = 0, binding = 0) buffer foo {
            uint8_t x[];
        } b;

        void main() {
            coopvecNV<float16_t, 8> v;
            coopVecLoadNV(v, b.x, 0);
            coopVecMatMulNV(v, v, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat16NV, 8, 8, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
            coopVecMatMulAddNV(v, v, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat16NV, 8, 8, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
            coopVecStoreNV(v, b.x, 16);
            coopVecReduceSumAccumulateNV(v, b.x, 0);
            coopVecOuterProductAccumulateNV(v, v, b.x, 0, 0, gl_CooperativeVectorMatrixLayoutTrainingOptimalNV, gl_ComponentTypeFloat16NV);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
