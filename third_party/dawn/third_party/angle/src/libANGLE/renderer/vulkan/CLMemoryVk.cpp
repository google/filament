//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryVk.cpp: Implements the class methods for CLMemoryVk.

#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/vk_cl_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

#include "libANGLE/renderer/CLMemoryImpl.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/FormatID_autogen.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/Error.h"
#include "libANGLE/cl_utils.h"

#include "CL/cl_half.h"

namespace rx
{
namespace
{
cl_int NormalizeFloatValue(float value, float maximum)
{
    if (value < 0)
    {
        return 0;
    }
    if (value > 1.f)
    {
        return static_cast<cl_int>(maximum);
    }
    float valueToRound = (value * maximum);

    if (fabsf(valueToRound) < 0x1.0p23f)
    {
        constexpr float magic[2] = {0x1.0p23f, -0x1.0p23f};
        float magicVal           = magic[valueToRound < 0.0f];
        valueToRound += magicVal;
        valueToRound -= magicVal;
    }

    return static_cast<cl_int>(valueToRound);
}

angle::FormatID CLImageFormatToAngleFormat(cl_image_format format)
{
    switch (format.image_channel_order)
    {
        case CL_R:
        case CL_LUMINANCE:
        case CL_INTENSITY:
            return angle::Format::CLRFormatToID(format.image_channel_data_type);
        case CL_RG:
            return angle::Format::CLRGFormatToID(format.image_channel_data_type);
        case CL_RGB:
            return angle::Format::CLRGBFormatToID(format.image_channel_data_type);
        case CL_RGBA:
            return angle::Format::CLRGBAFormatToID(format.image_channel_data_type);
        case CL_BGRA:
            return angle::Format::CLBGRAFormatToID(format.image_channel_data_type);
        case CL_sRGBA:
            return angle::Format::CLsRGBAFormatToID(format.image_channel_data_type);
        case CL_DEPTH:
            return angle::Format::CLDEPTHFormatToID(format.image_channel_data_type);
        case CL_DEPTH_STENCIL:
            return angle::Format::CLDEPTHSTENCILFormatToID(format.image_channel_data_type);
        default:
            return angle::FormatID::NONE;
    }
}

}  // namespace

CLMemoryVk::CLMemoryVk(const cl::Memory &memory)
    : CLMemoryImpl(memory),
      mContext(&memory.getContext().getImpl<CLContextVk>()),
      mRenderer(mContext->getRenderer()),
      mMappedMemory(nullptr),
      mMapCount(0),
      mParent(nullptr)
{}

CLMemoryVk::~CLMemoryVk()
{
    mContext->mAssociatedObjects->mMemories.erase(mMemory.getNative());
}

VkBufferUsageFlags CLMemoryVk::getVkUsageFlags()
{
    return cl_vk::GetBufferUsageFlags(mMemory.getFlags());
}

VkMemoryPropertyFlags CLMemoryVk::getVkMemPropertyFlags()
{
    return cl_vk::GetMemoryPropertyFlags(mMemory.getFlags());
}

angle::Result CLMemoryVk::map(uint8_t *&ptrOut, size_t offset)
{
    if (!isMapped())
    {
        ANGLE_TRY(mapImpl());
    }
    ptrOut = mMappedMemory + offset;
    return angle::Result::Continue;
}

angle::Result CLMemoryVk::copyTo(void *dst, size_t srcOffset, size_t size)
{
    uint8_t *src = nullptr;
    ANGLE_TRY(map(src, srcOffset));
    std::memcpy(dst, src, size);
    unmap();
    return angle::Result::Continue;
}

angle::Result CLMemoryVk::copyTo(CLMemoryVk *dst, size_t srcOffset, size_t dstOffset, size_t size)
{
    uint8_t *dstPtr = nullptr;
    ANGLE_TRY(dst->map(dstPtr, dstOffset));
    ANGLE_TRY(copyTo(dstPtr, srcOffset, size));
    dst->unmap();
    return angle::Result::Continue;
}

angle::Result CLMemoryVk::copyFrom(const void *src, size_t srcOffset, size_t size)
{
    uint8_t *dst = nullptr;
    ANGLE_TRY(map(dst, srcOffset));
    std::memcpy(dst, src, size);
    unmap();
    return angle::Result::Continue;
}

// Create a sub-buffer from the given buffer object
angle::Result CLMemoryVk::createSubBuffer(const cl::Buffer &buffer,
                                          cl::MemFlags flags,
                                          size_t size,
                                          CLMemoryImpl::Ptr *subBufferOut)
{
    ASSERT(buffer.isSubBuffer());

    CLBufferVk *bufferVk = new CLBufferVk(buffer);
    if (!bufferVk)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(bufferVk->create(nullptr));
    *subBufferOut = CLMemoryImpl::Ptr(bufferVk);

    return angle::Result::Continue;
}

CLBufferVk::CLBufferVk(const cl::Buffer &buffer) : CLMemoryVk(buffer)
{
    if (buffer.isSubBuffer())
    {
        mParent = &buffer.getParent()->getImpl<CLBufferVk>();
    }
    mDefaultBufferCreateInfo             = {};
    mDefaultBufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    mDefaultBufferCreateInfo.size        = buffer.getSize();
    mDefaultBufferCreateInfo.usage       = getVkUsageFlags();
    mDefaultBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}

CLBufferVk::~CLBufferVk()
{
    if (isMapped())
    {
        unmap();
    }
    mBuffer.destroy(mRenderer);
}

vk::BufferHelper &CLBufferVk::getBuffer()
{
    if (isSubBuffer())
    {
        return getParent()->getBuffer();
    }
    return mBuffer;
}

angle::Result CLBufferVk::create(void *hostPtr)
{
    if (!isSubBuffer())
    {
        VkBufferCreateInfo createInfo  = mDefaultBufferCreateInfo;
        createInfo.size                = getSize();
        VkMemoryPropertyFlags memFlags = getVkMemPropertyFlags();
        if (IsError(mBuffer.init(mContext, createInfo, memFlags)))
        {
            ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
        }
        if (mMemory.getFlags().intersects(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))
        {
            ASSERT(hostPtr);
            ANGLE_CL_IMPL_TRY_ERROR(setDataImpl(static_cast<uint8_t *>(hostPtr), getSize(), 0),
                                    CL_OUT_OF_RESOURCES);
        }
    }
    return angle::Result::Continue;
}

angle::Result CLBufferVk::copyToWithPitch(void *hostPtr,
                                          size_t srcOffset,
                                          size_t size,
                                          size_t rowPitch,
                                          size_t slicePitch,
                                          cl::Coordinate region,
                                          const size_t elementSize)
{
    uint8_t *ptrInBase  = nullptr;
    uint8_t *ptrOutBase = nullptr;
    cl::BufferRect stagingBufferRect{
        {static_cast<int>(0), static_cast<int>(0), static_cast<int>(0)},
        {region.x, region.y, region.z},
        0,
        0,
        elementSize};

    ptrOutBase = static_cast<uint8_t *>(hostPtr);
    ANGLE_TRY(getBuffer().map(mContext, &ptrInBase));

    for (size_t slice = 0; slice < region.z; slice++)
    {
        for (size_t row = 0; row < region.y; row++)
        {
            size_t stagingBufferOffset = stagingBufferRect.getRowOffset(slice, row);
            size_t hostPtrOffset       = (slice * slicePitch + row * rowPitch);
            uint8_t *dst               = ptrOutBase + hostPtrOffset;
            uint8_t *src               = ptrInBase + stagingBufferOffset;
            memcpy(dst, src, region.x * elementSize);
        }
    }
    getBuffer().unmap(mContext->getRenderer());
    return angle::Result::Continue;
}

angle::Result CLBufferVk::mapImpl()
{
    ASSERT(!isMapped());

    if (isSubBuffer())
    {
        ANGLE_TRY(mParent->map(mMappedMemory, getOffset()));
        return angle::Result::Continue;
    }
    ANGLE_TRY(mBuffer.map(mContext, &mMappedMemory));
    return angle::Result::Continue;
}

void CLBufferVk::unmapImpl()
{
    if (!isSubBuffer())
    {
        mBuffer.unmap(mRenderer);
    }
    mMappedMemory = nullptr;
}

angle::Result CLBufferVk::setRect(const void *data,
                                  const cl::BufferRect &srcRect,
                                  const cl::BufferRect &rect)
{
    ASSERT(srcRect.valid() && rect.valid());
    ASSERT(srcRect.mSize == rect.mSize);

    uint8_t *mapPtr = nullptr;
    ANGLE_TRY(map(mapPtr));
    const uint8_t *srcData = reinterpret_cast<const uint8_t *>(data);
    for (size_t slice = 0; slice < rect.mSize.depth; slice++)
    {
        for (size_t row = 0; row < rect.mSize.height; row++)
        {
            const uint8_t *src = srcData + srcRect.getRowOffset(slice, row);
            uint8_t *dst       = mapPtr + rect.getRowOffset(slice, row);

            memcpy(dst, src, srcRect.mSize.width * srcRect.mElementSize);
        }
    }

    return angle::Result::Continue;
}

angle::Result CLBufferVk::getRect(const cl::BufferRect &srcRect,
                                  const cl::BufferRect &outRect,
                                  void *outData)
{
    ASSERT(srcRect.valid() && outRect.valid());
    ASSERT(srcRect.mSize == outRect.mSize);

    uint8_t *mapPtr = nullptr;
    ANGLE_TRY(map(mapPtr));
    uint8_t *dstData = reinterpret_cast<uint8_t *>(outData);
    for (size_t slice = 0; slice < srcRect.mSize.depth; slice++)
    {
        for (size_t row = 0; row < srcRect.mSize.height; row++)
        {
            const uint8_t *src = mapPtr + srcRect.getRowOffset(slice, row);
            uint8_t *dst       = dstData + outRect.getRowOffset(slice, row);

            memcpy(dst, src, srcRect.mSize.width * srcRect.mElementSize);
        }
    }

    return angle::Result::Continue;
}

std::vector<VkBufferCopy> CLBufferVk::rectCopyRegions(const cl::BufferRect &bufferRect)
{
    std::vector<VkBufferCopy> copyRegions;
    for (unsigned int slice = 0; slice < bufferRect.mSize.depth; slice++)
    {
        for (unsigned int row = 0; row < bufferRect.mSize.height; row++)
        {
            VkBufferCopy copyRegion = {};
            copyRegion.size         = bufferRect.mSize.width * bufferRect.mElementSize;
            copyRegion.srcOffset = copyRegion.dstOffset = bufferRect.getRowOffset(slice, row);
            copyRegions.push_back(copyRegion);
        }
    }
    return copyRegions;
}

// offset is for mapped pointer
angle::Result CLBufferVk::setDataImpl(const uint8_t *data, size_t size, size_t offset)
{
    // buffer cannot be in use state
    ASSERT(mBuffer.valid());
    ASSERT(!isCurrentlyInUse());
    ASSERT(size + offset <= getSize());
    ASSERT(data != nullptr);

    // Assuming host visible buffers for now
    // TODO: http://anglebug.com/42267019
    if (!mBuffer.isHostVisible())
    {
        UNIMPLEMENTED();
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    uint8_t *mapPointer = nullptr;
    ANGLE_TRY(mBuffer.mapWithOffset(mContext, &mapPointer, offset));
    ASSERT(mapPointer != nullptr);

    std::memcpy(mapPointer, data, size);
    mBuffer.unmap(mRenderer);

    return angle::Result::Continue;
}

bool CLBufferVk::isCurrentlyInUse() const
{
    return !mRenderer->hasResourceUseFinished(mBuffer.getResourceUse());
}

VkImageUsageFlags CLImageVk::getVkImageUsageFlags()
{
    VkImageUsageFlags usageFlags =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (mMemory.getFlags().intersects(CL_MEM_WRITE_ONLY))
    {
        usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    else if (mMemory.getFlags().intersects(CL_MEM_READ_ONLY))
    {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    else
    {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    }

    return usageFlags;
}

VkImageType CLImageVk::getVkImageType(const cl::ImageDescriptor &desc)
{
    VkImageType imageType = VK_IMAGE_TYPE_MAX_ENUM;

    switch (desc.type)
    {
        case cl::MemObjectType::Image1D_Buffer:
        case cl::MemObjectType::Image1D:
        case cl::MemObjectType::Image1D_Array:
            return VK_IMAGE_TYPE_1D;
        case cl::MemObjectType::Image2D:
        case cl::MemObjectType::Image2D_Array:
            return VK_IMAGE_TYPE_2D;
        case cl::MemObjectType::Image3D:
            return VK_IMAGE_TYPE_3D;
        default:
            UNREACHABLE();
    }

    return imageType;
}

angle::Result CLImageVk::createStagingBuffer(size_t size)
{
    const VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                           nullptr,
                                           0,
                                           size,
                                           getVkUsageFlags(),
                                           VK_SHARING_MODE_EXCLUSIVE,
                                           0,
                                           nullptr};
    if (IsError(mStagingBuffer.init(mContext, createInfo, getVkMemPropertyFlags())))
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    mStagingBufferInitialized = true;
    return angle::Result::Continue;
}

angle::Result CLImageVk::copyStagingFrom(void *ptr, size_t offset, size_t size)
{
    uint8_t *ptrOut;
    uint8_t *ptrIn = static_cast<uint8_t *>(ptr);
    ANGLE_TRY(getStagingBuffer().map(mContext, &ptrOut));
    std::memcpy(ptrOut, ptrIn + offset, size);
    ANGLE_TRY(getStagingBuffer().flush(mRenderer));
    getStagingBuffer().unmap(mContext->getRenderer());
    return angle::Result::Continue;
}

angle::Result CLImageVk::copyStagingTo(void *ptr, size_t offset, size_t size)
{
    uint8_t *ptrOut;
    ANGLE_TRY(getStagingBuffer().map(mContext, &ptrOut));
    ANGLE_TRY(getStagingBuffer().invalidate(mRenderer));
    std::memcpy(ptr, ptrOut + offset, size);
    getStagingBuffer().unmap(mContext->getRenderer());
    return angle::Result::Continue;
}

angle::Result CLImageVk::copyStagingToFromWithPitch(void *hostPtr,
                                                    const cl::Coordinate &region,
                                                    const size_t rowPitch,
                                                    const size_t slicePitch,
                                                    StagingBufferCopyDirection copyStagingTo)
{
    uint8_t *ptrInBase  = nullptr;
    uint8_t *ptrOutBase = nullptr;
    cl::BufferRect stagingBufferRect{{}, {region.x, region.y, region.z}, 0, 0, getElementSize()};

    if (copyStagingTo == StagingBufferCopyDirection::ToHost)
    {
        ptrOutBase = static_cast<uint8_t *>(hostPtr);
        ANGLE_TRY(getStagingBuffer().map(mContext, &ptrInBase));
    }
    else
    {
        ptrInBase = static_cast<uint8_t *>(hostPtr);
        ANGLE_TRY(getStagingBuffer().map(mContext, &ptrOutBase));
    }
    for (size_t slice = 0; slice < region.z; slice++)
    {
        for (size_t row = 0; row < region.y; row++)
        {
            size_t stagingBufferOffset = stagingBufferRect.getRowOffset(slice, row);
            size_t hostPtrOffset       = (slice * slicePitch + row * rowPitch);
            uint8_t *dst               = (copyStagingTo == StagingBufferCopyDirection::ToHost)
                                             ? ptrOutBase + hostPtrOffset
                                             : ptrOutBase + stagingBufferOffset;
            uint8_t *src               = (copyStagingTo == StagingBufferCopyDirection::ToHost)
                                             ? ptrInBase + stagingBufferOffset
                                             : ptrInBase + hostPtrOffset;
            memcpy(dst, src, region.x * getElementSize());
        }
    }
    getStagingBuffer().unmap(mContext->getRenderer());
    return angle::Result::Continue;
}

CLImageVk::CLImageVk(const cl::Image &image)
    : CLMemoryVk(image),
      mExtent(cl::GetExtentFromDescriptor(image.getDescriptor())),
      mAngleFormat(CLImageFormatToAngleFormat(image.getFormat())),
      mStagingBufferInitialized(false),
      mImageViewType(cl_vk::GetImageViewType(image.getDescriptor().type))
{
    if (image.getParent())
    {
        mParent = &image.getParent()->getImpl<CLMemoryVk>();
    }
}

CLImageVk::~CLImageVk()
{
    if (isMapped())
    {
        unmap();
    }

    if (mBufferViews.isInitialized())
    {
        mBufferViews.release(mContext->getRenderer());
    }

    mImage.destroy(mRenderer);
    mImageView.destroy(mContext->getDevice());
    if (isStagingBufferInitialized())
    {
        mStagingBuffer.destroy(mRenderer);
    }
}

angle::Result CLImageVk::createFromBuffer()
{
    ASSERT(mParent);
    ASSERT(IsBufferType(getParentType()));

    // initialize the buffer views
    mBufferViews.init(mContext->getRenderer(), 0, getSize());

    return angle::Result::Continue;
}

angle::Result CLImageVk::create(void *hostPtr)
{
    if (mParent)
    {
        if (getType() == cl::MemObjectType::Image1D_Buffer)
        {
            return createFromBuffer();
        }
        else
        {
            UNIMPLEMENTED();
            ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
        }
    }

    ANGLE_CL_IMPL_TRY_ERROR(
        mImage.initStaging(mContext, false, mRenderer->getMemoryProperties(),
                           getVkImageType(getDescriptor()), cl_vk::GetExtent(mExtent), mAngleFormat,
                           mAngleFormat, VK_SAMPLE_COUNT_1_BIT, getVkImageUsageFlags(), 1,
                           (uint32_t)getArraySize()),
        CL_OUT_OF_RESOURCES);

    if (mMemory.getFlags().intersects(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))
    {
        ASSERT(hostPtr);
        ANGLE_CL_IMPL_TRY_ERROR(createStagingBuffer(getSize()), CL_OUT_OF_RESOURCES);
        if (getDescriptor().rowPitch == 0 && getDescriptor().slicePitch == 0)
        {
            ANGLE_CL_IMPL_TRY_ERROR(copyStagingFrom(hostPtr, 0, getSize()), CL_OUT_OF_RESOURCES);
        }
        else
        {
            ANGLE_TRY(copyStagingToFromWithPitch(
                hostPtr, {mExtent.width, mExtent.height, mExtent.depth}, getDescriptor().rowPitch,
                getDescriptor().slicePitch, StagingBufferCopyDirection::ToStagingBuffer));
        }
        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset      = 0;
        copyRegion.bufferRowLength   = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageExtent =
            cl_vk::GetExtent(getExtentForCopy({mExtent.width, mExtent.height, mExtent.depth}));
        copyRegion.imageOffset      = cl_vk::GetOffset(getOffsetForCopy(cl::kMemOffsetsZero));
        copyRegion.imageSubresource = getSubresourceLayersForCopy(
            cl::kMemOffsetsZero, {mExtent.width, mExtent.height, mExtent.depth}, getType(),
            ImageCopyWith::Buffer);

        ANGLE_CL_IMPL_TRY_ERROR(mImage.copyToBufferOneOff(mContext, &mStagingBuffer, copyRegion),
                                CL_OUT_OF_RESOURCES);
    }

    ANGLE_TRY(initImageViewImpl());
    return angle::Result::Continue;
}

angle::Result CLImageVk::initImageViewImpl()
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.flags                 = 0;
    viewInfo.image                 = getImage().getImage().getHandle();
    viewInfo.format                = getImage().getActualVkFormat(mContext->getRenderer());
    viewInfo.viewType              = mImageViewType;

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // We don't support mip map levels and should have been validated
    ASSERT(getDescriptor().numMipLevels == 0);
    viewInfo.subresourceRange.baseMipLevel   = getDescriptor().numMipLevels;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = static_cast<uint32_t>(getArraySize());

    // no swizzle support for now
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageViewUsageCreateInfo imageViewUsageCreateInfo = {};
    imageViewUsageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
    imageViewUsageCreateInfo.usage = getVkImageUsageFlags();

    viewInfo.pNext = &imageViewUsageCreateInfo;

    ANGLE_VK_TRY(mContext, mImageView.init(mContext->getDevice(), viewInfo));
    return angle::Result::Continue;
}

bool CLImageVk::isCurrentlyInUse() const
{
    return !mRenderer->hasResourceUseFinished(mImage.getResourceUse());
}

bool CLImageVk::containsHostMemExtension()
{
    const vk::ExtensionNameList &enabledDeviceExtensions = mRenderer->getEnabledDeviceExtensions();
    return std::find(enabledDeviceExtensions.begin(), enabledDeviceExtensions.end(),
                     "VK_EXT_external_memory_host") != enabledDeviceExtensions.end();
}

void CLImageVk::packPixels(const void *fillColor, PixelColor *packedColor)
{
    size_t channelCount = cl::GetChannelCount(getFormat().image_channel_order);

    switch (getFormat().image_channel_data_type)
    {
        case CL_UNORM_INT8:
        {
            float *srcVector = static_cast<float *>(const_cast<void *>(fillColor));
            if (getFormat().image_channel_order == CL_BGRA)
            {
                packedColor->u8[0] =
                    static_cast<unsigned char>(NormalizeFloatValue(srcVector[2], 255.f));
                packedColor->u8[1] =
                    static_cast<unsigned char>(NormalizeFloatValue(srcVector[1], 255.f));
                packedColor->u8[2] =
                    static_cast<unsigned char>(NormalizeFloatValue(srcVector[0], 255.f));
                packedColor->u8[3] =
                    static_cast<unsigned char>(NormalizeFloatValue(srcVector[3], 255.f));
            }
            else
            {
                for (unsigned int i = 0; i < channelCount; i++)
                    packedColor->u8[i] =
                        static_cast<unsigned char>(NormalizeFloatValue(srcVector[i], 255.f));
            }
            break;
        }
        case CL_SIGNED_INT8:
        {
            int *srcVector = static_cast<int *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->s8[i] = static_cast<char>(std::clamp(srcVector[i], -128, 127));
            break;
        }
        case CL_UNSIGNED_INT8:
        {
            unsigned int *srcVector = static_cast<unsigned int *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->u8[i] = static_cast<unsigned char>(
                    std::clamp(static_cast<unsigned int>(srcVector[i]),
                               static_cast<unsigned int>(0), static_cast<unsigned int>(255)));
            break;
        }
        case CL_UNORM_INT16:
        {
            float *srcVector = static_cast<float *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->u16[i] =
                    static_cast<unsigned short>(NormalizeFloatValue(srcVector[i], 65535.f));
            break;
        }
        case CL_SIGNED_INT16:
        {
            int *srcVector = static_cast<int *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->s16[i] = static_cast<short>(std::clamp(srcVector[i], -32768, 32767));
            break;
        }
        case CL_UNSIGNED_INT16:
        {
            unsigned int *srcVector = static_cast<unsigned int *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->u16[i] = static_cast<unsigned short>(
                    std::clamp(static_cast<unsigned int>(srcVector[i]),
                               static_cast<unsigned int>(0), static_cast<unsigned int>(65535)));
            break;
        }
        case CL_HALF_FLOAT:
        {
            float *srcVector = static_cast<float *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->fp16[i] = cl_half_from_float(srcVector[i], CL_HALF_RTE);
            break;
        }
        case CL_SIGNED_INT32:
        {
            int *srcVector = static_cast<int *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->s32[i] = static_cast<int>(srcVector[i]);
            break;
        }
        case CL_UNSIGNED_INT32:
        {
            unsigned int *srcVector = static_cast<unsigned int *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->u32[i] = static_cast<unsigned int>(srcVector[i]);
            break;
        }
        case CL_FLOAT:
        {
            float *srcVector = static_cast<float *>(const_cast<void *>(fillColor));
            for (unsigned int i = 0; i < channelCount; i++)
                packedColor->fp32[i] = srcVector[i];
            break;
        }
        default:
            UNIMPLEMENTED();
            break;
    }
}

void CLImageVk::fillImageWithColor(const cl::MemOffsets &origin,
                                   const cl::Coordinate &region,
                                   uint8_t *imagePtr,
                                   PixelColor *packedColor)
{
    size_t elementSize = getElementSize();
    cl::BufferRect stagingBufferRect{
        {}, {mExtent.width, mExtent.height, mExtent.depth}, 0, 0, elementSize};
    uint8_t *ptrBase = imagePtr + (origin.z * stagingBufferRect.getSlicePitch()) +
                       (origin.y * stagingBufferRect.getRowPitch()) + (origin.x * elementSize);
    for (size_t slice = 0; slice < region.z; slice++)
    {
        for (size_t row = 0; row < region.y; row++)
        {
            size_t stagingBufferOffset = stagingBufferRect.getRowOffset(slice, row);
            uint8_t *pixelPtr          = ptrBase + stagingBufferOffset;
            for (size_t x = 0; x < region.x; x++)
            {
                memcpy(pixelPtr, packedColor, elementSize);
                pixelPtr += elementSize;
            }
        }
    }
}

cl::Extents CLImageVk::getExtentForCopy(const cl::Coordinate &region)
{
    cl::Extents extent = {};
    extent.width       = region.x;
    extent.height      = region.y;
    extent.depth       = region.z;
    switch (getDescriptor().type)
    {
        case cl::MemObjectType::Image1D_Array:

            extent.height = 1;
            extent.depth  = 1;
            break;
        case cl::MemObjectType::Image2D_Array:
            extent.depth = 1;
            break;
        default:
            break;
    }
    return extent;
}

cl::Offset CLImageVk::getOffsetForCopy(const cl::MemOffsets &origin)
{
    cl::Offset offset = {};
    offset.x          = origin.x;
    offset.y          = origin.y;
    offset.z          = origin.z;
    switch (getDescriptor().type)
    {
        case cl::MemObjectType::Image1D_Array:
            offset.y = 0;
            offset.z = 0;
            break;
        case cl::MemObjectType::Image2D_Array:
            offset.z = 0;
            break;
        default:
            break;
    }
    return offset;
}

VkImageSubresourceLayers CLImageVk::getSubresourceLayersForCopy(const cl::MemOffsets &origin,
                                                                const cl::Coordinate &region,
                                                                cl::MemObjectType copyToType,
                                                                ImageCopyWith imageCopy)
{
    VkImageSubresourceLayers subresource = {};
    subresource.aspectMask               = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel                 = 0;
    switch (getDescriptor().type)
    {
        case cl::MemObjectType::Image1D_Array:
            subresource.baseArrayLayer = static_cast<uint32_t>(origin.y);
            if (imageCopy == ImageCopyWith::Image)
            {
                subresource.layerCount = static_cast<uint32_t>(region.y);
            }
            else
            {
                subresource.layerCount = static_cast<uint32_t>(getArraySize());
            }
            break;
        case cl::MemObjectType::Image2D_Array:
            subresource.baseArrayLayer = static_cast<uint32_t>(origin.z);
            if (copyToType == cl::MemObjectType::Image2D ||
                copyToType == cl::MemObjectType::Image3D)
            {
                subresource.layerCount = 1;
            }
            else if (imageCopy == ImageCopyWith::Image)
            {
                subresource.layerCount = static_cast<uint32_t>(region.z);
            }
            else
            {
                subresource.layerCount = static_cast<uint32_t>(getArraySize());
            }
            break;
        default:
            subresource.baseArrayLayer = 0;
            subresource.layerCount     = 1;
            break;
    }
    return subresource;
}

angle::Result CLImageVk::mapImpl()
{
    ASSERT(!isMapped());

    if (mParent)
    {
        ANGLE_TRY(mParent->map(mMappedMemory, getOffset()));
        return angle::Result::Continue;
    }

    ASSERT(isStagingBufferInitialized());
    ANGLE_TRY(getStagingBuffer().map(mContext, &mMappedMemory));

    return angle::Result::Continue;
}
void CLImageVk::unmapImpl()
{
    if (!mParent)
    {
        getStagingBuffer().unmap(mContext->getRenderer());
    }
    mMappedMemory = nullptr;
}

size_t CLImageVk::getRowPitch() const
{
    return getFrontendObject().getRowSize();
}

size_t CLImageVk::getSlicePitch() const
{
    return getFrontendObject().getSliceSize();
}

template <>
CLBufferVk *CLImageVk::getParent<CLBufferVk>() const
{
    if (mParent)
    {
        ASSERT(cl::IsBufferType(getParentType()));
        return static_cast<CLBufferVk *>(mParent);
    }
    return nullptr;
}

template <>
CLImageVk *CLImageVk::getParent<CLImageVk>() const
{
    if (mParent)
    {
        ASSERT(cl::IsImageType(getParentType()));
        return static_cast<CLImageVk *>(mParent);
    }
    return nullptr;
}

cl::MemObjectType CLImageVk::getParentType() const
{
    if (mParent)
    {
        return mParent->getType();
    }
    return cl::MemObjectType::InvalidEnum;
}

angle::Result CLImageVk::getBufferView(const vk::BufferView **viewOut)
{
    if (!mBufferViews.isInitialized())
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    CLBufferVk *parent = getParent<CLBufferVk>();

    return mBufferViews.getView(
        mContext, parent->getBuffer(), parent->getOffset(),
        mContext->getRenderer()->getFormat(CLImageFormatToAngleFormat(getFormat())), viewOut);
}

}  // namespace rx
