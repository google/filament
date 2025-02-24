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
#include "../framework/descriptor_helper.h"

class NegativeShaderStorageImage : public VkLayerTest {};

TEST_F(NegativeShaderStorageImage, MissingFormatRead) {
    TEST_DESCRIPTION("Create a shader reading a storage image without an image format");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
    // disabled if VK_KHR_format_feature_flags2 is supported.
    //
    //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
    //
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
               OpCapability Shader
               OpCapability StorageImageReadWithoutFormat
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
               OpDecorate %12 NonReadable
               OpDecorate %12 NonWritable
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %13 = OpLoad %10 %12
         %18 = OpImageRead %7 %13 %17
               OpStore %9 %18
               OpReturn
               OpFunctionEnd
              )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderStorageImage, MissingFormatWrite) {
    TEST_DESCRIPTION("Create a shader writing a storage image without an image format");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
    // disabled if VK_KHR_format_feature_flags2 is supported.
    //
    //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
    //
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main" %img
                  OpExecutionMode %main LocalSize 1 1 1
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
                  ; incase shaderStorageImageReadWithoutFormat is not supported
                  OpDecorate %img NonReadable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderStorageImage, MissingFormatReadForFormat) {
    TEST_DESCRIPTION("Create a shader reading a storage image without an image format");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    struct {
        VkFormat format;
        VkFormatProperties3KHR props;
    } tests[2] = {};
    int n_tests = 0;
    bool has_without_format_test = false, has_with_format_test = false;

    // Find storage formats with & without read without format support
    for (uint32_t fmt = VK_FORMAT_R4G4_UNORM_PACK8; fmt < VK_FORMAT_D16_UNORM; fmt++) {
        if (has_without_format_test && has_with_format_test) break;
        if (!vkuFormatIsSampledFloat((VkFormat)fmt)) continue;

        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(Gpu(), (VkFormat)fmt, &fmt_props);

        const bool has_storage = (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT) != 0;
        const bool has_read_without_format =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR) != 0;

        if (!has_storage) continue;

        if (has_read_without_format) {
            if (has_without_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_without_format_test = true;
            n_tests++;
        } else {
            if (has_with_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_with_format_test = true;
            n_tests++;
        }
    }

    if (n_tests == 0) {
        GTEST_SKIP() << "Could not build a test case.";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
               OpCapability Shader
               OpCapability StorageImageReadWithoutFormat
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
               OpDecorate %12 NonReadable
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %13 = OpLoad %10 %12
         %18 = OpImageRead %7 %13 %17
               OpStore %9 %18
               OpReturn
               OpFunctionEnd
              )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(false);                      // need false to prevent late binding

    for (int t = 0; t < n_tests; t++) {
        VkFormat format = tests[t].format;

        vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
        image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        vkt::ImageView view = image.CreateView();

        ds.Clear();
        ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
        ds.UpdateDescriptorSets();

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        {
            VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
            img_barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
            img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            img_barrier.image = image.handle();  // Image mis-matches with FB image
            img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            img_barrier.subresourceRange.baseArrayLayer = 0;
            img_barrier.subresourceRange.baseMipLevel = 0;
            img_barrier.subresourceRange.layerCount = 1;
            img_barrier.subresourceRange.levelCount = 1;
            vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                   0, nullptr, 0, nullptr, 1, &img_barrier);
        }

        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(),
                                  0, 1, &ds.set_, 0, nullptr);

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-OpTypeImage-07028");
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        if ((tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR) == 0) {
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeShaderStorageImage, MissingFormatWriteForFormat) {
    TEST_DESCRIPTION("Create a shader writing a storage image without an image format");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    struct {
        VkFormat format;
        VkFormatProperties3KHR props;
    } tests[2] = {};
    int n_tests = 0;
    bool has_without_format_test = false, has_with_format_test = false;

    // Find storage formats with & without write without format support
    for (uint32_t fmt = VK_FORMAT_R4G4_UNORM_PACK8; fmt < VK_FORMAT_D16_UNORM; fmt++) {
        if (has_without_format_test && has_with_format_test) break;
        if (!vkuFormatIsSampledFloat((VkFormat)fmt)) continue;

        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(Gpu(), (VkFormat)fmt, &fmt_props);

        const bool has_storage = (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT) != 0;
        const bool has_write_without_format =
            (fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) != 0;

        if (!has_storage) continue;

        if (has_write_without_format) {
            if (has_without_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_without_format_test = true;
            n_tests++;
        } else {
            if (has_with_format_test) continue;

            tests[n_tests].format = (VkFormat)fmt;
            tests[n_tests].props = fmt_props_3;
            has_with_format_test = true;
            n_tests++;
        }
    }

    if (n_tests == 0) {
        GTEST_SKIP() << "Could not build a test case.";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpName %main "main"
                  OpName %img "img"
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(false);                      // need false to prevent late binding

    for (int t = 0; t < n_tests; t++) {
        VkFormat format = tests[t].format;

        vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
        image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        vkt::ImageView view = image.CreateView();

        ds.Clear();
        ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
        ds.UpdateDescriptorSets();

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        {
            VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
            img_barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
            img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            img_barrier.image = image.handle();  // Image mis-matches with FB image
            img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            img_barrier.subresourceRange.baseArrayLayer = 0;
            img_barrier.subresourceRange.baseMipLevel = 0;
            img_barrier.subresourceRange.layerCount = 1;
            img_barrier.subresourceRange.levelCount = 1;
            vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                   0, nullptr, 0, nullptr, 1, &img_barrier);
        }

        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(),
                                  0, 1, &ds.set_, 0, nullptr);

        if ((tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) == 0) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-OpTypeImage-07027");
        }
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        if ((tests[t].props.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) == 0) {
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeShaderStorageImage, MissingNonReadableDecorationFormatRead) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format not marked as non readable");

    // We need to skip this test with VK_KHR_format_feature_flags2 supported,
    // because checks for read/write without format has to be done per format
    // rather than as a device feature. The code we test here only looks at
    // the shader.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
               OpDecorate %12 NonWritable
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %l = OpLabel
          %9 = OpVariable %8 Function
               OpReturn
               OpFunctionEnd
              )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-apiVersion-07955");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderStorageImage, MissingNonWritableDecorationFormatWrite) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format but not marked a non writable");

    // We need to skip this test with VK_KHR_format_feature_flags2 supported,
    // because checks for read/write without format has to be done per format
    // rather than as a device feature. The code we test here only looks at
    // the shader.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main" %img
                  OpExecutionMode %main LocalSize 1 1 1
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonReadable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %l = OpLabel
                  OpReturn
                  OpFunctionEnd
                  )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-apiVersion-07954");
    VkShaderObj const cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderStorageImage, WriteLessComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(NegativeShaderStorageImage, WriteLessComponentCopyObject) {
    TEST_DESCRIPTION("Test writing to image with less components, but use OpCopyObject.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
   %var_copy = OpCopyObject %ptr %var
  %var_copy2 = OpCopyObject %ptr %var_copy
       %load = OpLoad %image %var_copy
  %load_copy = OpCopyObject %image %load
 %load_copy2 = OpCopyObject %image %load_copy
               OpImageWrite %load_copy2 %coord %texelU3
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(NegativeShaderStorageImage, WriteSpecConstantLessComponent) {
    TEST_DESCRIPTION("Test writing to image with less components with Texel being a spec constant.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout (constant_id = 0) const uint sc = 1;
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, sc, sc + 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
         %sc = OpSpecConstant %uint 1
      %sc_p1 = OpSpecConstantOp %uint IAdd %sc %uint_1
    %texelU3 = OpSpecConstantComposite %v3uint %uint_1 %sc %sc_p1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    uint32_t data = 2;
    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);
    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM,
                                                   &specialization_info);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(NegativeShaderStorageImage, UnknownWriteLessComponent) {
    TEST_DESCRIPTION("Test writing to image unknown format with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderStorageImageWriteWithoutFormat);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) readonly uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Unknown will become a 4-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
               OpDecorate %var NonReadable
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), format, &fmt_props);
    if ((fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) == 0) {
        GTEST_SKIP() << "Format doesn't support storage write without format";
    }

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    ds.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-OpImageWrite-08795");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeShaderStorageImage, UnknownWriteComponentA8Unorm) {
    TEST_DESCRIPTION("Test writing to image unknown format with VK_FORMAT_A8_UNORM.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderStorageImageWriteWithoutFormat);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) readonly uniform image2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), vec3(1, 1, 1));
    //
    // only have 3 components
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
               OpDecorate %var NonReadable
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
      %image = OpTypeImage %float 2D 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3float = OpTypeVector %float 3
     %float_1 = OpConstant %float 1
    %texelU3 = OpConstantComposite %v3float %float_1 %float_1 %float_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3
               OpReturn
               OpFunctionEnd
        )";
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_A8_UNORM;
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), format, &fmt_props);
    if ((fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) == 0) {
        GTEST_SKIP() << "Format doesn't support storage write without format";
    }

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    ds.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-OpImageWrite-08796");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
