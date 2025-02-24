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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

const char *vkComponentTypeToGLSL(VkComponentTypeKHR type) {
    switch (type) {
        case VK_COMPONENT_TYPE_FLOAT16_KHR:
            return "float16_t";
        case VK_COMPONENT_TYPE_FLOAT32_KHR:
            return "float32_t";
        case VK_COMPONENT_TYPE_FLOAT64_KHR:
            return "float64_t";
        case VK_COMPONENT_TYPE_SINT8_KHR:
            return "int8_t";
        case VK_COMPONENT_TYPE_SINT16_KHR:
            return "int16_t";
        case VK_COMPONENT_TYPE_SINT32_KHR:
            return "int32_t";
        case VK_COMPONENT_TYPE_SINT64_KHR:
            return "int64_t";
        case VK_COMPONENT_TYPE_UINT8_KHR:
            return "uint8_t";
        case VK_COMPONENT_TYPE_UINT16_KHR:
            return "uint16_t";
        case VK_COMPONENT_TYPE_UINT32_KHR:
            return "uint32_t";
        case VK_COMPONENT_TYPE_UINT64_KHR:
            return "uint64_t";
        default:
            return "unknown";
    }
}

void CooperativeMatrixTest::InitCooperativeMatrixKHR() {
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());
    uint32_t props_count = 0;
    vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(Gpu(), &props_count, nullptr);
    for (uint32_t i = 0; i < props_count; i++) {
        coop_matrix_props.emplace_back(vku::InitStruct<VkCooperativeMatrixPropertiesKHR>());
    }
    vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(Gpu(), &props_count, coop_matrix_props.data());

    if (IsExtensionsEnabled(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME)) {
        props_count = 0;
        vk::GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(Gpu(), &props_count, nullptr);
        for (uint32_t i = 0; i < props_count; i++) {
            coop_matrix_flex_props.emplace_back(vku::InitStruct<VkCooperativeMatrixFlexibleDimensionsPropertiesNV>());
        }
        vk::GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(Gpu(), &props_count, coop_matrix_flex_props.data());
    }
}

bool CooperativeMatrixTest::HasValidProperty(VkScopeKHR scope, uint32_t m, uint32_t n, uint32_t k, VkComponentTypeKHR type) {
    bool found_a = false;
    bool found_b = false;
    bool found_c = false;
    bool found_r = false;
    for (const auto &prop : coop_matrix_props) {
        if (prop.scope == scope && prop.AType == type && prop.MSize == m && prop.KSize == k) {
            found_a = true;
        }
        if (prop.scope == scope && prop.BType == type && prop.KSize == k && prop.NSize == n) {
            found_b = true;
        }
        if (prop.scope == scope && prop.CType == type && prop.MSize == m && prop.NSize == n) {
            found_c = true;
        }
        if (prop.scope == scope && prop.ResultType == type && prop.MSize == m && prop.NSize == n) {
            found_r = true;
        }
    }
    if (found_a && found_b && found_c && found_r) {
        return true;
    }

    found_a = false;
    found_b = false;
    found_c = false;
    found_r = false;
    for (const auto &prop : coop_matrix_flex_props) {
        if (prop.scope == scope && prop.AType == type && (m % prop.MGranularity) == 0 && (k % prop.KGranularity) == 0) {
            found_a = true;
        }
        if (prop.scope == scope && prop.BType == type && (k % prop.KGranularity) == 0 && (n % prop.NGranularity) == 0) {
            found_b = true;
        }
        if (prop.scope == scope && prop.CType == type && (m % prop.MGranularity) == 0 && (n % prop.NGranularity) == 0) {
            found_c = true;
        }
        if (prop.scope == scope && prop.ResultType == type && (m % prop.MGranularity) == 0 && (n % prop.NGranularity) == 0) {
            found_r = true;
        }
    }
    if (found_a && found_b && found_c && found_r) {
        return true;
    }

    return false;
}

class PositiveShaderCooperativeMatrix : public CooperativeMatrixTest {};

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrixNV) {
    TEST_DESCRIPTION("Test VK_NV_cooperative_matrix.");

    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesNV cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_NV_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        layout(constant_id = 0) const uint C0 = 1;
        layout(constant_id = 1) const uint C1 = 1;
        void main() {
           // Bad type
           fcoopmatNV<16, gl_ScopeSubgroup, 3, 5> badSize = fcoopmatNV<16, gl_ScopeSubgroup, 3, 5>(float16_t(0.0));
           // Not a valid multiply when C0 != C1
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> A;
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> B;
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> C;
           coopMatMulAddNV(A, B, C);
        }
    )glsl";

    const uint32_t specData[] = {
        16,
        8,
    };
    VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };

    VkSpecializationInfo specInfo = {
        2,
        entries,
        sizeof(specData),
        specData,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo);
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrixKHR) {
    TEST_DESCRIPTION("Test VK_KHR_cooperative_matrix.");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    VkPhysicalDeviceCooperativeMatrixPropertiesKHR props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props);
    if ((props.cooperativeMatrixSupportedStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "Compute stage is not supported";
    }

    VkCooperativeMatrixPropertiesKHR subgroup_prop = vku::InitStructHelper();
    bool found_scope_subgroup = false;
    for (const auto &prop : coop_matrix_props) {
        if (prop.scope == VK_SCOPE_SUBGROUP_KHR) {
            found_scope_subgroup = true;
            subgroup_prop = prop;
            break;
        }
    }
    if (!found_scope_subgroup) {
        GTEST_SKIP() << "VK_SCOPE_SUBGROUP_KHR not Found";
    }

    const VkSampler *ptr = nullptr;
    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
    };
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    std::string css = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer InputA { %type_A% x[]; } inputA;
         layout(set=0, binding=1) coherent buffer InputB { %type_B% x[]; } inputB;
         layout(set=0, binding=2) coherent buffer InputC { %type_C% x[]; } inputC;
         layout(set=0, binding=3) coherent buffer Output { %type_R% x[]; } outputO;
         coopmat<%type_A%, gl_ScopeSubgroup, %M%, %K%, gl_MatrixUseA> matA;
         coopmat<%type_B%, gl_ScopeSubgroup, %K%, %N%, gl_MatrixUseB> matB;
         coopmat<%type_C%, gl_ScopeSubgroup, %M%, %N%, gl_MatrixUseAccumulator> matC;
         coopmat<%type_R%, gl_ScopeSubgroup, %M%, %N%, gl_MatrixUseAccumulator> matO;
         void main()
         {
             coopMatLoad(matA, inputA.x, 0, %M%, gl_CooperativeMatrixLayoutRowMajor);
             coopMatLoad(matB, inputB.x, 0, %K%, gl_CooperativeMatrixLayoutRowMajor);
             coopMatLoad(matC, inputC.x, 0, %M%, gl_CooperativeMatrixLayoutRowMajor);
             matO = coopMatMulAdd(matA, matB, matC);
             coopMatStore(matO, outputO.x, 0, %M%, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    auto replace = [](std::string &str, const std::string &from, const std::string &to) {
        size_t pos;
        while ((pos = str.find(from)) != std::string::npos) str.replace(pos, from.length(), to);
    };
    replace(css, "%M%", std::to_string(subgroup_prop.MSize));
    replace(css, "%N%", std::to_string(subgroup_prop.NSize));
    replace(css, "%K%", std::to_string(subgroup_prop.KSize));
    replace(css, "%type_A%", vkComponentTypeToGLSL(subgroup_prop.AType));
    replace(css, "%type_B%", vkComponentTypeToGLSL(subgroup_prop.BType));
    replace(css, "%type_C%", vkComponentTypeToGLSL(subgroup_prop.CType));
    replace(css, "%type_R%", vkComponentTypeToGLSL(subgroup_prop.ResultType));

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, css.c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&dsl});
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}
