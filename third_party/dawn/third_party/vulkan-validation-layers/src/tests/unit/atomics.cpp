/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"

class NegativeAtomic : public VkLayerTest {};

TEST_F(NegativeAtomic, VertexStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if vertexPipelineStoresAndAtomics disable.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test StoreOp
    {
        char const *vsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageStore(si0, ivec2(0), vec4(0));
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, "VUID-RuntimeSpirv-NonWritable-06341");
    }

    // Test AtomicOp
    {
        char const *vsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform image2D si0;
            void main() {
                  imageAtomicExchange(si0, ivec2(0), 1);
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
                info.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
            };

            CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, "VUID-RuntimeSpirv-NonWritable-06341");
        }
    }
}

TEST_F(NegativeAtomic, FragmentStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if fragmentStoresAndAtomics disable.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test StoreOp
    {
        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageStore(si0, ivec2(0), vec4(0));
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            info.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, "VUID-RuntimeSpirv-NonWritable-06340");
    }

    // Test AtomicOp
    {
        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform image2D si0;
            void main() {
                  imageAtomicExchange(si0, ivec2(0), 1);
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
        if (VK_SUCCESS == fs.InitFromGLSLTry()) {
            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
                info.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
            };

            CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, "VUID-RuntimeSpirv-NonWritable-06340");
        }
    }
}

TEST_F(NegativeAtomic, FragmentStoresAndAtomicsFeatureBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo { int y; };
        void main() {
                atomicMin(y, 1);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto info_override = [&](CreatePipelineHelper &info) {
        info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        info.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    };

    CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, "VUID-RuntimeSpirv-NonWritable-06340");
}

TEST_F(NegativeAtomic, VertexStoresAndAtomicsFeatureDisableShaderObject) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if vertexPipelineStoresAndAtomics disable.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);

    RETURN_IF_SKIP(Init());

    char const *vs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D si0;
        void main() {
            imageStore(si0, ivec2(0), vec4(0));
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                 });

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-NonWritable-06341");
    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source),
                                  &descriptor_set.layout_.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAtomic, Int64) {
    TEST_DESCRIPTION("Test VK_KHR_shader_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(Init());

    // For sanity check without GL_EXT_shader_atomic_int64
    std::string cs_positive = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
           y = x + 1;
        }
    )glsl";

    std::string cs_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_atomic_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
    )glsl";

    // clang-format off
    // StorageBuffer storage class
    std::string cs_storage_buffer = cs_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    // StorageBuffer storage class using AtomicStore
    // atomicStore is slightly different than other atomics, so good edge case
    std::string cs_store = cs_base + R"glsl(
           atomicStore(y, 1ul, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    // Workgroup storage class
    std::string cs_workgroup = cs_base + R"glsl(
           atomicAdd(x, 1);
           barrier();
           y = x + 1;
        }
    )glsl";
    // clang-format on

    { VkShaderObj const cs(this, cs_positive.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1); }

    {
        // shaderBufferInt64Atomics
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06278");
        VkShaderObj const cs(this, cs_storage_buffer.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06278");
        VkShaderObj const cs(this, cs_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        // shaderSharedInt64Atomics
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06279");
        VkShaderObj const cs(this, cs_workgroup.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, ImageInt64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(Init());

    // clang-format off
    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        layout(set = 0, binding = 1, r64ui) uniform u64image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    // shaderImageInt64Atomics
    // Need 01091 VUID check for both Int64ImageEXT and Int64Atomics.. test could be rewritten to be more complex in order to set
    // capability requirements with other features, but this is simpler
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06288");
        VkShaderObj const cs(this, cs_image_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // glslang doesn't omit Int64Atomics for store currently
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06288");
        VkShaderObj const cs(this, cs_image_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06288");
        VkShaderObj const cs(this, cs_image_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06288");
        VkShaderObj const cs(this, cs_image_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, ImageInt64Drawtime64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 draw time with 64 bit image view.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderImageInt64Atomics);
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, r64ui) uniform u64image2D z;
        void main() {
            uint64_t y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-SampledType-04471");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeAtomic, ImageInt64Drawtime32) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 draw time with 32 bit image view.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderImageInt64Atomics);
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, r32ui) uniform uimage2D z;
        void main() {
            uint y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    // "64-bit integer atomic support is guaranteed for optimally tiled images with the VK_FORMAT_R64_UINT"
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R64_UINT, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-SampledType-04470");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeAtomic, ImageInt64DrawtimeSparse) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 at draw time with Sparse image.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    AddRequiredFeature(vkt::Feature::shaderImageInt64Atomics);
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        layout(set = 0, binding = 1, r64ui) uniform u64image2D z;
        void main() {
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R64_UINT, VK_IMAGE_USAGE_STORAGE_BIT);
    image_ci.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    vkt::ImageView image_view = image.CreateView();
    pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-sparseImageInt64Atomics-04474");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeAtomic, ImageInt64Mesh32) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 draw time with 32 bit image view in Mesh shaders.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::shaderImageInt64Atomics);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        #extension GL_KHR_memory_scope_semantics : enable

        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(set = 0, binding = 0, r32ui) uniform uimage2D z;

        void main() {
            SetMeshOutputsEXT(3,1);
            uint y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr};
    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();

    // "64-bit integer atomic support is guaranteed for optimally tiled images with the VK_FORMAT_R64_UINT"
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R64_UINT, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-SampledType-04470");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeAtomic, Float) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    // Create device without VK_EXT_shader_atomic_float extension or features enabled
    RETURN_IF_SKIP(Init());

    // clang-format off
    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_add = cs_32_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_store = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_32_add = cs_32_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_store = cs_32_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_add = cs_64_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_store = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_64_add = cs_64_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_store = cs_64_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    // shaderBufferFloat32Atomics
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_32_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_32_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_32_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderBufferFloat32AtomicAdd
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_32_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat32Atomics
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_32_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_32_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_32_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat32AtomicAdd
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_32_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderBufferFloat64Atomics (requires shaderFloat64)
    {{m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
    VkShaderObj const cs(this, cs_buffer_float_64_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    m_errorMonitor->VerifyFound();
        }
        {
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
            VkShaderObj const cs(this, cs_buffer_float_64_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }
        {
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
            VkShaderObj const cs(this, cs_buffer_float_64_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }

        // shaderBufferFloat64AtomicAdd
        {
            m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
            m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
            VkShaderObj const cs(this, cs_buffer_float_64_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }

        // shaderSharedFloat64Atomics
        {
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
            VkShaderObj const cs(this, cs_shared_float_64_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }
        {
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
            VkShaderObj const cs(this, cs_shared_float_64_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }
        {
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
            VkShaderObj const cs(this, cs_shared_float_64_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }

        // shaderSharedFloat64AtomicAdd
        {
            m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
            m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
            VkShaderObj const cs(this, cs_shared_float_64_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
            m_errorMonitor->VerifyFound();
        }
    }

    // shaderImageFloat32Atomics
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06286");
        VkShaderObj const cs(this, cs_image_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06286");
        VkShaderObj const cs(this, cs_image_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06286");
        VkShaderObj const cs(this, cs_image_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderImageFloat32AtomicAdd
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06286");
        VkShaderObj const cs(this, cs_image_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, Float2) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    // Create device without VK_EXT_shader_atomic_float2 extension or features enabled
    RETURN_IF_SKIP(Init());

    // clang-format off
    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_min = cs_32_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_max = cs_32_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_32_min = cs_32_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_max = cs_32_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_image_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_32_min = cs_image_32_base + R"glsl(
           y = imageAtomicMin(z, ivec2(1, 1), y);
        }
    )glsl";

    std::string cs_image_32_max = cs_image_32_base + R"glsl(
           y = imageAtomicMax(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    // shaderBufferFloat32AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_32_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_32_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat32AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_32_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_32_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat32AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06286");
        VkShaderObj const cs(this, cs_image_32_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06286");
        VkShaderObj const cs(this, cs_image_32_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, Float2With16bit) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    // Create device without VK_EXT_shader_atomic_float2 extension or features enabled
    RETURN_IF_SKIP(Init());

    // clang-format off
    std::string cs_16_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_16bit_storage: enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared float16_t x;
        layout(set = 0, binding = 0) buffer ssbo { float16_t y; };
        void main() {
    )glsl";

     std::string cs_buffer_float_16_add = cs_16_base + R"glsl(
           atomicAdd(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_store = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_buffer_float_16_min = cs_16_base + R"glsl(
           atomicMin(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_max = cs_16_base + R"glsl(
           atomicMax(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_add = cs_16_base + R"glsl(
           y = atomicAdd(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_store = cs_16_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_shared_float_16_min = cs_16_base + R"glsl(
           y = atomicMin(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_max = cs_16_base + R"glsl(
           y = atomicMax(x, float16_t(1.0));
        }
    )glsl";

    // clang-format on

    // shaderBufferFloat16Atomics
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_16_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_16_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_16_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderBufferFloat16AtomicAdd
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742", 2);
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_16_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderBufferFloat16AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_16_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_16_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat16Atomics
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_16_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_16_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_16_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat16AtomicAdd
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742", 2);
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_16_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat16AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_16_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_16_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, Float2With64bit) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    // Create device without VK_EXT_shader_atomic_float2 extension or features enabled
    RETURN_IF_SKIP(Init());

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_min = cs_64_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_max = cs_64_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_64_min = cs_64_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_max = cs_64_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    // shaderBufferFloat64AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_64_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06284");
        VkShaderObj const cs(this, cs_buffer_float_64_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // shaderSharedFloat64AtomicMinMax
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_64_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06285");
        VkShaderObj const cs(this, cs_shared_float_64_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, Float2WidthMismatch) {
    TEST_DESCRIPTION("VK_EXT_shader_atomic_float2 but enable wrong bitwidth.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::shaderBufferFloat16AtomicMinMax);
    AddRequiredFeature(vkt::Feature::shaderSharedFloat32AtomicMinMax);
    // shaderBufferFloat32AtomicMinMax not enabled
    RETURN_IF_SKIP(Init());

    // clang-format off
    std::string cs_buffer_float_16_min = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_16bit_storage: enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        shared float16_t x;
        layout(set = 0, binding = 0) buffer ssbo { float16_t y; };
        void main() {
           atomicMin(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_32_min = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
           atomicMin(y, 1);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    const auto set_info = [this, &current_shader](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    };

    // shaderBufferFloat16AtomicMinMax - valid - everything enabled
    current_shader = cs_buffer_float_16_min.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // shaderBufferFloat32AtomicMinMax - not enabled
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06338");
    VkShaderObj const cs(this, cs_buffer_float_32_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAtomic, FloatOpSource) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"(
               OpCapability Shader
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_shader_atomic_float : enable
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable

layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
void main() {
    y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
               OpSourceExtension "GL_EXT_shader_atomic_float"
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float32"
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpName %main "main"
               OpName %ssbo "ssbo"
               OpMemberName %ssbo 0 "y"
               OpName %_ ""
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.3"
               OpModuleProcessed "target-env vulkan1.1"
               OpModuleProcessed "entry-point main"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %ssbo = OpTypeStruct %float
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
      %int_1 = OpConstant %int 1
     %int_64 = OpConstant %int 64
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
    %uint_64 = OpConstant %uint 64
               OpLine %1 7 11
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 8 0
         %15 = OpAccessChain %_ptr_StorageBuffer_float %_ %int_0
         %22 = OpAtomicLoad %float %15 %int_1 %uint_64
         %23 = OpFAdd %float %float_1 %22
         %24 = OpAccessChain %_ptr_StorageBuffer_float %_ %int_0
               OpStore %24 %23
               OpLine %1 9 0
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-None-06284
    m_errorMonitor->SetDesiredError("atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);");
    VkShaderObj const cs(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAtomic, InvalidStorageOperation) {
    TEST_DESCRIPTION(
        "If storage view use atomic operation, the view's format MUST support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT or "
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT ");

    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(Init());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT to
                                                       // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, image_format, usage);

    if (ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        GTEST_SKIP() << "Cannot make VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported.";
    }

    VkFormat buffer_view_format =
        VK_FORMAT_R8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT to
                             // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    if (BufferFormatAndFeaturesSupported(Gpu(), buffer_view_format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
        GTEST_SKIP() << "Cannot make VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported.";
    }
    m_errorMonitor->SetUnexpectedError("VUID-VkBufferViewCreateInfo-format-08779");
    InitRenderTarget();

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer.handle();
    bvci.format = buffer_view_format;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);

    char const *fsSource = R"glsl(
        #version 450
        layout(set = 0, binding = 3, r32f) uniform image2D si0;
        layout(set = 0, binding = 2, r32f) uniform image2D si1[2];
        layout(set = 0, binding = 1, r32f) uniform imageBuffer stb2;
        layout(set = 0, binding = 0, r32f) uniform imageBuffer stb3[2];
        void main() {
              imageAtomicExchange(si0, ivec2(0), 1);
              imageAtomicExchange(si1[0], ivec2(0), 1);
              imageAtomicExchange(si1[1], ivec2(0), 1);
              imageAtomicExchange(stb2, 0, 1);
              imageAtomicExchange(stb3[0], 0, 1);
              imageAtomicExchange(stb3[1], 0, 1);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.CreateGraphicsPipeline();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 1);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(1, buffer_view.handle());
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-02691", 2);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07888", 2);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeAtomic, ImageFloat16Vector) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_NV_SHADER_ATOMIC_FLOAT16_VECTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    // shaderFloat16VectorAtomics not enabled
    RETURN_IF_SKIP(Init());

    // clang-format off
    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_NV_shader_atomic_fp16_vector : enable
        layout(set = 0, binding = 0) buffer ssbo { f16vec2 y; };
        layout(set = 0, binding = 1, rg16f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), f16vec2(1,2));
        }
    )glsl";

    std::string cs_image_min = cs_image_base + R"glsl(
           y = imageAtomicMin(z, ivec2(1, 1), f16vec2(1,2));
        }
    )glsl";

    std::string cs_image_max = cs_image_base + R"glsl(
           y = imageAtomicMax(z, ivec2(1, 1), f16vec2(1,2));
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           y = imageAtomicExchange(z, ivec2(1, 1), f16vec2(1,2));
        }
    )glsl";
    // clang-format on

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderFloat16VectorAtomics-09581");
        VkShaderObj const cs(this, cs_image_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderFloat16VectorAtomics-09581");
        VkShaderObj const cs(this, cs_image_min.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderFloat16VectorAtomics-09581");
        VkShaderObj const cs(this, cs_image_max.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderFloat16VectorAtomics-09581");
        VkShaderObj const cs(this, cs_image_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAtomic, VertexPipelineStoresAndAtomics) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8223");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // This is the following GLSL, but with a member decoration missing NonWritable
    //
    // layout(set=0, binding=0, std430) readonly buffer SSBO {
    //     float a;
    //     float b;
    // } data;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %o
               OpDecorate %o Location 0
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 NonWritable
               OpMemberDecorate %SSBO 1 Offset 4
               OpDecorate %SSBO BufferBlock
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %o = OpVariable %_ptr_Output_float Output
       %SSBO = OpTypeStruct %float %float
%_ptr_Uniform_SSBO = OpTypePointer Uniform %SSBO
       %data = OpVariable %_ptr_Uniform_SSBO Uniform
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpAccessChain %_ptr_Uniform_float %data %int_1
         %16 = OpLoad %float %15
               OpStore %o %16
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-NonWritable-06341");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}
