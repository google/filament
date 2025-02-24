//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVk.cpp:
//    Implements the class methods for BufferVk.
//

#include "libANGLE/renderer/vulkan/BufferVk.h"

#include "common/FixedVector.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
VkBufferUsageFlags GetDefaultBufferUsageFlags(vk::Renderer *renderer)
{
    // We could potentially use multiple backing buffers for different usages.
    // For now keep a single buffer with all relevant usage flags.
    VkBufferUsageFlags defaultBufferUsageFlags =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    if (renderer->getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        defaultBufferUsageFlags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT |
                                   VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
    }
    return defaultBufferUsageFlags;
}

namespace
{
constexpr VkMemoryPropertyFlags kDeviceLocalFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
constexpr VkMemoryPropertyFlags kDeviceLocalHostCoherentFlags =
    (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
constexpr VkMemoryPropertyFlags kHostCachedFlags =
    (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
     VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
constexpr VkMemoryPropertyFlags kHostUncachedFlags =
    (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
constexpr VkMemoryPropertyFlags kHostCachedNonCoherentFlags =
    (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

// Vertex attribute buffers are used as storage buffers for conversion in compute, where access to
// the buffer is made in 4-byte chunks.  Assume the size of the buffer is 4k+n where n is in [0, 3).
// On some hardware, reading 4 bytes from address 4k returns 0, making it impossible to read the
// last n bytes.  By rounding up the buffer sizes to a multiple of 4, the problem is alleviated.
constexpr size_t kBufferSizeGranularity = 4;
static_assert(gl::isPow2(kBufferSizeGranularity), "use as alignment, must be power of two");

// Start with a fairly small buffer size. We can increase this dynamically as we convert more data.
constexpr size_t kConvertedArrayBufferInitialSize = 1024 * 8;

// Buffers that have a static usage pattern will be allocated in
// device local memory to speed up access to and from the GPU.
// Dynamic usage patterns or that are frequently mapped
// will now request host cached memory to speed up access from the CPU.
VkMemoryPropertyFlags GetPreferredMemoryType(vk::Renderer *renderer,
                                             gl::BufferBinding target,
                                             gl::BufferUsage usage)
{
    if (target == gl::BufferBinding::PixelUnpack)
    {
        return kHostCachedFlags;
    }

    switch (usage)
    {
        case gl::BufferUsage::StaticCopy:
        case gl::BufferUsage::StaticDraw:
        case gl::BufferUsage::StaticRead:
            // For static usage, request a device local memory
            return renderer->getFeatures().preferDeviceLocalMemoryHostVisible.enabled
                       ? kDeviceLocalHostCoherentFlags
                       : kDeviceLocalFlags;
        case gl::BufferUsage::DynamicDraw:
        case gl::BufferUsage::StreamDraw:
            // For non-static usage where the CPU performs a write-only access, request
            // a host uncached memory
            return renderer->getFeatures().preferHostCachedForNonStaticBufferUsage.enabled
                       ? kHostCachedFlags
                       : kHostUncachedFlags;
        case gl::BufferUsage::DynamicCopy:
        case gl::BufferUsage::DynamicRead:
        case gl::BufferUsage::StreamCopy:
        case gl::BufferUsage::StreamRead:
            // For all other types of usage, request a host cached memory
            return renderer->getFeatures()
                           .preferCachedNoncoherentForDynamicStreamBufferUsage.enabled
                       ? kHostCachedNonCoherentFlags
                       : kHostCachedFlags;
        default:
            UNREACHABLE();
            return kHostCachedFlags;
    }
}

VkMemoryPropertyFlags GetStorageMemoryType(vk::Renderer *renderer,
                                           GLbitfield storageFlags,
                                           bool externalBuffer)
{
    const bool hasMapAccess =
        (storageFlags & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT)) != 0;

    if (renderer->getFeatures().preferDeviceLocalMemoryHostVisible.enabled)
    {
        const bool canUpdate = (storageFlags & GL_DYNAMIC_STORAGE_BIT_EXT) != 0;
        if (canUpdate || hasMapAccess || externalBuffer)
        {
            // We currently allocate coherent memory for persistently mapped buffers.
            // GL_EXT_buffer_storage allows non-coherent memory, but currently the implementation of
            // |glMemoryBarrier(CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT)| relies on the mapping being
            // coherent.
            //
            // If persistently mapped buffers ever use non-coherent memory, then said
            // |glMemoryBarrier| call must result in |vkInvalidateMappedMemoryRanges| for all
            // persistently mapped buffers.
            return kDeviceLocalHostCoherentFlags;
        }
        return kDeviceLocalFlags;
    }

    return hasMapAccess ? kHostCachedFlags : kDeviceLocalFlags;
}

bool ShouldAllocateNewMemoryForUpdate(ContextVk *contextVk, size_t subDataSize, size_t bufferSize)
{
    // A sub-data update with size > 50% of buffer size meets the threshold to acquire a new
    // BufferHelper from the pool.
    size_t halfBufferSize = bufferSize / 2;
    if (subDataSize > halfBufferSize)
    {
        return true;
    }

    // If the GPU is busy, it is possible to use the CPU for updating sub-data instead, but since it
    // would need to create a duplicate of the buffer, a large enough buffer copy could result in a
    // performance regression.
    if (contextVk->getFeatures().preferCPUForBufferSubData.enabled)
    {
        // If the buffer is small enough, the cost of barrier associated with the GPU copy likely
        // exceeds the overhead with the CPU copy. Duplicating the buffer allows the CPU to write to
        // the buffer immediately, thus avoiding the barrier that prevents parallel operation.
        constexpr size_t kCpuCopyBufferSizeThreshold = 32 * 1024;
        if (bufferSize < kCpuCopyBufferSizeThreshold)
        {
            return true;
        }

        // To use CPU for the sub-data update in larger buffers, the update should be sizable enough
        // compared to the whole buffer size. The threshold is chosen based on perf data collected
        // from Pixel devices. At 1/8 of buffer size, the CPU overhead associated with extra data
        // copy weighs less than serialization caused by barriers.
        size_t subDataThreshold = bufferSize / 8;
        if (subDataSize > subDataThreshold)
        {
            return true;
        }
    }

    return false;
}

bool ShouldUseCPUToCopyData(ContextVk *contextVk,
                            const vk::BufferHelper &buffer,
                            size_t copySize,
                            size_t bufferSize)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // If the buffer is not host-visible, or if it's busy on the GPU, can't read from it from the
    // CPU
    if (!buffer.isHostVisible() || !renderer->hasResourceUseFinished(buffer.getWriteResourceUse()))
    {
        return false;
    }

    // For some GPUs (e.g. ARM) we always prefer using CPU to do copy instead of using the GPU to
    // avoid pipeline bubbles. If the GPU is currently busy and data copy size is less than certain
    // threshold, we choose to use CPU to do the copy over GPU to achieve better parallelism.
    return renderer->getFeatures().preferCPUForBufferSubData.enabled ||
           (renderer->isCommandQueueBusy() &&
            copySize < renderer->getMaxCopyBytesUsingCPUWhenPreservingBufferData());
}

bool RenderPassUsesBufferForReadOnly(ContextVk *contextVk, const vk::BufferHelper &buffer)
{
    if (!contextVk->hasActiveRenderPass())
    {
        return false;
    }

    vk::RenderPassCommandBufferHelper &renderPassCommands =
        contextVk->getStartedRenderPassCommands();
    return renderPassCommands.usesBuffer(buffer) && !renderPassCommands.usesBufferForWrite(buffer);
}

// If a render pass is open which uses the buffer in read-only mode, render pass break can be
// avoided by using acquireAndUpdate.  This can be costly however if the update is very small, and
// is limited to platforms where render pass break is itself costly (i.e. tiled-based renderers).
bool ShouldAvoidRenderPassBreakOnUpdate(ContextVk *contextVk,
                                        const vk::BufferHelper &buffer,
                                        size_t bufferSize)
{
    // Only avoid breaking the render pass if the buffer is not so big such that duplicating it
    // would outweight the cost of breaking the render pass.  A value of 1KB is temporary chosen as
    // a heuristic, and can be adjusted when such a situation is encountered.
    constexpr size_t kPreferDuplicateOverRenderPassBreakMaxBufferSize = 1024;
    if (!contextVk->getFeatures().preferCPUForBufferSubData.enabled ||
        bufferSize > kPreferDuplicateOverRenderPassBreakMaxBufferSize)
    {
        return false;
    }

    return RenderPassUsesBufferForReadOnly(contextVk, buffer);
}

BufferUsageType GetBufferUsageType(gl::BufferUsage usage)
{
    return (usage == gl::BufferUsage::DynamicDraw || usage == gl::BufferUsage::DynamicCopy ||
            usage == gl::BufferUsage::DynamicRead)
               ? BufferUsageType::Dynamic
               : BufferUsageType::Static;
}

angle::Result GetMemoryTypeIndex(ContextVk *contextVk,
                                 VkDeviceSize size,
                                 VkMemoryPropertyFlags memoryPropertyFlags,
                                 uint32_t *memoryTypeIndexOut)
{
    vk::Renderer *renderer         = contextVk->getRenderer();
    const vk::Allocator &allocator = renderer->getAllocator();

    bool persistentlyMapped = renderer->getFeatures().persistentlyMappedBuffers.enabled;
    VkBufferUsageFlags defaultBufferUsageFlags = GetDefaultBufferUsageFlags(renderer);

    VkBufferCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.size                  = size;
    createInfo.usage                 = defaultBufferUsageFlags;
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    // Host visible is required, all other bits are preferred, (i.e., optional)
    VkMemoryPropertyFlags requiredFlags =
        (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkMemoryPropertyFlags preferredFlags =
        (memoryPropertyFlags & (~VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    // Check that the allocation is not too large.
    uint32_t memoryTypeIndex = 0;
    ANGLE_VK_TRY(contextVk, allocator.findMemoryTypeIndexForBufferInfo(
                                createInfo, requiredFlags, preferredFlags, persistentlyMapped,
                                &memoryTypeIndex));
    *memoryTypeIndexOut = memoryTypeIndex;

    return angle::Result::Continue;
}

bool IsSelfCopy(const BufferDataSource &dataSource, const vk::BufferHelper &destination)
{
    return dataSource.data == nullptr &&
           dataSource.buffer->getBufferSerial() == destination.getBufferSerial();
}

angle::Result CopyBuffers(ContextVk *contextVk,
                          vk::BufferHelper *srcBuffer,
                          vk::BufferHelper *dstBuffer,
                          uint32_t regionCount,
                          const VkBufferCopy *copyRegions)
{
    ASSERT(srcBuffer->valid() && dstBuffer->valid());

    // Enqueue a copy command on the GPU
    vk::CommandBufferAccess access;
    if (srcBuffer->getBufferSerial() == dstBuffer->getBufferSerial())
    {
        access.onBufferSelfCopy(srcBuffer);
    }
    else
    {
        access.onBufferTransferRead(srcBuffer);
        access.onBufferTransferWrite(dstBuffer);
    }

    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    commandBuffer->copyBuffer(srcBuffer->getBuffer(), dstBuffer->getBuffer(), regionCount,
                              copyRegions);

    return angle::Result::Continue;
}
}  // namespace

// ConversionBuffer implementation.
ConversionBuffer::ConversionBuffer(vk::Renderer *renderer,
                                   VkBufferUsageFlags usageFlags,
                                   size_t initialSize,
                                   size_t alignment,
                                   bool hostVisible)
    : mEntireBufferDirty(true)
{
    mData = std::make_unique<vk::BufferHelper>();
    mDirtyRanges.reserve(32);
}

ConversionBuffer::~ConversionBuffer()
{
    ASSERT(!mData || !mData->valid());
    mDirtyRanges.clear();
}

ConversionBuffer::ConversionBuffer(ConversionBuffer &&other) = default;

// dirtyRanges may be overlap or continuous. In order to reduce the redunant conversion, we try to
// consolidate the dirty ranges. First we sort it by the range's low. Then we walk the range again
// and check it with previous range and merge them if possible. That merge will remove the
// overlapped area as well as reduce the number of ranges.
void ConversionBuffer::consolidateDirtyRanges()
{
    ASSERT(!mEntireBufferDirty);

    auto comp = [](const RangeDeviceSize &a, const RangeDeviceSize &b) -> bool {
        return a.low() < b.low();
    };
    std::sort(mDirtyRanges.begin(), mDirtyRanges.end(), comp);

    size_t prev = 0;
    for (size_t i = 1; i < mDirtyRanges.size(); i++)
    {
        if (mDirtyRanges[prev].intersectsOrContinuous(mDirtyRanges[i]))
        {
            mDirtyRanges[prev].merge(mDirtyRanges[i]);
            mDirtyRanges[i].invalidate();
        }
        else
        {
            prev = i;
        }
    }
}

// VertexConversionBuffer implementation.
VertexConversionBuffer::VertexConversionBuffer(vk::Renderer *renderer, const CacheKey &cacheKey)
    : ConversionBuffer(renderer,
                       vk::kVertexBufferUsageFlags,
                       kConvertedArrayBufferInitialSize,
                       vk::kVertexBufferAlignment,
                       cacheKey.hostVisible),
      mCacheKey(cacheKey)
{}

VertexConversionBuffer::VertexConversionBuffer(VertexConversionBuffer &&other) = default;

VertexConversionBuffer::~VertexConversionBuffer() = default;

// BufferVk implementation.
BufferVk::BufferVk(const gl::BufferState &state)
    : BufferImpl(state),
      mClientBuffer(nullptr),
      mMemoryTypeIndex(0),
      mMemoryPropertyFlags(0),
      mIsStagingBufferMapped(false),
      mHasValidData(false),
      mIsMappedForWrite(false),
      mUsageType(BufferUsageType::Static)
{
    mMappedRange.invalidate();
}

BufferVk::~BufferVk() {}

void BufferVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    (void)release(contextVk);
}

void BufferVk::releaseConversionBuffers(vk::Context *context)
{
    for (ConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.release(context);
    }
    mVertexConversionBuffers.clear();
}

angle::Result BufferVk::release(ContextVk *contextVk)
{
    if (mBuffer.valid())
    {
        ANGLE_TRY(contextVk->releaseBufferAllocation(&mBuffer));
    }
    if (mStagingBuffer.valid())
    {
        mStagingBuffer.release(contextVk);
    }

    releaseConversionBuffers(contextVk);

    return angle::Result::Continue;
}

angle::Result BufferVk::setExternalBufferData(const gl::Context *context,
                                              gl::BufferBinding target,
                                              GLeglClientBufferEXT clientBuffer,
                                              size_t size,
                                              VkMemoryPropertyFlags memoryPropertyFlags)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Release and re-create the memory and buffer.
    ANGLE_TRY(release(contextVk));

    VkBufferCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.size                  = size;
    createInfo.usage                 = GetDefaultBufferUsageFlags(contextVk->getRenderer());
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    return mBuffer.initExternal(contextVk, memoryPropertyFlags, createInfo, clientBuffer);
}

angle::Result BufferVk::setDataWithUsageFlags(const gl::Context *context,
                                              gl::BufferBinding target,
                                              GLeglClientBufferEXT clientBuffer,
                                              const void *data,
                                              size_t size,
                                              gl::BufferUsage usage,
                                              GLbitfield flags)
{
    ContextVk *contextVk                      = vk::GetImpl(context);
    VkMemoryPropertyFlags memoryPropertyFlags = 0;
    bool persistentMapRequired                = false;
    const bool isExternalBuffer               = clientBuffer != nullptr;

    switch (usage)
    {
        case gl::BufferUsage::InvalidEnum:
        {
            // glBufferStorage API call
            memoryPropertyFlags =
                GetStorageMemoryType(contextVk->getRenderer(), flags, isExternalBuffer);
            persistentMapRequired = (flags & GL_MAP_PERSISTENT_BIT_EXT) != 0;
            break;
        }
        default:
        {
            // glBufferData API call
            memoryPropertyFlags = GetPreferredMemoryType(contextVk->getRenderer(), target, usage);
            break;
        }
    }

    if (isExternalBuffer)
    {
        ANGLE_TRY(setExternalBufferData(context, target, clientBuffer, size, memoryPropertyFlags));
        if (!mBuffer.isHostVisible())
        {
            // If external buffer's memory does not support host visible memory property, we cannot
            // support a persistent map request.
            ANGLE_VK_CHECK(contextVk, !persistentMapRequired, VK_ERROR_MEMORY_MAP_FAILED);
        }

        mClientBuffer = clientBuffer;

        return angle::Result::Continue;
    }
    return setDataWithMemoryType(context, target, data, size, memoryPropertyFlags, usage);
}

angle::Result BufferVk::setData(const gl::Context *context,
                                gl::BufferBinding target,
                                const void *data,
                                size_t size,
                                gl::BufferUsage usage)
{
    ContextVk *contextVk = vk::GetImpl(context);
    // Assume host visible/coherent memory available.
    VkMemoryPropertyFlags memoryPropertyFlags =
        GetPreferredMemoryType(contextVk->getRenderer(), target, usage);
    return setDataWithMemoryType(context, target, data, size, memoryPropertyFlags, usage);
}

angle::Result BufferVk::setDataWithMemoryType(const gl::Context *context,
                                              gl::BufferBinding target,
                                              const void *data,
                                              size_t size,
                                              VkMemoryPropertyFlags memoryPropertyFlags,
                                              gl::BufferUsage usage)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    // Since the buffer is being entirely reinitialized, reset the valid-data flag. If the caller
    // passed in data to fill the buffer, the flag will be updated when the data is copied to the
    // buffer.
    mHasValidData = false;

    if (size == 0)
    {
        // Nothing to do.
        return angle::Result::Continue;
    }

    if (!mVertexConversionBuffers.empty())
    {
        for (ConversionBuffer &buffer : mVertexConversionBuffers)
        {
            buffer.clearDirty();
        }
    }

    const BufferUsageType usageType = GetBufferUsageType(usage);
    const BufferUpdateType updateType =
        calculateBufferUpdateTypeOnFullUpdate(renderer, size, memoryPropertyFlags, usageType, data);

    if (updateType == BufferUpdateType::StorageRedefined)
    {
        mUsageType           = usageType;
        mMemoryPropertyFlags = memoryPropertyFlags;
        ANGLE_TRY(GetMemoryTypeIndex(contextVk, size, memoryPropertyFlags, &mMemoryTypeIndex));
        ANGLE_TRY(acquireBufferHelper(contextVk, size, mUsageType));
    }
    else if (size != static_cast<size_t>(mState.getSize()))
    {
        if (mBuffer.onBufferUserSizeChange(renderer))
        {
            // If we have a dedicated VkBuffer created with user size, even if the storage is
            // reused, we have to recreate that VkBuffer with user size when user size changes.
            // When this happens, we must notify other objects that observing this buffer, such as
            // vertex array. The reason vertex array is observing the buffer's storage change is
            // because they uses VkBuffer. Now VkBuffer have changed, vertex array needs to
            // re-process it just like storage has been reallocated.
            onStateChange(angle::SubjectMessage::InternalMemoryAllocationChanged);
        }
    }

    if (data != nullptr)
    {
        BufferDataSource dataSource = {};
        dataSource.data             = data;

        // Handle full-buffer updates similarly to glBufferSubData
        ANGLE_TRY(setDataImpl(contextVk, size, dataSource, size, 0, updateType));
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::setSubData(const gl::Context *context,
                                   gl::BufferBinding target,
                                   const void *data,
                                   size_t size,
                                   size_t offset)
{
    ASSERT(mBuffer.valid());

    BufferDataSource dataSource = {};
    dataSource.data             = data;

    ContextVk *contextVk = vk::GetImpl(context);
    return setDataImpl(contextVk, static_cast<size_t>(mState.getSize()), dataSource, size, offset,
                       BufferUpdateType::ContentsUpdate);
}

angle::Result BufferVk::copySubData(const gl::Context *context,
                                    BufferImpl *source,
                                    GLintptr sourceOffset,
                                    GLintptr destOffset,
                                    GLsizeiptr size)
{
    ASSERT(mBuffer.valid());

    ContextVk *contextVk = vk::GetImpl(context);
    BufferVk *sourceVk   = GetAs<BufferVk>(source);

    BufferDataSource dataSource = {};
    dataSource.buffer           = &sourceVk->getBuffer();
    dataSource.bufferOffset     = static_cast<VkDeviceSize>(sourceOffset);

    ASSERT(dataSource.buffer->valid());

    return setDataImpl(contextVk, static_cast<size_t>(mState.getSize()), dataSource, size,
                       destOffset, BufferUpdateType::ContentsUpdate);
}

angle::Result BufferVk::allocStagingBuffer(ContextVk *contextVk,
                                           vk::MemoryCoherency coherency,
                                           VkDeviceSize size,
                                           uint8_t **mapPtr)
{
    ASSERT(!mIsStagingBufferMapped);

    if (mStagingBuffer.valid())
    {
        if (size <= mStagingBuffer.getSize() && IsCached(coherency) == mStagingBuffer.isCached() &&
            contextVk->getRenderer()->hasResourceUseFinished(mStagingBuffer.getResourceUse()))
        {
            // If size is big enough and it is idle, then just reuse the existing staging buffer
            *mapPtr                = mStagingBuffer.getMappedMemory();
            mIsStagingBufferMapped = true;
            return angle::Result::Continue;
        }
        mStagingBuffer.release(contextVk);
    }

    ANGLE_TRY(
        contextVk->initBufferForBufferCopy(&mStagingBuffer, static_cast<size_t>(size), coherency));
    *mapPtr                = mStagingBuffer.getMappedMemory();
    mIsStagingBufferMapped = true;

    return angle::Result::Continue;
}

angle::Result BufferVk::flushStagingBuffer(ContextVk *contextVk,
                                           VkDeviceSize offset,
                                           VkDeviceSize size)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    ASSERT(mIsStagingBufferMapped);
    ASSERT(mStagingBuffer.valid());

    if (!mStagingBuffer.isCoherent())
    {
        ANGLE_TRY(mStagingBuffer.flush(renderer));
    }

    VkBufferCopy copyRegion = {mStagingBuffer.getOffset(), mBuffer.getOffset() + offset, size};
    ANGLE_TRY(CopyBuffers(contextVk, &mStagingBuffer, &mBuffer, 1, &copyRegion));

    return angle::Result::Continue;
}

angle::Result BufferVk::handleDeviceLocalBufferMap(ContextVk *contextVk,
                                                   VkDeviceSize offset,
                                                   VkDeviceSize size,
                                                   uint8_t **mapPtr)
{
    vk::Renderer *renderer = contextVk->getRenderer();
    ANGLE_TRY(
        allocStagingBuffer(contextVk, vk::MemoryCoherency::CachedPreferCoherent, size, mapPtr));
    ANGLE_TRY(mStagingBuffer.flush(renderer));

    // Copy data from device local buffer to host visible staging buffer.
    VkBufferCopy copyRegion = {mBuffer.getOffset() + offset, mStagingBuffer.getOffset(), size};
    ANGLE_TRY(CopyBuffers(contextVk, &mBuffer, &mStagingBuffer, 1, &copyRegion));
    ANGLE_TRY(mStagingBuffer.waitForIdle(contextVk, "GPU stall due to mapping device local buffer",
                                         RenderPassClosureReason::DeviceLocalBufferMap));
    // Since coherent is prefer, we may end up getting non-coherent. Always call invalidate here (it
    // will check memory flag before it actually calls into driver).
    ANGLE_TRY(mStagingBuffer.invalidate(renderer));

    return angle::Result::Continue;
}

angle::Result BufferVk::mapHostVisibleBuffer(ContextVk *contextVk,
                                             VkDeviceSize offset,
                                             GLbitfield access,
                                             uint8_t **mapPtr)
{
    ANGLE_TRY(mBuffer.mapWithOffset(contextVk, mapPtr, static_cast<size_t>(offset)));

    // Invalidate non-coherent for READ case.
    if (!mBuffer.isCoherent() && (access & GL_MAP_READ_BIT) != 0)
    {
        ANGLE_TRY(mBuffer.invalidate(contextVk->getRenderer()));
    }
    return angle::Result::Continue;
}

angle::Result BufferVk::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    ASSERT(mBuffer.valid());
    ASSERT(access == GL_WRITE_ONLY_OES);

    return mapImpl(vk::GetImpl(context), GL_MAP_WRITE_BIT, mapPtr);
}

angle::Result BufferVk::mapRange(const gl::Context *context,
                                 size_t offset,
                                 size_t length,
                                 GLbitfield access,
                                 void **mapPtr)
{
    return mapRangeImpl(vk::GetImpl(context), offset, length, access, mapPtr);
}

angle::Result BufferVk::mapImpl(ContextVk *contextVk, GLbitfield access, void **mapPtr)
{
    return mapRangeImpl(contextVk, 0, static_cast<VkDeviceSize>(mState.getSize()), access, mapPtr);
}

angle::Result BufferVk::ghostMappedBuffer(ContextVk *contextVk,
                                          VkDeviceSize offset,
                                          VkDeviceSize length,
                                          GLbitfield access,
                                          void **mapPtr)
{
    // We shouldn't get here if it is external memory
    ASSERT(!isExternalBuffer());

    ++contextVk->getPerfCounters().buffersGhosted;

    // If we are creating a new buffer because the GPU is using it as read-only, then we
    // also need to copy the contents of the previous buffer into the new buffer, in
    // case the caller only updates a portion of the new buffer.
    vk::BufferHelper src = std::move(mBuffer);
    ANGLE_TRY(acquireBufferHelper(contextVk, static_cast<size_t>(mState.getSize()),
                                  BufferUsageType::Dynamic));

    // Before returning the new buffer, map the previous buffer and copy its entire
    // contents into the new buffer.
    uint8_t *srcMapPtr = nullptr;
    uint8_t *dstMapPtr = nullptr;
    ANGLE_TRY(src.map(contextVk, &srcMapPtr));
    ANGLE_TRY(mBuffer.map(contextVk, &dstMapPtr));

    ASSERT(src.isCoherent());
    ASSERT(mBuffer.isCoherent());

    // No need to copy over [offset, offset + length), just around it
    if ((access & GL_MAP_INVALIDATE_RANGE_BIT) != 0)
    {
        if (offset != 0)
        {
            memcpy(dstMapPtr, srcMapPtr, static_cast<size_t>(offset));
        }
        size_t totalSize      = static_cast<size_t>(mState.getSize());
        size_t remainingStart = static_cast<size_t>(offset + length);
        size_t remainingSize  = totalSize - remainingStart;
        if (remainingSize != 0)
        {
            memcpy(dstMapPtr + remainingStart, srcMapPtr + remainingStart, remainingSize);
        }
    }
    else
    {
        memcpy(dstMapPtr, srcMapPtr, static_cast<size_t>(mState.getSize()));
    }

    ANGLE_TRY(contextVk->releaseBufferAllocation(&src));

    // Return the already mapped pointer with the offset adjustment to avoid the call to unmap().
    *mapPtr = dstMapPtr + offset;

    return angle::Result::Continue;
}

angle::Result BufferVk::mapRangeImpl(ContextVk *contextVk,
                                     VkDeviceSize offset,
                                     VkDeviceSize length,
                                     GLbitfield access,
                                     void **mapPtr)
{
    vk::Renderer *renderer = contextVk->getRenderer();
    ASSERT(mBuffer.valid());

    // Record map call parameters in case this call is from angle internal (the access/offset/length
    // will be inconsistent from mState).
    mIsMappedForWrite = (access & GL_MAP_WRITE_BIT) != 0;
    mMappedRange      = RangeDeviceSize(offset, offset + length);

    uint8_t **mapPtrBytes = reinterpret_cast<uint8_t **>(mapPtr);
    bool hostVisible      = mBuffer.isHostVisible();

    // MAP_UNSYNCHRONIZED_BIT, so immediately map.
    if ((access & GL_MAP_UNSYNCHRONIZED_BIT) != 0)
    {
        if (hostVisible)
        {
            return mapHostVisibleBuffer(contextVk, offset, access, mapPtrBytes);
        }
        return handleDeviceLocalBufferMap(contextVk, offset, length, mapPtrBytes);
    }

    // Read case
    if ((access & GL_MAP_WRITE_BIT) == 0)
    {
        // If app is not going to write, all we need is to ensure GPU write is finished.
        // Concurrent reads from CPU and GPU is allowed.
        if (!renderer->hasResourceUseFinished(mBuffer.getWriteResourceUse()))
        {
            // If there are unflushed write commands for the resource, flush them.
            if (contextVk->hasUnsubmittedUse(mBuffer.getWriteResourceUse()))
            {
                ANGLE_TRY(contextVk->flushAndSubmitCommands(
                    nullptr, nullptr, RenderPassClosureReason::BufferWriteThenMap));
            }
            ANGLE_TRY(renderer->finishResourceUse(contextVk, mBuffer.getWriteResourceUse()));
        }
        if (hostVisible)
        {
            return mapHostVisibleBuffer(contextVk, offset, access, mapPtrBytes);
        }
        return handleDeviceLocalBufferMap(contextVk, offset, length, mapPtrBytes);
    }

    // Write case
    if (!hostVisible)
    {
        return handleDeviceLocalBufferMap(contextVk, offset, length, mapPtrBytes);
    }

    // Write case, buffer not in use.
    if (isExternalBuffer() || !isCurrentlyInUse(contextVk->getRenderer()))
    {
        return mapHostVisibleBuffer(contextVk, offset, access, mapPtrBytes);
    }

    // Write case, buffer in use.
    //
    // Here, we try to map the buffer, but it's busy. Instead of waiting for the GPU to
    // finish, we just allocate a new buffer if:
    // 1.) Caller has told us it doesn't care about previous contents, or
    // 2.) The GPU won't write to the buffer.

    bool rangeInvalidate = (access & GL_MAP_INVALIDATE_RANGE_BIT) != 0;
    bool entireBufferInvalidated =
        ((access & GL_MAP_INVALIDATE_BUFFER_BIT) != 0) ||
        (rangeInvalidate && offset == 0 && static_cast<VkDeviceSize>(mState.getSize()) == length);

    if (entireBufferInvalidated)
    {
        ANGLE_TRY(acquireBufferHelper(contextVk, static_cast<size_t>(mState.getSize()),
                                      BufferUsageType::Dynamic));
        return mapHostVisibleBuffer(contextVk, offset, access, mapPtrBytes);
    }

    bool smallMapRange = (length < static_cast<VkDeviceSize>(mState.getSize()) / 2);

    if (smallMapRange && rangeInvalidate)
    {
        ANGLE_TRY(allocStagingBuffer(contextVk, vk::MemoryCoherency::CachedNonCoherent,
                                     static_cast<size_t>(length), mapPtrBytes));
        return angle::Result::Continue;
    }

    if (renderer->hasResourceUseFinished(mBuffer.getWriteResourceUse()))
    {
        // This will keep the new buffer mapped and update mapPtr, so return immediately.
        return ghostMappedBuffer(contextVk, offset, length, access, mapPtr);
    }

    // Write case (worst case, buffer in use for write)
    ANGLE_TRY(mBuffer.waitForIdle(contextVk, "GPU stall due to mapping buffer in use by the GPU",
                                  RenderPassClosureReason::BufferInUseWhenSynchronizedMap));
    return mapHostVisibleBuffer(contextVk, offset, access, mapPtrBytes);
}

angle::Result BufferVk::unmap(const gl::Context *context, GLboolean *result)
{
    ANGLE_TRY(unmapImpl(vk::GetImpl(context)));

    // This should be false if the contents have been corrupted through external means.  Vulkan
    // doesn't provide such information.
    *result = true;

    return angle::Result::Continue;
}

angle::Result BufferVk::unmapImpl(ContextVk *contextVk)
{
    ASSERT(mBuffer.valid());

    if (mIsStagingBufferMapped)
    {
        ASSERT(mStagingBuffer.valid());
        // The buffer is device local or optimization of small range map.
        if (mIsMappedForWrite)
        {
            ANGLE_TRY(flushStagingBuffer(contextVk, mMappedRange.low(), mMappedRange.length()));
        }

        mIsStagingBufferMapped = false;
    }
    else
    {
        ASSERT(mBuffer.isHostVisible());
        vk::Renderer *renderer = contextVk->getRenderer();
        if (!mBuffer.isCoherent())
        {
            ANGLE_TRY(mBuffer.flush(renderer));
        }
        mBuffer.unmap(renderer);
    }

    if (mIsMappedForWrite)
    {
        if (mMappedRange == RangeDeviceSize(0, static_cast<VkDeviceSize>(getSize())))
        {
            dataUpdated();
        }
        else
        {
            dataRangeUpdated(mMappedRange);
        }
    }

    // Reset the mapping parameters
    mIsMappedForWrite = false;
    mMappedRange.invalidate();

    return angle::Result::Continue;
}

angle::Result BufferVk::getSubData(const gl::Context *context,
                                   GLintptr offset,
                                   GLsizeiptr size,
                                   void *outData)
{
    ASSERT(offset + size <= getSize());
    ASSERT(mBuffer.valid());
    ContextVk *contextVk = vk::GetImpl(context);
    void *mapPtr;
    ANGLE_TRY(mapRangeImpl(contextVk, offset, size, GL_MAP_READ_BIT, &mapPtr));
    memcpy(outData, mapPtr, size);
    return unmapImpl(contextVk);
}

angle::Result BufferVk::getIndexRange(const gl::Context *context,
                                      gl::DrawElementsType type,
                                      size_t offset,
                                      size_t count,
                                      bool primitiveRestartEnabled,
                                      gl::IndexRange *outRange)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    // This is a workaround for the mock ICD not implementing buffer memory state.
    // Could be removed if https://github.com/KhronosGroup/Vulkan-Tools/issues/84 is fixed.
    if (renderer->isMockICDEnabled())
    {
        outRange->start = 0;
        outRange->end   = 0;
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "BufferVk::getIndexRange");

    void *mapPtr;
    ANGLE_TRY(mapRangeImpl(contextVk, offset, getSize(), GL_MAP_READ_BIT, &mapPtr));
    *outRange = gl::ComputeIndexRange(type, mapPtr, count, primitiveRestartEnabled);
    ANGLE_TRY(unmapImpl(contextVk));

    return angle::Result::Continue;
}

angle::Result BufferVk::updateBuffer(ContextVk *contextVk,
                                     size_t bufferSize,
                                     const BufferDataSource &dataSource,
                                     size_t updateSize,
                                     size_t updateOffset)
{
    // To copy on the CPU, destination must be host-visible.  The source should also be either a CPU
    // pointer or other a host-visible buffer that is not being written to by the GPU.
    const bool shouldCopyOnCPU =
        mBuffer.isHostVisible() &&
        (dataSource.data != nullptr ||
         ShouldUseCPUToCopyData(contextVk, *dataSource.buffer, updateSize, bufferSize));

    if (shouldCopyOnCPU)
    {
        ANGLE_TRY(directUpdate(contextVk, dataSource, updateSize, updateOffset));
    }
    else
    {
        ANGLE_TRY(stagedUpdate(contextVk, dataSource, updateSize, updateOffset));
    }
    return angle::Result::Continue;
}

angle::Result BufferVk::directUpdate(ContextVk *contextVk,
                                     const BufferDataSource &dataSource,
                                     size_t size,
                                     size_t offset)
{
    vk::Renderer *renderer    = contextVk->getRenderer();
    uint8_t *srcPointerMapped = nullptr;
    const uint8_t *srcPointer = nullptr;
    uint8_t *dstPointer       = nullptr;

    // Map the destination buffer.
    ASSERT(mBuffer.isHostVisible());
    ANGLE_TRY(mBuffer.mapWithOffset(contextVk, &dstPointer, offset));
    ASSERT(dstPointer);

    // If source data is coming from a buffer, map it.  If this is a self-copy, avoid double-mapping
    // the buffer.
    if (dataSource.data != nullptr)
    {
        srcPointer = static_cast<const uint8_t *>(dataSource.data);
    }
    else
    {
        ANGLE_TRY(dataSource.buffer->mapWithOffset(contextVk, &srcPointerMapped,
                                                   static_cast<size_t>(dataSource.bufferOffset)));
        srcPointer = srcPointerMapped;
    }

    memcpy(dstPointer, srcPointer, size);

    // External memory may end up with noncoherent
    if (!mBuffer.isCoherent())
    {
        ANGLE_TRY(mBuffer.flush(renderer, offset, size));
    }

    // Unmap the destination and source buffers if applicable.
    //
    // If the buffer has dynamic usage then the intent is frequent client side updates to the
    // buffer. Don't CPU unmap the buffer, we will take care of unmapping when releasing the buffer
    // to either the renderer or mBufferFreeList.
    if (GetBufferUsageType(mState.getUsage()) == BufferUsageType::Static)
    {
        mBuffer.unmap(renderer);
    }

    if (srcPointerMapped != nullptr)
    {
        dataSource.buffer->unmap(renderer);
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::stagedUpdate(ContextVk *contextVk,
                                     const BufferDataSource &dataSource,
                                     size_t size,
                                     size_t offset)
{
    // If data is coming from a CPU pointer, stage it in a temporary staging buffer.
    // Otherwise, do a GPU copy directly from the given buffer.
    if (dataSource.data != nullptr)
    {
        uint8_t *mapPointer = nullptr;
        ANGLE_TRY(allocStagingBuffer(contextVk, vk::MemoryCoherency::CachedNonCoherent, size,
                                     &mapPointer));
        memcpy(mapPointer, dataSource.data, size);
        ANGLE_TRY(flushStagingBuffer(contextVk, offset, size));
        mIsStagingBufferMapped = false;
    }
    else
    {
        // Check for self-dependency.
        vk::CommandBufferAccess access;
        if (dataSource.buffer->getBufferSerial() == mBuffer.getBufferSerial())
        {
            access.onBufferSelfCopy(&mBuffer);
        }
        else
        {
            access.onBufferTransferRead(dataSource.buffer);
            access.onBufferTransferWrite(&mBuffer);
        }

        vk::OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        // Enqueue a copy command on the GPU.
        const VkBufferCopy copyRegion = {dataSource.bufferOffset + dataSource.buffer->getOffset(),
                                         static_cast<VkDeviceSize>(offset) + mBuffer.getOffset(),
                                         static_cast<VkDeviceSize>(size)};

        commandBuffer->copyBuffer(dataSource.buffer->getBuffer(), mBuffer.getBuffer(), 1,
                                  &copyRegion);
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::acquireAndUpdate(ContextVk *contextVk,
                                         size_t bufferSize,
                                         const BufferDataSource &dataSource,
                                         size_t updateSize,
                                         size_t updateOffset,
                                         BufferUpdateType updateType)
{
    // We shouldn't get here if this is external memory
    ASSERT(!isExternalBuffer());
    // If StorageRedefined, we cannot use mState.getSize() to allocate a new buffer.
    ASSERT(updateType != BufferUpdateType::StorageRedefined);
    ASSERT(mBuffer.valid());
    ASSERT(mBuffer.getSize() >= bufferSize);

    // Here we acquire a new BufferHelper and directUpdate() the new buffer.
    // If the subData size was less than the buffer's size we additionally enqueue
    // a GPU copy of the remaining regions from the old mBuffer to the new one.
    vk::BufferHelper prevBuffer;
    size_t offsetAfterSubdata      = (updateOffset + updateSize);
    bool updateRegionBeforeSubData = mHasValidData && (updateOffset > 0);
    bool updateRegionAfterSubData  = mHasValidData && (offsetAfterSubdata < bufferSize);

    uint8_t *prevMapPtrBeforeSubData = nullptr;
    uint8_t *prevMapPtrAfterSubData  = nullptr;
    if (updateRegionBeforeSubData || updateRegionAfterSubData)
    {
        prevBuffer = std::move(mBuffer);

        // The total bytes that we need to copy from old buffer to new buffer
        size_t copySize = bufferSize - updateSize;

        // If the buffer is host visible and the GPU is not writing to it, we use the CPU to do the
        // copy. We need to save the source buffer pointer before we acquire a new buffer.
        if (ShouldUseCPUToCopyData(contextVk, prevBuffer, copySize, bufferSize))
        {
            uint8_t *mapPointer = nullptr;
            // prevBuffer buffer will be recycled (or released and unmapped) by acquireBufferHelper
            ANGLE_TRY(prevBuffer.map(contextVk, &mapPointer));
            ASSERT(mapPointer);
            prevMapPtrBeforeSubData = mapPointer;
            prevMapPtrAfterSubData  = mapPointer + offsetAfterSubdata;
        }
    }

    ANGLE_TRY(acquireBufferHelper(contextVk, bufferSize, BufferUsageType::Dynamic));
    ANGLE_TRY(updateBuffer(contextVk, bufferSize, dataSource, updateSize, updateOffset));

    constexpr int kMaxCopyRegions = 2;
    angle::FixedVector<VkBufferCopy, kMaxCopyRegions> copyRegions;

    if (updateRegionBeforeSubData)
    {
        if (prevMapPtrBeforeSubData)
        {
            BufferDataSource beforeSrc = {};
            beforeSrc.data             = prevMapPtrBeforeSubData;

            ANGLE_TRY(directUpdate(contextVk, beforeSrc, updateOffset, 0));
        }
        else
        {
            copyRegions.push_back({prevBuffer.getOffset(), mBuffer.getOffset(), updateOffset});
        }
    }

    if (updateRegionAfterSubData)
    {
        size_t copySize = bufferSize - offsetAfterSubdata;
        if (prevMapPtrAfterSubData)
        {
            BufferDataSource afterSrc = {};
            afterSrc.data             = prevMapPtrAfterSubData;

            ANGLE_TRY(directUpdate(contextVk, afterSrc, copySize, offsetAfterSubdata));
        }
        else
        {
            copyRegions.push_back({prevBuffer.getOffset() + offsetAfterSubdata,
                                   mBuffer.getOffset() + offsetAfterSubdata, copySize});
        }
    }

    if (!copyRegions.empty())
    {
        ANGLE_TRY(CopyBuffers(contextVk, &prevBuffer, &mBuffer,
                              static_cast<uint32_t>(copyRegions.size()), copyRegions.data()));
    }

    if (prevBuffer.valid())
    {
        ANGLE_TRY(contextVk->releaseBufferAllocation(&prevBuffer));
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::setDataImpl(ContextVk *contextVk,
                                    size_t bufferSize,
                                    const BufferDataSource &dataSource,
                                    size_t updateSize,
                                    size_t updateOffset,
                                    BufferUpdateType updateType)
{
    // if the buffer is currently in use
    //     if it isn't an external buffer and not a self-copy and sub data size meets threshold
    //          acquire a new BufferHelper from the pool
    //     else stage the update
    // else update the buffer directly
    if (isCurrentlyInUse(contextVk->getRenderer()))
    {
        // The acquire-and-update path creates a new buffer, which is sometimes more efficient than
        // trying to update the existing one.  Firstly, this is not done in the following
        // situations:
        //
        // - For external buffers, the underlying storage cannot be reallocated.
        // - If storage has just been redefined, this path is not taken because a new buffer has
        //   already been created by the caller. Besides, this path uses mState.getSize(), which the
        //   frontend updates only after this call in situations where the storage may be redefined.
        //   This could happen if the buffer memory is DEVICE_LOCAL and
        //   renderer->getFeatures().allocateNonZeroMemory.enabled is true. In this case a
        //   copyToBuffer is immediately issued after allocation and isCurrentlyInUse will be true.
        // - If this is a self copy through glCopyBufferSubData, |dataSource| will contain a
        //   reference to |mBuffer|, in which case source information is lost after acquiring a new
        //   buffer.
        //
        // Additionally, this path is taken only if either of the following conditions are true:
        //
        // - If BufferVk does not have any valid data.  This means that there is no data to be
        //   copied from the old buffer to the new one after acquiring it.  This could happen when
        //   the application calls glBufferData with the same size and we reuse the existing buffer
        //   storage.
        // - If the buffer is used read-only in the current render pass.  In this case, acquiring a
        //   new buffer is preferred to avoid breaking the render pass.
        // - The update modifies a significant portion of the buffer
        // - The preferCPUForBufferSubData feature is enabled.
        //
        const bool canAcquireAndUpdate = !isExternalBuffer() &&
                                         updateType != BufferUpdateType::StorageRedefined &&
                                         !IsSelfCopy(dataSource, mBuffer);
        if (canAcquireAndUpdate &&
            (!mHasValidData || ShouldAvoidRenderPassBreakOnUpdate(contextVk, mBuffer, bufferSize) ||
             ShouldAllocateNewMemoryForUpdate(contextVk, updateSize, bufferSize)))
        {
            ANGLE_TRY(acquireAndUpdate(contextVk, bufferSize, dataSource, updateSize, updateOffset,
                                       updateType));
        }
        else
        {
            if (canAcquireAndUpdate && RenderPassUsesBufferForReadOnly(contextVk, mBuffer))
            {
                ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_LOW,
                                      "Breaking the render pass on small upload to large buffer");
            }

            ANGLE_TRY(stagedUpdate(contextVk, dataSource, updateSize, updateOffset));
        }
    }
    else
    {
        ANGLE_TRY(updateBuffer(contextVk, bufferSize, dataSource, updateSize, updateOffset));
    }

    // Update conversions.
    if (updateOffset == 0 && updateSize == bufferSize)
    {
        dataUpdated();
    }
    else
    {
        dataRangeUpdated(RangeDeviceSize(updateOffset, updateOffset + updateSize));
    }

    return angle::Result::Continue;
}

VertexConversionBuffer *BufferVk::getVertexConversionBuffer(
    vk::Renderer *renderer,
    const VertexConversionBuffer::CacheKey &cacheKey)
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        if (buffer.match(cacheKey))
        {
            ASSERT(buffer.valid());
            return &buffer;
        }
    }

    mVertexConversionBuffers.emplace_back(renderer, cacheKey);
    return &mVertexConversionBuffers.back();
}

void BufferVk::dataRangeUpdated(const RangeDeviceSize &range)
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.addDirtyBufferRange(range);
    }
    // Now we have valid data
    mHasValidData = true;
}

void BufferVk::dataUpdated()
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.setEntireBufferDirty();
    }
    // Now we have valid data
    mHasValidData = true;
}

void BufferVk::onDataChanged()
{
    dataUpdated();
}

angle::Result BufferVk::acquireBufferHelper(ContextVk *contextVk,
                                            size_t sizeInBytes,
                                            BufferUsageType usageType)
{
    vk::Renderer *renderer = contextVk->getRenderer();
    size_t size            = roundUpPow2(sizeInBytes, kBufferSizeGranularity);
    size_t alignment       = renderer->getDefaultBufferAlignment();

    if (mBuffer.valid())
    {
        ANGLE_TRY(contextVk->releaseBufferAllocation(&mBuffer));
    }

    // Allocate the buffer directly
    ANGLE_TRY(
        contextVk->initBufferAllocation(&mBuffer, mMemoryTypeIndex, size, alignment, usageType));

    // Tell the observers (front end) that a new buffer was created, so the necessary
    // dirty bits can be set. This allows the buffer views pointing to the old buffer to
    // be recreated and point to the new buffer, along with updating the descriptor sets
    // to use the new buffer.
    onStateChange(angle::SubjectMessage::InternalMemoryAllocationChanged);

    return angle::Result::Continue;
}

bool BufferVk::isCurrentlyInUse(vk::Renderer *renderer) const
{
    return !renderer->hasResourceUseFinished(mBuffer.getResourceUse());
}

// When a buffer is being completely changed, calculate whether it's better to allocate a new buffer
// or overwrite the existing one.
BufferUpdateType BufferVk::calculateBufferUpdateTypeOnFullUpdate(
    vk::Renderer *renderer,
    size_t size,
    VkMemoryPropertyFlags memoryPropertyFlags,
    BufferUsageType usageType,
    const void *data) const
{
    // 0-sized updates should be no-op'd before this call.
    ASSERT(size > 0);

    // If there is no existing buffer, this cannot be a content update.
    if (!mBuffer.valid())
    {
        return BufferUpdateType::StorageRedefined;
    }

    const bool inUseAndRespecifiedWithoutData = data == nullptr && isCurrentlyInUse(renderer);
    bool redefineStorage = shouldRedefineStorage(renderer, usageType, memoryPropertyFlags, size);

    // Create a new buffer if the buffer is busy and it's being redefined without data.
    // Additionally, a new buffer is created if any of the parameters change (memory type, usage,
    // size).
    return redefineStorage || inUseAndRespecifiedWithoutData ? BufferUpdateType::StorageRedefined
                                                             : BufferUpdateType::ContentsUpdate;
}

bool BufferVk::shouldRedefineStorage(vk::Renderer *renderer,
                                     BufferUsageType usageType,
                                     VkMemoryPropertyFlags memoryPropertyFlags,
                                     size_t size) const
{
    if (mUsageType != usageType)
    {
        return true;
    }

    if (mMemoryPropertyFlags != memoryPropertyFlags)
    {
        return true;
    }

    if (size > mBuffer.getSize())
    {
        return true;
    }
    else
    {
        size_t paddedBufferSize =
            (renderer->getFeatures().padBuffersToMaxVertexAttribStride.enabled)
                ? (size + static_cast<size_t>(renderer->getMaxVertexAttribStride()))
                : size;
        size_t sizeInBytes = roundUpPow2(paddedBufferSize, kBufferSizeGranularity);
        size_t alignedSize = roundUp(sizeInBytes, renderer->getDefaultBufferAlignment());
        if (alignedSize > mBuffer.getSize())
        {
            return true;
        }
    }

    return false;
}
}  // namespace rx
