/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveAtomic : public VkLayerTest {};

TEST_F(PositiveAtomic, ImageInt64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderImageInt64Atomics);
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

    const char *current_shader = nullptr;
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    //  shaderImageInt64Atomics
    current_shader = cs_image_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_image_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_image_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_image_add.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveAtomic, ImageInt64DrawtimeSparse) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 at draw time with Sparse image.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::shaderImageInt64Atomics);
    AddRequiredFeature(vkt::Feature::sparseImageInt64Atomics);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
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
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveAtomic, Float) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(atomic_float_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

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

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        helper.dsl_bindings_ = current_bindings;
    };

    if (atomic_float_features.shaderBufferFloat32Atomics == VK_TRUE) {
        current_shader = cs_buffer_float_32_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_buffer_float_32_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_buffer_float_32_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (atomic_float_features.shaderBufferFloat32AtomicAdd == VK_TRUE) {
        current_shader = cs_buffer_float_32_add.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (features2.features.shaderFloat64 == VK_TRUE) {
        if (atomic_float_features.shaderBufferFloat64Atomics == VK_TRUE) {
            current_shader = cs_buffer_float_64_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_buffer_float_64_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_buffer_float_64_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float_features.shaderBufferFloat64AtomicAdd == VK_TRUE) {
            current_shader = cs_buffer_float_64_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }

    if (atomic_float_features.shaderSharedFloat32Atomics == VK_TRUE) {
        current_shader = cs_shared_float_32_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_shared_float_32_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_shared_float_32_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (atomic_float_features.shaderSharedFloat32AtomicAdd == VK_TRUE) {
        current_shader = cs_shared_float_32_add.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (features2.features.shaderFloat64 == VK_TRUE) {
        if (atomic_float_features.shaderSharedFloat64Atomics == VK_TRUE) {
            current_shader = cs_shared_float_64_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_shared_float_64_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_shared_float_64_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float_features.shaderSharedFloat64AtomicAdd == VK_TRUE) {
            current_shader = cs_shared_float_64_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    if (atomic_float_features.shaderImageFloat32Atomics == VK_TRUE) {
        current_shader = cs_image_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_image_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_image_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (atomic_float_features.shaderImageFloat32AtomicAdd == VK_TRUE) {
        current_shader = cs_image_add.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveAtomic, Float2) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT atomic_float2_features = vku::InitStructHelper(&atomic_float_features);
    VkPhysicalDeviceShaderFloat16Int8Features float16int8_features = vku::InitStructHelper(&atomic_float2_features);
    VkPhysicalDevice16BitStorageFeatures storage_16_bit_features = vku::InitStructHelper(&float16int8_features);
    auto features2 = GetPhysicalDeviceFeatures2(storage_16_bit_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

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

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // This could get triggered in the event that the shader fails to compile
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = VkShaderObj::CreateFromGLSL(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        // Skip the test if shader failed to compile
        helper.override_skip_ = !static_cast<bool>(helper.cs_);
        helper.dsl_bindings_ = current_bindings;
    };

    if (float16int8_features.shaderFloat16 == VK_TRUE && storage_16_bit_features.storageBuffer16BitAccess == VK_TRUE) {
        if (atomic_float2_features.shaderBufferFloat16Atomics == VK_TRUE) {
            current_shader = cs_buffer_float_16_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_buffer_float_16_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_buffer_float_16_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float2_features.shaderBufferFloat16AtomicAdd == VK_TRUE) {
            current_shader = cs_buffer_float_16_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float2_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) {
            current_shader = cs_buffer_float_16_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_buffer_float_16_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float2_features.shaderSharedFloat16Atomics == VK_TRUE) {
            current_shader = cs_shared_float_16_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_shared_float_16_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_shared_float_16_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float2_features.shaderSharedFloat16AtomicAdd == VK_TRUE) {
            current_shader = cs_shared_float_16_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float2_features.shaderSharedFloat16AtomicMinMax == VK_TRUE) {
            current_shader = cs_shared_float_16_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_shared_float_16_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }

    if (atomic_float2_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) {
        current_shader = cs_buffer_float_32_min.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_buffer_float_32_max.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (atomic_float2_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) {
        current_shader = cs_shared_float_32_min.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_shared_float_32_max.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (features2.features.shaderFloat64 == VK_TRUE) {
        if (atomic_float2_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) {
            current_shader = cs_buffer_float_64_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_buffer_float_64_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (atomic_float2_features.shaderSharedFloat64AtomicMinMax == VK_TRUE) {
            current_shader = cs_shared_float_64_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

            current_shader = cs_shared_float_64_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    if (atomic_float2_features.shaderImageFloat32AtomicMinMax == VK_TRUE) {
        current_shader = cs_image_32_min.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

        current_shader = cs_image_32_max.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveAtomic, PhysicalPointer) {
    TEST_DESCRIPTION("Make sure atomic validation handles if from a OpConvertUToPtr (physical pointer)");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(Init());

    const char *spv_source = R"(
               OpCapability Int64
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpExtension "SPV_EXT_descriptor_indexing"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %tex_ref 0 Offset 0
               OpMemberDecorate %tex_ref 1 Offset 4
               OpDecorate %_runtimearr_tex_ref ArrayStride 8
               OpMemberDecorate %outbuftype 0 Offset 0
               OpDecorate %outbuftype BufferBlock
               OpDecorate %outbuf DescriptorSet 0
               OpDecorate %outbuf Binding 0
               OpMemberDecorate %__rd_feedbackStruct 0 Offset 0
               OpDecorate %__rd_feedbackStruct Block
       %void = OpTypeVoid
      %voidf = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
    %tex_ref = OpTypeStruct %uint %uint
%_runtimearr_tex_ref = OpTypeRuntimeArray %tex_ref
 %outbuftype = OpTypeStruct %_runtimearr_tex_ref
%_runtimearr_outbuftype = OpTypeRuntimeArray %outbuftype
%_ptr_Uniform__runtimearr_outbuftype = OpTypePointer Uniform %_runtimearr_outbuftype
     %outbuf = OpVariable %_ptr_Uniform__runtimearr_outbuftype Uniform
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
     %v3uint = OpTypeVector %uint 3
      %ulong = OpTypeInt 64 0
  %ulong_2 = OpConstant %ulong 2
  %ulong_1 = OpConstant %ulong 1
%__rd_feedbackStruct = OpTypeStruct %uint
%__feedbackOffset_set0_bind0 = OpConstant %ulong 0
%__rd_feedbackAddress = OpConstant %ulong 260636672
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
%uint_4294967295 = OpConstant %uint 4294967295
     %uint_4 = OpConstant %uint 4
   %uint_0_0 = OpConstant %uint 0
       %main = OpFunction %void None %voidf
         %60 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_uint %outbuf %int_0 %int_0 %int_0 %int_0
         %65 = OpExtInst %ulong %1 UMin %ulong_1 %ulong_2
         %66 = OpIAdd %ulong %__rd_feedbackAddress %__feedbackOffset_set0_bind0
         %67 = OpShiftLeftLogical %ulong %65 %uint_4
         %68 = OpIAdd %ulong %66 %67
         %69 = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_uint %68
         %70 = OpAtomicUMax %uint %69 %uint_4 %uint_0_0 %uint_4294967295
               OpStore %63 %uint_0
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(PositiveAtomic, Int64) {
    TEST_DESCRIPTION("Test VK_KHR_shader_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderBufferInt64Atomics);
    AddRequiredExtensions(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    const char *current_shader = nullptr;
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    };

    current_shader = cs_storage_buffer.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveAtomic, Int64Shared) {
    TEST_DESCRIPTION("Test VK_KHR_shader_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderSharedInt64Atomics);
    AddRequiredExtensions(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs_workgroup = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_atomic_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
           atomicAdd(x, 1);
           barrier();
           y = x + 1;
        }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_workgroup, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    };

    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveAtomic, OpImageTexelPointerWithNoAtomic) {
    TEST_DESCRIPTION("Have a OpImageTexelPointer without an actual OpAtomic* accessing it");

    RETURN_IF_SKIP(Init());

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;
    // Need to have VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
    //      but not VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), format, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), format, formatProps);

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    const char *spv_source = R"(
             OpCapability Shader
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpDecorate %image DescriptorSet 0
             OpDecorate %image Binding 0
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
     %uint = OpTypeInt 32 0
        %7 = OpTypeImage %uint 2D 0 0 0 2 R32ui
   %ptr_uc = OpTypePointer UniformConstant %7
    %image = OpVariable %ptr_uc UniformConstant
      %int = OpTypeInt 32 1
    %v2int = OpTypeVector %int 2
    %int_0 = OpConstant %int 0
       %13 = OpConstantComposite %v2int %int_0 %int_0
   %uint_0 = OpConstant %uint 0
%ptr_image = OpTypePointer Image %uint
     %main = OpFunction %void None %3
        %5 = OpLabel
       %18 = OpImageTexelPointer %ptr_image %image %13 %uint_0
             OpReturn
             OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveAtomic, ImageFloat16Vector) {
    TEST_DESCRIPTION("Test VK_NV_shader_atomic_float16_vector.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_NV_SHADER_ATOMIC_FLOAT16_VECTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderFloat16VectorAtomics);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
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

    const char *current_shader = nullptr;
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    current_shader = cs_image_add.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_image_min.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_image_max.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    current_shader = cs_image_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveAtomic, VertexPipelineStoresAndAtomics) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8223");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(set=0, binding=0, std430) readonly buffer SSBO {
            float a;
            float b;
        } data;
        layout(location=0) out float o;
        void main(void) {
            o = data.b;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    pipe.CreateGraphicsPipeline();
}
