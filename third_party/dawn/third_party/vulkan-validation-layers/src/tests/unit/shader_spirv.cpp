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

struct icd_spv_header {
    uint32_t magic = 0x07230203;
    uint32_t version = 99;
    uint32_t gen_magic = 0;  // Generator's magic number
};

class NegativeShaderSpirv : public VkLayerTest {};

TEST_F(NegativeShaderSpirv, CodeSize) {
    TEST_DESCRIPTION("Test that errors are produced for a spirv modules with invalid code sizes");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    {
        VkShaderModule module;
        VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();

        module_create_info.pCode = nullptr;
        module_create_info.codeSize = 0;

        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-codeSize-01085");
        vk::CreateShaderModule(device(), &module_create_info, nullptr, &module);
        m_errorMonitor->VerifyFound();
    }

    {
        VkShaderModule module;
        VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();

        constexpr icd_spv_header spv = {};
        module_create_info.pCode = reinterpret_cast<const uint32_t *>(&spv);
        module_create_info.codeSize = 4;

        m_errorMonitor->SetDesiredError("Invalid SPIR-V header");
        vk::CreateShaderModule(device(), &module_create_info, nullptr, &module);
        m_errorMonitor->VerifyFound();
    }

    {
        std::vector<uint32_t> shader;
        VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
        VkShaderModule module;
        GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl, shader);
        module_create_info.pCode = shader.data();
        // Introduce failure by making codeSize a non-multiple of 4
        module_create_info.codeSize = shader.size() * sizeof(uint32_t) - 1;

        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-codeSize-08735");
        vk::CreateShaderModule(m_device->handle(), &module_create_info, nullptr, &module);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderSpirv, CodeSizeMaintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = nullptr;
    module_create_info.codeSize = 0;

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-codeSize-01085");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    std::vector<uint32_t> shader;
    GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl, shader);
    module_create_info.pCode = shader.data();
    // Introduce failure by making codeSize a non-multiple of 4
    module_create_info.codeSize = shader.size() * sizeof(uint32_t) - 1;

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-codeSize-08735");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, CodeSizeMaintenance5Compute) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = nullptr;
    module_create_info.codeSize = 0;

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    vkt::PipelineLayout layout(*m_device, {});
    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = stage_ci;
    pipe.cp_ci_.layout = layout.handle();

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-codeSize-01085");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();

    std::vector<uint32_t> shader;
    GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_VERTEX_BIT, kMinimalShaderGlsl, shader);
    module_create_info.pCode = shader.data();
    // Introduce failure by making codeSize a non-multiple of 4
    module_create_info.codeSize = shader.size() * sizeof(uint32_t) - 1;

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-codeSize-08735");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, Magic) {
    TEST_DESCRIPTION("Test that an error is produced for a spirv module with a bad magic number");
    RETURN_IF_SKIP(Init());

    VkShaderModule module;
    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();

    constexpr uint32_t bad_magic = 4175232508U;
    constexpr icd_spv_header spv = {bad_magic};

    module_create_info.pCode = reinterpret_cast<const uint32_t *>(&spv);
    module_create_info.codeSize = sizeof(spv);

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-07912");
    vk::CreateShaderModule(device(), &module_create_info, nullptr, &module);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, MagicMaintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    constexpr uint32_t bad_magic = 4175232508U;
    constexpr icd_spv_header spv = {bad_magic};

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = reinterpret_cast<const uint32_t *>(&spv);
    module_create_info.codeSize = sizeof(spv);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-07912");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, MagicMaintenance5Compute) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    constexpr uint32_t bad_magic = 4175232508U;
    constexpr icd_spv_header spv = {bad_magic};

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = reinterpret_cast<const uint32_t *>(&spv);
    module_create_info.codeSize = sizeof(spv);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";
    vkt::PipelineLayout layout(*m_device, {});
    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = stage_ci;
    pipe.cp_ci_.layout = layout.handle();

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-07912");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderFloatControl) {
    TEST_DESCRIPTION("Test VK_KHR_shader_float_controls");

    // Need 1.1 to get SPIR-V 1.3 since OpExecutionModeId was added in SPIR-V 1.2
    SetTargetApiVersion(VK_API_VERSION_1_1);

    // The issue with revision 4 of this extension should not be an issue with the tests
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);

    if (shader_float_control.denormBehaviorIndependence == VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        GTEST_SKIP() << "denormBehaviorIndependence is NONE";
    }
    if (shader_float_control.roundingModeIndependence == VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        GTEST_SKIP() << "roundingModeIndependence is NONE";
    }

    // Check for support of 32-bit properties, but only will test if they are not supported
    // in case all 16/32/64 version are not supported will set SetUnexpectedError for capability check
    bool signed_zero_inf_nan_preserve = (shader_float_control.shaderSignedZeroInfNanPreserveFloat32 == VK_TRUE);
    bool denorm_preserve = (shader_float_control.shaderDenormPreserveFloat32 == VK_TRUE);
    bool denorm_flush_to_zero = (shader_float_control.shaderDenormFlushToZeroFloat32 == VK_TRUE);
    bool rounding_mode_rte = (shader_float_control.shaderRoundingModeRTEFloat32 == VK_TRUE);
    bool rounding_mode_rtz = (shader_float_control.shaderRoundingModeRTZFloat32 == VK_TRUE);

    // same body for each shader, only the start is different
    // this is just "float a = 1.0 + 2.0;" in SPIR-V
    const std::string source_body = R"(
             OpExecutionMode %main LocalSize 1 1 1
             OpSource GLSL 450
             OpName %main "main"
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %float = OpTypeFloat 32
%pFunction = OpTypePointer Function %float
  %float_3 = OpConstant %float 3
     %main = OpFunction %void None %3
        %5 = OpLabel
        %6 = OpVariable %pFunction Function
             OpStore %6 %float_3
             OpReturn
             OpFunctionEnd
)";

    if (!signed_zero_inf_nan_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability SignedZeroInfNanPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main SignedZeroInfNanPreserve 32
)" + source_body;

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294");
        VkShaderObj::CreateFromASM(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    if (!denorm_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormPreserve 32
)" + source_body;

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297");
        VkShaderObj::CreateFromASM(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    if (!denorm_flush_to_zero) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormFlushToZero
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormFlushToZero 32
)" + source_body;

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300");
        VkShaderObj::CreateFromASM(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    if (!rounding_mode_rte) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTE
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTE 32
)" + source_body;

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303");
        VkShaderObj::CreateFromASM(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    if (!rounding_mode_rtz) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTZ
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTZ 32
)" + source_body;

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306");
        VkShaderObj::CreateFromASM(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderSpirv, Storage8and16bitCapability) {
    TEST_DESCRIPTION("Test VK_KHR_8bit_storage and VK_KHR_16bit_storage not having feature bits required for SPIR-V capability");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // storageBuffer8BitAccess
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(set = 0, binding = 0) buffer SSBO { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer8BitAccess-06328");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);     // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }
    // uniformAndStorageBuffer8BitAccess
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(set = 0, binding = 0) uniform UBO { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);               // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();
    }

    // storagePushConstant8
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(push_constant) uniform PushConstant { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);  // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();
    }

    // storageBuffer16BitAccess - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(set = 0, binding = 0) buffer SSBO { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);      // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // uniformAndStorageBuffer16BitAccess - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(set = 0, binding = 0) uniform UBO { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);                // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();
    }

    // storagePushConstant16 - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(push_constant) uniform PushConstant { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant16-06333");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);   // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // storageInputOutput16 - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(location = 0) out float16_t outData;
            void main(){
               outData = float16_t(1);
               gl_Position = vec4(0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);  // Int8
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();

        // Need to match in/out
        char const *fsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(location = 0) in float16_t x;
            layout(location = 0) out vec4 uFragColor;
            void main(){
               uFragColor = vec4(0,1,0,1);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // Int8
        VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();
    }

    // storageBuffer16BitAccess - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(set = 0, binding = 0) buffer SSBO { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);      // Int16 and StorageBuffer16BitAccess
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // uniformAndStorageBuffer16BitAccess - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(set = 0, binding = 0) uniform UBO { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740",
                                        2);  // Int16 and UniformAndStorageBuffer16BitAccess
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();
    }

    // storagePushConstant16 - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(push_constant) uniform PushConstant { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant16-06333");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);   // Int16 and StoragePushConstant16
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);
        m_errorMonitor->VerifyFound();
    }

    // storageInputOutput16 - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(location = 0) out int16_t outData;
            void main(){
               outData = int16_t(1);
               gl_Position = vec4(0.0);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);  // Int16 and StorageInputOutput16
        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();

        // Need to match in/out
        char const *fsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(location = 0) flat in int16_t x;
            layout(location = 0) out vec4 uFragColor;
            void main(){
               uFragColor = vec4(0,1,0,1);
            }
        )glsl";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");  // feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // StorageInputOutput16
        VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderSpirv, SpirvStatelessMaintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::maintenance5);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_8bit_storage: enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
        layout(set = 0, binding = 0) buffer SSBO { int8_t x; } data;
        void main(){
            int8_t a = data.x + data.x;
            gl_Position = vec4(float(a) * 0.0);
        }
    )glsl";
    std::vector<uint32_t> shader;
    GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_VERTEX_BIT, vsSource, shader);

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = shader.data();
    module_create_info.codeSize = shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740", 2);     // Int8
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, Storage8and16bitFeatures) {
    TEST_DESCRIPTION(
        "Test VK_KHR_8bit_storage and VK_KHR_16bit_storage where the Int8/Int16 capability are only used and since they are "
        "superset of a capabilty");

    // the following [OpCapability UniformAndStorageBuffer8BitAccess] requires the uniformAndStorageBuffer8BitAccess feature bit or
    // the generated capability checking code will catch it
    //
    // But having just [OpCapability Int8] is still a legal SPIR-V shader because the Int8 capabilty allows all storage classes in
    // the SPIR-V spec... but the shaderInt8 feature bit in Vulkan spec explains how you still need the
    // uniformAndStorageBuffer8BitAccess feature bit for Uniform storage class from Vulkan's perspective

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // Prevent extra errors for not having the support for the SPV extensions

    VkPhysicalDeviceShaderFloat16Int8Features float16Int8 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(float16Int8);
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    if (float16Int8.shaderInt8 == VK_TRUE) {
        // storageBuffer8BitAccess
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer8BitAccess-06328");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // uniformAndStorageBuffer8BitAccess
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // storagePushConstant8
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }
    }

    if (float16Int8.shaderFloat16 == VK_TRUE) {
        // storageBuffer16BitAccess - float
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // uniformAndStorageBuffer16BitAccess - float
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // storagePushConstant16 - float
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant16-06333");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // storageInputOutput16 - float
        {
            const char *vs_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %var
               OpDecorate %var Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
        %ptr = OpTypePointer Output %float16
        %var = OpVariable %ptr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");
            auto vs = VkShaderObj::CreateFromASM(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();

            const char *fs_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in %out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %in Location 0
               OpDecorate %out Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
      %inPtr = OpTypePointer Input %float16
         %in = OpVariable %inPtr Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %outPtr = OpTypePointer Output %v4float
        %out = OpVariable %outPtr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");
            auto fs = VkShaderObj::CreateFromASM(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
            m_errorMonitor->VerifyFound();
        }
    }

    if (features2.features.shaderInt16 == VK_TRUE) {
        // storageBuffer16BitAccess - int
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // uniformAndStorageBuffer16BitAccess - int
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // storagePushConstant16 - int
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant16-06333");
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();
        }

        // storageInputOutput16 - int
        {
            const char *vs_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %var
               OpDecorate %var Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
        %ptr = OpTypePointer Output %int16
        %var = OpVariable %ptr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");
            auto vs = VkShaderObj::CreateFromASM(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
            m_errorMonitor->VerifyFound();

            const char *fs_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in %out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %in Location 0
               OpDecorate %in Flat
               OpDecorate %out Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
      %inPtr = OpTypePointer Input %int16
         %in = OpVariable %inPtr Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %outPtr = OpTypePointer Output %v4float
        %out = OpVariable %outPtr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageInputOutput16-06334");
            auto fs = VkShaderObj::CreateFromASM(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
            m_errorMonitor->VerifyFound();
        }
    }

    // tests struct with multiple types
    if (float16Int8.shaderInt8 == VK_TRUE && features2.features.shaderInt16 == VK_TRUE) {
        // struct X {
        //   u16vec2 a;
        // };
        // struct {
        //   uint a;
        //   X b;
        //   uint8_t c;
        // } Data;
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpCapability Int16
               OpExtension "SPV_KHR_8bit_storage"
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %X 0 Offset 0
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 4
               OpMemberDecorate %Data 2 Offset 8
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
      %int16 = OpTypeInt 16 0
    %v2int16 = OpTypeVector %int16 2
      %int32 = OpTypeInt 32 0
          %X = OpTypeStruct %v2int16
       %Data = OpTypeStruct %int32 %X %int8
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");  // 16 bit var
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storageBuffer8BitAccess-06328");   // 8 bit var
        auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderSpirv, ReadShaderClock) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    // Don't enable either feature bit on
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Device scope using GL_EXT_shader_realtime_clock
    char const *vsSourceDevice = R"glsl(
        #version 450
        #extension GL_EXT_shader_realtime_clock: enable
        void main(){
           uvec2 a = clockRealtime2x32EXT();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderDeviceClock-06268");
    VkShaderObj vs_device(this, vsSourceDevice, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();

    // Subgroup scope using ARB_shader_clock
    char const *vsSourceScope = R"glsl(
        #version 450
        #extension GL_ARB_shader_clock: enable
        void main(){
           uvec2 a = clock2x32ARB();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSubgroupClock-06267");
    VkShaderObj vs_subgroup(this, vsSourceScope, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, SpecializationApplied) {
    TEST_DESCRIPTION(
        "Make sure specialization constants get applied during shader validation by using a value that breaks compilation.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Size an array using a specialization constant of default value equal to 1.
    const char *fs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %size "size"
               OpName %array "array"
               OpDecorate %size SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
       %size = OpSpecConstant %int 1
%_arr_float_size = OpTypeArray %float %size
%_ptr_Function__arr_float_size = OpTypePointer Function %_arr_float_size
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
       %main = OpFunction %void None %3
          %5 = OpLabel
      %array = OpVariable %_ptr_Function__arr_float_size Function
         %15 = OpAccessChain %_ptr_Function_float %array %int_0
               OpStore %15 %float_0
               OpReturn
               OpFunctionEnd)";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    // Set the specialization constant to 0.
    const VkSpecializationMapEntry entry = {
        0,                // id
        0,                // offset
        sizeof(uint32_t)  // size
    };
    uint32_t data = 0;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint32_t),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
}

TEST_F(NegativeShaderSpirv, SpecializationOffsetOutOfBounds) {
    TEST_DESCRIPTION("Validate VkSpecializationInfo offset.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Entry offset is greater than dataSize.
    const VkSpecializationMapEntry entry = {0, 5, sizeof(uint32_t)};

    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-offset-00773");
}

TEST_F(NegativeShaderSpirv, SpecializationOffsetOutOfBoundsWithIdentifier) {
    TEST_DESCRIPTION("Validate VkSpecializationInfo offset using a shader module identifier.");

    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineCreationCacheControl);
    AddRequiredFeature(vkt::Feature::shaderModuleIdentifier);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        layout (constant_id = 0) const float x = 0.0f;
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineShaderStageModuleIdentifierCreateInfoEXT sm_id_create_info = vku::InitStructHelper();
    VkShaderModuleIdentifierEXT get_identifier = vku::InitStructHelper();
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &get_identifier);
    sm_id_create_info.identifierSize = get_identifier.identifierSize;
    sm_id_create_info.pIdentifier = get_identifier.identifier;

    // Entry offset is greater than dataSize.
    const VkSpecializationMapEntry entry = {0, 5, sizeof(uint32_t)};
    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&sm_id_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";
    stage_ci.pSpecializationInfo = &specialization_info;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkSpecializationInfo-offset-00773");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, SpecializationSizeOutOfBounds) {
    TEST_DESCRIPTION("Challenge core_validation with shader validation issues related to vkCreateGraphicsPipelines.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Entry size is greater than dataSize minus offset.
    const VkSpecializationMapEntry entry = {0, 3, sizeof(uint32_t)};

    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-pMapEntries-00774");
}

TEST_F(NegativeShaderSpirv, SpecializationSizeZero) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is 0");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *cs_src = R"glsl(
        #version 450
        layout (constant_id = 0) const int c = 3;
        layout (local_size_x = 1) in;
        void main() {
            if (gl_GlobalInvocationID.x >= c) { return; }
        }
    )glsl";

    // Set the specialization constant size to 0 (anything other than 1, 2, 4, or 8 will produce the expected error).
    VkSpecializationMapEntry entry = {
        0,  // id
        0,  // offset
        0,  // size
    };
    int32_t data = 0;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(decltype(data)),
        &data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    m_errorMonitor->SetDesiredError("VUID-VkSpecializationMapEntry-constantID-00776");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    entry.size = sizeof(decltype(data));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.CreateComputePipeline();
}

TEST_F(NegativeShaderSpirv, SpecializationSizeMismatch) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is not correct with type");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());

    // layout (constant_id = 0) const int a = 3;
    // layout (constant_id = 1) const uint b = 3;
    // layout (constant_id = 2) const float c = 3.0f;
    // layout (constant_id = 3) const bool d = true;
    // layout (constant_id = 4) const bool f = false;
    const char *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
               OpDecorate %c SpecId 2
               OpDecorate %d SpecId 3
               OpDecorate %f SpecId 4
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
       %bool = OpTypeBool
          %a = OpSpecConstant %int 3
          %b = OpSpecConstant %uint 3
          %c = OpSpecConstant %float 3
          %d = OpSpecConstantTrue %bool
          %f = OpSpecConstantFalse %bool
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    // use same offset to keep simple since unused data being read
    VkSpecializationMapEntry entries[5] = {
        {0, 0, 4},                 // OpTypeInt 32
        {1, 0, 4},                 // OpTypeInt 32
        {2, 0, 4},                 // OpTypeFloat 32
        {3, 0, sizeof(VkBool32)},  // OpTypeBool
        {4, 0, sizeof(VkBool32)}   // OpTypeBool
    };

    std::array<int32_t, 4> data;  // enough garbage data to grab from
    VkSpecializationInfo specialization_info = {
        5,
        entries,
        data.size() * sizeof(decltype(data)::value_type),
        data.data(),
    };

    std::unique_ptr<VkShaderObj> cs;
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };

    // Sanity check
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    if (!cs) {
        GTEST_SKIP() << "Driver bug - not able to create shader module";
    }

    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // signed int mismatch
    entries[0].size = 0;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 2;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 8;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 4;  // reset

    // unsigned int mismatch
    entries[1].size = 1;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[1].size = 8;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[1].size = 3;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[1].size = 4;  // reset

    // float mismatch
    entries[2].size = 0;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[2].size = 8;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[2].size = 7;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[2].size = 4;  // reset

    // bool mismatch
    entries[3].size = sizeof(VkBool32) / 2;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[3].size = sizeof(VkBool32) + 1;
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
}

TEST_F(NegativeShaderSpirv, SpecializationSizeMismatchInt8) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is not correct with type");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    RETURN_IF_SKIP(Init());

    // use same offset to keep simple since unused data being read
    VkSpecializationMapEntry entries[2] = {
        {0, 0, 4},  // OpTypeInt 32
        {1, 0, 4},  // OpTypeInt 32
    };

    std::array<int32_t, 2> data;  // enough garbage data to grab from
    VkSpecializationInfo specialization_info = {
        2,
        entries,
        data.size() * sizeof(decltype(data)::value_type),
        data.data(),
    };

    std::unique_ptr<VkShaderObj> cs;
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };

    // #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
    // layout (constant_id = 0) const int8_t a = int8_t(3);
    // layout (constant_id = 1) const uint8_t b = uint8_t(3);
    const char *cs_int8 = R"(
            OpCapability Shader
            OpCapability Int8
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main LocalSize 1 1 1
            OpSource GLSL 450
            OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int8"
            OpDecorate %a SpecId 0
            OpDecorate %b SpecId 1
    %void = OpTypeVoid
    %func = OpTypeFunction %void
    %char = OpTypeInt 8 1
    %uchar = OpTypeInt 8 0
        %a = OpSpecConstant %char 3
        %b = OpSpecConstant %uchar 3
    %main = OpFunction %void None %func
    %label = OpLabel
            OpReturn
            OpFunctionEnd
        )";

    specialization_info.mapEntryCount = 2;
    entries[0] = {0, 0, 1};  // OpTypeInt 8
    entries[1] = {1, 0, 1};  // OpTypeInt 8

    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    if (!cs) {
        GTEST_SKIP() << "Driver bug - not able to create shader module";
    }
    // Sanity check
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // signed int 8 mismatch
    entries[0].size = 0;
    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 2;
    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 4;
    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 1;  // reset

    // unsigned int 8 mismatch
    entries[1].size = 0;
    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[1].size = 2;
    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[1].size = 4;
    cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
}

TEST_F(NegativeShaderSpirv, SpecializationSizeMismatchFloat64) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is not correct with type");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());

    // use same offset to keep simple since unused data being read
    VkSpecializationMapEntry entries[3] = {
        {0, 0, 4},  // OpTypeInt 32
        {1, 0, 4},  // OpTypeInt 32
        {2, 0, 4},  // OpTypeFloat 32
    };

    std::array<int32_t, 4> data;  // enough garbage data to grab from
    VkSpecializationInfo specialization_info = {
        3,
        entries,
        data.size() * sizeof(decltype(data)::value_type),
        data.data(),
    };

    std::unique_ptr<VkShaderObj> cs;
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };

    // #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
    // layout (constant_id = 0) const float64_t a = 3.0f;
    const char *cs_float64 = R"(
            OpCapability Shader
            OpCapability Float64
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main LocalSize 1 1 1
            OpSource GLSL 450
            OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float64"
            OpDecorate %a SpecId 0
    %void = OpTypeVoid
    %func = OpTypeFunction %void
    %double = OpTypeFloat 64
        %a = OpSpecConstant %double 3
    %main = OpFunction %void None %func
    %label = OpLabel
            OpReturn
            OpFunctionEnd
        )";

    specialization_info.mapEntryCount = 1;
    entries[0] = {0, 0, 8};  // OpTypeFloat

    cs = VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    if (!cs) {
        GTEST_SKIP() << "Driver bug - not able to create shader module";
    }

    // Sanity check
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // float 64 mismatch
    entries[0].size = 1;
    cs = VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 2;
    cs = VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 4;
    cs = VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    entries[0].size = 16;
    cs = VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
}

TEST_F(NegativeShaderSpirv, DuplicatedSpecializationConstantID) {
    TEST_DESCRIPTION("Create a pipeline with non unique constantID in specialization pMapEntries.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkSpecializationMapEntry entries[2];
    entries[0].constantID = 0;
    entries[0].offset = 0;
    entries[0].size = sizeof(uint32_t);
    entries[1].constantID = 0;
    entries[1].offset = 0;
    entries[1].size = sizeof(uint32_t);

    uint32_t data = 1;
    VkSpecializationInfo specialization_info;
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-constantID-04911");
}

TEST_F(NegativeShaderSpirv, ShaderModuleCheckCapability) {
    TEST_DESCRIPTION("Create a shader in which a capability declared by the shader is not supported.");
    // Note that this failure message comes from spirv-tools, specifically the validator.

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *spv_source = R"(
                  OpCapability ImageRect
                  OpEntryPoint Vertex %main "main"
          %main = OpFunction %void None %3
                  OpReturn
                  OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("Capability ImageRect is not allowed by Vulkan");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderNotEnabled) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline in which a capability declared by the shader requires a feature not enabled on the device.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main(){
           dvec4 green = vec4(0.0, 1.0, 0.0, 1.0);
           color = vec4(green);
        }
    )glsl";
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NonSemanticInfoEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_non_semantic_info.");

    RETURN_IF_SKIP(Init());
    if (!DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_shader_non_semantic_info not supported";
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    const char *source = R"(
                   OpCapability Shader
                   OpExtension "SPV_KHR_non_semantic_info"
   %non_semantic = OpExtInstImport "NonSemantic.Validation.Test"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
              %1 = OpExtInst %void %non_semantic 55 %void
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
              %2 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
    VkShaderObj::CreateFromASM(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderImageFootprintEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the shader image footprint feature which has not enabled on the device.");
    AddRequiredExtensions(VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_NV_shader_texture_footprint  : require
        layout(set=0, binding=0) uniform sampler2D s;
        layout(location=0) out vec4 color;
        void main(){
          gl_TextureFootprint2DNV footprint;
          if (textureFootprintNV(s, vec2(1.0), 5, false, footprint)) {
            color = vec4(0.0, 1.0, 0.0, 1.0);
          } else {
            color = vec4(vec2(footprint.anchor), vec2(footprint.offset));
          }
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, FragmentShaderBarycentricEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader barycentric feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_NV_fragment_shader_barycentric : require
        layout(location=0) out float value;
        void main(){
          value = gl_BaryCoordNV.x;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ComputeShaderDerivativesEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the compute shader derivatives feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    char const *csSource = R"glsl(
        #version 450
        #extension GL_NV_compute_shader_derivatives : require
        layout(local_size_x=2, local_size_y=4) in;
        layout(derivative_group_quadsNV) in;
        layout(set=0, binding=0) buffer InputOutputBuffer {
          float values[];
        };
        void main(){
           values[gl_LocalInvocationIndex] = dFdx(values[gl_LocalInvocationIndex]);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, FragmentShaderInterlockEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader interlock feature which has not enabled on the device.");

    AddRequiredExtensions(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_ARB_fragment_shader_interlock : require
        layout(sample_interlock_ordered) in;
        void main(){
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DemoteToHelperInvocation) {
    TEST_DESCRIPTION("Create a pipeline requiring the demote to helper invocation feature which has not enabled on the device.");

    AddRequiredExtensions(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_demote_to_helper_invocation : require
        void main(){
            demote;
        }

    )glsl";
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoUniformBufferStandardLayout10) {
    TEST_DESCRIPTION("Don't enable uniformBufferStandardLayout in Vulkan 1.0 and have spirv-val catch invalid shader");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init());
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // layout(std430, set = 0, binding = 0) uniform ubo430 {
    //     float floatArray430[8];
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %ubo430 0 Offset 0
               OpDecorate %ubo430 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
     %ubo430 = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_ubo430 = OpTypePointer Uniform %ubo430
          %_ = OpVariable %_ptr_Uniform_ubo430 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoUniformBufferStandardLayout12) {
    TEST_DESCRIPTION(
        "Don't enable uniformBufferStandardLayout in Vulkan1.2 when VK_KHR_uniform_buffer_standard_layout was promoted");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout(std430, set = 0, binding = 0) uniform ubo430 {
    //     float floatArray430[8];
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %ubo430 0 Offset 0
               OpDecorate %ubo430 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
     %ubo430 = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_ubo430 = OpTypePointer Uniform %ubo430
          %_ = OpVariable %_ptr_Uniform_ubo430 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoScalarBlockLayout10) {
    TEST_DESCRIPTION("Don't enable scalarBlockLayout in Vulkan 1.0 and have spirv-val catch invalid shader");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init());
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // layout (scalar, set = 0, binding = 0) buffer ssbo {
    //     layout(offset = 4) vec3 x;
    // };
    //
    // Note: using BufferBlock for Vulkan 1.0
    // Note: Relaxed Block Layout would also make this valid if enabled
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %ssbo 0 Offset 4
               OpDecorate %ssbo BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
       %ssbo = OpTypeStruct %v3float
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoScalarBlockLayout12) {
    TEST_DESCRIPTION("Don't enable scalarBlockLayout in Vulkan1.2 when VK_EXT_scalar_block_layout was promoted");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // layout (scalar, set = 0, binding = 0) buffer ssbo {
    //     layout(offset = 0) vec3 a;
    //     layout(offset = 12) vec2 b;
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %ssbo 0 Offset 0
               OpMemberDecorate %ssbo 1 Offset 12
               OpDecorate %ssbo Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
       %ssbo = OpTypeStruct %v3float %v2float
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, SubgroupRotate) {
    TEST_DESCRIPTION("Missing shaderSubgroupRotate");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    char const *source = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_rotate: enable
        layout(binding = 0) buffer Buffers { vec4  x; } data;
        void main() {
            data.x = subgroupRotate(data.x, 1);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, SubgroupRotateClustered) {
    TEST_DESCRIPTION("Missing shaderSubgroupRotateClustered");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderSubgroupRotate);

    RETURN_IF_SKIP(Init());

    char const *source = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_rotate: enable
        layout(binding = 0) buffer Buffers { vec4  x; } data;
        void main() {
            data.x = subgroupClusteredRotate(data.x, 1, 1);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSubgroupRotateClustered-09566");
    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DeviceMemoryScope) {
    TEST_DESCRIPTION("Validate using Device memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);

    RETURN_IF_SKIP(Init());

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
       }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-vulkanMemoryModel-06265");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, QueueFamilyMemoryScope) {
    TEST_DESCRIPTION("Validate using QueueFamily memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(Init());

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeQueueFamily, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
       }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-vulkanMemoryModel-06266");
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DeviceMemoryScopeDebugInfo) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    char const *csSource = R"(
               OpCapability Shader
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "// OpModuleProcessed client vulkan100
// OpModuleProcessed target-env vulkan1.0
// OpModuleProcessed entry-point main
#line 1
#version 450
#extension GL_KHR_memory_scope_semantics : enable
layout(set = 0, binding = 0) buffer ssbo { uint y; };
void main() {
    atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpName %main "main"
               OpName %ssbo "ssbo"
               OpMemberName %ssbo 0 "y"
               OpName %_ ""
               OpDecorate %ssbo BufferBlock
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %ssbo = OpTypeStruct %uint
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
     %uint_1 = OpConstant %uint 1
      %int_1 = OpConstant %int 1
     %int_64 = OpConstant %int 64
     %uint_0 = OpConstant %uint 0
    %uint_64 = OpConstant %uint 64
               OpLine %1 4 19
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 5 0
         %14 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
               OpAtomicStore %14 %int_1 %uint_64 %uint_1
               OpLine %1 6 0
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-vulkanMemoryModel-06265
    m_errorMonitor->SetDesiredError("5:     atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ConservativeRasterizationPostDepthCoverage) {
    TEST_DESCRIPTION("Make sure conservativeRasterizationPostDepthCoverage is set if needed.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(conservative_rasterization_props);
    if (conservative_rasterization_props.conservativeRasterizationPostDepthCoverage) {
        GTEST_SKIP() << "need conservativeRasterizationPostDepthCoverage to not be supported";
    }
    InitRenderTarget();

    std::string const source{R"(
               OpCapability Shader
               OpCapability SampleMaskPostDepthCoverage
               OpCapability FragmentFullyCoveredEXT
               OpExtension "SPV_EXT_fragment_fully_covered"
               OpExtension "SPV_KHR_post_depth_coverage"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %12
               OpExecutionMode %4 OriginUpperLeft
               OpExecutionMode %4 EarlyFragmentTests
               OpExecutionMode %4 PostDepthCoverage
               OpDecorate %12 BuiltIn FullyCoveredEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
         %12 = OpVariable %_ptr_Input_bool Input
          %4 = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)"};

    m_errorMonitor->SetDesiredError("VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235");
    VkShaderObj::CreateFromASM(this, source.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DynamicUniformIndex) {
    TEST_DESCRIPTION("Check for the array dynamic array index features when the SPIR-V capabilities are requested.");

    VkPhysicalDeviceFeatures features{};
    features.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
    RETURN_IF_SKIP(Init(&features));

    InitRenderTarget();

    std::string const source{R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)"};

    {
        std::string const capability{"OpCapability UniformBufferArrayDynamicIndexing"};
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        std::string const capability{"OpCapability SampledImageArrayDynamicIndexing"};
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        std::string const capability{"OpCapability StorageBufferArrayDynamicIndexing"};
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        std::string const capability{"OpCapability StorageImageArrayDynamicIndexing"};
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }
}

// TODO - This logic is illegal, but should be done in SPIRV-Tools
// see https://github.com/KhronosGroup/SPIRV-Tools/pull/4748
TEST_F(NegativeShaderSpirv, DISABLED_ImageFormatTypeMismatchWithZeroExtend) {
    TEST_DESCRIPTION("Use ZeroExtend to turn a SINT resource into a UINT, but only for some of the accesses.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    const char *csSource = R"(
                     OpCapability Shader
                     OpCapability StorageImageExtendedFormats
                     OpMemoryModel Logical GLSL450
                     OpEntryPoint GLCompute %main "main" %image_ptr
                     OpExecutionMode %main LocalSize 1 1 1
                     OpSource GLSL 450
                     OpDecorate %image_ptr DescriptorSet 0
                     OpDecorate %image_ptr Binding 0
                     OpDecorate %image_ptr NonReadable
%type_void         = OpTypeVoid
%type_u32          = OpTypeInt 32 0
%type_i32          = OpTypeInt 32 1
%type_vec3_u32     = OpTypeVector %type_u32 3
%type_vec4_u32     = OpTypeVector %type_u32 4
%type_vec2_i32     = OpTypeVector %type_i32 2
%type_fn_void      = OpTypeFunction %type_void
%type_ptr_fn       = OpTypePointer Function %type_vec4_u32
%type_image        = OpTypeImage %type_u32 2D 0 0 0 2 Rgba32ui
%type_ptr_image    = OpTypePointer UniformConstant %type_image
%image_ptr         = OpVariable %type_ptr_image UniformConstant
%const_i32_0       = OpConstant %type_i32 0
%const_vec2_i32_00 = OpConstantComposite %type_vec2_i32 %const_i32_0 %const_i32_0
%main              = OpFunction %type_void None %type_fn_void
%label             = OpLabel
%store_location    = OpVariable %type_ptr_fn Function
%image             = OpLoad %type_image %image_ptr
                   ; Is accessing a SINT, no error
%value             = OpImageRead %type_vec4_u32 %image %const_vec2_i32_00 SignExtend
                   ; But this will still be accessing a UINT, so errors
%value2            = OpImageRead %type_vec4_u32 %image %const_vec2_i32_00
                     OpStore %store_location %value
                     OpReturn
                     OpFunctionEnd
              )";

    m_errorMonitor->SetDesiredError("VUID-StandaloneSpirv-Image-04965");
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

// TODO - https://github.com/KhronosGroup/SPIRV-Tools/issues/5468
TEST_F(NegativeShaderSpirv, DISABLED_SpecConstantTextureIndex) {
    TEST_DESCRIPTION("Apply spec constant to lower array size and detect array being oob");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fragment_source = R"glsl(
        #version 400
        #extension GL_ARB_separate_shader_objects : enable
        #extension GL_ARB_shading_language_420pack : enable

        layout (location = 0) out vec4 out_color;

        layout (constant_id = 0) const int num_textures = 3;
        layout (binding = 0) uniform sampler2D textures[num_textures];

        void main() {
            out_color = texture(textures[2], vec2(0.0));
        }
    )glsl";

    uint32_t data = 2;  // will make textures[2] OOB
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr}};
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, SpecConstantArraySize) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/SPIRV-Tools/issues/5921");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fragment_source = R"glsl(
        #version 450
        layout (constant_id = 0) const int array_size = 4;
        layout(set = 0, binding = 0, std430) buffer foo {
            uint a[array_size];
            uint b; // offset 16 to start
        };

        void main() {
            b = 0;
        }
    )glsl";

    uint32_t new_array_size = 6;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &new_array_size};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr}};
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DescriptorCountConstant) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler2D tex[3];
        layout (location = 0) out vec4 out_color;
        void main() {
            out_color = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07991");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

// This is not working because of a bug in the Spec Constant logic
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5911
TEST_F(NegativeShaderSpirv, DISABLED_DescriptorCountSpecConstant) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const int index = 2;
        layout (set = 0, binding = 0) uniform sampler2D tex[index];
        layout (location = 0) out vec4 out_color;
        void main() {
            out_color = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
        }
    )glsl";

    uint32_t data = 4;  // over VkDescriptorSetLayoutBinding::descriptorCount
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07991");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DescriptorCountConstantRuntimeArray) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/issues/4111");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        layout (set = 0, binding = 0) uniform sampler2D tex[];
        layout (set = 0, binding = 1) uniform UBO { uint index; };
        layout (location = 0) out vec4 out_color;
        void main() {
            out_color = textureLodOffset(tex[index], vec2(0), 0, ivec2(0));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07991");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, InvalidExtension) {
    TEST_DESCRIPTION("Use an invalid SPIR-V extension in OpExtension.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    const char *vertex_source = R"spirv(
               OpCapability Shader
               OpExtension "GL_EXT_scalar_block_layout"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main"
               OpSource GLSL 450
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )spirv";
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08741");
    VkShaderObj::CreateFromASM(this, vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, FPFastMathDefault) {
    TEST_DESCRIPTION("FPFastMathDefault missing required modes");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloatControls2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);
    if (shader_float_control.shaderSignedZeroInfNanPreserveFloat32) {
        GTEST_SKIP() << "shaderSignedZeroInfNanPreserveFloat32 is supported";
    }

    // Missing NotNaN
    const char *spv_source = R"(
        OpCapability Shader
        OpCapability FloatControls2
        OpExtension "SPV_KHR_float_controls2"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main"
        OpExecutionModeId %main FPFastMathDefault %float %constant
        OpExecutionMode %main LocalSize 1 1 1
        OpDecorate %add FPFastMathMode NSZ|NotInf|NotNaN
        %void = OpTypeVoid
        %int = OpTypeInt 32 0
        %constant = OpConstant %int 6
        %float = OpTypeFloat 32
        %zero = OpConstant %float 0
        %void_fn = OpTypeFunction %void
        %main = OpFunction %void None %void_fn
        %entry = OpLabel
        OpReturn
        OpFunctionEnd
        %func = OpFunction %void None %void_fn
        %func_entry = OpLabel
        %add = OpFAdd %float %zero %zero
        OpReturn
        OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-09561");
    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, FPFastMathMode) {
    TEST_DESCRIPTION("FPFastMathMode missing required modes");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloatControls2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);
    if (shader_float_control.shaderSignedZeroInfNanPreserveFloat32) {
        GTEST_SKIP() << "shaderSignedZeroInfNanPreserveFloat32 is supported";
    }

    // Missing NotNaN
    const char *spv_source = R"(
        OpCapability Shader
        OpCapability FloatControls2
        OpExtension "SPV_KHR_float_controls2"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main"
        OpExecutionModeId %main FPFastMathDefault %float %constant
        OpExecutionMode %main LocalSize 1 1 1
        OpDecorate %add FPFastMathMode NSZ|NotInf
        %void = OpTypeVoid
        %int = OpTypeInt 32 0
        %constant = OpConstant %int 7
        %float = OpTypeFloat 32
        %zero = OpConstant %float 0
        %void_fn = OpTypeFunction %void
        %main = OpFunction %void None %void_fn
        %entry = OpLabel
        OpReturn
        OpFunctionEnd
        %func = OpFunction %void None %void_fn
        %func_entry = OpLabel
        %add = OpFAdd %float %zero %zero
        OpReturn
        OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-09562");
    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ScalarBlockLayoutShaderCache) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8031");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    // will NOT set --scalar-block-layout
    RETURN_IF_SKIP(Init());

    // Matches glsl from other ScalarBlockLayoutShaderCache test
    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_buffer_reference : require
        #extension GL_EXT_scalar_block_layout : require

        struct Transform {
            mat3x3 rotScaMatrix; //  0, 36
            vec3 pos;            // 36, 12
            vec3 pos_err;        // 48, 12
            float padding;       // 60, 4
        };

        layout(scalar, buffer_reference, buffer_reference_align = 64) readonly buffer Transforms {
            Transform transforms[];
        };
        layout(std430, push_constant) uniform PushConstant {
            Transforms pTransforms;
        };

        void main() {
            Transform transform = pTransforms.transforms[0];
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj cs(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ImageGatherOffsetMaintenance8) {
    AddRequiredFeature(vkt::Feature::shaderImageGatherExtended);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *spv_source = R"(
               OpCapability Shader
               OpCapability ImageGatherExtended
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 2
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
        %tex = OpVariable %_ptr_UniformConstant_9 UniformConstant
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %15 = OpConstantComposite %v2float %float_0 %float_0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %19 = OpConstantComposite %v2int %int_0 %int_0
    %v4float = OpTypeVector %float 4
       %main = OpFunction %void None %4
          %6 = OpLabel
         %12 = OpLoad %9 %tex
         %21 = OpImageSampleExplicitLod %v4float %12 %15 Lod|Offset %float_0 %19
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Offset-10213");
    VkShaderObj const fs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, VkShaderModuleCreateInfoPNext) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkPhysicalDeviceFeatures2 pd_features2 = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkCreateShaderModule-pCreateInfo-06904");
    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                   &pd_features2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NullShaderModuleCreateInfo) {
    RETURN_IF_SKIP(Init());
    VkShaderModule module;
    m_errorMonitor->SetDesiredError("VUID-vkCreateShaderModule-pCreateInfo-parameter");
    vk::CreateShaderModule(device(), nullptr, nullptr, &module);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderTileImageDisabled) {
    TEST_DESCRIPTION("Validate creating graphics pipeline without shader tile image features enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    {
        // shaderTileImageDepthReadAccess
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, kShaderTileImageDepthReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        // shaderTileImageStencilReadAccess
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, kShaderTileImageStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        // shaderTileImageColorReadAccess
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        VkShaderObj::CreateFromASM(this, kShaderTileImageColorReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_errorMonitor->VerifyFound();
    }
}