/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 * Copyright (c) 2024 Google, Inc.
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
#include "../framework/gpu_av_helper.h"
#include "error_message/log_message_type.h"

class NegativeGpuAVDebugPrintf : public virtual VkLayerTest {
  public:
    void InitGpuAvDebugPrintfFramework(void *p_next = nullptr);
    void InitWithLayerSettings(bool enable_printf, bool enable_gpuav, bool shader_instrumentation);

    VkValidationFeaturesEXT GetGpuAvDebugPrintfValidationFeatures();

    void BasicComputeTest();
};

static const std::array gpu_av_enables = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                          VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
                                          VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};

VkValidationFeaturesEXT NegativeGpuAVDebugPrintf::GetGpuAvDebugPrintfValidationFeatures() {
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = size32(gpu_av_enables);
    features.pEnabledValidationFeatures = gpu_av_enables.data();
    return features;
}

void NegativeGpuAVDebugPrintf::InitGpuAvDebugPrintfFramework(void *p_next) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    VkValidationFeaturesEXT validation_features = GetGpuAvDebugPrintfValidationFeatures();
    validation_features.pNext = p_next;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV/Printf are not met";
    }
}

void NegativeGpuAVDebugPrintf::InitWithLayerSettings(bool enable_printf, bool enable_gpuav, bool shader_instrumentation) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    VkBool32 printf_value = enable_printf ? VK_TRUE : VK_FALSE;
    VkLayerSettingEXT printf_setting = {OBJECT_LAYER_NAME, "printf_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &printf_value};

    VkBool32 gpuav_value = enable_gpuav ? VK_TRUE : VK_FALSE;
    VkLayerSettingEXT gpuav_setting = {OBJECT_LAYER_NAME, "gpuav_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &gpuav_value};

    VkBool32 shader_instrumentation_value = shader_instrumentation ? VK_TRUE : VK_FALSE;
    VkLayerSettingEXT shader_instrumentation_setting = {OBJECT_LAYER_NAME, "gpuav_shader_instrumentation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &shader_instrumentation_value};

    std::array<VkLayerSettingEXT, 3> layer_settings = {printf_setting, gpuav_setting, shader_instrumentation_setting};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
    layer_settings_create_info.settingCount = static_cast<uint32_t>(layer_settings.size());
    layer_settings_create_info.pSettings = layer_settings.data();
    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV/Printf are not met";
    }
}

void NegativeGpuAVDebugPrintf::BasicComputeTest() {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer foo_0 {
            uint a;
            uint b[];
        };
        void main() {
            debugPrintfEXT("b.length == %u", b.length());
            a = b[32]; // OOB
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDebugPrintf, Basic) {
    TEST_DESCRIPTION("Both trip a GPU-AV error while calling into DebugPrintf");

    RETURN_IF_SKIP(InitGpuAvDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_errorMonitor->SetDesiredInfo("b.length == 3");
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, BasicLayerSettings) {
    RETURN_IF_SKIP(InitWithLayerSettings(true, true, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_errorMonitor->SetDesiredInfo("b.length == 3");
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, BasicLayerSettingsNoPrintf) {
    RETURN_IF_SKIP(InitWithLayerSettings(false, true, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, BasicLayerSettingsNoGpuAV) {
    AddRequiredFeature(vkt::Feature::robustBufferAccess); // prevent crashing
    RETURN_IF_SKIP(InitWithLayerSettings(true, false, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredInfo("b.length == 3");
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, BasicLayerSettingsNoShaderInstrumentation) {
    AddRequiredFeature(vkt::Feature::robustBufferAccess); // prevent crashing
    RETURN_IF_SKIP(InitWithLayerSettings(true, true, false));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredInfo("b.length == 3");
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, BasicLayerSettingsNoPrintfOrGpuAV) {
    AddRequiredFeature(vkt::Feature::robustBufferAccess); // prevent crashing
    RETURN_IF_SKIP(InitWithLayerSettings(false, false, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, BasicLayerSettingsPrintfPreset) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::robustBufferAccess); // prevent crashing
    VkBool32 value = VK_TRUE;
    VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "printf_only_preset", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
    layer_settings_create_info.settingCount = 1;
    layer_settings_create_info.pSettings = &setting;
    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV/Printf are not met";
    }
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredInfo("b.length == 3");
    BasicComputeTest();
}

TEST_F(NegativeGpuAVDebugPrintf, Graphics) {
    TEST_DESCRIPTION("Make sure graphics flow works");

    RETURN_IF_SKIP(InitGpuAvDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer foo_0 {
            uint a;
            uint b[];
        };
        void main() {
            debugPrintfEXT("b.length == %u", b.length());
            a = b[32]; // OOB
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_errorMonitor->SetDesiredInfo("b.length == 3", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDebugPrintf, GPL) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer foo_0 {
            uint a;
            uint b[];
        };
        void main() {
            debugPrintfEXT("b.length == %u", b.length());
            a = b[32]; // OOB
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_errorMonitor->SetDesiredInfo("b.length == 3", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDebugPrintf, ShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);

    RETURN_IF_SKIP(InitGpuAvDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer foo_0 {
            uint a;
            uint b[];
        };
        void main() {
            debugPrintfEXT("b.length == %u", b.length());
            a = b[32]; // OOB
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkDescriptorSetLayout dsl_handle = descriptor_set_0.layout_.handle();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();

    const vkt::Shader cs(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source),
                         &dsl_handle);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, stages, &cs.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08613");
    m_errorMonitor->SetDesiredInfo("b.length == 3");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDebugPrintf, DynamicRendering) {
    TEST_DESCRIPTION("Make sure graphics flow works");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitGpuAvDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer foo_0 {
            uint a;
            uint b[];
        };
        void main() {
            debugPrintfEXT("b.length == %u", b.length());
            a = b[32]; // OOB
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set_0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_0.UpdateDescriptorSets();

    VkFormat color_formats = {GetRenderTargetFormat()};
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_errorMonitor->SetDesiredInfo("b.length == 3", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
