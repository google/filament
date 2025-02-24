//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryVk.h: Defines the class interface for CLMemoryVk, implementing CLMemoryImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLMEMORYVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLMEMORYVK_H_

#include "common/PackedCLEnums_autogen.h"
#include "common/SimpleMutex.h"

#include "libANGLE/renderer/vulkan/cl_types.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

#include "libANGLE/renderer/CLMemoryImpl.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLMemory.h"

#include "libANGLE/renderer/vulkan/vk_wrapper.h"
#include "vulkan/vulkan_core.h"

namespace rx
{

union PixelColor
{
    uint8_t u8[4];
    int8_t s8[4];
    uint16_t u16[4];
    int16_t s16[4];
    uint32_t u32[4];
    int32_t s32[4];
    cl_half fp16[4];
    cl_float fp32[4];
};

class CLMemoryVk : public CLMemoryImpl
{
  public:
    ~CLMemoryVk() override;

    // TODO: http://anglebug.com/42267017
    angle::Result createSubBuffer(const cl::Buffer &buffer,
                                  cl::MemFlags flags,
                                  size_t size,
                                  CLMemoryImpl::Ptr *subBufferOut) override;

    angle::Result map(uint8_t *&ptrOut, size_t offset = 0);
    void unmap() { unmapImpl(); }

    VkBufferUsageFlags getVkUsageFlags();
    VkMemoryPropertyFlags getVkMemPropertyFlags();
    virtual size_t getSize() const = 0;
    size_t getOffset() const { return mMemory.getOffset(); }
    cl::MemFlags getFlags() const { return mMemory.getFlags(); }
    cl::MemObjectType getType() const { return mMemory.getType(); }

    angle::Result copyTo(void *ptr, size_t offset, size_t size);
    angle::Result copyTo(CLMemoryVk *dst, size_t srcOffset, size_t dstOffset, size_t size);
    angle::Result copyFrom(const void *ptr, size_t offset, size_t size);

    bool isWritable()
    {
        constexpr VkBufferUsageFlags kWritableUsage =
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        return (getVkUsageFlags() & kWritableUsage) != 0;
    }

    virtual bool isCurrentlyInUse() const = 0;
    bool isMapped() const { return mMappedMemory != nullptr; }

  protected:
    CLMemoryVk(const cl::Memory &memory);

    virtual angle::Result mapImpl() = 0;
    virtual void unmapImpl()        = 0;

    CLContextVk *mContext;
    vk::Renderer *mRenderer;
    vk::Allocation mAllocation;
    angle::SimpleMutex mMapLock;
    uint8_t *mMappedMemory;
    uint32_t mMapCount;
    CLMemoryVk *mParent;
};

class CLBufferVk : public CLMemoryVk
{
  public:
    CLBufferVk(const cl::Buffer &buffer);
    ~CLBufferVk() override;

    vk::BufferHelper &getBuffer();
    CLBufferVk *getParent() { return static_cast<CLBufferVk *>(mParent); }
    const cl::Buffer &getFrontendObject() { return reinterpret_cast<const cl::Buffer &>(mMemory); }

    angle::Result create(void *hostPtr);
    angle::Result createStagingBuffer(size_t size);
    angle::Result copyToWithPitch(void *hostPtr,
                                  size_t srcOffset,
                                  size_t size,
                                  size_t rowPitch,
                                  size_t slicePitch,
                                  cl::Coordinate region,
                                  const size_t elementSize);

    angle::Result fillWithPattern(const void *pattern,
                                  size_t patternSize,
                                  size_t offset,
                                  size_t size)
    {
        getBuffer().fillWithPattern(pattern, patternSize, getOffset() + offset, size);
        return angle::Result::Continue;
    }

    bool isSubBuffer() const { return mParent != nullptr; }

    angle::Result setRect(const void *data,
                          const cl::BufferRect &srcRect,
                          const cl::BufferRect &bufferRect);
    angle::Result getRect(const cl::BufferRect &srcRect,
                          const cl::BufferRect &outRect,
                          void *outData);
    std::vector<VkBufferCopy> rectCopyRegions(const cl::BufferRect &bufferRect);

    bool isCurrentlyInUse() const override;
    size_t getSize() const override { return mMemory.getSize(); }

  private:
    angle::Result mapImpl() override;
    void unmapImpl() override;

    angle::Result setDataImpl(const uint8_t *data, size_t size, size_t offset);

    vk::BufferHelper mBuffer;
    VkBufferCreateInfo mDefaultBufferCreateInfo;
};

class CLImageVk : public CLMemoryVk
{
  public:
    CLImageVk(const cl::Image &image);
    ~CLImageVk() override;

    vk::ImageHelper &getImage() { return mImage; }
    vk::BufferHelper &getStagingBuffer() { return mStagingBuffer; }
    const cl::Image &getFrontendObject() const
    {
        return reinterpret_cast<const cl::Image &>(mMemory);
    }
    cl_image_format getFormat() const { return getFrontendObject().getFormat(); }
    cl::ImageDescriptor getDescriptor() const { return getFrontendObject().getDescriptor(); }
    size_t getElementSize() const { return getFrontendObject().getElementSize(); }
    size_t getArraySize() const { return getFrontendObject().getArraySize(); }
    size_t getSize() const override { return mMemory.getSize(); }
    size_t getRowPitch() const;
    size_t getSlicePitch() const;

    cl::MemObjectType getParentType() const;
    template <typename T>
    T *getParent() const;

    angle::Result create(void *hostPtr);
    angle::Result createFromBuffer();

    bool isCurrentlyInUse() const override;
    bool containsHostMemExtension();

    angle::Result createStagingBuffer(size_t size);
    angle::Result copyStagingFrom(void *ptr, size_t offset, size_t size);
    angle::Result copyStagingTo(void *ptr, size_t offset, size_t size);
    angle::Result copyStagingToFromWithPitch(void *ptr,
                                             const cl::Coordinate &region,
                                             const size_t rowPitch,
                                             const size_t slicePitch,
                                             StagingBufferCopyDirection copyStagingTo);
    VkImageUsageFlags getVkImageUsageFlags();
    VkImageType getVkImageType(const cl::ImageDescriptor &desc);
    bool isStagingBufferInitialized() { return mStagingBufferInitialized; }
    cl::Extents getImageExtent() { return mExtent; }
    uint8_t *getMappedPtr() { return mMappedMemory; }
    vk::ImageView &getImageView() { return mImageView; }
    void packPixels(const void *fillColor, PixelColor *packedColor);
    void fillImageWithColor(const cl::MemOffsets &origin,
                            const cl::Coordinate &region,
                            uint8_t *imagePtr,
                            PixelColor *packedColor);
    cl::Extents getExtentForCopy(const cl::Coordinate &region);
    cl::Offset getOffsetForCopy(const cl::MemOffsets &origin);
    VkImageSubresourceLayers getSubresourceLayersForCopy(const cl::MemOffsets &origin,
                                                         const cl::Coordinate &region,
                                                         cl::MemObjectType copyToType,
                                                         ImageCopyWith imageCopy);

    angle::Result getBufferView(const vk::BufferView **viewOut);

  private:
    angle::Result initImageViewImpl();

    angle::Result mapImpl() override;
    void unmapImpl() override;
    angle::Result setDataImpl(const uint8_t *data, size_t size, size_t offset);
    size_t calculateRowPitch();
    size_t calculateSlicePitch(size_t imageRowPitch);

    vk::ImageHelper mImage;
    vk::BufferHelper mStagingBuffer;
    cl::Extents mExtent;
    angle::FormatID mAngleFormat;
    bool mStagingBufferInitialized;
    vk::ImageView mImageView;
    VkImageViewType mImageViewType;

    // Images created from buffer create texel buffer views. BufferViewHelper contain the view
    // corresponding to the attached buffer.
    vk::BufferViewHelper mBufferViews;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLMEMORYVK_H_
