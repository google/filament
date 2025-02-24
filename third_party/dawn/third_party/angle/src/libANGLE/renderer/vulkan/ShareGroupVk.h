//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShareGroupVk.h:
//    Defines the class interface for ShareGroupVk, implementing ShareGroupImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SHAREGROUPVK_H_
#define LIBANGLE_RENDERER_VULKAN_SHAREGROUPVK_H_

#include "libANGLE/renderer/ShareGroupImpl.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
constexpr VkDeviceSize kMaxTotalEmptyBufferBytes = 16 * 1024 * 1024;

class TextureUpload
{
  public:
    TextureUpload() { mPrevUploadedMutableTexture = nullptr; }
    ~TextureUpload() { resetPrevTexture(); }
    angle::Result onMutableTextureUpload(ContextVk *contextVk, TextureVk *newTexture);
    void onTextureRelease(TextureVk *textureVk);
    void resetPrevTexture() { mPrevUploadedMutableTexture = nullptr; }

  private:
    // Keep track of the previously stored texture. Used to flush mutable textures.
    TextureVk *mPrevUploadedMutableTexture;
};

class ShareGroupVk : public ShareGroupImpl
{
  public:
    ShareGroupVk(const egl::ShareGroupState &state, vk::Renderer *renderer);
    void onDestroy(const egl::Display *display) override;

    void onContextAdd() override;

    FramebufferCache &getFramebufferCache() { return mFramebufferCache; }

    bool hasAnyContextWithRobustness() const { return mState.hasAnyContextWithRobustness(); }

    // PipelineLayoutCache and DescriptorSetLayoutCache can be shared between multiple threads
    // accessing them via shared contexts. The ShareGroup locks around gl entrypoints ensuring
    // synchronous update to the caches.
    PipelineLayoutCache &getPipelineLayoutCache() { return mPipelineLayoutCache; }
    DescriptorSetLayoutCache &getDescriptorSetLayoutCache() { return mDescriptorSetLayoutCache; }
    const egl::ContextMap &getContexts() const { return mState.getContexts(); }
    vk::DescriptorSetArray<vk::MetaDescriptorPool> &getMetaDescriptorPools()
    {
        return mMetaDescriptorPools;
    }

    // Used to flush the mutable textures more often.
    angle::Result onMutableTextureUpload(ContextVk *contextVk, TextureVk *newTexture);

    vk::BufferPool *getDefaultBufferPool(VkDeviceSize size,
                                         uint32_t memoryTypeIndex,
                                         BufferUsageType usageType);

    void pruneDefaultBufferPools();

    void calculateTotalBufferCount(size_t *bufferCount, VkDeviceSize *totalSize) const;
    void logBufferPools() const;

    // Temporary workaround until VkSemaphore(s) will be used between different priorities.
    angle::Result unifyContextsPriority(ContextVk *newContextVk);
    // Temporary workaround until VkSemaphore(s) will be used between different priorities.
    angle::Result lockDefaultContextsPriority(ContextVk *contextVk);

    UpdateDescriptorSetsBuilder *getUpdateDescriptorSetsBuilder()
    {
        return &mUpdateDescriptorSetsBuilder;
    }

    void onTextureRelease(TextureVk *textureVk);

    VertexInputGraphicsPipelineCache *getVertexInputGraphicsPipelineCache()
    {
        return &mVertexInputGraphicsPipelineCache;
    }
    FragmentOutputGraphicsPipelineCache *getFragmentOutputGraphicsPipelineCache()
    {
        return &mFragmentOutputGraphicsPipelineCache;
    }

    angle::Result scheduleMonolithicPipelineCreationTask(
        ContextVk *contextVk,
        vk::WaitableMonolithicPipelineCreationTask *taskOut);
    void waitForCurrentMonolithicPipelineCreationTask();

    vk::RefCountedEventsGarbageRecycler *getRefCountedEventsGarbageRecycler()
    {
        return &mRefCountedEventsGarbageRecycler;
    }
    void cleanupRefCountedEventGarbage() { mRefCountedEventsGarbageRecycler.cleanup(mRenderer); }
    void cleanupExcessiveRefCountedEventGarbage()
    {
        // TODO: b/336844257 needs tune.
        constexpr size_t kExcessiveGarbageCountThreshold = 256;
        if (mRefCountedEventsGarbageRecycler.getGarbageCount() > kExcessiveGarbageCountThreshold)
        {
            mRefCountedEventsGarbageRecycler.cleanup(mRenderer);
        }
    }

    void onFramebufferBoundary();
    uint32_t getCurrentFrameCount() const { return mCurrentFrameCount; }

  private:
    angle::Result updateContextsPriority(ContextVk *contextVk, egl::ContextPriority newPriority);

    bool isDueForBufferPoolPrune();

    vk::Renderer *mRenderer;

    // Tracks the total number of frames rendered.
    uint32_t mCurrentFrameCount;

    // VkFramebuffer caches
    FramebufferCache mFramebufferCache;

    void resetPrevTexture() { mTextureUpload.resetPrevTexture(); }

    // ANGLE uses a PipelineLayout cache to store compatible pipeline layouts.
    PipelineLayoutCache mPipelineLayoutCache;

    // DescriptorSetLayouts are also managed in a cache.
    DescriptorSetLayoutCache mDescriptorSetLayoutCache;

    // Descriptor set caches
    vk::DescriptorSetArray<vk::MetaDescriptorPool> mMetaDescriptorPools;

    // Priority of all Contexts in the context set
    egl::ContextPriority mContextsPriority;
    bool mIsContextsPriorityLocked;

    // Storage for vkUpdateDescriptorSets
    UpdateDescriptorSetsBuilder mUpdateDescriptorSetsBuilder;

    // The per shared group buffer pools that all buffers should sub-allocate from.
    vk::BufferPoolPointerArray mDefaultBufferPools;

    // The system time when last pruneEmptyBuffer gets called.
    double mLastPruneTime;

    // Used when VK_EXT_graphics_pipeline_library is available, the vertex input and fragment output
    // partial pipelines are created in the following caches.  These caches are in the share group
    // because linked pipelines using these pipeline libraries are referenced from
    // ProgramExecutableVk, and as such must stay alive as long as the program may be alive.
    VertexInputGraphicsPipelineCache mVertexInputGraphicsPipelineCache;
    FragmentOutputGraphicsPipelineCache mFragmentOutputGraphicsPipelineCache;

    // The system time when the last monolithic pipeline creation job was launched.  This is
    // rate-limited to avoid hogging all cores and interfering with the application threads.  A
    // single pipeline creation job is currently supported.
    double mLastMonolithicPipelineJobTime;
    std::shared_ptr<angle::WaitableEvent> mMonolithicPipelineCreationEvent;

    // Texture update manager used to flush uploaded mutable textures.
    TextureUpload mTextureUpload;

    // Holds RefCountedEvent that are free and ready to reuse
    vk::RefCountedEventsGarbageRecycler mRefCountedEventsGarbageRecycler;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SHAREGROUPVK_H_
