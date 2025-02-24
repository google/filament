/* Copyright (c) 2021-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 LunarG, Inc.
 * Copyright (c) 2021 NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <array>
#include <cassert>
#include <vector>

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

// Common data structures needed for tests.
class ViewportInheritanceTestData {
    // Borrowed owner device.
    VkDevice m_device{};

    // Renderpass, draw to one attachment of format m_colorFormat.
    VkFormat m_colorFormat{};
    VkRenderPass m_renderPass{};

    // Framebuffer data.
    vkt::Image m_colorImageObj;
    VkImageView m_colorImageView{};
    VkFramebuffer m_framebuffer{};

    // Do-nothing vertex and fragment programs.
    static const uint32_t kVertexSpirV[166];
    static const uint32_t kFragmentSpirV[83];
    std::array<VkPipelineShaderStageCreateInfo, 2> m_shaderStages{};

    // Do-nothing graphics pipelines.
    // dynamic state pipelines have viewport/scissor as sole dynamic state; static state pipelines have no dynamic state.
    // the i-th pipeline needs i viewports/scissors (0th uses EXT dynamic viewport/scissor count).
    // m_staticStatePipelines[0] is unused.
    VkPipelineLayout m_pipelineLayout{};
    std::array<VkPipeline, 33> m_dynamicStatePipelines{}, m_staticStatePipelines{};

    // Various premade state structs for graphics pipeline.
    static const VkPipelineVertexInputStateCreateInfo kVertexInputState;
    static const VkPipelineInputAssemblyStateCreateInfo kInputAssemblyState;
    static const VkPipelineRasterizationStateCreateInfo kRasterizationState;
    static const VkPipelineMultisampleStateCreateInfo kMultisampleState;
    static const VkPipelineDepthStencilStateCreateInfo kDepthStencilState;
    static const VkPipelineColorBlendAttachmentState kBlendAttachmentState;
    static const VkPipelineColorBlendStateCreateInfo kBlendState;
    static const VkPipelineDynamicStateCreateInfo kStaticState;
    static const VkPipelineDynamicStateCreateInfo kDynamicState;
    static const VkPipelineDynamicStateCreateInfo kDynamicStateWithCount;

  public:
    // Premade viewport and scissor arrays for testing.
    static const VkViewport kViewportArray[32];
    static const VkViewport kViewportDepthOnlyArray[32];
    static const VkViewport kViewportAlternateDepthArray[32];
    static const VkRect2D kScissorArray[32];

  private:
    // Set to a failure message if initialization failed.
    const char* m_failureReason = nullptr;

    void PickColorFormat(VkPhysicalDevice physical_device) noexcept {
        std::array<VkFormat, 7> formats = {VK_FORMAT_R8G8B8A8_SRGB,      VK_FORMAT_B8G8R8A8_SRGB,  VK_FORMAT_R8G8B8A8_UNORM,
                                           VK_FORMAT_B8G8R8A8_UNORM,     VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_B8G8R8A8_SNORM,
                                           VK_FORMAT_R32G32B32A32_SFLOAT};
        for (VkFormat candidate : formats) {
            VkImageFormatProperties properties;
            VkResult result =
                vk::GetPhysicalDeviceImageFormatProperties(physical_device, candidate, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &properties);
            if (result == VK_SUCCESS) {
                m_colorFormat = candidate;
                return;
            }
        }
        m_failureReason = "No color attachment format found";
    }

    void CreateRenderPass() noexcept {
        assert(m_colorFormat != VK_FORMAT_UNDEFINED);
        VkAttachmentDescription color_attachment = {0,
                                                    m_colorFormat,
                                                    VK_SAMPLE_COUNT_1_BIT,
                                                    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                    VK_ATTACHMENT_STORE_OP_STORE,
                                                    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                    VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference color_reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass = {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &color_reference, nullptr, nullptr, 0, nullptr};
        VkRenderPassCreateInfo info = {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &color_attachment, 1, &subpass, 0, nullptr};
        VkResult result = vk::CreateRenderPass(m_device, &info, nullptr, &m_renderPass);
        if (result != VK_SUCCESS) m_failureReason = "Could not create render pass.";
    }

    void CreateColorImageObj(vkt::Device& device_obj) noexcept {
        assert(m_colorFormat != VK_FORMAT_UNDEFINED);
        m_colorImageObj.Init(device_obj, 128, 128, 1, m_colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        if (!m_colorImageObj.initialized()) {
            m_failureReason = "Image not initialized";
        }
    }

    void CreateColorView() noexcept {
        assert(!m_colorImageView);
        VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      nullptr,
                                      0,
                                      m_colorImageObj.handle(),
                                      VK_IMAGE_VIEW_TYPE_2D,
                                      m_colorFormat,
                                      {},
                                      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        VkResult result = vk::CreateImageView(m_device, &info, nullptr, &m_colorImageView);
        if (result != VK_SUCCESS) m_failureReason = "Could not create image view";
    }

    void CreateFramebuffer() noexcept {
        assert(!m_framebuffer);
        VkFramebufferCreateInfo info = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &m_colorImageView, 128, 128, 1};
        VkResult result = vk::CreateFramebuffer(m_device, &info, nullptr, &m_framebuffer);
        if (result != VK_SUCCESS) m_failureReason = "Could not create framebuffer";
    }

    void CreatePipelineLayout() noexcept {
        assert(!m_pipelineLayout);
        VkPipelineLayoutCreateInfo info = vku::InitStructHelper();
        VkResult result = vk::CreatePipelineLayout(m_device, &info, nullptr, &m_pipelineLayout);
        if (result != VK_SUCCESS) m_failureReason = "Could not create pipeline layout";
    }

    void CreateShaderStages() noexcept {
        VkShaderModuleCreateInfo vertex_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, sizeof kVertexSpirV,
                                                kVertexSpirV};
        m_shaderStages[0] = vku::InitStructHelper();
        m_shaderStages[0].flags = 0;
        m_shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        VkResult result = vk::CreateShaderModule(m_device, &vertex_info, nullptr, &m_shaderStages[0].module);
        m_shaderStages[0].pName = "main";
        m_shaderStages[0].pSpecializationInfo = nullptr;
        if (result != VK_SUCCESS) {
            m_failureReason = "Could not create vertex program";
            return;
        }

        VkShaderModuleCreateInfo fragment_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, sizeof kFragmentSpirV,
                                                  kFragmentSpirV};
        m_shaderStages[1] = vku::InitStructHelper();
        m_shaderStages[1].flags = 0;
        m_shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        result = vk::CreateShaderModule(m_device, &fragment_info, nullptr, &m_shaderStages[1].module);
        m_shaderStages[1].pName = "main";
        m_shaderStages[1].pSpecializationInfo = nullptr;
        if (result != VK_SUCCESS) {
            m_failureReason = "Could not create fragment program";
            return;
        }
    }

    void Cleanup() {
        for (VkPipeline& pipeline : m_dynamicStatePipelines) {
            vk::DestroyPipeline(m_device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        for (VkPipeline& pipeline : m_staticStatePipelines) {
            vk::DestroyPipeline(m_device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        for (auto& stage : m_shaderStages) {
            vk::DestroyShaderModule(m_device, stage.module, nullptr);
            stage.module = VK_NULL_HANDLE;
        }
        vk::DestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
        vk::DestroyFramebuffer(m_device, m_framebuffer, nullptr);
        m_framebuffer = VK_NULL_HANDLE;
        vk::DestroyImageView(m_device, m_colorImageView, nullptr);
        m_colorImageView = VK_NULL_HANDLE;
        vk::DestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

  public:
    // Check if the gpu has the needed features, and call InitState requesting the needed features.
    // Return whether the needed features were found or not.
    template <typename AddDeviceExtension>
    static bool InitState(VkRenderFramework* p_framework, AddDeviceExtension add_device_extension, const char** pp_reason,
                          bool inheritedViewportScissor2D, bool extended_dynamic_state_multi_viewport,
                          bool disable_multi_viewport = false) {
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ext = vku::InitStructHelper();
        VkPhysicalDeviceInheritedViewportScissorFeaturesNV nv =
            vku::InitStructHelper(&ext);
        VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&nv);
        VkPhysicalDevice gpu = p_framework->Gpu();

        // Enable extended dynamic state if requested.
        if (extended_dynamic_state_multi_viewport) {
            if (!p_framework->DeviceExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
                *pp_reason = "missing " VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME;
                return false;
            }
            add_device_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        }

        // Always try to enable the tested extension (but maybe won't actually enable relevant feature.)
        if (p_framework->DeviceExtensionSupported(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME)) {
            add_device_extension(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME);
        } else {
            *pp_reason = "missing " VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME;
            return false;
        }

        vk::GetPhysicalDeviceFeatures2(gpu, &features2);

        if (extended_dynamic_state_multi_viewport) {
            if (!features2.features.multiViewport) {
                *pp_reason = "missing multiViewport feature";
                return false;
            }
            if (!ext.extendedDynamicState) {
                *pp_reason = "missing extendedDynamicState feature";
                return false;
            }
        }
        if (!nv.inheritedViewportScissor2D && !inheritedViewportScissor2D) {
            *pp_reason = "missing inheritedViewportScissor2D feature";
            return false;
        }
        nv.inheritedViewportScissor2D = inheritedViewportScissor2D;

        if (disable_multi_viewport) {
            features2.features.multiViewport = VK_FALSE;
        }

        p_framework->InitState(nullptr, &features2);
        return true;
    }

    ViewportInheritanceTestData(vkt::Device* p_device_obj, VkPhysicalDevice physical_device) noexcept {
        m_device = p_device_obj->handle();

        PickColorFormat(physical_device);
        if (m_failureReason) return;
        CreateRenderPass();
        if (m_failureReason) return;
        CreateColorImageObj(*p_device_obj);
        if (m_failureReason) return;
        CreateColorView();
        if (m_failureReason) return;
        CreateFramebuffer();
        if (m_failureReason) return;
        CreatePipelineLayout();
        if (m_failureReason) return;
        CreateShaderStages();
        if (m_failureReason) return;
    }

    ~ViewportInheritanceTestData() { Cleanup(); }

    // nullptr indicates successful construction.
    const char* FailureReason() const { return m_failureReason; };

    // Get the graphics pipeline with the specified viewport/scissor state configuration, creating it if needed.
    // viewport_scissor_count == 0 and dynamic_viewport_scissor == true indicates EXT viewport/scissor with count dynamic state.
    // All pipelines are destroyed when the class is destroyed.
    VkPipeline GetGraphicsPipeline(bool dynamic_viewport_scissor, uint32_t viewport_scissor_count) {
        assert(dynamic_viewport_scissor || viewport_scissor_count != 0);
        assert(size_t(viewport_scissor_count) < m_dynamicStatePipelines.size());
        VkPipeline* p_pipeline =
            &(dynamic_viewport_scissor ? m_dynamicStatePipelines : m_staticStatePipelines)[viewport_scissor_count];
        if (*p_pipeline) {
            return *p_pipeline;
        }

        // Need some static viewport/scissors if no dynamic state. Their values don't really matter; the only purpose
        // of static viewport/scissor pipelines is to test messing up the dynamic state.
        std::vector<VkViewport> static_viewports;
        std::vector<VkRect2D>   static_scissors;
        if (!dynamic_viewport_scissor) {
            VkViewport viewport { 0, 0, 128, 128, 0, 1 };
            static_viewports = std::vector<VkViewport>(viewport_scissor_count, viewport);
            static_scissors.resize(viewport_scissor_count);
        }
        VkPipelineViewportStateCreateInfo viewport_state = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                                            nullptr,
                                                            0,
                                                            viewport_scissor_count,
                                                            static_viewports.data(),
                                                            viewport_scissor_count,
                                                            static_scissors.data()};
        const VkPipelineDynamicStateCreateInfo& dynamic_state =
            dynamic_viewport_scissor == false ? kStaticState : viewport_scissor_count == 0 ? kDynamicStateWithCount : kDynamicState;

        VkGraphicsPipelineCreateInfo info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                             nullptr,
                                             0,
                                             uint32_t(m_shaderStages.size()),
                                             m_shaderStages.data(),
                                             &kVertexInputState,
                                             &kInputAssemblyState,
                                             nullptr,  // tess
                                             &viewport_state,
                                             &kRasterizationState,
                                             &kMultisampleState,
                                             &kDepthStencilState,
                                             &kBlendState,
                                             &dynamic_state,
                                             m_pipelineLayout,
                                             m_renderPass,
                                             0};
        VkResult result = vk::CreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &info, nullptr, p_pipeline);
        if (result < 0) m_failureReason = "Failed to create graphics pipeline";
        return result >= 0 ? *p_pipeline : VK_NULL_HANDLE;
    }

    // Bind the graphics pipeline with the specified viewport/scissor state configuration.
    void BindGraphicsPipeline(VkCommandBuffer cmd, bool dynamic_viewport_scissor, uint32_t viewport_scissor_count) {
        VkPipeline pipeline = GetGraphicsPipeline(dynamic_viewport_scissor, viewport_scissor_count);
        if (pipeline) vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    // Make a primary command buffer and begin recording.
    VkCommandBuffer MakeBeginPrimaryCommandBuffer(VkCommandPool pool) const {
        VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool,
                                            VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
        VkCommandBuffer cmd;
        vk::AllocateCommandBuffers(m_device, &info, &cmd);
        BeginPrimaryCommandBuffer(cmd);
        return cmd;
    }

    // Begin recording the primary command buffer.
    VkResult BeginPrimaryCommandBuffer(VkCommandBuffer cmd) const {
        VkCommandBufferBeginInfo info = vku::InitStructHelper();
        return vk::BeginCommandBuffer(cmd, &info);
    }

    // Begin the render pass, with subpass contents provided by secondary command buffers.
    void BeginRenderPass(VkCommandBuffer cmd) const {
        VkRenderPassBeginInfo info =
            vku::InitStruct<VkRenderPassBeginInfo>(nullptr, m_renderPass, m_framebuffer, VkRect2D{{0, 0}, {128u, 128u}}, 0u, nullptr);
        vk::CmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    }

    // Make a secondary (non-subpass) command buffer and begin recording.
    VkCommandBuffer MakeBeginSecondaryCommandBuffer(VkCommandPool pool, VkFlags usage = 0,
                                                    const void* inheritance_pNext = nullptr) const {
        VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool,
                                            VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1};
        VkCommandBuffer cmd;
        vk::AllocateCommandBuffers(m_device, &info, &cmd);
        BeginSecondaryCommandBuffer(cmd, usage, inheritance_pNext);
        return cmd;
    }

    // Begin recording the (non-subpass) secondary command buffer.
    VkResult BeginSecondaryCommandBuffer(VkCommandBuffer cmd, VkFlags usage = 0, const void* inheritance_pNext = nullptr) const {
        VkCommandBufferInheritanceInfo inheritance = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, inheritance_pNext,
                                                      m_renderPass};
        VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, usage, &inheritance};
        return vk::BeginCommandBuffer(cmd, &info);
    }

    // Make a subpass secondary command buffer (for the class's render pass) and begin recording.
    // If a nonzero array of viewports is given, this enabled viewport/scissor inheritance and
    // passes the list of expected viewport depths.
    VkCommandBuffer MakeBeginSubpassCommandBuffer(VkCommandPool pool, uint32_t inherited_viewport_count,
                                                  const VkViewport* p_viewport_depths) const {
        VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool,
                                            VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1};
        VkCommandBuffer cmd;
        vk::AllocateCommandBuffers(m_device, &info, &cmd);
        BeginSubpassCommandBuffer(cmd, inherited_viewport_count, p_viewport_depths);
        return cmd;
    }

    // Same as above, but recycle the given secondary command buffer.
    VkResult BeginSubpassCommandBuffer(VkCommandBuffer cmd, uint32_t inherited_viewport_count,
                                       const VkViewport* p_viewport_depths) const {
        VkCommandBufferInheritanceViewportScissorInfoNV viewport_scissor = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV, nullptr,
            inherited_viewport_count != 0, inherited_viewport_count, p_viewport_depths };
        VkCommandBufferInheritanceInfo inheritance = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, &viewport_scissor, m_renderPass, 0, m_framebuffer };
        VkCommandBufferBeginInfo info = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, &inheritance};
        return vk::BeginCommandBuffer(cmd, &info);
    }
};

class NegativeViewportInheritance : public VkLayerTest {};

TEST_F(NegativeViewportInheritance, BasicUsage) {
#if defined(VVL_ENABLE_TSAN)
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    TEST_DESCRIPTION("Simple correct and incorrect usage of VK_NV_inherited_viewport_scissor");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::inheritedViewportScissor2D);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    AddRequiredFeature(vkt::Feature::nestedCommandBufferSimultaneousUse);
    RETURN_IF_SKIP(Init());

    ViewportInheritanceTestData test_data(m_device, Gpu());
    if (test_data.FailureReason()) {
        GTEST_SKIP() << "Test internal failure: " << test_data.FailureReason();
    }
    VkCommandPool pool = m_command_pool.handle();

    VkCommandBuffer subpass_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 1, test_data.kViewportDepthOnlyArray);
    test_data.BindGraphicsPipeline(subpass_cmd, true, 1);
    vk::CmdDraw(subpass_cmd, 3, 1, 0, 0);
    vk::EndCommandBuffer(subpass_cmd);

    // Basic correct usage, provide viewport in primary that has the correct depth.
    VkCommandBuffer primary_cmd = test_data.MakeBeginPrimaryCommandBuffer(pool);
    vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportArray);
    vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
    test_data.BeginRenderPass(primary_cmd);
    vk::CmdExecuteCommands(primary_cmd, 1, &subpass_cmd);
    vk::CmdEndRenderPass(primary_cmd);
    vk::EndCommandBuffer(primary_cmd);

    // Viewport with incorrect depth range.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");
    test_data.BeginPrimaryCommandBuffer(primary_cmd);
    vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportAlternateDepthArray);
    vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
    test_data.BeginRenderPass(primary_cmd);
    vk::CmdExecuteCommands(primary_cmd, 1, &subpass_cmd);
    vk::CmdEndRenderPass(primary_cmd);
    vk::EndCommandBuffer(primary_cmd);
    m_errorMonitor->VerifyFound();

    // Viewport not provided.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");
    test_data.BeginPrimaryCommandBuffer(primary_cmd);
    vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
    test_data.BeginRenderPass(primary_cmd);
    vk::CmdExecuteCommands(primary_cmd, 1, &subpass_cmd);
    vk::CmdEndRenderPass(primary_cmd);
    vk::EndCommandBuffer(primary_cmd);
    m_errorMonitor->VerifyFound();

    // Scissor not provided.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");
    test_data.BeginPrimaryCommandBuffer(primary_cmd);
    vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportArray);
    test_data.BeginRenderPass(primary_cmd);
    vk::CmdExecuteCommands(primary_cmd, 1, &subpass_cmd);
    vk::CmdEndRenderPass(primary_cmd);
    vk::EndCommandBuffer(primary_cmd);
    m_errorMonitor->VerifyFound();

    // again (i.e. no stale state left over when resetting a secondary command buffer).
    // Don't swap the loop order or you'll mess up subpass_cmd for upcoming tests.
    for (int should_fail = 1; should_fail >= 0; --should_fail) {
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07832");  // viewport
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07831");  // scissor
            test_data.BeginSubpassCommandBuffer(subpass_cmd, 0, nullptr);
        } else {
            test_data.BeginSubpassCommandBuffer(subpass_cmd, 1, test_data.kViewportDepthOnlyArray);
        }
        test_data.BindGraphicsPipeline(subpass_cmd, true, 1);
        vk::CmdDraw(subpass_cmd, 3, 1, 0, 0);
        vk::EndCommandBuffer(subpass_cmd);

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportArray);
        vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
        test_data.BeginRenderPass(primary_cmd);
        vk::CmdExecuteCommands(primary_cmd, 1, &subpass_cmd);
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Secondary that binds a static viewport/scissor pipeline.
    VkCommandBuffer static_state_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);
    test_data.BindGraphicsPipeline(static_state_cmd, false, 1);
    vk::EndCommandBuffer(static_state_cmd);

    // Test that the validation layers still flag missing state when inheritance is disabled, then stops flagging it when enabled
    // Test that inheritance fails if a static viewport/scissor pipeline
    // trashes the state before it is inherited (but it's okay if it's
    // trashed afterwards).
    for (int should_fail = 0; should_fail < 2; ++should_fail) {
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // scissor
        }
        std::array<VkCommandBuffer, 2> secondaries = {should_fail ? static_state_cmd : subpass_cmd,
                                                      should_fail ? subpass_cmd : static_state_cmd};

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportArray);
        vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
        test_data.BeginRenderPass(primary_cmd);
        vk::CmdExecuteCommands(primary_cmd, secondaries.size(), secondaries.data());
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Check that the validation layers don't count the primary
    // command buffer state when overwritten by static
    // viewport/scissor pipeline.
    for (int should_fail = 0; should_fail < 2; ++should_fail) {
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // scissor
        }

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportArray);
        vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
        if (should_fail) test_data.BindGraphicsPipeline(primary_cmd, false, 1);
        test_data.BeginRenderPass(primary_cmd);
        vk::CmdExecuteCommands(primary_cmd, 1, &subpass_cmd);
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Check that the validation layers DON'T report mismatched viewport depth when the secondary command buffer does not actually
    // consume the viewport in drawing commands (weird corner case).
    VkCommandBuffer no_draw_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 1, test_data.kViewportDepthOnlyArray);
    test_data.BindGraphicsPipeline(no_draw_cmd, true, 1); // but no subsequent draw call.
    vk::EndCommandBuffer(no_draw_cmd);

    for (int should_fail = 0; should_fail < 2; ++should_fail) {
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport
        } else {
        }

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportAlternateDepthArray);
        vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
        test_data.BeginRenderPass(primary_cmd);
        vk::CmdExecuteCommands(primary_cmd, 1, should_fail ? &subpass_cmd : &no_draw_cmd);
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Check that the validation layers are not okay with binding static viewport/scissor pipelines when inheritance enabled, or
    // setting viewport/scissor explicitly, but are okay if inheritance is not enabled (no regression).
    for (int should_fail = 0; should_fail < 2; ++should_fail) {
        if (should_fail) m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commandBuffer-04808");
        test_data.BeginSubpassCommandBuffer(no_draw_cmd, should_fail, test_data.kViewportDepthOnlyArray);
        test_data.BindGraphicsPipeline(no_draw_cmd, false, 1);
        if (should_fail) m_errorMonitor->VerifyFound();

        // Check that the validation layers flag setting viewport/scissor with inheritance.
        if (should_fail) m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewport-commandBuffer-04821");
        vk::CmdSetViewport(no_draw_cmd, 0, 1, test_data.kViewportArray);
        if (should_fail) m_errorMonitor->VerifyFound();
        if (should_fail) m_errorMonitor->SetDesiredError("VUID-vkCmdSetScissor-viewportScissor2D-04789");
        vk::CmdSetScissor(no_draw_cmd, 0, 1, test_data.kScissorArray);
        if (should_fail) m_errorMonitor->VerifyFound();

        vk::EndCommandBuffer(no_draw_cmd);
    }

    // Check for at least 1 viewport depth given when enabling inheritance.
    for (int should_fail = 0; should_fail < 2; ++should_fail) {
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04784");
        } else {
        }
        VkCommandBufferInheritanceViewportScissorInfoNV viewport_scissor = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV, nullptr, should_fail ? VK_TRUE : VK_FALSE,
            0, test_data.kViewportArray /* avoid null pointer crash still */ };
        VkCommandBuffer cmd =
            test_data.MakeBeginSecondaryCommandBuffer(pool, VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, &viewport_scissor);
        // vk::EndCommandBuffer(cmd); // seg faults.
        (void)cmd;

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Check for VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04786");
    VkCommandBufferInheritanceViewportScissorInfoNV viewport_scissor = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV, nullptr, VK_TRUE, 1, test_data.kViewportArray};
    test_data.MakeBeginSecondaryCommandBuffer(pool, 0, &viewport_scissor);
    m_errorMonitor->VerifyFound();

    // Check that validation layers allow getting inherited viewport/scissor state from earlier secondary command buffer, but not
    // from a different vkCmdExecuteCommands.
    VkCommandBuffer set_viewport_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);
    vk::CmdSetViewport(set_viewport_cmd, 0, 1, test_data.kViewportArray);
    vk::EndCommandBuffer(set_viewport_cmd);
    VkCommandBuffer set_scissor_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);
    vk::CmdSetScissor(set_scissor_cmd, 0, 1, test_data.kScissorArray);
    vk::EndCommandBuffer(set_scissor_cmd);

    for (int should_fail = 0; should_fail < 2; ++should_fail) {
        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        test_data.BeginRenderPass(primary_cmd);
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport
            vk::CmdExecuteCommands(primary_cmd, 1, &set_viewport_cmd);
            VkCommandBuffer secondaries[2] = {set_scissor_cmd, subpass_cmd};
            vk::CmdExecuteCommands(primary_cmd, 2, secondaries);
        } else {
            VkCommandBuffer secondaries[3] = {set_viewport_cmd, set_scissor_cmd, subpass_cmd};
            vk::CmdExecuteCommands(primary_cmd, 3, secondaries);
        }
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeViewportInheritance, MissingFeature) {
    TEST_DESCRIPTION("Error using VK_NV_inherited_viewport_scissor without enabling feature.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    bool has_features = false;
    const char* missing_feature_string = nullptr;
    auto self = this;
    RETURN_IF_SKIP(has_features = ViewportInheritanceTestData::InitState(
                       this, [self](const char* extension) { self->m_device_extension_names.push_back(extension); },
                       &missing_feature_string, false, false));
    if (!has_features) {
        GTEST_SKIP() << missing_feature_string;
    }

    ViewportInheritanceTestData test_data(m_device, Gpu());
    if (test_data.FailureReason()) {
        GTEST_SKIP() << "Test internal failure: " << test_data.FailureReason();
    }
    VkCommandPool pool = m_command_pool.handle();

    test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04782");
    test_data.MakeBeginSubpassCommandBuffer(pool, 1, test_data.kViewportArray);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeViewportInheritance, MultiViewport) {
#if defined(VVL_ENABLE_TSAN)
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    TEST_DESCRIPTION("VK_NV_inherited_viewport_scissor tests with multiple viewports/scissors");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::inheritedViewportScissor2D);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    AddRequiredFeature(vkt::Feature::nestedCommandBufferSimultaneousUse);
    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());

    ViewportInheritanceTestData test_data(m_device, Gpu());
    if (test_data.FailureReason()) {
        GTEST_SKIP() << "Test internal failure: " << test_data.FailureReason();
    }
    VkCommandPool pool = m_command_pool.handle();

    // Test using viewport/scissor with count state without providing it to be inherited.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // scissor
    VkCommandBuffer draw_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 1, test_data.kViewportArray);
    test_data.BindGraphicsPipeline(draw_cmd, true, 0 /* dynamic viewport and scissor count */);
    vk::CmdDraw(draw_cmd, 3, 1, 0, 0);
    vk::EndCommandBuffer(draw_cmd);

    VkCommandBuffer primary_cmd = test_data.MakeBeginPrimaryCommandBuffer(pool);
    vk::CmdSetViewport(primary_cmd, 0, 1, test_data.kViewportArray); // inadequate, needs with count
    vk::CmdSetScissor(primary_cmd, 0, 1, test_data.kScissorArray);
    test_data.BeginRenderPass(primary_cmd);
    vk::CmdExecuteCommands(primary_cmd, 1, &draw_cmd);
    vk::CmdEndRenderPass(primary_cmd);
    vk::EndCommandBuffer(primary_cmd);
    m_errorMonitor->VerifyFound();

    // Test drawing with pipeline that uses more viewports than have been inherited.
    for (int i = 0; i < 4; ++i) {
        auto should_fail = i & 1;
        if (should_fail) m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport

        test_data.BeginSubpassCommandBuffer(draw_cmd, should_fail ? 1 : 2, test_data.kViewportDepthOnlyArray);
        test_data.BindGraphicsPipeline(draw_cmd, true, 2); // Uses 2 viewports
        vk::CmdDraw(draw_cmd, 3, 1, 0, 0);
        if (i & 2) {
            // Uses only 1 viewport, should not cause us to "forget" the earlier need for 2 viewports.
            test_data.BindGraphicsPipeline(draw_cmd, true, 1);
            vk::CmdDraw(draw_cmd, 3, 1, 0, 0);
        }
        vk::EndCommandBuffer(draw_cmd);

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        vk::CmdSetViewport(primary_cmd, 0, 2, test_data.kViewportArray);
        vk::CmdSetScissor(primary_cmd, 0, 2, test_data.kScissorArray);
        test_data.BeginRenderPass(primary_cmd);
        vk::CmdExecuteCommands(primary_cmd, 1, &draw_cmd);
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Test providing needed viewports in secondary command buffer, and trashing it by binding static state pipeline in another
    // secondary command buffer.
    VkCommandBuffer set_state_fixed_count_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);
    vk::CmdSetViewport(set_state_fixed_count_cmd, 0, 2, test_data.kViewportArray);
    vk::CmdSetScissor(set_state_fixed_count_cmd, 0, 2, test_data.kScissorArray);
    vk::EndCommandBuffer(set_state_fixed_count_cmd);

    VkCommandBuffer set_state_with_count_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);
    vk::CmdSetViewportWithCountEXT(set_state_with_count_cmd, 2, test_data.kViewportArray);
    vk::CmdSetScissorWithCountEXT(set_state_with_count_cmd, 2, test_data.kScissorArray);
    vk::EndCommandBuffer(set_state_with_count_cmd);

    VkCommandBuffer static_state_cmd = test_data.MakeBeginSubpassCommandBuffer(pool, 0, nullptr);
    test_data.BindGraphicsPipeline(static_state_cmd, false, 2);
    test_data.BindGraphicsPipeline(static_state_cmd, true, 1);  // Should not "forgive" eariler static state pipeline.
    vk::EndCommandBuffer(static_state_cmd);

    for (int i = 0; i < 8; ++i) {
        auto should_fail = i & 1;
        auto use_with_count = i & 4;
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport 0 (or with count)
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // scissor 0 (or with count)
            if (!use_with_count) {
                m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport 1
                m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // scissor 1
            }
        } else {
        }

        test_data.BeginSubpassCommandBuffer(draw_cmd, 2, test_data.kViewportDepthOnlyArray);
        test_data.BindGraphicsPipeline(draw_cmd, true, use_with_count ? 0 : 2);
        vk::CmdDraw(draw_cmd, 3, 1, 0, 0);
        vk::EndCommandBuffer(draw_cmd);

        VkCommandBuffer set_state_cmd = use_with_count ? set_state_with_count_cmd : set_state_fixed_count_cmd;
        VkCommandBuffer secondaries[3];
        uint32_t secondaries_count;

        switch (i % 4) {
            case 0:
                secondaries[0] = static_state_cmd;
                secondaries[1] = set_state_cmd;
                secondaries[2] = draw_cmd;
                secondaries_count = 3;
                break;
            case 1:
                secondaries[0] = draw_cmd;
                secondaries_count = 1;
                break;
            case 2:
                secondaries[0] = set_state_cmd;
                secondaries[1] = draw_cmd;
                secondaries[2] = static_state_cmd; // Okay as it's after the drawing commands.
                secondaries_count = 3;
                break;
            case 3: default:
                secondaries[0] = set_state_cmd;
                secondaries[1] = static_state_cmd; // Trashes the dynamic state
                secondaries[2] = draw_cmd;
                secondaries_count = 3;
                break;
        }

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        test_data.BeginRenderPass(primary_cmd);
        vk::CmdExecuteCommands(primary_cmd, secondaries_count, secondaries);
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Test mismatched depth count detection, but allow it if the mismatched viewport is not actually used.
    // 0: viewport 1 mismatch, passes as draw only consumes correct 0th viewport.
    // 1: fail, mismatched viewport 1.
    // 2: pass, 2 correct viewports used. (also tests vkCmdSetViewport split between primary and secondary).
    for (int i = 0; i < 3; ++i) {
        auto should_fail = i & 1;
        if (should_fail) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");  // viewport 1
        } else {
        }

        test_data.BeginSubpassCommandBuffer(draw_cmd, 2, test_data.kViewportDepthOnlyArray);
        test_data.BindGraphicsPipeline(draw_cmd, true, i == 0 ? 1 : 2);
        vk::CmdDraw(draw_cmd, 3, 1, 0, 0);
        vk::EndCommandBuffer(draw_cmd);

        test_data.BeginSubpassCommandBuffer(set_state_fixed_count_cmd, 0, nullptr);
        vk::CmdSetViewport(set_state_fixed_count_cmd, 1, 1, i == 2 ? &test_data.kViewportArray[1] : &test_data.kViewportAlternateDepthArray[1]);
        vk::CmdSetScissor(set_state_fixed_count_cmd, 1, 1, &test_data.kScissorArray[1]);
        vk::EndCommandBuffer(set_state_fixed_count_cmd);

        test_data.BeginPrimaryCommandBuffer(primary_cmd);
        vk::CmdSetViewport(primary_cmd, 0, 1, &test_data.kViewportArray[0]);
        vk::CmdSetScissor(primary_cmd, 0, 1, &test_data.kScissorArray[0]);
        test_data.BeginRenderPass(primary_cmd);
        VkCommandBuffer secondaries[] = {set_state_fixed_count_cmd, draw_cmd};
        vk::CmdExecuteCommands(primary_cmd, 2, secondaries);
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }

    // Test dynamic viewport count failing due to either exceeding the viewport inheritance limit, or inheriting a viewport with
    // incorrect depth.
    for (int i = 0; i < 8; ++i) {
        auto state_in_secondary = i & 1;
        auto should_fail = i & 2;
        auto inherited_incorrect_depth = i & 4;

        if (should_fail) m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07850");

        VkViewport viewports[3];
        viewports[0] = test_data.kViewportArray[0];
        viewports[1] = test_data.kViewportArray[1];
        if (inherited_incorrect_depth) {
            viewports[2] = test_data.kViewportAlternateDepthArray[2];  // Will be a problem only when using all 3 viewports.
        }
        else {
            viewports[2] = test_data.kViewportArray[2];
        }

        uint32_t inherited_count = should_fail && !inherited_incorrect_depth ? 2 : 3;
        test_data.BeginSubpassCommandBuffer(draw_cmd, inherited_count, test_data.kViewportDepthOnlyArray);
        test_data.BindGraphicsPipeline(draw_cmd, true, 0);
        vk::CmdDraw(draw_cmd, 3, 1, 0, 0);
        vk::EndCommandBuffer(draw_cmd);

        test_data.BeginPrimaryCommandBuffer(primary_cmd);

        // Decoy command, should be overwritten later.
        vk::CmdSetViewportWithCountEXT(primary_cmd, 3, test_data.kViewportAlternateDepthArray);

        // Record state setting commands in either the primary, or auxilliary secondary, command buffer.
        VkCommandBuffer set_state_cmd;
        if (state_in_secondary) {
            set_state_cmd = set_state_with_count_cmd;
            test_data.BeginSubpassCommandBuffer(set_state_cmd, 0, nullptr);
        }
        else {
            set_state_cmd = primary_cmd;
        }
        uint32_t count = should_fail ? 3 : 2;
        vk::CmdSetViewportWithCountEXT(set_state_cmd, count, viewports);
        vk::CmdSetScissorWithCountEXT(set_state_cmd, count, test_data.kScissorArray);
        if (state_in_secondary) {
            vk::EndCommandBuffer(set_state_cmd);
        }

        test_data.BeginRenderPass(primary_cmd);
        if (state_in_secondary) {
            VkCommandBuffer secondaries[] = {set_state_cmd, draw_cmd};
            vk::CmdExecuteCommands(primary_cmd, 2, secondaries);
        }
        else {
            vk::CmdExecuteCommands(primary_cmd, 1, &draw_cmd);
        }
        vk::CmdEndRenderPass(primary_cmd);
        vk::EndCommandBuffer(primary_cmd);

        if (should_fail) m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeViewportInheritance, ScissorMissingFeature) {
    TEST_DESCRIPTION("Error using VK_NV_inherited_viewport_scissor without enabling multiViewport feature.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    bool has_features = false;
    const char* missing_feature_string = nullptr;
    auto self = this;
    RETURN_IF_SKIP(has_features = ViewportInheritanceTestData::InitState(
                       this, [self](const char* extension) { self->m_device_extension_names.push_back(extension); },
                       &missing_feature_string, true, false, true));
    if (!has_features) {
        GTEST_SKIP() << missing_feature_string;
    }

    ViewportInheritanceTestData test_data(m_device, Gpu());
    if (test_data.FailureReason()) {
        GTEST_SKIP() << "Test internal failure: " << test_data.FailureReason();
    }
    VkCommandPool pool = m_command_pool.handle();

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04783");
    test_data.MakeBeginSubpassCommandBuffer(pool, 2, test_data.kViewportArray);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeViewportInheritance, PipelineMissingDynamicStateDiscardRectangle) {
    TEST_DESCRIPTION("Bind pipeline with missing dynamic state discard rectangle.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rect_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rect_props);
    if (discard_rect_props.maxDiscardRectangles < 1) {
        GTEST_SKIP() << "maxDiscardRectangles property is less than 1.";
    }

    bool has_features = false;
    const char* missing_feature_string = nullptr;
    auto self = this;
    RETURN_IF_SKIP(has_features = ViewportInheritanceTestData::InitState(
                       this, [self](const char* extension) { self->m_device_extension_names.push_back(extension); },
                       &missing_feature_string, true, false, true));
    if (!has_features) {
        GTEST_SKIP() << missing_feature_string;
    }

    InitRenderTarget();

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    ViewportInheritanceTestData test_data(m_device, Gpu());
    if (test_data.FailureReason()) {
        GTEST_SKIP() << "Test internal failure: " << test_data.FailureReason();
    }

    VkCommandBufferInheritanceViewportScissorInfoNV viewport_scissor =
        vku::InitStructHelper();
    viewport_scissor.viewportScissor2D = VK_TRUE;
    viewport_scissor.viewportDepthCount = 1;
    viewport_scissor.pViewportDepths = test_data.kViewportArray;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&viewport_scissor);
    cbii.renderPass = m_renderPass;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    VkRect2D discard_rectangle = {};
    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rectangle_state = vku::InitStructHelper();
    discard_rectangle_state.discardRectangleCount = 1;
    discard_rectangle_state.pDiscardRectangles = &discard_rectangle;

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                         VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
    dyn_state_ci.dynamicStateCount = 2;
    dyn_state_ci.pDynamicStates = dyn_states;

    CreatePipelineHelper pipe(*this, &discard_rectangle_state);
    pipe.gp_ci_.pDynamicState = &dyn_state_ci;
    pipe.CreateGraphicsPipeline();

    vk::BeginCommandBuffer(secondary.handle(), &cbbi);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commandBuffer-04809");
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->VerifyFound();

    if (DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        dyn_state_ci.dynamicStateCount = 3;

        CreatePipelineHelper pipe2(*this);
        pipe2.gp_ci_.pDynamicState = &dyn_state_ci;
        pipe2.CreateGraphicsPipeline();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commandBuffer-04809");
        vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
        m_errorMonitor->VerifyFound();
    }
}

// SPIR-V blobs for graphics pipeline.

// #version 460
// void main() { gl_Position = vec4(1); }
const uint32_t ViewportInheritanceTestData::kVertexSpirV[166] = {
    0x07230203, 0x00010000, 0x0008000a, 0x00000014, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47,
    0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0006000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000d, 0x00030003, 0x00000002, 0x000001cc, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005,
    0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f,
    0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b,
    0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75,
    0x61747369, 0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
    0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003,
    0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002, 0x00020013, 0x00000002,
    0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
    0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a,
    0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c,
    0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x0004002b, 0x00000006, 0x00000010, 0x3f800000, 0x0007002c, 0x00000007,
    0x00000011, 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x00040020, 0x00000012, 0x00000003, 0x00000007, 0x00050036,
    0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000012, 0x00000013, 0x0000000d,
    0x0000000f, 0x0003003e, 0x00000013, 0x00000011, 0x000100fd, 0x00010038};

// #version 460
// layout(location = 0) out vec4 color;
// void main() { color = vec4(1); }
const uint32_t ViewportInheritanceTestData::kFragmentSpirV[83] = {
    0x07230203, 0x00010000, 0x0008000a, 0x0000000c, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47,
    0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0006000f, 0x00000004, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000009, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001cc, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00040005, 0x00000009, 0x6f6c6f63, 0x00000072, 0x00040047, 0x00000009, 0x0000001e, 0x00000000,
    0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003,
    0x0004002b, 0x00000006, 0x0000000a, 0x3f800000, 0x0007002c, 0x00000007, 0x0000000b, 0x0000000a, 0x0000000a, 0x0000000a,
    0x0000000a, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0003003e, 0x00000009,
    0x0000000b, 0x000100fd, 0x00010038};

const VkPipelineVertexInputStateCreateInfo ViewportInheritanceTestData::kVertexInputState = {
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr};

const VkPipelineInputAssemblyStateCreateInfo ViewportInheritanceTestData::kInputAssemblyState = {
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};

const VkPipelineRasterizationStateCreateInfo ViewportInheritanceTestData::kRasterizationState = {
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    nullptr,
    0,
    VK_FALSE,
    VK_FALSE,
    VK_POLYGON_MODE_FILL,
    VK_CULL_MODE_BACK_BIT,
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
    false,
    0.0f,
    0.0f,
    0.0f,
    1.0f};

const VkPipelineMultisampleStateCreateInfo ViewportInheritanceTestData::kMultisampleState = {
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    nullptr,
    0,
    VK_SAMPLE_COUNT_1_BIT,
};

const VkPipelineDepthStencilStateCreateInfo ViewportInheritanceTestData::kDepthStencilState =
    vku::InitStructHelper();

const VkPipelineColorBlendAttachmentState ViewportInheritanceTestData::kBlendAttachmentState = {
    VK_FALSE,
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_OP_ADD,
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_OP_ADD,
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

const VkPipelineColorBlendStateCreateInfo ViewportInheritanceTestData::kBlendState = {
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    nullptr,
    0,
    VK_FALSE,
    VK_LOGIC_OP_CLEAR,
    1,
    &kBlendAttachmentState,
    {}};

const VkPipelineDynamicStateCreateInfo ViewportInheritanceTestData::kStaticState = {
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 0, nullptr};

static const std::array<VkDynamicState, 2> kDynamicStateArray = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
const VkPipelineDynamicStateCreateInfo ViewportInheritanceTestData::kDynamicState = {
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0,
    static_cast<uint32_t>(kDynamicStateArray.size()), kDynamicStateArray.data()};

static const std::array<VkDynamicState, 2> kDynamicStateWithCountArray = {VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
                                                                          VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT};
const VkPipelineDynamicStateCreateInfo ViewportInheritanceTestData::kDynamicStateWithCount = {
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0,
    static_cast<uint32_t>(kDynamicStateWithCountArray.size()), kDynamicStateWithCountArray.data()};

const VkViewport ViewportInheritanceTestData::kViewportArray[32] = {
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 1.00f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.01f, 0.99f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.02f, 0.98f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.03f, 0.97f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.04f, 0.96f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.05f, 0.95f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.06f, 0.94f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.07f, 0.93f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.08f, 0.92f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.09f, 0.91f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.10f, 0.90f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.11f, 0.89f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.12f, 0.88f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.13f, 0.87f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.14f, 0.86f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.15f, 0.85f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.16f, 0.84f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.17f, 0.83f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.18f, 0.82f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.19f, 0.81f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.20f, 0.80f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.21f, 0.79f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.22f, 0.78f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.23f, 0.77f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.24f, 0.76f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.25f, 0.75f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.26f, 0.74f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.27f, 0.73f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.28f, 0.72f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.29f, 0.71f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.30f, 0.70f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.31f, 0.69f},
};

const VkViewport ViewportInheritanceTestData::kViewportDepthOnlyArray[32] = {
    {0.0f, 0.0f, 0.0f, 0.0f, 0.00f, 1.00f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.99f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.98f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.03f, 0.97f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.04f, 0.96f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.95f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.06f, 0.94f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.07f, 0.93f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.08f, 0.92f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.09f, 0.91f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.10f, 0.90f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.11f, 0.89f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.12f, 0.88f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.13f, 0.87f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.14f, 0.86f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.85f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.16f, 0.84f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.17f, 0.83f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.18f, 0.82f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.19f, 0.81f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.20f, 0.80f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.21f, 0.79f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.22f, 0.78f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.23f, 0.77f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.24f, 0.76f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.75f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.26f, 0.74f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.27f, 0.73f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.28f, 0.72f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.29f, 0.71f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.30f, 0.70f}, {0.0f, 0.0f, 0.0f, 0.0f, 0.31f, 0.69f},
};

const VkViewport ViewportInheritanceTestData::kViewportAlternateDepthArray[32] = {
    {0.0f, 0.0f, 128.0f, 128.0f, 0.88f, 1.00f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.01f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.98f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.03f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.96f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.05f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.94f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.07f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.92f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.09f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.90f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.11f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.88f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.13f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.86f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.15f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.84f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.17f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.82f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.19f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.80f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.21f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.78f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.23f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.76f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.25f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.74f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.27f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.72f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.29f, 0.00f},
    {0.0f, 0.0f, 128.0f, 128.0f, 0.00f, 0.70f}, {0.0f, 0.0f, 128.0f, 128.0f, 0.31f, 0.00f},
};

const VkRect2D ViewportInheritanceTestData::kScissorArray[32] = {
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
    {{0, 0}, {128, 128}}, {{0, 0}, {128, 128}},
};
