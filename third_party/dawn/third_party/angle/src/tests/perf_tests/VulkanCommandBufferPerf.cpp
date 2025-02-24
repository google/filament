// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanCommandBufferPerf:
//   Performance benchmark for Vulkan Primary/Secondary Command Buffer implementations.
//  Can run just these tests by adding "--gtest_filter=VulkanCommandBufferPerfTest*"
//   option to angle_white_box_perftests.
//  When running on Android with run_angle_white_box_perftests, use "-v" option.

#include "ANGLEPerfTest.h"
#include "common/platform.h"
#include "test_utils/third_party/vulkan_command_buffer_utils.h"

#if defined(ANDROID)
#    define NUM_CMD_BUFFERS 1000
// Android devices tend to be slower so only do 10 frames to avoid timeout
#    define NUM_FRAMES 10
#else
#    define NUM_CMD_BUFFERS 1000
#    define NUM_FRAMES 100
#endif

// These are minimal shaders used to submit trivial draw commands to command
//  buffers so that we can create large batches of cmd buffers with consistent
//  draw patterns but size/type of cmd buffers can be varied to test cmd buffer
//  differences across devices.
constexpr char kVertShaderText[] = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 outColor;
void main() {
   outColor = inColor;
   gl_Position = myBufferVals.mvp * pos;
})";

constexpr char kFragShaderText[] = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;
void main() {
   outColor = color;
})";

using CommandBufferImpl = void (*)(sample_info &info,
                                   VkClearValue *clear_values,
                                   VkFence drawFence,
                                   VkSemaphore imageAcquiredSemaphore,
                                   int numBuffers);

struct CommandBufferTestParams
{
    CommandBufferImpl CBImplementation;
    std::string story;
    int frames  = NUM_FRAMES;
    int buffers = NUM_CMD_BUFFERS;
};

class VulkanCommandBufferPerfTest : public ANGLEPerfTest,
                                    public ::testing::WithParamInterface<CommandBufferTestParams>
{
  public:
    VulkanCommandBufferPerfTest();

    void SetUp() override;
    void TearDown() override;
    void step() override;

  private:
    VkClearValue mClearValues[2]        = {};
    VkSemaphore mImageAcquiredSemaphore = VK_NULL_HANDLE;
    VkFence mDrawFence                  = VK_NULL_HANDLE;

    VkResult res             = VK_NOT_READY;
    const bool mDepthPresent = true;
    struct sample_info mInfo = {};
    std::string mSampleTitle;
    CommandBufferImpl mCBImplementation = nullptr;
    int mFrames                         = 0;
    int mBuffers                        = 0;
};

VulkanCommandBufferPerfTest::VulkanCommandBufferPerfTest()
    : ANGLEPerfTest("VulkanCommandBufferPerfTest", "", GetParam().story, GetParam().frames)
{
    mInfo             = {};
    mSampleTitle      = "Draw Textured Cube";
    mCBImplementation = GetParam().CBImplementation;
    mFrames           = GetParam().frames;
    mBuffers          = GetParam().buffers;
}

void VulkanCommandBufferPerfTest::SetUp()
{
    ANGLEPerfTest::SetUp();

    init_global_layer_properties(mInfo);
    init_instance_extension_names(mInfo);
    init_device_extension_names(mInfo);
    init_instance(mInfo, mSampleTitle.c_str());
    init_enumerate_device(mInfo);
    init_window_size(mInfo, 500, 500);
    init_connection(mInfo);
    init_window(mInfo);
    init_swapchain_extension(mInfo);
    init_device(mInfo);

    init_command_pool(mInfo, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    init_command_buffer(mInfo);                   // Primary command buffer to hold secondaries
    init_command_buffer_array(mInfo, mBuffers);   // Array of primary command buffers
    init_command_buffer2_array(mInfo, mBuffers);  // Array containing all secondary buffers
    init_device_queue(mInfo);
    init_swap_chain(mInfo);
    init_depth_buffer(mInfo);
    init_uniform_buffer(mInfo);
    init_descriptor_and_pipeline_layouts(mInfo, false);
    init_renderpass(mInfo, mDepthPresent);
    init_shaders(mInfo, kVertShaderText, kFragShaderText);
    init_framebuffers(mInfo, mDepthPresent);
    init_vertex_buffer(mInfo, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
                       sizeof(g_vb_solid_face_colors_Data[0]), false);
    init_descriptor_pool(mInfo, false);
    init_descriptor_set(mInfo);
    init_pipeline_cache(mInfo);
    init_pipeline(mInfo, mDepthPresent);

    mClearValues[0].color.float32[0]     = 0.2f;
    mClearValues[0].color.float32[1]     = 0.2f;
    mClearValues[0].color.float32[2]     = 0.2f;
    mClearValues[0].color.float32[3]     = 0.2f;
    mClearValues[1].depthStencil.depth   = 1.0f;
    mClearValues[1].depthStencil.stencil = 0;

    VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    imageAcquiredSemaphoreCreateInfo.flags = 0;
    res = vkCreateSemaphore(mInfo.device, &imageAcquiredSemaphoreCreateInfo, NULL,
                            &mImageAcquiredSemaphore);
    ASSERT_EQ(VK_SUCCESS, res);

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = 0;
    res             = vkCreateFence(mInfo.device, &fenceInfo, NULL, &mDrawFence);
    ASSERT_EQ(VK_SUCCESS, res);
}

void VulkanCommandBufferPerfTest::step()
{
    for (int x = 0; x < mFrames; x++)
    {
        mInfo.current_buffer = x % mInfo.swapchainImageCount;

        // Get the index of the next available swapchain image:
        res = vkAcquireNextImageKHR(mInfo.device, mInfo.swap_chain, UINT64_MAX,
                                    mImageAcquiredSemaphore, VK_NULL_HANDLE, &mInfo.current_buffer);
        // Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
        // return codes
        ASSERT_EQ(VK_SUCCESS, res);
        mCBImplementation(mInfo, mClearValues, mDrawFence, mImageAcquiredSemaphore, mBuffers);
    }
}

void VulkanCommandBufferPerfTest::TearDown()
{
    vkDestroySemaphore(mInfo.device, mImageAcquiredSemaphore, NULL);
    vkDestroyFence(mInfo.device, mDrawFence, NULL);
    destroy_pipeline(mInfo);
    destroy_pipeline_cache(mInfo);
    destroy_descriptor_pool(mInfo);
    destroy_vertex_buffer(mInfo);
    destroy_framebuffers(mInfo);
    destroy_shaders(mInfo);
    destroy_renderpass(mInfo);
    destroy_descriptor_and_pipeline_layouts(mInfo);
    destroy_uniform_buffer(mInfo);
    destroy_depth_buffer(mInfo);
    destroy_swap_chain(mInfo);
    destroy_command_buffer2_array(mInfo, mBuffers);
    destroy_command_buffer_array(mInfo, mBuffers);
    destroy_command_buffer(mInfo);
    destroy_command_pool(mInfo);
    destroy_device(mInfo);
    destroy_window(mInfo);
    destroy_instance(mInfo);
    ANGLEPerfTest::TearDown();
}

// Common code to present image used by all tests
void Present(sample_info &info, VkFence drawFence)
{
    // Now present the image in the window

    VkPresentInfoKHR present;
    present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext              = NULL;
    present.swapchainCount     = 1;
    present.pSwapchains        = &info.swap_chain;
    present.pImageIndices      = &info.current_buffer;
    present.pWaitSemaphores    = NULL;
    present.waitSemaphoreCount = 0;
    present.pResults           = NULL;

    // Make sure command buffer is finished before presenting
    VkResult res;
    do
    {
        res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    } while (res == VK_TIMEOUT);
    vkResetFences(info.device, 1, &drawFence);

    ASSERT_EQ(VK_SUCCESS, res);
    res = vkQueuePresentKHR(info.present_queue, &present);
    ASSERT_EQ(VK_SUCCESS, res);
}

// 100 separate primary cmd buffers, each with 1 Draw
void PrimaryCommandBufferBenchmarkHundredIndividual(sample_info &info,
                                                    VkClearValue *clear_values,
                                                    VkFence drawFence,
                                                    VkSemaphore imageAcquiredSemaphore,
                                                    int numBuffers)
{
    VkResult res;

    VkRenderPassBeginInfo rpBegin;
    rpBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.pNext                    = NULL;
    rpBegin.renderPass               = info.render_pass;
    rpBegin.framebuffer              = info.framebuffers[info.current_buffer];
    rpBegin.renderArea.offset.x      = 0;
    rpBegin.renderArea.offset.y      = 0;
    rpBegin.renderArea.extent.width  = info.width;
    rpBegin.renderArea.extent.height = info.height;
    rpBegin.clearValueCount          = 2;
    rpBegin.pClearValues             = clear_values;

    VkCommandBufferBeginInfo cmdBufferInfo = {};
    cmdBufferInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferInfo.pNext                    = NULL;
    cmdBufferInfo.flags                    = 0;
    cmdBufferInfo.pInheritanceInfo         = NULL;

    for (int x = 0; x < numBuffers; x++)
    {
        vkBeginCommandBuffer(info.cmds[x], &cmdBufferInfo);
        vkCmdBeginRenderPass(info.cmds[x], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(info.cmds[x], VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
        vkCmdBindDescriptorSets(info.cmds[x], VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline_layout,
                                0, NUM_DESCRIPTOR_SETS, info.desc_set.data(), 0, NULL);

        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(info.cmds[x], 0, 1, &info.vertex_buffer.buf, offsets);

        init_viewports_array(info, x);
        init_scissors_array(info, x);

        vkCmdDraw(info.cmds[x], 0, 1, 0, 0);
        vkCmdEndRenderPass(info.cmds[x]);
        res = vkEndCommandBuffer(info.cmds[x]);
        ASSERT_EQ(VK_SUCCESS, res);
    }

    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo[1]            = {};
    submitInfo[0].pNext                   = NULL;
    submitInfo[0].sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].waitSemaphoreCount      = 1;
    submitInfo[0].pWaitSemaphores         = &imageAcquiredSemaphore;
    submitInfo[0].pWaitDstStageMask       = &pipe_stage_flags;
    submitInfo[0].commandBufferCount      = numBuffers;
    submitInfo[0].pCommandBuffers         = info.cmds.data();
    submitInfo[0].signalSemaphoreCount    = 0;
    submitInfo[0].pSignalSemaphores       = NULL;

    // Queue the command buffer for execution
    res = vkQueueSubmit(info.graphics_queue, 1, submitInfo, drawFence);
    ASSERT_EQ(VK_SUCCESS, res);

    Present(info, drawFence);
}

// 100 of the same Draw cmds in the same primary cmd buffer
void PrimaryCommandBufferBenchmarkOneWithOneHundred(sample_info &info,
                                                    VkClearValue *clear_values,
                                                    VkFence drawFence,
                                                    VkSemaphore imageAcquiredSemaphore,
                                                    int numBuffers)
{
    VkResult res;

    VkRenderPassBeginInfo rpBegin;
    rpBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.pNext                    = NULL;
    rpBegin.renderPass               = info.render_pass;
    rpBegin.framebuffer              = info.framebuffers[info.current_buffer];
    rpBegin.renderArea.offset.x      = 0;
    rpBegin.renderArea.offset.y      = 0;
    rpBegin.renderArea.extent.width  = info.width;
    rpBegin.renderArea.extent.height = info.height;
    rpBegin.clearValueCount          = 2;
    rpBegin.pClearValues             = clear_values;

    VkCommandBufferBeginInfo cmdBufferInfo = {};
    cmdBufferInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferInfo.pNext                    = NULL;
    cmdBufferInfo.flags                    = 0;
    cmdBufferInfo.pInheritanceInfo         = NULL;

    vkBeginCommandBuffer(info.cmd, &cmdBufferInfo);
    for (int x = 0; x < numBuffers; x++)
    {
        vkCmdBeginRenderPass(info.cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
        vkCmdBindDescriptorSets(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline_layout, 0,
                                NUM_DESCRIPTOR_SETS, info.desc_set.data(), 0, NULL);

        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(info.cmd, 0, 1, &info.vertex_buffer.buf, offsets);

        init_viewports(info);
        init_scissors(info);

        vkCmdDraw(info.cmd, 0, 1, 0, 0);
        vkCmdEndRenderPass(info.cmd);
    }
    res = vkEndCommandBuffer(info.cmd);
    ASSERT_EQ(VK_SUCCESS, res);

    const VkCommandBuffer cmd_bufs[]      = {info.cmd};
    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo[1]            = {};
    submitInfo[0].pNext                   = NULL;
    submitInfo[0].sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].waitSemaphoreCount      = 1;
    submitInfo[0].pWaitSemaphores         = &imageAcquiredSemaphore;
    submitInfo[0].pWaitDstStageMask       = &pipe_stage_flags;
    submitInfo[0].commandBufferCount      = 1;
    submitInfo[0].pCommandBuffers         = cmd_bufs;
    submitInfo[0].signalSemaphoreCount    = 0;
    submitInfo[0].pSignalSemaphores       = NULL;

    // Queue the command buffer for execution
    res = vkQueueSubmit(info.graphics_queue, 1, submitInfo, drawFence);
    ASSERT_EQ(VK_SUCCESS, res);

    Present(info, drawFence);
}

// 100 separate secondary cmd buffers, each with 1 Draw
void SecondaryCommandBufferBenchmark(sample_info &info,
                                     VkClearValue *clear_values,
                                     VkFence drawFence,
                                     VkSemaphore imageAcquiredSemaphore,
                                     int numBuffers)
{
    VkResult res;

    // Record Secondary Command Buffer
    VkCommandBufferInheritanceInfo inheritInfo = {};
    inheritInfo.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritInfo.pNext                          = NULL;
    inheritInfo.renderPass                     = info.render_pass;
    inheritInfo.subpass                        = 0;
    inheritInfo.framebuffer                    = info.framebuffers[info.current_buffer];
    inheritInfo.occlusionQueryEnable           = false;
    inheritInfo.queryFlags                     = 0;
    inheritInfo.pipelineStatistics             = 0;

    VkCommandBufferBeginInfo secondaryCommandBufferInfo = {};
    secondaryCommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    secondaryCommandBufferInfo.pNext = NULL;
    secondaryCommandBufferInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
                                       VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    secondaryCommandBufferInfo.pInheritanceInfo = &inheritInfo;

    for (int x = 0; x < numBuffers; x++)
    {
        vkBeginCommandBuffer(info.cmd2s[x], &secondaryCommandBufferInfo);
        vkCmdBindPipeline(info.cmd2s[x], VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
        vkCmdBindDescriptorSets(info.cmd2s[x], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                info.pipeline_layout, 0, NUM_DESCRIPTOR_SETS, info.desc_set.data(),
                                0, NULL);
        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(info.cmd2s[x], 0, 1, &info.vertex_buffer.buf, offsets);
        init_viewports2_array(info, x);
        init_scissors2_array(info, x);
        vkCmdDraw(info.cmd2s[x], 0, 1, 0, 0);
        vkEndCommandBuffer(info.cmd2s[x]);
    }
    // Record Secondary Command Buffer End

    // Record Primary Command Buffer Begin
    VkRenderPassBeginInfo rpBegin;
    rpBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.pNext                    = NULL;
    rpBegin.renderPass               = info.render_pass;
    rpBegin.framebuffer              = info.framebuffers[info.current_buffer];
    rpBegin.renderArea.offset.x      = 0;
    rpBegin.renderArea.offset.y      = 0;
    rpBegin.renderArea.extent.width  = info.width;
    rpBegin.renderArea.extent.height = info.height;
    rpBegin.clearValueCount          = 2;
    rpBegin.pClearValues             = clear_values;

    VkCommandBufferBeginInfo primaryCommandBufferInfo = {};
    primaryCommandBufferInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    primaryCommandBufferInfo.pNext                    = NULL;
    primaryCommandBufferInfo.flags                    = 0;
    primaryCommandBufferInfo.pInheritanceInfo         = NULL;

    vkBeginCommandBuffer(info.cmd, &primaryCommandBufferInfo);
    for (int x = 0; x < numBuffers; x++)
    {
        vkCmdBeginRenderPass(info.cmd, &rpBegin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vkCmdExecuteCommands(info.cmd, 1, &info.cmd2s[x]);
        vkCmdEndRenderPass(info.cmd);
    }
    vkEndCommandBuffer(info.cmd);
    // Record Primary Command Buffer End

    const VkCommandBuffer cmd_bufs[]      = {info.cmd};
    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo[1]            = {};
    submitInfo[0].pNext                   = NULL;
    submitInfo[0].sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].waitSemaphoreCount      = 1;
    submitInfo[0].pWaitSemaphores         = &imageAcquiredSemaphore;
    submitInfo[0].pWaitDstStageMask       = &pipe_stage_flags;
    submitInfo[0].commandBufferCount      = 1;
    submitInfo[0].pCommandBuffers         = cmd_bufs;
    submitInfo[0].signalSemaphoreCount    = 0;
    submitInfo[0].pSignalSemaphores       = NULL;

    // Queue the command buffer for execution
    res = vkQueueSubmit(info.graphics_queue, 1, submitInfo, drawFence);
    ASSERT_EQ(VK_SUCCESS, res);

    Present(info, drawFence);
}

// Details on the following functions that stress various cmd buffer reset methods.
// All of these functions wrap the SecondaryCommandBufferBenchmark() test above,
// adding additional overhead with various reset methods.
// -CommandPoolDestroyBenchmark: Reset command buffers by destroying and re-creating
//   command buffer pool.
// -CommandPoolHardResetBenchmark: Reset the command pool w/ the RELEASE_RESOURCES
//   bit set.
// -CommandPoolSoftResetBenchmark: Reset to command pool w/o the RELEASE_RESOURCES
//   bit set.
// -CommandBufferExplicitHardResetBenchmark: Reset each individual command buffer
//   w/ the RELEASE_RESOURCES bit set.
// -CommandBufferExplicitSoftResetBenchmark: Reset each individual command buffer
//   w/o the RELEASE_RESOURCES bit set.
// -CommandBufferImplicitResetBenchmark: Reset each individual command buffer
//   implicitly by calling "Begin" on previously used cmd buffer.

void CommandPoolDestroyBenchmark(sample_info &info,
                                 VkClearValue *clear_values,
                                 VkFence drawFence,
                                 VkSemaphore imageAcquiredSemaphore,
                                 int numBuffers)
{
    // Save setup cmd buffer data to be restored
    auto saved_cmd_pool = info.cmd_pool;
    auto saved_cb       = info.cmd;
    auto saved_cb2s     = info.cmd2s;
    // Now re-allocate & destroy cmd buffers to stress those calls
    init_command_pool(info, 0);
    init_command_buffer2_array(info, numBuffers);

    SecondaryCommandBufferBenchmark(info, clear_values, drawFence, imageAcquiredSemaphore,
                                    numBuffers);

    destroy_command_pool(info);

    // Restore original cmd buffer data for cleanup
    info.cmd_pool = saved_cmd_pool;
    info.cmd      = saved_cb;
    info.cmd2s    = saved_cb2s;
}

void CommandPoolHardResetBenchmark(sample_info &info,
                                   VkClearValue *clear_values,
                                   VkFence drawFence,
                                   VkSemaphore imageAcquiredSemaphore,
                                   int numBuffers)
{
    SecondaryCommandBufferBenchmark(info, clear_values, drawFence, imageAcquiredSemaphore,
                                    numBuffers);
    reset_command_pool(info, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

void CommandPoolSoftResetBenchmark(sample_info &info,
                                   VkClearValue *clear_values,
                                   VkFence drawFence,
                                   VkSemaphore imageAcquiredSemaphore,
                                   int numBuffers)
{
    SecondaryCommandBufferBenchmark(info, clear_values, drawFence, imageAcquiredSemaphore,
                                    numBuffers);
    reset_command_pool(info, 0);
}

void CommandBufferExplicitHardResetBenchmark(sample_info &info,
                                             VkClearValue *clear_values,
                                             VkFence drawFence,
                                             VkSemaphore imageAcquiredSemaphore,
                                             int numBuffers)
{
    SecondaryCommandBufferBenchmark(info, clear_values, drawFence, imageAcquiredSemaphore,
                                    numBuffers);
    // Explicitly resetting cmd buffers
    reset_command_buffer2_array(info, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void CommandBufferExplicitSoftResetBenchmark(sample_info &info,
                                             VkClearValue *clear_values,
                                             VkFence drawFence,
                                             VkSemaphore imageAcquiredSemaphore,
                                             int numBuffers)
{
    SecondaryCommandBufferBenchmark(info, clear_values, drawFence, imageAcquiredSemaphore,
                                    numBuffers);
    // Explicitly resetting cmd buffers w/ soft reset (don't release resources)
    reset_command_buffer2_array(info, 0);
}

void CommandBufferImplicitResetBenchmark(sample_info &info,
                                         VkClearValue *clear_values,
                                         VkFence drawFence,
                                         VkSemaphore imageAcquiredSemaphore,
                                         int numBuffers)
{
    // Repeated call SCBBenchmark & BeginCmdBuffer calls will implicitly reset each cmd buffer
    SecondaryCommandBufferBenchmark(info, clear_values, drawFence, imageAcquiredSemaphore,
                                    numBuffers);
}

CommandBufferTestParams PrimaryCBHundredIndividualParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = PrimaryCommandBufferBenchmarkHundredIndividual;
    params.story            = "_PrimaryCB_Submit_100_With_1_Draw";
    return params;
}

CommandBufferTestParams PrimaryCBOneWithOneHundredParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = PrimaryCommandBufferBenchmarkOneWithOneHundred;
    params.story            = "_PrimaryCB_Submit_1_With_100_Draw";
    return params;
}

CommandBufferTestParams SecondaryCBParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = SecondaryCommandBufferBenchmark;
    params.story            = "_SecondaryCB_Submit_1_With_100_Draw_In_Individual_Secondary";
    return params;
}

CommandBufferTestParams CommandPoolDestroyParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = CommandPoolDestroyBenchmark;
    params.story            = "_Reset_CBs_With_Destroy_Command_Pool";
    return params;
}

CommandBufferTestParams CommandPoolHardResetParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = CommandPoolHardResetBenchmark;
    params.story            = "_Reset_CBs_With_Hard_Reset_Command_Pool";
    return params;
}

CommandBufferTestParams CommandPoolSoftResetParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = CommandPoolSoftResetBenchmark;
    params.story            = "_Reset_CBs_With_Soft_Reset_Command_Pool";
    return params;
}

CommandBufferTestParams CommandBufferExplicitHardResetParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = CommandBufferExplicitHardResetBenchmark;
    params.story            = "_Reset_CBs_With_Explicit_Hard_Reset_Command_Buffers";
    return params;
}

CommandBufferTestParams CommandBufferExplicitSoftResetParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = CommandBufferExplicitSoftResetBenchmark;
    params.story            = "_Reset_CBs_With_Explicit_Soft_Reset_Command_Buffers";
    return params;
}

CommandBufferTestParams CommandBufferImplicitResetParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = CommandBufferImplicitResetBenchmark;
    params.story            = "_Reset_CBs_With_Implicit_Reset_Command_Buffers";
    return params;
}

TEST_P(VulkanCommandBufferPerfTest, Run)
{
    run();
}

INSTANTIATE_TEST_SUITE_P(,
                         VulkanCommandBufferPerfTest,
                         ::testing::Values(PrimaryCBHundredIndividualParams(),
                                           PrimaryCBOneWithOneHundredParams(),
                                           SecondaryCBParams(),
                                           CommandPoolDestroyParams(),
                                           CommandPoolHardResetParams(),
                                           CommandPoolSoftResetParams(),
                                           CommandBufferExplicitHardResetParams(),
                                           CommandBufferExplicitSoftResetParams(),
                                           CommandBufferImplicitResetParams()));
