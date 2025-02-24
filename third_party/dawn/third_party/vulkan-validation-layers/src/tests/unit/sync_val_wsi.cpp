/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../framework/layer_validation_tests.h"

struct NegativeSyncValWsi : public VkSyncValTest {};

// Wrap FAIL:
//  * DRY for common messages
//  * for test stability reasons sometimes cleanup code is required *prior* to the return hidden in FAIL
//  * result_arg_ *can* (should) have side-effect, but is referenced exactly once
//  * label_ must be converitble to bool, and *should* *not* have side-effects
//    * "{}" or ";" are valid clean_ values for noop
#define REQUIRE_SUCCESS(result_arg_, label_)                            \
    {                                                                   \
        const VkResult result_ = (result_arg_);                         \
        if (result_ != VK_SUCCESS) {                                    \
            {                                                           \
                m_device->Wait();                                       \
            }                                                           \
            if (bool(label_)) {                                         \
                FAIL() << string_VkResult(result_) << ": " << (label_); \
            } else {                                                    \
                FAIL() << string_VkResult(result_);                     \
            }                                                           \
        }                                                               \
    }

TEST_F(NegativeSyncValWsi, PresentAcquire) {
    TEST_DESCRIPTION("Try destroying a swapchain presentable image with vkDestroyImage");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());

    const std::vector<VkImage> images = m_swapchain.GetImages();
    std::vector<bool> image_used(images.size(), false);
    vkt::Fence fence(*m_device);

    // Loop through the indices until we find one we are reusing...
    // When fence is non-null this can timeout so we need to track results
    // Acquire can always timeout, so we need to track results
    auto acquire_used_image_semaphore = [this, &image_used](const vkt::Semaphore& sem, uint32_t& index) {
        VkResult result = VK_SUCCESS;
        while (true) {
            index = m_swapchain.AcquireNextImage(sem, kWaitTimeout, &result);
            if ((result != VK_SUCCESS) || image_used[index]) break;

            result = m_default_queue->Present(m_swapchain, index, sem);
            if (result != VK_SUCCESS) break;
            image_used[index] = true;
        }
        return result;
    };
    auto acquire_used_image_fence = [this, &image_used](const vkt::Fence& fence, uint32_t& index) {
        VkResult result = VK_SUCCESS;
        while (true) {
            index = m_swapchain.AcquireNextImage(fence, kWaitTimeout, &result);
            if ((result != VK_SUCCESS) || image_used[index]) break;

            result = fence.Wait(kWaitTimeout);
            fence.Reset();
            m_default_queue->Present(m_swapchain, index, vkt::no_semaphore);

            if (result != VK_SUCCESS) break;
            image_used[index] = true;
        }
        return result;
    };

    auto write_barrier_cb = [this](const VkImage h_image, VkImageLayout from, VkImageLayout to) {
        VkImageSubresourceRange full_image{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
        image_barrier.srcAccessMask = 0U;
        image_barrier.dstAccessMask = 0U;
        image_barrier.oldLayout = from;
        image_barrier.newLayout = to;
        image_barrier.image = h_image;

        image_barrier.subresourceRange = full_image;
        m_command_buffer.Begin();
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);
        m_command_buffer.End();
    };

    // Transition swapchain images to PRESENT_SRC layout for presentation
    for (VkImage image : images) {
        write_barrier_cb(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
        m_command_buffer.Reset();
    }

    uint32_t acquired_index = 0;
    REQUIRE_SUCCESS(acquire_used_image_fence(fence, acquired_index), "acquire_used_image");

    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Look for errors between the acquire and first use...
    // No sync operations...
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-PRESENT");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Sync operations that should ignore present operations
    m_device->Wait();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-PRESENT");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Finally we wait for the fence associated with the acquire
    REQUIRE_SUCCESS(vk::WaitForFences(m_device->handle(), 1, &fence.handle(), VK_TRUE, kWaitTimeout), "WaitForFences");
    fence.Reset();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Release the image back to the present engine, so we don't run out
    m_default_queue->Present(m_swapchain, acquired_index, vkt::no_semaphore);  // present without fence can't timeout

    vkt::Semaphore sem(*m_device);
    REQUIRE_SUCCESS(acquire_used_image_semaphore(sem, acquired_index), "acquire_used_image");

    m_command_buffer.Reset();
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // The wait mask doesn't match the operations in the command buffer
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_default_queue->Submit(m_command_buffer, vkt::Wait(sem, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT));
    m_errorMonitor->VerifyFound();

    // Now then wait mask matches the operations in the command buffer
    m_default_queue->Submit(m_command_buffer, vkt::Wait(sem, VK_PIPELINE_STAGE_TRANSFER_BIT));

    // Try presenting without waiting for the ILT to finish
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    m_default_queue->Present(m_swapchain, acquired_index, vkt::no_semaphore);  // present without fence can't timeout
    m_errorMonitor->VerifyFound();

    // Let the ILT complete, and the release the image back
    m_device->Wait();
    m_default_queue->Present(m_swapchain, acquired_index, vkt::no_semaphore);  // present without fence can't timeout

    REQUIRE_SUCCESS(acquire_used_image_fence(fence, acquired_index), "acquire_used_index");
    REQUIRE_SUCCESS(fence.Wait(kWaitTimeout), "WaitForFences");

    m_command_buffer.Reset();
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    fence.Reset();
    m_default_queue->Submit(m_command_buffer, vkt::Signal(sem));

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    m_default_queue->Present(m_swapchain, acquired_index, vkt::no_semaphore);  // present without fence can't timeout
    m_errorMonitor->VerifyFound();

    m_default_queue->Present(m_swapchain, acquired_index, sem);  // present without fence can't timeout
    m_device->Wait();
}

// TODO: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5240
TEST_F(NegativeSyncValWsi, SubmitDoesNotWaitForAcquire) {
    TEST_DESCRIPTION("Submit does not wait for the swapchain acquire semaphore");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());
    RETURN_IF_SKIP(InitSwapchain());
    const vkt::Semaphore acquire_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.srcAccessMask = 0;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_command_buffer.End();

    // TODO: current implementation does not report that the image is still being read by the acquire
    // and at the same time it is being transitioned (there is no wait on the acquire semaphore).
    // Fix this and ensure the following submit triggers validation error.
    m_default_queue->Submit2(m_command_buffer);
    m_device->Wait();
}

TEST_F(NegativeSyncValWsi, PresentDoesNotWaitForSubmit2) {
    TEST_DESCRIPTION("Present does not specify semaphore to wait for submit.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.srcAccessMask = 0;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layout_transition.subresourceRange.baseMipLevel = 0;
    layout_transition.subresourceRange.levelCount = 1;
    layout_transition.subresourceRange.baseArrayLayer = 0;
    layout_transition.subresourceRange.layerCount = 1;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_command_buffer.End();

    m_default_queue->Submit2(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                             vkt::Signal(submit_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT));

    // DO NOT wait for submit. This should generate present after write (ILT) harard.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValWsi, PresentDoesNotWaitForSubmit) {
    TEST_DESCRIPTION("Present does not specify semaphore to wait for submit.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());
    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    VkImageMemoryBarrier layout_transition = vku::InitStructHelper();
    layout_transition.srcAccessMask = 0;
    layout_transition.dstAccessMask = 0;

    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layout_transition.subresourceRange.baseMipLevel = 0;
    layout_transition.subresourceRange.levelCount = 1;
    layout_transition.subresourceRange.baseArrayLayer = 0;
    layout_transition.subresourceRange.layerCount = 1;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &layout_transition);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                            vkt::Signal(submit_semaphore));

    // DO NOT wait for submit. This should generate present after write (ILT) harard.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-PRESENT-AFTER-WRITE");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}
