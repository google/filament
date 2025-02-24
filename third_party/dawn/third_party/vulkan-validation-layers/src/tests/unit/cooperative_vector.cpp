/*
 * Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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

class NegativeCooperativeVector : public VkLayerTest {
  public:
    void SetupConvertCooperativeVectorMatrixNVTest();
    void SetupCmdConvertCooperativeVectorMatrixNVTest();
    void RunSPIRVTest(const char *expected_error, const char *shaderBody, uint32_t count = 1);

    VkConvertCooperativeVectorMatrixInfoNV info;
    size_t dstSize;
    uint8_t src[16 * 32 * 4], dst[16 * 32 * 4];
    vkt::Buffer src_buffer, dst_buffer;
};

void NegativeCooperativeVector::SetupConvertCooperativeVectorMatrixNVTest() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_REPLICATED_COMPOSITES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    AddRequiredFeature(vkt::Feature::shaderReplicatedComposites);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    AddRequiredFeature(vkt::Feature::cooperativeVectorTraining);
    RETURN_IF_SKIP(InitState());

    // initialize info to reasonable values
    info = vku::InitStructHelper();
    dstSize = 16 * 32 * 2;
    info.srcSize = 16 * 32 * 2;
    info.srcData.hostAddress = nullptr;
    info.pDstSize = &dstSize;
    info.dstData.hostAddress = nullptr;
    info.srcComponentType = VK_COMPONENT_TYPE_FLOAT16_KHR;
    info.dstComponentType = VK_COMPONENT_TYPE_FLOAT16_KHR;
    info.numRows = 16;
    info.numColumns = 32;
    info.srcLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.srcStride = 64;
    info.dstLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.dstStride = 64;

    info.srcData.hostAddress = src;
    info.dstData.hostAddress = dst;
}

void NegativeCooperativeVector::SetupCmdConvertCooperativeVectorMatrixNVTest() {
    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    src_buffer = vkt::Buffer(*m_device, 16 * 32 * 4, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);
    dst_buffer = vkt::Buffer(*m_device, 16 * 32 * 4, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);
    info.srcData.deviceAddress = src_buffer.Address();
    info.dstData.deviceAddress = dst_buffer.Address();

    m_command_buffer.Begin();
}

TEST_F(NegativeCooperativeVector, HostConvertDstAddressNull) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - dst address should also be null");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcData.hostAddress = nullptr;
    m_errorMonitor->SetDesiredError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10073");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertSrcTooSmall) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - src size is too small");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcSize--;
    m_errorMonitor->SetDesiredError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10074");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertDstTooSmall) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - dst size is too small");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    dstSize--;
    m_errorMonitor->SetDesiredError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10075");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertOverlap) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - detect overlapping ranges");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.dstData.hostAddress = &src[info.srcSize - 1];
    m_errorMonitor->SetDesiredError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10076");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertSrcStrideTooSmall) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - src stride too small");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcStride = 62;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcLayout-10077");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertSrcStrideUnaligned) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - src stride unaligned");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcStride = 65;
    info.srcSize = 65 * 16;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcLayout-10077");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertDstStrideTooSmall) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - dst stride too small");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.dstStride = 62;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstLayout-10078");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertDstStrideUnaligned) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - dst stride unaligned");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.dstStride = 65;
    dstSize = 65 * 16;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstLayout-10078");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertSrcComponentUnsupported) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - src component unsupported");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcComponentType = VK_COMPONENT_TYPE_SINT32_KHR;
    info.srcStride = 32 * 4;
    info.srcSize = 16 * 32 * 4;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcComponentType-10081");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertUnsupportedConversion) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - unsupported conversion");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcComponentType = VK_COMPONENT_TYPE_FLOAT_E4M3_NV;
    info.dstComponentType = VK_COMPONENT_TYPE_FLOAT_E5M2_NV;
    info.srcLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV;
    info.dstLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcComponentType-10081");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertUnsupportedFP8Layout) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - unsupported fp8 layout");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.dstComponentType = VK_COMPONENT_TYPE_FLOAT_E4M3_NV;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstComponentType-10082");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, HostConvertUnsupportedMatrixType) {
    TEST_DESCRIPTION("vkConvertCooperativeVectorMatrixNV - unsupported matrix type");

    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    info.srcComponentType = VK_COMPONENT_TYPE_SINT32_KHR;
    info.srcStride = 32 * 4;
    info.srcSize = 16 * 32 * 4;
    info.dstComponentType = VK_COMPONENT_TYPE_SINT32_KHR;
    info.dstStride = 32 * 4;
    dstSize = 16 * 32 * 4;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcComponentType-10079");
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstComponentType-10080");
    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertUnalignedSrcAddress) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - unaligned src address");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    info.srcData.deviceAddress++;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10084");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertUnalignedDstAddress) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - unaligned dst address");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    info.dstData.deviceAddress++;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10085");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertSrcTooSmall) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - src too small");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    info.srcSize--;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10086");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertDstTooSmall) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - dst too small");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    dstSize--;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10087");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertOverlap) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - overlapping ranges");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    VkConvertCooperativeVectorMatrixInfoNV infos[2] = {info, info};
    infos[1].srcData.deviceAddress += 64;
    infos[1].dstData.deviceAddress = infos[0].srcData.deviceAddress + infos[1].srcSize;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-None-10088");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 2, infos);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertSrcAddressInvalid) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - src not from a buffer");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    info.srcData.deviceAddress = 64;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10083");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertDstAddressInvalid) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - dst not from a buffer");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    info.dstData.deviceAddress = 64;
    m_errorMonitor->SetDesiredError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10083");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, DeviceConvertUnsupportedMatrixType) {
    TEST_DESCRIPTION("vkCmdConvertCooperativeVectorMatrixNV - unsupported matrix type");

    RETURN_IF_SKIP(SetupCmdConvertCooperativeVectorMatrixNVTest());

    info.srcComponentType = VK_COMPONENT_TYPE_SINT32_KHR;
    info.srcStride = 32 * 4;
    info.srcSize = 16 * 32 * 4;
    info.dstComponentType = VK_COMPONENT_TYPE_SINT32_KHR;
    info.dstStride = 32 * 4;
    dstSize = 16 * 32 * 4;
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcComponentType-10079");
    m_errorMonitor->SetDesiredError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstComponentType-10080");
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_errorMonitor->VerifyFound();
}

void NegativeCooperativeVector::RunSPIRVTest(const char *expected_error, const char *shaderBody, uint32_t count) {
    RETURN_IF_SKIP(SetupConvertCooperativeVectorMatrixNVTest());

    std::string shader_source = std::string(R"(
        #version 450
        #extension GL_NV_cooperative_vector : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(set=0, binding=0) buffer Buf { uint8_t x[]; } b;
        shared uint8_t sh[128];
        void main() {
        )") + std::string(shaderBody) +
                                std::string(R"(
       }
    )");

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError(expected_error, count);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCooperativeVector, SPIRVTooManyComponents) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-maxCooperativeVectorComponents-10094",
                 R"(
                coopvecNV<float16_t, 9999> A = coopvecNV<float16_t, 9999>(0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVUnsupportedComponentType) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpTypeCooperativeVector-10095",
                 R"(
                coopvecNV<float64_t, 16> A = coopvecNV<float64_t, 16>(0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVMatMulAddParams) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10089",
                 R"(
            coopvecNV<float16_t, 16> A;
            coopvecNV<float16_t, 32> R;
            coopVecMatMulAddNV(R, A, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat64NV, b.x, 0, gl_ComponentTypeFloat16NV, 32, 16, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVMatMulParams) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10089",
                 R"(
            coopvecNV<float16_t, 16> A;
            coopvecNV<float16_t, 32> R;
            coopVecMatMulNV(R, A, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat64NV, 32, 16, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVFP8Layout) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10090",
                 R"(
            coopvecNV<float16_t, 16> A;
            coopvecNV<float16_t, 32> R;
            coopVecMatMulAddNV(R, A, gl_ComponentTypeFloatE4M3NV, b.x, 0, gl_ComponentTypeFloatE4M3NV, b.x, 0, gl_ComponentTypeFloat16NV, 32, 16, gl_CooperativeVectorMatrixLayoutRowMajorNV, false, 0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVReduceSumType) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                 R"(
            coopvecNV<int32_t, 16> A;
            coopVecReduceSumAccumulateNV(A, b.x, 0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVReduceSumStorageClass) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                 R"(
            coopvecNV<float16_t, 16> A;
            coopVecReduceSumAccumulateNV(A, sh, 0);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVOuterProductType) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                 R"(
            coopvecNV<float32_t, 16> A;
            coopvecNV<float32_t, 16> B;
            coopVecOuterProductAccumulateNV(A, B, b.x, 0, 0, gl_CooperativeVectorMatrixLayoutTrainingOptimalNV, gl_ComponentTypeFloat16NV);
        )",
                 2);
}

TEST_F(NegativeCooperativeVector, SPIRVOuterProductInterpretation) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                 R"(
            coopvecNV<float16_t, 16> A;
            coopvecNV<float16_t, 16> B;
            coopVecOuterProductAccumulateNV(A, B, b.x, 0, 0, gl_CooperativeVectorMatrixLayoutTrainingOptimalNV, gl_ComponentTypeFloat64NV);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVOuterProductLayout) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                 R"(
                coopvecNV<float16_t, 16> A;
                coopVecOuterProductAccumulateNV(A, A, b.x, 0, 0, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, gl_ComponentTypeFloat16NV);
        )");
}

TEST_F(NegativeCooperativeVector, SPIRVOuterProductStorageClass) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    RunSPIRVTest("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                 R"(
                coopvecNV<float16_t, 16> A;
                coopVecOuterProductAccumulateNV(A, A, sh, 0, 0, gl_CooperativeVectorMatrixLayoutTrainingOptimalNV, gl_ComponentTypeFloat16NV);
        )");
}
