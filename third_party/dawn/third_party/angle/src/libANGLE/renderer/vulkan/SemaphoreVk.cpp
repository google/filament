// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SemaphoreVk.cpp: Defines the class interface for SemaphoreVk, implementing
// SemaphoreImpl.

#include "libANGLE/renderer/vulkan/SemaphoreVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

SemaphoreVk::SemaphoreVk() = default;

SemaphoreVk::~SemaphoreVk() = default;

void SemaphoreVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    contextVk->addGarbage(&mSemaphore);
}

angle::Result SemaphoreVk::importFd(gl::Context *context, gl::HandleType handleType, GLint fd)
{
    ContextVk *contextVk = vk::GetImpl(context);

    switch (handleType)
    {
        case gl::HandleType::OpaqueFd:
            return importOpaqueFd(contextVk, fd);

        default:
            ANGLE_VK_UNREACHABLE(contextVk);
            return angle::Result::Stop;
    }
}

angle::Result SemaphoreVk::importZirconHandle(gl::Context *context,
                                              gl::HandleType handleType,
                                              GLuint handle)
{
    ContextVk *contextVk = vk::GetImpl(context);

    switch (handleType)
    {
        case gl::HandleType::ZirconEvent:
            return importZirconEvent(contextVk, handle);

        default:
            ANGLE_VK_UNREACHABLE(contextVk);
            return angle::Result::Stop;
    }
}

angle::Result SemaphoreVk::wait(gl::Context *context,
                                const gl::BufferBarrierVector &bufferBarriers,
                                const gl::TextureBarrierVector &textureBarriers)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (!bufferBarriers.empty() || !textureBarriers.empty())
    {
        // Create one global memory barrier to cover all barriers.
        ANGLE_TRY(contextVk->syncExternalMemory());
    }

    if (!bufferBarriers.empty())
    {
        // Perform a queue ownership transfer for each buffer.
        for (gl::Buffer *buffer : bufferBarriers)
        {
            BufferVk *bufferVk             = vk::GetImpl(buffer);
            vk::BufferHelper &bufferHelper = bufferVk->getBuffer();

            vk::CommandBufferAccess access;
            vk::OutsideRenderPassCommandBuffer *commandBuffer;
            access.onBufferExternalAcquireRelease(&bufferHelper);
            ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

            // Queue ownership transfer.
            bufferHelper.acquireFromExternal(vk::kExternalDeviceQueueIndex,
                                             contextVk->getDeviceQueueIndex(), commandBuffer);
        }
    }

    if (!textureBarriers.empty())
    {
        // Perform a queue ownership transfer for each texture.  Additionally, we are being
        // informed that the layout of the image has been externally transitioned, so we need to
        // update our internal state tracking.
        for (const gl::TextureAndLayout &textureBarrier : textureBarriers)
        {
            TextureVk *textureVk   = vk::GetImpl(textureBarrier.texture);
            vk::ImageHelper &image = textureVk->getImage();
            vk::ImageLayout layout =
                vk::GetImageLayoutFromGLImageLayout(contextVk, textureBarrier.layout);

            vk::CommandBufferAccess access;
            vk::OutsideRenderPassCommandBuffer *commandBuffer;
            access.onExternalAcquireRelease(&image);
            ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

            // Image should not be accessed while unowned. Emulated formats may have staged updates
            // to clear the image after initialization.
            ASSERT(!image.hasStagedUpdatesInAllocatedLevels() || image.hasEmulatedImageChannels());

            // Queue ownership transfer and layout transition.
            image.acquireFromExternal(contextVk, vk::kExternalDeviceQueueIndex,
                                      contextVk->getDeviceQueueIndex(), layout, commandBuffer);
        }
    }

    contextVk->addWaitSemaphore(mSemaphore.getHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    return angle::Result::Continue;
}

angle::Result SemaphoreVk::signal(gl::Context *context,
                                  const gl::BufferBarrierVector &bufferBarriers,
                                  const gl::TextureBarrierVector &textureBarriers)
{
    ContextVk *contextVk   = vk::GetImpl(context);

    if (!bufferBarriers.empty())
    {
        // Perform a queue ownership transfer for each buffer.
        for (gl::Buffer *buffer : bufferBarriers)
        {
            BufferVk *bufferVk             = vk::GetImpl(buffer);
            vk::BufferHelper &bufferHelper = bufferVk->getBuffer();

            ANGLE_TRY(contextVk->onBufferReleaseToExternal(bufferHelper));
            vk::CommandBufferAccess access;
            vk::OutsideRenderPassCommandBuffer *commandBuffer;
            access.onBufferExternalAcquireRelease(&bufferHelper);
            ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

            // Queue ownership transfer.
            bufferHelper.releaseToExternal(vk::kExternalDeviceQueueIndex, commandBuffer);
        }
    }

    if (!textureBarriers.empty())
    {
        // Perform a queue ownership transfer for each texture.  Additionally, transition the image
        // to the requested layout.
        for (const gl::TextureAndLayout &textureBarrier : textureBarriers)
        {
            TextureVk *textureVk   = vk::GetImpl(textureBarrier.texture);
            vk::ImageHelper &image = textureVk->getImage();
            vk::ImageLayout layout =
                vk::GetImageLayoutFromGLImageLayout(contextVk, textureBarrier.layout);

            // Don't transition to Undefined layout.  If external wants to transition the image away
            // from Undefined after this operation, it's perfectly fine to keep the layout as is in
            // ANGLE.  Note that vk::ImageHelper doesn't expect transitions to Undefined.
            if (layout == vk::ImageLayout::Undefined)
            {
                layout = image.getCurrentImageLayout();
            }

            ANGLE_TRY(textureVk->ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

            ANGLE_TRY(contextVk->onImageReleaseToExternal(image));
            vk::CommandBufferAccess access;
            vk::OutsideRenderPassCommandBuffer *commandBuffer;
            access.onExternalAcquireRelease(&image);
            ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

            // Queue ownership transfer and layout transition.
            image.releaseToExternal(contextVk, vk::kExternalDeviceQueueIndex, layout,
                                    commandBuffer);
        }
    }

    if (!bufferBarriers.empty() || !textureBarriers.empty())
    {
        // Create one global memory barrier to cover all barriers.
        ANGLE_TRY(contextVk->syncExternalMemory());
    }

    return contextVk->flushAndSubmitCommands(&mSemaphore, nullptr,
                                             RenderPassClosureReason::ExternalSemaphoreSignal);
}

angle::Result SemaphoreVk::importOpaqueFd(ContextVk *contextVk, GLint fd)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    if (!mSemaphore.valid())
    {
        mSemaphore.init(renderer->getDevice());
    }

    ASSERT(mSemaphore.valid());

    VkImportSemaphoreFdInfoKHR importSemaphoreFdInfo = {};
    importSemaphoreFdInfo.sType      = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    importSemaphoreFdInfo.semaphore  = mSemaphore.getHandle();
    importSemaphoreFdInfo.flags      = 0;
    importSemaphoreFdInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    importSemaphoreFdInfo.fd         = fd;

    ANGLE_VK_TRY(contextVk, vkImportSemaphoreFdKHR(renderer->getDevice(), &importSemaphoreFdInfo));

    return angle::Result::Continue;
}

angle::Result SemaphoreVk::importZirconEvent(ContextVk *contextVk, GLuint handle)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    if (!mSemaphore.valid())
    {
        mSemaphore.init(renderer->getDevice());
    }

    ASSERT(mSemaphore.valid());

    VkImportSemaphoreZirconHandleInfoFUCHSIA importSemaphoreZirconHandleInfo = {};
    importSemaphoreZirconHandleInfo.sType =
        VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA;
    importSemaphoreZirconHandleInfo.semaphore = mSemaphore.getHandle();
    importSemaphoreZirconHandleInfo.flags     = 0;
    importSemaphoreZirconHandleInfo.handleType =
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;
    importSemaphoreZirconHandleInfo.zirconHandle = handle;

    // TODO(spang): Add vkImportSemaphoreZirconHandleFUCHSIA to volk.
    static PFN_vkImportSemaphoreZirconHandleFUCHSIA vkImportSemaphoreZirconHandleFUCHSIA =
        reinterpret_cast<PFN_vkImportSemaphoreZirconHandleFUCHSIA>(
            vkGetInstanceProcAddr(renderer->getInstance(), "vkImportSemaphoreZirconHandleFUCHSIA"));

    ANGLE_VK_TRY(contextVk, vkImportSemaphoreZirconHandleFUCHSIA(renderer->getDevice(),
                                                                 &importSemaphoreZirconHandleInfo));

    return angle::Result::Continue;
}

}  // namespace rx
