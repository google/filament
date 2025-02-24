/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

// Basic Vertex shader with Xfb OpExecutionMode added
static const char *kXfbVsSource = R"asm(
               OpCapability Shader
               OpCapability TransformFeedback
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_
               OpExecutionMode %main Xfb
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd
        )asm";

class NegativeTransformFeedback : public VkLayerTest {
  public:
    void InitBasicTransformFeedback();
};

void NegativeTransformFeedback::InitBasicTransformFeedback() {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(Init());
}

TEST_F(NegativeTransformFeedback, FeatureEnabled) {
    TEST_DESCRIPTION("VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback must be enabled");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    // transformFeedback not enabled
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    {
        vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
        VkDeviceSize offsets[1]{};

        m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-transformFeedback-02355");
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-transformFeedback-02366");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-transformFeedback-02374");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndTransformFeedbackEXT-None-02375");
        vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTransformFeedback, NoBoundPipeline) {
    TEST_DESCRIPTION("Call vkCmdBeginTransformFeedbackEXT without a bound pipeline");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-09630");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-06233");
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTransformFeedback, CmdBindTransformFeedbackBuffersEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdBindTransformFeedbackBuffersEXT");
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    {
        VkPhysicalDeviceTransformFeedbackPropertiesEXT tf_properties = vku::InitStructHelper();
        GetPhysicalDeviceProperties2(tf_properties);

        vkt::Buffer const buffer_obj(*m_device, 8, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);

        // Request a firstBinding that is too large.
        {
            auto const firstBinding = tf_properties.maxTransformFeedbackBuffers;
            VkDeviceSize const offsets[1]{};

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), firstBinding, 1, &buffer_obj.handle(), offsets,
                                                   nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request too many bindings.
        if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
            auto const bindingCount = tf_properties.maxTransformFeedbackBuffers + 1;
            std::vector<VkBuffer> buffers(bindingCount, buffer_obj.handle());

            std::vector<VkDeviceSize> offsets(bindingCount);

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, bindingCount, buffers.data(), offsets.data(),
                                                   nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request a size that is larger than the maximum size.
        if (tf_properties.maxTransformFeedbackBufferSize < std::numeric_limits<VkDeviceSize>::max()) {
            VkDeviceSize const offsets[1]{};
            VkDeviceSize const sizes[1]{tf_properties.maxTransformFeedbackBufferSize + 1};

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        const uint32_t buffer_size = 8;
        vkt::Buffer const buffer_obj(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);

        // Request an offset that is too large.
        {
            VkDeviceSize const offsets[1]{buffer_size + 4};

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02358");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request an offset that is not a multiple of 4.
        {
            VkDeviceSize const offsets[1]{1};

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request a size that is larger than the buffer's size.
        {
            VkDeviceSize const offsets[1]{};
            VkDeviceSize const sizes[1]{buffer_size + 1};

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pSizes-02362");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }

        // Request an offset and size whose sum is larger than the buffer's size.
        {
            VkDeviceSize const offsets[1]{4};
            VkDeviceSize const sizes[1]{buffer_size - 3};

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02363");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }

        // Bind while transform feedback is active.
        {
            VkDeviceSize const offsets[1]{0};
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);

            vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);

            m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-None-02365");
            vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();

            vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
        }
    }

    // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT.
    {
        vkt::Buffer const buffer_obj(*m_device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02360");
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // Don't bind memory.
    {
        VkBufferCreateInfo info = vku::InitStructHelper();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 4;
        vkt::Buffer buffer(*m_device, info, vkt::no_mem);

        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02364");
        vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTransformFeedback, CmdBeginTransformFeedbackEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdBeginTransformFeedbackEXT");
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    {
        VkPhysicalDeviceTransformFeedbackPropertiesEXT tf_properties = vku::InitStructHelper();
        GetPhysicalDeviceProperties2(tf_properties);

        // Request a firstCounterBuffer that is too large.
        {
            auto const firstCounterBuffer = tf_properties.maxTransformFeedbackBuffers;

            m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369");
            vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), firstCounterBuffer, 1, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request too many buffers.
        if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
            auto const counterBufferCount = tf_properties.maxTransformFeedbackBuffers + 1;

            m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369");
            vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, counterBufferCount, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }
    }

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);

    // Request an out-of-bounds location.
    {
        vkt::Buffer const buffer_obj(*m_device, 4, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT);
        VkDeviceSize const offsets[1]{1};

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-02370");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets);
        m_errorMonitor->VerifyFound();
    }

    // Request specific offsets without specifying buffers.
    {
        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffer-02371");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, offsets);
        m_errorMonitor->VerifyFound();
    }

    // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT.
    {
        vkt::Buffer const buffer_obj(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    // Begin while transform feedback is active.
    {
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-02367");
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();

        vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    }
}

TEST_F(NegativeTransformFeedback, CmdEndTransformFeedbackEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdEndTransformFeedbackEXT");
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);

    {
        // Activate transform feedback.
        vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);

        {
            VkPhysicalDeviceTransformFeedbackPropertiesEXT tf_properties = vku::InitStructHelper();
            GetPhysicalDeviceProperties2(tf_properties);

            // Request a firstCounterBuffer that is too large.
            {
                auto const firstCounterBuffer = tf_properties.maxTransformFeedbackBuffers;

                m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376");
                m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377");
                vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), firstCounterBuffer, 1, nullptr, nullptr);
                m_errorMonitor->VerifyFound();
            }

            // Request too many buffers.
            if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
                auto const counterBufferCount = tf_properties.maxTransformFeedbackBuffers + 1;

                m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377");
                vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, counterBufferCount, nullptr, nullptr);
                m_errorMonitor->VerifyFound();
            }
        }

        // Request an out-of-bounds location.
        {
            vkt::Buffer const buffer_obj(*m_device, 4, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT);
            VkDeviceSize const offsets[1]{1};

            m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-02378");
            vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets);
            m_errorMonitor->VerifyFound();
        }

        // Request specific offsets without specifying buffers.
        {
            VkDeviceSize const offsets[1]{};

            m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffer-02379");
            vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, offsets);
            m_errorMonitor->VerifyFound();
        }

        // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT.
        {
            vkt::Buffer const buffer_obj(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

            m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380");
            vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), nullptr);
            m_errorMonitor->VerifyFound();
        }
    }

    // End while transform feedback is inactive.
    {
        vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);

        m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedbackEXT-None-02375");
        vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTransformFeedback, ExecuteSecondaryCommandBuffers) {
    TEST_DESCRIPTION("Call CmdExecuteCommandBuffers when transform feedback is active");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;
    hinfo.renderPass = m_renderPassBeginInfo.renderPass;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    secondary.Begin(&info);
    secondary.End();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-commandBuffer-recording");
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, BindPipeline) {
    TEST_DESCRIPTION("Call CmdBindPipeline when transform feedback is active");
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    CreatePipelineHelper pipe_one(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe_one.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe_one.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_two(*this);
    pipe_two.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_one.Handle());
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-None-02323");
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_two.Handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTransformFeedback, EndRenderPass) {
    TEST_DESCRIPTION("Call CmdEndRenderPass when transform feedback is active");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndRenderPass-None-02351");
    m_command_buffer.EndRenderPass();
    m_errorMonitor->VerifyFound();
    vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTransformFeedback, DrawIndirectByteCountEXT) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirectByteCountEXT");
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT tf_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(tf_properties);
    if (!tf_properties.transformFeedbackDraw) {
        GTEST_SKIP() << "transformFeedbackDraw is not supported";
    }

    vkt::Buffer counter_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    {
        CreatePipelineHelper pipeline(*this);
        pipeline.CreateGraphicsPipeline();

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Handle());

        // if property is not a multiple of 4
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-09475");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289");
        vk::CmdDrawIndirectByteCountEXT(m_command_buffer.handle(), 1, 0, counter_buffer.handle(), 0, 0,
                                        tf_properties.maxTransformFeedbackBufferDataStride + 4);
        m_errorMonitor->VerifyFound();

        // non-4 multiple stride
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568");
        vk::CmdDrawIndirectByteCountEXT(m_command_buffer.handle(), 1, 0, counter_buffer.handle(), 1, 0, 4);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectByteCountEXT-counterOffset-09474");
        vk::CmdDrawIndirectByteCountEXT(m_command_buffer.handle(), 1, 0, counter_buffer.handle(), 0, 1, 4);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-09475");
        vk::CmdDrawIndirectByteCountEXT(m_command_buffer.handle(), 1, 0, counter_buffer.handle(), 0, 0, 1);
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    std::vector<const char *> device_extension_names;
    device_extension_names.push_back(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    vkt::Device test_device(Gpu(), device_extension_names);
    vkt::CommandPool commandPool(test_device, 0);
    vkt::CommandBuffer commandBuffer(test_device, commandPool);
    vkt::Buffer counter_buffer2(test_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    vkt::PipelineLayout pipelineLayout(test_device);

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    VkSubpassDescription subpass = {};
    rp_info.pSubpasses = &subpass;
    rp_info.subpassCount = 1;
    vkt::RenderPass renderpass(test_device, rp_info);
    ASSERT_TRUE(renderpass.handle());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(&test_device);
    fs.InitFromGLSLTry(&test_device);

    CreatePipelineHelper pipeline(*this);
    pipeline.device_ = &test_device;
    pipeline.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipeline.gp_ci_.layout = pipelineLayout.handle();
    pipeline.gp_ci_.renderPass = renderpass.handle();
    pipeline.CreateGraphicsPipeline();

    vkt::Framebuffer fb(test_device, renderpass.handle(), 0, nullptr, 256, 256);

    m_renderPassBeginInfo.renderPass = renderpass.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_renderPassBeginInfo.renderPass = renderpass.handle();
    commandBuffer.Begin();
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Handle());
    commandBuffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287");
    vk::CmdDrawIndirectByteCountEXT(commandBuffer.handle(), 1, 0, counter_buffer2.handle(), 0, 0, 4);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, UsingRasterizationStateStreamExtDisabled) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT but it doesn't enable geometryStreams.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    // geometryStreams not enabled
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    VkPipelineRasterizationStateStreamCreateInfoEXT rasterization_state_stream_ci = vku::InitStructHelper();
    pipe.rs_state_ci_.pNext = &rasterization_state_stream_ci;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-geometryStreams-02324");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, RuntimeSpirv) {
    TEST_DESCRIPTION("Test runtime spirv transform feedback.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);

    // seen sometimes when using profiles and will crash
    if (transform_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "maxTransformFeedbackStreams is zero";
    }

    {
        std::stringstream vsSource;
        vsSource << R"asm(
               OpCapability Shader
               OpCapability TransformFeedback
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %tf
               OpExecutionMode %main Xfb

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %8

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride )asm";
        vsSource << transform_feedback_props.maxTransformFeedbackBufferDataStride + 4;
        vsSource << R"asm(
               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )asm";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-XfbStride-06313");
        VkShaderObj::CreateFromASM(this, vsSource.str().c_str(), VK_SHADER_STAGE_VERTEX_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %tf
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 1

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %10

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf Stream 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
     %int_17 = OpConstant %int )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackStreams;
        gsSource << R"asm(
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_17
               OpReturn
               OpFunctionEnd
        )asm";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpEmitStreamVertex-06310");
        VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
        m_errorMonitor->VerifyFound();
    }

    if (transform_feedback_props.transformFeedbackStreamsLinesTriangles == VK_FALSE) {
        const char *gsSource = R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a %b
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %11
               OpName %b "b"  ; id %12

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream 0
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 4
               OpDecorate %a Offset 0
               OpDecorate %b Location 1
               OpDecorate %b Stream 0
               OpDecorate %b XfbBuffer 1
               OpDecorate %b XfbStride 4
               OpDecorate %b Offset 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output
          %b = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpEmitStreamVertex %int_1
               OpReturn
               OpFunctionEnd
        )asm";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-transformFeedbackStreamsLinesTriangles-06311");
        VkShaderObj::CreateFromASM(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %10

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream 0
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 20
               OpDecorate %a Offset  )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackBufferDataSize;
        gsSource << R"asm(

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Offset-06308");
        if (transform_feedback_props.maxTransformFeedbackBufferDataSize + 4 >=
            transform_feedback_props.maxTransformFeedbackStreamDataSize) {
            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-XfbBuffer-06309");
        }
        VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %10

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream  )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackStreams;
        gsSource << R"asm(
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 4
               OpDecorate %a Offset 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Stream-06312");
        VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
        m_errorMonitor->VerifyFound();
    }

    {
        uint32_t offset = transform_feedback_props.maxTransformFeedbackBufferDataSize / 2;
        uint32_t count = transform_feedback_props.maxTransformFeedbackStreamDataSize / offset + 1;
        // Limit to 25, because we are dynamically adding variables using letters as names
        if (count < 25) {
            std::stringstream gsSource;
            gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main"
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4)asm";

            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + static_cast<char>(i);
                gsSource << "\nOpName %var" << v << " \"" << v << "\"";
            }
            gsSource << "\n; Annotations\n";

            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + static_cast<char>(i);
                gsSource << "OpDecorate %var" << v << " Location " << i << "\n";
                gsSource << "OpDecorate %var" << v << " Stream 0\n";
                gsSource << "OpDecorate %var" << v << " XfbBuffer " << i << "\n";
                gsSource << "OpDecorate %var" << v << " XfbStride 20\n";
                gsSource << "OpDecorate %var" << v << " Offset " << offset << "\n";
            }
            gsSource << R"asm(

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float)asm";

            gsSource << "\n";
            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + static_cast<char>(i);
                gsSource << "%var" << v << " = OpVariable %_ptr_Output_float Output\n";
            }

            gsSource << R"asm(

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

            m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-XfbBuffer-06309");
            VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeTransformFeedback, PipelineRasterizationStateStreamCreateInfoEXT) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT with invalid rasterizationStream.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transfer_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transfer_feedback_props);

    if (!transfer_feedback_props.transformFeedbackRasterizationStreamSelect &&
        transfer_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackRasterizationStreamSelect is 0";
    }

    CreatePipelineHelper pipe(*this);
    VkPipelineRasterizationStateStreamCreateInfoEXT rasterization_state_stream_ci = vku::InitStructHelper();
    rasterization_state_stream_ci.rasterizationStream = transfer_feedback_props.maxTransformFeedbackStreams;
    pipe.rs_state_ci_.pNext = &rasterization_state_stream_ci;

    if (transfer_feedback_props.transformFeedbackRasterizationStreamSelect) {
        m_errorMonitor->SetDesiredError("VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02325");
    } else {
        m_errorMonitor->SetDesiredError("VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02326");
    }
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, CmdNextSubpass) {
    TEST_DESCRIPTION("Call CmdNextSubpass while transform feeback is active");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               1,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    vkt::RenderPass rp(*m_device, rpci);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &imageView.handle());

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp.handle();
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);

    m_command_buffer.BeginRenderPass(rp.handle(), fb.handle(), 32, 32);
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdNextSubpass-None-02349");
    m_command_buffer.NextSubpass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, CmdBeginTransformFeedbackOutsideRenderPass) {
    TEST_DESCRIPTION("call vkCmdBeginTransformFeedbackEXT without renderpass");
    RETURN_IF_SKIP(InitBasicTransformFeedback());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer const buffer_obj(*m_device, 4, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT);
    VkDeviceSize const offsets[1]{1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-renderpass");
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, &buffer_obj.handle(), offsets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, XfbExecutionModeCommand) {
    TEST_DESCRIPTION("missing Xfb execution mode");
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    // default Vertex shader will not have Xfb
    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-04128");
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTransformFeedback, XfbExecutionModePipeline) {
    TEST_DESCRIPTION("missing Xfb execution mode");
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, kGeometryMinimalGlsl, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs->GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-pStages-02318");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, InvalidCounterBuffers) {
    TEST_DESCRIPTION("Begin transform feedback with invalid counter buffer handles");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkBuffer buffer_handle = buffer.handle();
    buffer.destroy();

    CreatePipelineHelper pipe(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDeviceSize offset = 0u;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-counterBufferCount-02607");
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0u, 1u, &buffer_handle, &offset);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeTransformFeedback, ExecuteSecondaryCommandBuffersWithDynamicRenderPass) {
    TEST_DESCRIPTION("Call CmdExecuteCommandBuffers when transform feedback is active");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_7_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::maintenance7);
    RETURN_IF_SKIP(InitBasicTransformFeedback());

    InitRenderTarget();

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkFormat format = m_renderTargets[0]->Format();
    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1u;
    inheritance_rendering_info.pColorAttachmentFormats = &format;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo secondary_begin = vku::InitStructHelper();
    secondary_begin.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_begin.pInheritanceInfo = &inheritance_info;
    secondary_cb.Begin(&secondary_begin);
    secondary_cb.End();

    CreatePipelineHelper pipe(*this);
    auto vs = VkShaderObj::CreateFromASM(this, kXfbVsSource, VK_SHADER_STAGE_VERTEX_BIT);
    pipe.shader_stages_[0] = vs->GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0u, 0u, NULL, NULL);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-None-02286");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_cb.handle());
    m_errorMonitor->VerifyFound();

    vk::CmdEndTransformFeedbackEXT(m_command_buffer.handle(), 0u, 0u, NULL, NULL);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
