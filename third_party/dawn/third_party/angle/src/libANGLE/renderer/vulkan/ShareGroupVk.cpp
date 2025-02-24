//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShareGroupVk.cpp:
//    Implements the class methods for ShareGroupVk.
//

#include "libANGLE/renderer/vulkan/ShareGroupVk.h"

#include "common/debug.h"
#include "common/system_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DeviceVk.h"
#include "libANGLE/renderer/vulkan/ImageVk.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/SyncVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/VkImageImageSiblingVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

namespace
{
// How often monolithic pipelines should be created, if preferMonolithicPipelinesOverLibraries is
// enabled.  Pipeline creation is typically O(hundreds of microseconds).  A value of 2ms is chosen
// arbitrarily; it ensures that there is always at most a single pipeline job in progress, while
// maintaining a high throughput of 500 pipelines / second for heavier applications.
constexpr double kMonolithicPipelineJobPeriod = 0.002;

// Time interval in seconds that we should try to prune default buffer pools.
constexpr double kTimeElapsedForPruneDefaultBufferPool = 0.25;

bool ValidateIdenticalPriority(const egl::ContextMap &contexts, egl::ContextPriority sharedPriority)
{
    if (sharedPriority == egl::ContextPriority::InvalidEnum)
    {
        return false;
    }

    for (auto context : contexts)
    {
        const ContextVk *contextVk = vk::GetImpl(context.second);
        if (contextVk->getPriority() != sharedPriority)
        {
            return false;
        }
    }

    return true;
}
}  // namespace

// Set to true will log bufferpool stats into INFO stream
#define ANGLE_ENABLE_BUFFER_POOL_STATS_LOGGING false

ShareGroupVk::ShareGroupVk(const egl::ShareGroupState &state, vk::Renderer *renderer)
    : ShareGroupImpl(state),
      mRenderer(renderer),
      mCurrentFrameCount(0),
      mContextsPriority(egl::ContextPriority::InvalidEnum),
      mIsContextsPriorityLocked(false),
      mLastMonolithicPipelineJobTime(0)
{
    mLastPruneTime = angle::GetCurrentSystemTime();
}

void ShareGroupVk::onContextAdd()
{
    ASSERT(ValidateIdenticalPriority(getContexts(), mContextsPriority));
}

angle::Result ShareGroupVk::unifyContextsPriority(ContextVk *newContextVk)
{
    const egl::ContextPriority newContextPriority = newContextVk->getPriority();
    ASSERT(newContextPriority != egl::ContextPriority::InvalidEnum);

    if (mContextsPriority == egl::ContextPriority::InvalidEnum)
    {
        ASSERT(!mIsContextsPriorityLocked);
        ASSERT(getContexts().empty());
        mContextsPriority = newContextPriority;
        return angle::Result::Continue;
    }

    static_assert(egl::ContextPriority::Low < egl::ContextPriority::Medium);
    static_assert(egl::ContextPriority::Medium < egl::ContextPriority::High);
    if (mContextsPriority >= newContextPriority || mIsContextsPriorityLocked)
    {
        newContextVk->setPriority(mContextsPriority);
        return angle::Result::Continue;
    }

    ANGLE_TRY(updateContextsPriority(newContextVk, newContextPriority));

    return angle::Result::Continue;
}

angle::Result ShareGroupVk::lockDefaultContextsPriority(ContextVk *contextVk)
{
    constexpr egl::ContextPriority kDefaultPriority = egl::ContextPriority::Medium;
    if (!mIsContextsPriorityLocked)
    {
        if (mContextsPriority != kDefaultPriority)
        {
            ANGLE_TRY(updateContextsPriority(contextVk, kDefaultPriority));
        }
        mIsContextsPriorityLocked = true;
    }
    ASSERT(mContextsPriority == kDefaultPriority);
    return angle::Result::Continue;
}

angle::Result ShareGroupVk::updateContextsPriority(ContextVk *contextVk,
                                                   egl::ContextPriority newPriority)
{
    ASSERT(!mIsContextsPriorityLocked);
    ASSERT(newPriority != egl::ContextPriority::InvalidEnum);
    ASSERT(newPriority != mContextsPriority);
    if (mContextsPriority == egl::ContextPriority::InvalidEnum)
    {
        ASSERT(getContexts().empty());
        mContextsPriority = newPriority;
        return angle::Result::Continue;
    }

    vk::ProtectionTypes protectionTypes;
    protectionTypes.set(contextVk->getProtectionType());
    for (auto context : getContexts())
    {
        protectionTypes.set(vk::GetImpl(context.second)->getProtectionType());
    }

    {
        vk::ScopedQueueSerialIndex index;
        ANGLE_TRY(mRenderer->allocateScopedQueueSerialIndex(&index));
        ANGLE_TRY(mRenderer->submitPriorityDependency(contextVk, protectionTypes, mContextsPriority,
                                                      newPriority, index.get()));
    }

    for (auto context : getContexts())
    {
        ContextVk *sharedContextVk = vk::GetImpl(context.second);

        ASSERT(sharedContextVk->getPriority() == mContextsPriority);
        sharedContextVk->setPriority(newPriority);
    }
    mContextsPriority = newPriority;

    return angle::Result::Continue;
}

void ShareGroupVk::onDestroy(const egl::Display *display)
{
    DisplayVk *displayVk   = vk::GetImpl(display);

    mRefCountedEventsGarbageRecycler.destroy(mRenderer);

    for (std::unique_ptr<vk::BufferPool> &pool : mDefaultBufferPools)
    {
        if (pool)
        {
            // If any context uses display texture share group, it is expected that a
            // BufferBlock may still in used by textures that outlived ShareGroup.  The
            // non-empty BufferBlock will be put into Renderer's orphan list instead.
            pool->destroy(mRenderer, mState.hasAnyContextWithDisplayTextureShareGroup());
        }
    }

    mPipelineLayoutCache.destroy(mRenderer);
    mDescriptorSetLayoutCache.destroy(mRenderer);

    mMetaDescriptorPools[DescriptorSetIndex::UniformsAndXfb].destroy(mRenderer);
    mMetaDescriptorPools[DescriptorSetIndex::Texture].destroy(mRenderer);
    mMetaDescriptorPools[DescriptorSetIndex::ShaderResource].destroy(mRenderer);

    mFramebufferCache.destroy(mRenderer);
    resetPrevTexture();

    mVertexInputGraphicsPipelineCache.destroy(displayVk);
    mFragmentOutputGraphicsPipelineCache.destroy(displayVk);
}

angle::Result ShareGroupVk::onMutableTextureUpload(ContextVk *contextVk, TextureVk *newTexture)
{
    return mTextureUpload.onMutableTextureUpload(contextVk, newTexture);
}

void ShareGroupVk::onTextureRelease(TextureVk *textureVk)
{
    mTextureUpload.onTextureRelease(textureVk);
}

angle::Result ShareGroupVk::scheduleMonolithicPipelineCreationTask(
    ContextVk *contextVk,
    vk::WaitableMonolithicPipelineCreationTask *taskOut)
{
    ASSERT(contextVk->getFeatures().preferMonolithicPipelinesOverLibraries.enabled);

    // Limit to a single task to avoid hogging all the cores.
    if (mMonolithicPipelineCreationEvent && !mMonolithicPipelineCreationEvent->isReady())
    {
        return angle::Result::Continue;
    }

    // Additionally, rate limit the job postings.
    double currentTime = angle::GetCurrentSystemTime();
    if (currentTime - mLastMonolithicPipelineJobTime < kMonolithicPipelineJobPeriod)
    {
        return angle::Result::Continue;
    }

    mLastMonolithicPipelineJobTime = currentTime;

    const vk::RenderPass *compatibleRenderPass = nullptr;
    // Pull in a compatible RenderPass to be used by the task.  This is done at the last minute,
    // just before the task is scheduled, to minimize the time this reference to the render pass
    // cache is held.  If the render pass cache needs to be cleared, the main thread will wait
    // for the job to complete.
    ANGLE_TRY(contextVk->getCompatibleRenderPass(taskOut->getTask()->getRenderPassDesc(),
                                                 &compatibleRenderPass));
    taskOut->setRenderPass(compatibleRenderPass);

    mMonolithicPipelineCreationEvent =
        mRenderer->getGlobalOps()->postMultiThreadWorkerTask(taskOut->getTask());

    taskOut->onSchedule(mMonolithicPipelineCreationEvent);

    return angle::Result::Continue;
}

void ShareGroupVk::waitForCurrentMonolithicPipelineCreationTask()
{
    if (mMonolithicPipelineCreationEvent)
    {
        mMonolithicPipelineCreationEvent->wait();
    }
}

angle::Result TextureUpload::onMutableTextureUpload(ContextVk *contextVk, TextureVk *newTexture)
{
    // This feature is currently disabled in the case of display-level texture sharing.
    ASSERT(!contextVk->hasDisplayTextureShareGroup());
    ASSERT(!newTexture->isImmutable());
    ASSERT(mPrevUploadedMutableTexture == nullptr || !mPrevUploadedMutableTexture->isImmutable());

    // If the previous texture is null, it should be set to the current texture. We also have to
    // make sure that the previous texture pointer is still a mutable texture. Otherwise, we skip
    // the optimization.
    if (mPrevUploadedMutableTexture == nullptr)
    {
        mPrevUploadedMutableTexture = newTexture;
        return angle::Result::Continue;
    }

    // Skip the optimization if we have not switched to a new texture yet.
    if (mPrevUploadedMutableTexture == newTexture)
    {
        return angle::Result::Continue;
    }

    // If the mutable texture is consistently specified, we initialize a full mip chain for it.
    if (mPrevUploadedMutableTexture->isMutableTextureConsistentlySpecifiedForFlush())
    {
        ANGLE_TRY(mPrevUploadedMutableTexture->ensureImageInitialized(
            contextVk, ImageMipLevels::EnabledLevels));
        contextVk->getPerfCounters().mutableTexturesUploaded++;
    }

    // Update the mutable texture pointer with the new pointer for the next potential flush.
    mPrevUploadedMutableTexture = newTexture;

    return angle::Result::Continue;
}

void TextureUpload::onTextureRelease(TextureVk *textureVk)
{
    if (mPrevUploadedMutableTexture == textureVk)
    {
        resetPrevTexture();
    }
}

void ShareGroupVk::onFramebufferBoundary()
{
    if (isDueForBufferPoolPrune())
    {
        pruneDefaultBufferPools();
    }

    // Always clean up event garbage and destroy the excessive free list at frame boundary.
    cleanupRefCountedEventGarbage();

    mCurrentFrameCount++;
}

vk::BufferPool *ShareGroupVk::getDefaultBufferPool(VkDeviceSize size,
                                                   uint32_t memoryTypeIndex,
                                                   BufferUsageType usageType)
{
    if (!mDefaultBufferPools[memoryTypeIndex])
    {
        const vk::Allocator &allocator = mRenderer->getAllocator();
        VkBufferUsageFlags usageFlags  = GetDefaultBufferUsageFlags(mRenderer);

        VkMemoryPropertyFlags memoryPropertyFlags;
        allocator.getMemoryTypeProperties(memoryTypeIndex, &memoryPropertyFlags);

        std::unique_ptr<vk::BufferPool> pool  = std::make_unique<vk::BufferPool>();
        vma::VirtualBlockCreateFlags vmaFlags = vma::VirtualBlockCreateFlagBits::GENERAL;
        pool->initWithFlags(mRenderer, vmaFlags, usageFlags, 0, memoryTypeIndex,
                            memoryPropertyFlags);
        mDefaultBufferPools[memoryTypeIndex] = std::move(pool);
    }

    return mDefaultBufferPools[memoryTypeIndex].get();
}

void ShareGroupVk::pruneDefaultBufferPools()
{
    mLastPruneTime = angle::GetCurrentSystemTime();

    // Bail out if no suballocation have been destroyed since last prune.
    if (mRenderer->getSuballocationDestroyedSize() == 0)
    {
        return;
    }

    for (std::unique_ptr<vk::BufferPool> &pool : mDefaultBufferPools)
    {
        if (pool)
        {
            pool->pruneEmptyBuffers(mRenderer);
        }
    }

    mRenderer->onBufferPoolPrune();

#if ANGLE_ENABLE_BUFFER_POOL_STATS_LOGGING
    logBufferPools();
#endif
}

bool ShareGroupVk::isDueForBufferPoolPrune()
{
    // Ensure we periodically prune to maintain the heuristic information
    double timeElapsed = angle::GetCurrentSystemTime() - mLastPruneTime;
    if (timeElapsed > kTimeElapsedForPruneDefaultBufferPool)
    {
        return true;
    }

    // If we have destroyed a lot of memory, also prune to ensure memory gets freed as soon as
    // possible
    if (mRenderer->getSuballocationDestroyedSize() >= kMaxTotalEmptyBufferBytes)
    {
        return true;
    }

    return false;
}

void ShareGroupVk::calculateTotalBufferCount(size_t *bufferCount, VkDeviceSize *totalSize) const
{
    *bufferCount = 0;
    *totalSize   = 0;
    for (const std::unique_ptr<vk::BufferPool> &pool : mDefaultBufferPools)
    {
        if (pool)
        {
            *bufferCount += pool->getBufferCount();
            *totalSize += pool->getMemorySize();
        }
    }
}

void ShareGroupVk::logBufferPools() const
{
    for (size_t i = 0; i < mDefaultBufferPools.size(); i++)
    {
        const std::unique_ptr<vk::BufferPool> &pool = mDefaultBufferPools[i];
        if (pool && pool->getBufferCount() > 0)
        {
            std::ostringstream log;
            pool->addStats(&log);
            INFO() << "Pool[" << i << "]:" << log.str();
        }
    }
}
}  // namespace rx
