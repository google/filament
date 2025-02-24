//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVk.h:
//    Defines the class interface for BufferVk, implementing BufferImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_BUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_BUFFERVK_H_

#include "libANGLE/Buffer.h"
#include "libANGLE/Observer.h"
#include "libANGLE/renderer/BufferImpl.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
typedef gl::Range<VkDeviceSize> RangeDeviceSize;

// Conversion buffers hold translated index and vertex data.
class ConversionBuffer
{
  public:
    ConversionBuffer() : mEntireBufferDirty(true)
    {
        mData = std::make_unique<vk::BufferHelper>();
        mDirtyRanges.reserve(32);
    }
    ConversionBuffer(vk::Renderer *renderer,
                     VkBufferUsageFlags usageFlags,
                     size_t initialSize,
                     size_t alignment,
                     bool hostVisible);
    ~ConversionBuffer();

    ConversionBuffer(ConversionBuffer &&other);

    bool dirty() const { return mEntireBufferDirty || !mDirtyRanges.empty(); }
    bool isEntireBufferDirty() const { return mEntireBufferDirty; }
    void setEntireBufferDirty() { mEntireBufferDirty = true; }
    void addDirtyBufferRange(const RangeDeviceSize &range) { mDirtyRanges.emplace_back(range); }
    void consolidateDirtyRanges();
    const std::vector<RangeDeviceSize> &getDirtyBufferRanges() const { return mDirtyRanges; }
    void clearDirty()
    {
        mEntireBufferDirty = false;
        mDirtyRanges.clear();
    }

    bool valid() const { return mData && mData->valid(); }
    vk::BufferHelper *getBuffer() const { return mData.get(); }
    void release(vk::Context *context) { mData->release(context); }
    void destroy(vk::Renderer *renderer) { mData->destroy(renderer); }

  private:
    // state value determines if we need to re-stream vertex data. mEntireBufferDirty indicates
    // entire buffer data has changed. mDirtyRange should be ignored when mEntireBufferDirty is
    // true. If mEntireBufferDirty is false, mDirtyRange is the ranges of data that has been
    // modified. Note that there is no guarantee that ranges will not overlap.
    bool mEntireBufferDirty;
    std::vector<RangeDeviceSize> mDirtyRanges;

    // Where the conversion data is stored.
    std::unique_ptr<vk::BufferHelper> mData;
};

class VertexConversionBuffer : public ConversionBuffer
{
  public:
    struct CacheKey final
    {
        angle::FormatID formatID;
        GLuint stride;
        size_t offset;
        bool hostVisible;
        bool offsetMustMatchExactly;
    };

    VertexConversionBuffer(vk::Renderer *renderer, const CacheKey &cacheKey);
    ~VertexConversionBuffer();

    VertexConversionBuffer(VertexConversionBuffer &&other);

    bool match(const CacheKey &cacheKey)
    {
        // If anything other than offset mismatch, it can't reuse.
        if (mCacheKey.formatID != cacheKey.formatID || mCacheKey.stride != cacheKey.stride ||
            mCacheKey.offsetMustMatchExactly != cacheKey.offsetMustMatchExactly ||
            mCacheKey.hostVisible != cacheKey.hostVisible)
        {
            return false;
        }

        // If offset matches, for sure we can reuse.
        if (mCacheKey.offset == cacheKey.offset)
        {
            return true;
        }

        // If offset exact match is not required and offsets are multiple strides apart, then we
        // adjust the offset to reuse the buffer. The benefit of reused the buffer is that the
        // previous conversion result is still valid. We only need to convert the modified data.
        if (!cacheKey.offsetMustMatchExactly)
        {
            int64_t offsetGap = cacheKey.offset - mCacheKey.offset;
            if ((offsetGap % cacheKey.stride) == 0)
            {
                if (cacheKey.offset < mCacheKey.offset)
                {
                    addDirtyBufferRange(RangeDeviceSize(cacheKey.offset, mCacheKey.offset));
                    mCacheKey.offset = cacheKey.offset;
                }
                return true;
            }
        }
        return false;
    }

    const CacheKey &getCacheKey() const { return mCacheKey; }

  private:
    // The conversion is identified by the triple of {format, stride, offset}.
    CacheKey mCacheKey;
};

enum class BufferUpdateType
{
    StorageRedefined,
    ContentsUpdate,
};

struct BufferDataSource
{
    // Buffer data can come from two sources:
    // glBufferData and glBufferSubData upload through a CPU pointer
    const void *data = nullptr;
    // glCopyBufferSubData copies data from another buffer
    vk::BufferHelper *buffer  = nullptr;
    VkDeviceSize bufferOffset = 0;
};

VkBufferUsageFlags GetDefaultBufferUsageFlags(vk::Renderer *renderer);

class BufferVk : public BufferImpl
{
  public:
    BufferVk(const gl::BufferState &state);
    ~BufferVk() override;
    void destroy(const gl::Context *context) override;

    angle::Result setExternalBufferData(const gl::Context *context,
                                        gl::BufferBinding target,
                                        GLeglClientBufferEXT clientBuffer,
                                        size_t size,
                                        VkMemoryPropertyFlags memoryPropertyFlags);
    angle::Result setDataWithUsageFlags(const gl::Context *context,
                                        gl::BufferBinding target,
                                        GLeglClientBufferEXT clientBuffer,
                                        const void *data,
                                        size_t size,
                                        gl::BufferUsage usage,
                                        GLbitfield flags) override;
    angle::Result setData(const gl::Context *context,
                          gl::BufferBinding target,
                          const void *data,
                          size_t size,
                          gl::BufferUsage usage) override;
    angle::Result setSubData(const gl::Context *context,
                             gl::BufferBinding target,
                             const void *data,
                             size_t size,
                             size_t offset) override;
    angle::Result copySubData(const gl::Context *context,
                              BufferImpl *source,
                              GLintptr sourceOffset,
                              GLintptr destOffset,
                              GLsizeiptr size) override;
    angle::Result map(const gl::Context *context, GLenum access, void **mapPtr) override;
    angle::Result mapRange(const gl::Context *context,
                           size_t offset,
                           size_t length,
                           GLbitfield access,
                           void **mapPtr) override;
    angle::Result unmap(const gl::Context *context, GLboolean *result) override;
    angle::Result getSubData(const gl::Context *context,
                             GLintptr offset,
                             GLsizeiptr size,
                             void *outData) override;

    angle::Result getIndexRange(const gl::Context *context,
                                gl::DrawElementsType type,
                                size_t offset,
                                size_t count,
                                bool primitiveRestartEnabled,
                                gl::IndexRange *outRange) override;

    GLint64 getSize() const { return mState.getSize(); }

    void onDataChanged() override;

    vk::BufferHelper &getBuffer()
    {
        ASSERT(isBufferValid());
        return mBuffer;
    }

    vk::BufferSerial getBufferSerial() { return mBuffer.getBufferSerial(); }

    bool isBufferValid() const { return mBuffer.valid(); }
    bool isCurrentlyInUse(vk::Renderer *renderer) const;

    angle::Result mapImpl(ContextVk *contextVk, GLbitfield access, void **mapPtr);
    angle::Result mapRangeImpl(ContextVk *contextVk,
                               VkDeviceSize offset,
                               VkDeviceSize length,
                               GLbitfield access,
                               void **mapPtr);
    angle::Result unmapImpl(ContextVk *contextVk);
    angle::Result ghostMappedBuffer(ContextVk *contextVk,
                                    VkDeviceSize offset,
                                    VkDeviceSize length,
                                    GLbitfield access,
                                    void **mapPtr);

    VertexConversionBuffer *getVertexConversionBuffer(
        vk::Renderer *renderer,
        const VertexConversionBuffer::CacheKey &cacheKey);

  private:
    angle::Result updateBuffer(ContextVk *contextVk,
                               size_t bufferSize,
                               const BufferDataSource &dataSource,
                               size_t size,
                               size_t offset);
    angle::Result directUpdate(ContextVk *contextVk,
                               const BufferDataSource &dataSource,
                               size_t size,
                               size_t offset);
    angle::Result stagedUpdate(ContextVk *contextVk,
                               const BufferDataSource &dataSource,
                               size_t size,
                               size_t offset);
    angle::Result allocStagingBuffer(ContextVk *contextVk,
                                     vk::MemoryCoherency coherency,
                                     VkDeviceSize size,
                                     uint8_t **mapPtr);
    angle::Result flushStagingBuffer(ContextVk *contextVk, VkDeviceSize offset, VkDeviceSize size);
    angle::Result acquireAndUpdate(ContextVk *contextVk,
                                   size_t bufferSize,
                                   const BufferDataSource &dataSource,
                                   size_t updateSize,
                                   size_t updateOffset,
                                   BufferUpdateType updateType);
    angle::Result setDataWithMemoryType(const gl::Context *context,
                                        gl::BufferBinding target,
                                        const void *data,
                                        size_t size,
                                        VkMemoryPropertyFlags memoryPropertyFlags,
                                        gl::BufferUsage usage);
    angle::Result handleDeviceLocalBufferMap(ContextVk *contextVk,
                                             VkDeviceSize offset,
                                             VkDeviceSize size,
                                             uint8_t **mapPtr);
    angle::Result mapHostVisibleBuffer(ContextVk *contextVk,
                                       VkDeviceSize offset,
                                       GLbitfield access,
                                       uint8_t **mapPtr);
    angle::Result setDataImpl(ContextVk *contextVk,
                              size_t bufferSize,
                              const BufferDataSource &dataSource,
                              size_t updateSize,
                              size_t updateOffset,
                              BufferUpdateType updateType);
    angle::Result release(ContextVk *context);
    void dataUpdated();
    void dataRangeUpdated(const RangeDeviceSize &range);

    angle::Result acquireBufferHelper(ContextVk *contextVk,
                                      size_t sizeInBytes,
                                      BufferUsageType usageType);

    bool isExternalBuffer() const { return mClientBuffer != nullptr; }
    BufferUpdateType calculateBufferUpdateTypeOnFullUpdate(
        vk::Renderer *renderer,
        size_t size,
        VkMemoryPropertyFlags memoryPropertyFlags,
        BufferUsageType usageType,
        const void *data) const;
    bool shouldRedefineStorage(vk::Renderer *renderer,
                               BufferUsageType usageType,
                               VkMemoryPropertyFlags memoryPropertyFlags,
                               size_t size) const;

    void releaseConversionBuffers(vk::Context *context);

    vk::BufferHelper mBuffer;

    // If not null, this is the external memory pointer passed from client API.
    void *mClientBuffer;

    uint32_t mMemoryTypeIndex;
    // Memory/Usage property that will be used for memory allocation.
    VkMemoryPropertyFlags mMemoryPropertyFlags;

    // The staging buffer to aid map operations. This is used when buffers are not host visible or
    // for performance optimization when only a smaller range of buffer is mapped.
    vk::BufferHelper mStagingBuffer;

    // A cache of converted vertex data.
    std::vector<VertexConversionBuffer> mVertexConversionBuffers;

    // Tracks whether mStagingBuffer has been mapped to user or not
    bool mIsStagingBufferMapped;

    // Tracks if BufferVk object has valid data or not.
    bool mHasValidData;

    // True if the buffer is currently mapped for CPU write access. If the map call is originated
    // from OpenGLES API call, then this should be consistent with mState.getAccessFlags() bits.
    // Otherwise it is mapped from ANGLE internal and will not be consistent with mState access
    // bits, so we have to keep record of it.
    bool mIsMappedForWrite;
    // True if usage is dynamic. May affect how we allocate memory.
    BufferUsageType mUsageType;
    // Similar as mIsMappedForWrite, this maybe different from mState's getMapOffset/getMapLength if
    // mapped from angle internal.
    RangeDeviceSize mMappedRange;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_BUFFERVK_H_
