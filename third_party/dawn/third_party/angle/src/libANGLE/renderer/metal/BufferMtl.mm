//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferMtl.mm:
//    Implements the class methods for BufferMtl.
//

#include "libANGLE/renderer/metal/BufferMtl.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/mtl_buffer_manager.h"

namespace rx
{

namespace
{

// Start with a fairly small buffer size. We can increase this dynamically as we convert more data.
constexpr size_t kConvertedElementArrayBufferInitialSize = 1024 * 8;

template <typename IndexType>
angle::Result GetFirstLastIndices(const IndexType *indices,
                                  size_t count,
                                  std::pair<uint32_t, uint32_t> *outIndices)
{
    IndexType first, last;
    // Use memcpy to avoid unaligned memory access crash:
    memcpy(&first, &indices[0], sizeof(first));
    memcpy(&last, &indices[count - 1], sizeof(last));

    outIndices->first  = first;
    outIndices->second = last;

    return angle::Result::Continue;
}

bool isOffsetAndSizeMetalBlitCompatible(size_t offset, size_t size)
{
    // Metal requires offset and size to be multiples of 4
    return offset % 4 == 0 && size % 4 == 0;
}

}  // namespace

// ConversionBufferMtl implementation.
ConversionBufferMtl::ConversionBufferMtl(ContextMtl *contextMtl,
                                         size_t initialSize,
                                         size_t alignment)
    : dirty(true), convertedBuffer(nullptr), convertedOffset(0)
{
    data.initialize(contextMtl, initialSize, alignment, 0);
}

ConversionBufferMtl::~ConversionBufferMtl() = default;

// IndexConversionBufferMtl implementation.
IndexConversionBufferMtl::IndexConversionBufferMtl(ContextMtl *context,
                                                   gl::DrawElementsType elemTypeIn,
                                                   bool primitiveRestartEnabledIn,
                                                   size_t offsetIn)
    : ConversionBufferMtl(context,
                          kConvertedElementArrayBufferInitialSize,
                          mtl::kIndexBufferOffsetAlignment),
      elemType(elemTypeIn),
      offset(offsetIn),
      primitiveRestartEnabled(primitiveRestartEnabledIn)
{}

IndexRange IndexConversionBufferMtl::getRangeForConvertedBuffer(size_t count)
{
    return IndexRange{0, count};
}

// UniformConversionBufferMtl implementation
UniformConversionBufferMtl::UniformConversionBufferMtl(ContextMtl *context,
                                                       std::pair<size_t, size_t> offsetIn,
                                                       size_t uniformBufferBlockSize)
    : ConversionBufferMtl(context, 0, mtl::kUniformBufferSettingOffsetMinAlignment),
      uniformBufferBlockSize(uniformBufferBlockSize),
      offset(offsetIn)
{}

// VertexConversionBufferMtl implementation.
VertexConversionBufferMtl::VertexConversionBufferMtl(ContextMtl *context,
                                                     angle::FormatID formatIDIn,
                                                     GLuint strideIn,
                                                     size_t offsetIn)
    : ConversionBufferMtl(context, 0, mtl::kVertexAttribBufferStrideAlignment),
      formatID(formatIDIn),
      stride(strideIn),
      offset(offsetIn)
{}

// BufferMtl implementation
BufferMtl::BufferMtl(const gl::BufferState &state) : BufferImpl(state) {}

BufferMtl::~BufferMtl() {}

void BufferMtl::destroy(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    mShadowCopy.clear();

    // if there's a buffer, give it back to the buffer manager
    if (mBuffer)
    {
        contextMtl->getBufferManager().returnBuffer(contextMtl, mBuffer);
        mBuffer = nullptr;
    }

    clearConversionBuffers();
}

angle::Result BufferMtl::setData(const gl::Context *context,
                                 gl::BufferBinding target,
                                 const void *data,
                                 size_t intendedSize,
                                 gl::BufferUsage usage)
{
    return setDataImpl(context, target, data, intendedSize, usage);
}

angle::Result BufferMtl::setSubData(const gl::Context *context,
                                    gl::BufferBinding target,
                                    const void *data,
                                    size_t size,
                                    size_t offset)
{
    return setSubDataImpl(context, data, size, offset);
}

angle::Result BufferMtl::copySubData(const gl::Context *context,
                                     BufferImpl *source,
                                     GLintptr sourceOffset,
                                     GLintptr destOffset,
                                     GLsizeiptr size)
{
    if (!source)
    {
        return angle::Result::Continue;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);
    auto srcMtl            = GetAs<BufferMtl>(source);

    markConversionBuffersDirty();

    if (mShadowCopy.size() > 0)
    {
        if (srcMtl->clientShadowCopyDataNeedSync(contextMtl) ||
            mBuffer->isBeingUsedByGPU(contextMtl))
        {
            // If shadow copy requires a synchronization then use blit command instead.
            // It might break a pending render pass, but still faster than synchronization with
            // GPU.
            mtl::BlitCommandEncoder *blitEncoder = contextMtl->getBlitCommandEncoder();
            blitEncoder->copyBuffer(srcMtl->getCurrentBuffer(), sourceOffset, mBuffer, destOffset,
                                    size);

            return angle::Result::Continue;
        }
        return setSubDataImpl(context, srcMtl->getBufferDataReadOnly(contextMtl) + sourceOffset,
                              size, destOffset);
    }

    mtl::BlitCommandEncoder *blitEncoder = contextMtl->getBlitCommandEncoder();
    blitEncoder->copyBuffer(srcMtl->getCurrentBuffer(), sourceOffset, mBuffer, destOffset, size);

    return angle::Result::Continue;
}

angle::Result BufferMtl::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    GLbitfield mapRangeAccess = 0;
    if ((access & GL_WRITE_ONLY_OES) != 0 || (access & GL_READ_WRITE) != 0)
    {
        mapRangeAccess |= GL_MAP_WRITE_BIT;
    }
    return mapRange(context, 0, size(), mapRangeAccess, mapPtr);
}

angle::Result BufferMtl::mapRange(const gl::Context *context,
                                  size_t offset,
                                  size_t length,
                                  GLbitfield access,
                                  void **mapPtr)
{
    if (access & GL_MAP_INVALIDATE_BUFFER_BIT)
    {
        ANGLE_TRY(setDataImpl(context, gl::BufferBinding::InvalidEnum, nullptr, size(),
                              mState.getUsage()));
    }

    if (mapPtr)
    {
        ContextMtl *contextMtl = mtl::GetImpl(context);
        if (mShadowCopy.size() == 0)
        {
            *mapPtr = mBuffer->mapWithOpt(contextMtl, (access & GL_MAP_WRITE_BIT) == 0,
                                          access & GL_MAP_UNSYNCHRONIZED_BIT) +
                      offset;
        }
        else
        {
            *mapPtr = syncAndObtainShadowCopy(contextMtl) + offset;
        }
    }

    return angle::Result::Continue;
}

angle::Result BufferMtl::unmap(const gl::Context *context, GLboolean *result)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    size_t offset          = static_cast<size_t>(mState.getMapOffset());
    size_t len             = static_cast<size_t>(mState.getMapLength());

    markConversionBuffersDirty();

    if (mShadowCopy.size() == 0)
    {
        ASSERT(mBuffer);
        if (mState.getAccessFlags() & GL_MAP_WRITE_BIT)
        {
            mBuffer->unmapAndFlushSubset(contextMtl, offset, len);
        }
        else
        {
            // Buffer is already mapped with readonly flag, so just unmap it, no flushing will
            // occur.
            mBuffer->unmap(contextMtl);
        }
    }
    else
    {
        if (mState.getAccessFlags() & GL_MAP_UNSYNCHRONIZED_BIT)
        {
            // Copy the mapped region without synchronization with GPU
            uint8_t *ptr =
                mBuffer->mapWithOpt(contextMtl, /* readonly */ false, /* noSync */ true) + offset;
            std::copy(mShadowCopy.data() + offset, mShadowCopy.data() + offset + len, ptr);
            mBuffer->unmapAndFlushSubset(contextMtl, offset, len);
        }
        else
        {
            // commit shadow copy data to GPU synchronously
            ANGLE_TRY(commitShadowCopy(contextMtl));
        }
    }

    if (result)
    {
        *result = true;
    }

    return angle::Result::Continue;
}

angle::Result BufferMtl::getIndexRange(const gl::Context *context,
                                       gl::DrawElementsType type,
                                       size_t offset,
                                       size_t count,
                                       bool primitiveRestartEnabled,
                                       gl::IndexRange *outRange)
{
    const uint8_t *indices = getBufferDataReadOnly(mtl::GetImpl(context)) + offset;

    *outRange = gl::ComputeIndexRange(type, indices, count, primitiveRestartEnabled);

    return angle::Result::Continue;
}

angle::Result BufferMtl::getFirstLastIndices(ContextMtl *contextMtl,
                                             gl::DrawElementsType type,
                                             size_t offset,
                                             size_t count,
                                             std::pair<uint32_t, uint32_t> *outIndices)
{
    const uint8_t *indices = getBufferDataReadOnly(contextMtl) + offset;

    switch (type)
    {
        case gl::DrawElementsType::UnsignedByte:
            return GetFirstLastIndices(static_cast<const GLubyte *>(indices), count, outIndices);
        case gl::DrawElementsType::UnsignedShort:
            return GetFirstLastIndices(reinterpret_cast<const GLushort *>(indices), count,
                                       outIndices);
        case gl::DrawElementsType::UnsignedInt:
            return GetFirstLastIndices(reinterpret_cast<const GLuint *>(indices), count,
                                       outIndices);
        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

void BufferMtl::onDataChanged()
{
    markConversionBuffersDirty();
}

const uint8_t *BufferMtl::getBufferDataReadOnly(ContextMtl *contextMtl)
{
    if (mShadowCopy.size() == 0)
    {
        // Don't need shadow copy in this case, use the buffer directly
        return mBuffer->mapReadOnly(contextMtl);
    }
    return syncAndObtainShadowCopy(contextMtl);
}

bool BufferMtl::clientShadowCopyDataNeedSync(ContextMtl *contextMtl)
{
    return mBuffer->isCPUReadMemDirty();
}

void BufferMtl::ensureShadowCopySyncedFromGPU(ContextMtl *contextMtl)
{
    if (mBuffer->isCPUReadMemDirty())
    {
        const uint8_t *ptr = mBuffer->mapReadOnly(contextMtl);
        memcpy(mShadowCopy.data(), ptr, size());
        mBuffer->unmap(contextMtl);

        mBuffer->resetCPUReadMemDirty();
    }
}
uint8_t *BufferMtl::syncAndObtainShadowCopy(ContextMtl *contextMtl)
{
    ASSERT(mShadowCopy.size());

    ensureShadowCopySyncedFromGPU(contextMtl);

    return mShadowCopy.data();
}

ConversionBufferMtl *BufferMtl::getVertexConversionBuffer(ContextMtl *context,
                                                          angle::FormatID formatID,
                                                          GLuint stride,
                                                          size_t offset)
{
    for (VertexConversionBufferMtl &buffer : mVertexConversionBuffers)
    {
        if (buffer.formatID == formatID && buffer.stride == stride && buffer.offset <= offset &&
            buffer.offset % buffer.stride == offset % stride)
        {
            return &buffer;
        }
    }

    mVertexConversionBuffers.emplace_back(context, formatID, stride, offset);
    ConversionBufferMtl *conv        = &mVertexConversionBuffers.back();
    const angle::Format &angleFormat = angle::Format::Get(formatID);
    conv->data.updateAlignment(context, angleFormat.pixelBytes);

    return conv;
}

IndexConversionBufferMtl *BufferMtl::getIndexConversionBuffer(ContextMtl *context,
                                                              gl::DrawElementsType elemType,
                                                              bool primitiveRestartEnabled,
                                                              size_t offset)
{
    for (auto &buffer : mIndexConversionBuffers)
    {
        if (buffer.elemType == elemType && buffer.offset == offset &&
            buffer.primitiveRestartEnabled == primitiveRestartEnabled)
        {
            return &buffer;
        }
    }

    mIndexConversionBuffers.emplace_back(context, elemType, primitiveRestartEnabled, offset);
    return &mIndexConversionBuffers.back();
}

ConversionBufferMtl *BufferMtl::getUniformConversionBuffer(ContextMtl *context,
                                                           std::pair<size_t, size_t> offset,
                                                           size_t stdSize)
{
    for (UniformConversionBufferMtl &buffer : mUniformConversionBuffers)
    {
        if (buffer.offset.first == offset.first && buffer.uniformBufferBlockSize == stdSize)
        {
            if (buffer.offset.second <= offset.second &&
                (offset.second - buffer.offset.second) % buffer.uniformBufferBlockSize == 0)
                return &buffer;
        }
    }

    mUniformConversionBuffers.emplace_back(context, offset, stdSize);
    return &mUniformConversionBuffers.back();
}

void BufferMtl::markConversionBuffersDirty()
{
    for (VertexConversionBufferMtl &buffer : mVertexConversionBuffers)
    {
        buffer.dirty           = true;
        buffer.convertedBuffer = nullptr;
        buffer.convertedOffset = 0;
    }

    for (IndexConversionBufferMtl &buffer : mIndexConversionBuffers)
    {
        buffer.dirty           = true;
        buffer.convertedBuffer = nullptr;
        buffer.convertedOffset = 0;
    }

    for (UniformConversionBufferMtl &buffer : mUniformConversionBuffers)
    {
        buffer.dirty           = true;
        buffer.convertedBuffer = nullptr;
        buffer.convertedOffset = 0;
    }
    mRestartRangeCache.reset();
}

void BufferMtl::clearConversionBuffers()
{
    mVertexConversionBuffers.clear();
    mIndexConversionBuffers.clear();
    mUniformConversionBuffers.clear();
    mRestartRangeCache.reset();
}

template <typename T>
static std::vector<IndexRange> calculateRestartRanges(ContextMtl *ctx, mtl::BufferRef idxBuffer)
{
    std::vector<IndexRange> result;
    const T *bufferData       = reinterpret_cast<const T *>(idxBuffer->mapReadOnly(ctx));
    const size_t numIndices   = idxBuffer->size() / sizeof(T);
    constexpr T restartMarker = std::numeric_limits<T>::max();
    for (size_t i = 0; i < numIndices; ++i)
    {
        // Find the start of the restart range, i.e. first index with value of restart marker.
        if (bufferData[i] != restartMarker)
            continue;
        size_t restartBegin = i;
        // Find the end of the restart range, i.e. last index with value of restart marker.
        do
        {
            ++i;
        } while (i < numIndices && bufferData[i] == restartMarker);
        result.emplace_back(restartBegin, i - 1);
    }
    idxBuffer->unmap(ctx);
    return result;
}

const std::vector<IndexRange> &BufferMtl::getRestartIndices(ContextMtl *ctx,
                                                            gl::DrawElementsType indexType)
{
    if (!mRestartRangeCache || mRestartRangeCache->indexType != indexType)
    {
        mRestartRangeCache.reset();
        std::vector<IndexRange> ranges;
        switch (indexType)
        {
            case gl::DrawElementsType::UnsignedByte:
                ranges = calculateRestartRanges<uint8_t>(ctx, getCurrentBuffer());
                break;
            case gl::DrawElementsType::UnsignedShort:
                ranges = calculateRestartRanges<uint16_t>(ctx, getCurrentBuffer());
                break;
            case gl::DrawElementsType::UnsignedInt:
                ranges = calculateRestartRanges<uint32_t>(ctx, getCurrentBuffer());
                break;
            default:
                ASSERT(false);
        }
        mRestartRangeCache.emplace(std::move(ranges), indexType);
    }
    return mRestartRangeCache->ranges;
}

const std::vector<IndexRange> BufferMtl::getRestartIndicesFromClientData(
    ContextMtl *ctx,
    gl::DrawElementsType indexType,
    mtl::BufferRef idxBuffer)
{
    std::vector<IndexRange> restartIndices;
    switch (indexType)
    {
        case gl::DrawElementsType::UnsignedByte:
            restartIndices = calculateRestartRanges<uint8_t>(ctx, idxBuffer);
            break;
        case gl::DrawElementsType::UnsignedShort:
            restartIndices = calculateRestartRanges<uint16_t>(ctx, idxBuffer);
            break;
        case gl::DrawElementsType::UnsignedInt:
            restartIndices = calculateRestartRanges<uint32_t>(ctx, idxBuffer);
            break;
        default:
            ASSERT(false);
    }
    return restartIndices;
}

angle::Result BufferMtl::allocateNewMetalBuffer(ContextMtl *contextMtl,
                                                MTLStorageMode storageMode,
                                                size_t size,
                                                bool returnOldBufferImmediately)
{
    mtl::BufferManager &bufferManager = contextMtl->getBufferManager();
    if (returnOldBufferImmediately && mBuffer)
    {
        // Return the current buffer to the buffer manager
        // It will not be re-used until it's no longer in use.
        bufferManager.returnBuffer(contextMtl, mBuffer);
        mBuffer = nullptr;
    }
    ANGLE_TRY(bufferManager.getBuffer(contextMtl, storageMode, size, mBuffer));

    onStateChange(angle::SubjectMessage::InternalMemoryAllocationChanged);

    return angle::Result::Continue;
}

angle::Result BufferMtl::setDataImpl(const gl::Context *context,
                                     gl::BufferBinding target,
                                     const void *data,
                                     size_t intendedSize,
                                     gl::BufferUsage usage)
{
    ContextMtl *contextMtl             = mtl::GetImpl(context);
    const angle::FeaturesMtl &features = contextMtl->getDisplay()->getFeatures();

    // Invalidate conversion buffers
    if (mState.getSize() != static_cast<GLint64>(intendedSize))
    {
        clearConversionBuffers();
    }
    else
    {
        markConversionBuffersDirty();
    }

    mUsage              = usage;
    mGLSize             = intendedSize;
    size_t adjustedSize = std::max<size_t>(1, intendedSize);

    // Ensures no validation layer issues in std140 with data types like vec3 being 12 bytes vs 16
    // in MSL.
    if (target == gl::BufferBinding::Uniform)
    {
        // This doesn't work! A buffer can be allocated on ARRAY_BUFFER and used in UNIFORM_BUFFER
        // TODO(anglebug.com/42266052)
        adjustedSize = roundUpPow2(adjustedSize, (size_t)16);
    }

    // Re-create the buffer
    auto storageMode = mtl::Buffer::getStorageModeForUsage(contextMtl, usage);
    ANGLE_TRY(allocateNewMetalBuffer(contextMtl, storageMode, adjustedSize,
                                     /*returnOldBufferImmediately=*/true));

#ifndef NDEBUG
    ANGLE_MTL_OBJC_SCOPE
    {
        mBuffer->get().label = [NSString stringWithFormat:@"BufferMtl=%p", this];
    }
#endif

    // We may use shadow copy to maintain consistent data between buffers in pool
    size_t shadowSize = (!features.preferCpuForBuffersubdata.enabled &&
                         features.useShadowBuffersWhenAppropriate.enabled &&
                         adjustedSize <= mtl::kSharedMemBufferMaxBufSizeHint)
                            ? adjustedSize
                            : 0;
    ANGLE_CHECK_GL_ALLOC(contextMtl, mShadowCopy.resize(shadowSize));

    if (data)
    {
        ANGLE_TRY(setSubDataImpl(context, data, intendedSize, 0));
    }

    return angle::Result::Continue;
}

// states:
//  * The buffer is not use
//
//    safe = true
//
//  * The buffer has a pending blit
//
//    In this case, as long as we are only reading from it
//    via blit to a new buffer our blits will happen after existing
//    blits
//
//    safe = true
//
//  * The buffer has pending writes in a commited render encoder
//
//    In this case we're encoding commands that will happen after
//    that encoder
//
//    safe = true
//
//  * The buffer has pending writes in the current render encoder
//
//    in this case we have to split/end the render encoder
//    before we can use the buffer.
//
//    safe = false
bool BufferMtl::isSafeToReadFromBufferViaBlit(ContextMtl *contextMtl)
{
    uint64_t serial   = mBuffer->getLastWritingRenderEncoderSerial();
    bool isSameSerial = contextMtl->isCurrentRenderEncoderSerial(serial);
    return !isSameSerial;
}

angle::Result BufferMtl::updateExistingBufferViaBlitFromStagingBuffer(ContextMtl *contextMtl,
                                                                      const uint8_t *srcPtr,
                                                                      size_t sizeToCopy,
                                                                      size_t offset)
{
    ASSERT(isOffsetAndSizeMetalBlitCompatible(offset, sizeToCopy));

    mtl::BufferManager &bufferManager = contextMtl->getBufferManager();
    return bufferManager.queueBlitCopyDataToBuffer(contextMtl, srcPtr, sizeToCopy, offset, mBuffer);
}

// * get a new or unused buffer
// * copy the new data to it
// * copy any old data not overwriten by the new data to the new buffer
// * start using the new buffer
angle::Result BufferMtl::putDataInNewBufferAndStartUsingNewBuffer(ContextMtl *contextMtl,
                                                                  const uint8_t *srcPtr,
                                                                  size_t sizeToCopy,
                                                                  size_t offset)
{
    ASSERT(isOffsetAndSizeMetalBlitCompatible(offset, sizeToCopy));

    mtl::BufferRef oldBuffer = mBuffer;
    auto storageMode         = mtl::Buffer::getStorageModeForUsage(contextMtl, mUsage);

    ANGLE_TRY(allocateNewMetalBuffer(contextMtl, storageMode, mGLSize,
                                     /*returnOldBufferImmediately=*/false));
    mBuffer->get().label = [NSString stringWithFormat:@"BufferMtl=%p(%lu)", this, ++mRevisionCount];

    uint8_t *ptr = mBuffer->mapWithOpt(contextMtl, false, true);
    std::copy(srcPtr, srcPtr + sizeToCopy, ptr + offset);
    mBuffer->unmapAndFlushSubset(contextMtl, offset, sizeToCopy);

    if (offset > 0 || offset + sizeToCopy < mGLSize)
    {
        mtl::BlitCommandEncoder *blitEncoder =
            contextMtl->getBlitCommandEncoderWithoutEndingRenderEncoder();
        if (offset > 0)
        {
            // copy old data before updated region
            blitEncoder->copyBuffer(oldBuffer, 0, mBuffer, 0, offset);
        }
        if (offset + sizeToCopy < mGLSize)
        {
            // copy old data after updated region
            const size_t endOffset     = offset + sizeToCopy;
            const size_t endSizeToCopy = mGLSize - endOffset;
            blitEncoder->copyBuffer(oldBuffer, endOffset, mBuffer, endOffset, endSizeToCopy);
        }
    }

    mtl::BufferManager &bufferManager = contextMtl->getBufferManager();
    bufferManager.returnBuffer(contextMtl, oldBuffer);
    return angle::Result::Continue;
}

angle::Result BufferMtl::copyDataToExistingBufferViaCPU(ContextMtl *contextMtl,
                                                        const uint8_t *srcPtr,
                                                        size_t sizeToCopy,
                                                        size_t offset)
{
    uint8_t *ptr = mBuffer->map(contextMtl);
    std::copy(srcPtr, srcPtr + sizeToCopy, ptr + offset);
    mBuffer->unmapAndFlushSubset(contextMtl, offset, sizeToCopy);
    return angle::Result::Continue;
}

angle::Result BufferMtl::updateShadowCopyThenCopyShadowToNewBuffer(ContextMtl *contextMtl,
                                                                   const uint8_t *srcPtr,
                                                                   size_t sizeToCopy,
                                                                   size_t offset)
{
    // 1. Before copying data from client, we need to synchronize modified data from GPU to
    // shadow copy first.
    ensureShadowCopySyncedFromGPU(contextMtl);

    // 2. Copy data from client to shadow copy.
    std::copy(srcPtr, srcPtr + sizeToCopy, mShadowCopy.data() + offset);

    // 3. Copy data from shadow copy to GPU.
    return commitShadowCopy(contextMtl);
}

angle::Result BufferMtl::setSubDataImpl(const gl::Context *context,
                                        const void *data,
                                        size_t size,
                                        size_t offset)
{
    if (!data)
    {
        return angle::Result::Continue;
    }

    ASSERT(mBuffer);

    ContextMtl *contextMtl             = mtl::GetImpl(context);
    const angle::FeaturesMtl &features = contextMtl->getDisplay()->getFeatures();

    ANGLE_CHECK(contextMtl, offset <= mGLSize, gl::err::kInternalError, GL_INVALID_OPERATION);

    auto srcPtr     = static_cast<const uint8_t *>(data);
    auto sizeToCopy = std::min<size_t>(size, mGLSize - offset);

    markConversionBuffersDirty();

    if (features.preferCpuForBuffersubdata.enabled)
    {
        return copyDataToExistingBufferViaCPU(contextMtl, srcPtr, sizeToCopy, offset);
    }

    if (mShadowCopy.size() > 0)
    {
        return updateShadowCopyThenCopyShadowToNewBuffer(contextMtl, srcPtr, sizeToCopy, offset);
    }
    else
    {
        bool alwaysUseStagedBufferUpdates = features.alwaysUseStagedBufferUpdates.enabled;

        if (isOffsetAndSizeMetalBlitCompatible(offset, size) &&
            (alwaysUseStagedBufferUpdates || mBuffer->isBeingUsedByGPU(contextMtl)))
        {
            if (alwaysUseStagedBufferUpdates || !isSafeToReadFromBufferViaBlit(contextMtl))
            {
                // We can't use the buffer now so copy the data
                // to a staging buffer and blit it in
                return updateExistingBufferViaBlitFromStagingBuffer(contextMtl, srcPtr, sizeToCopy,
                                                                    offset);
            }
            else
            {
                return putDataInNewBufferAndStartUsingNewBuffer(contextMtl, srcPtr, sizeToCopy,
                                                                offset);
            }
        }
        else
        {
            return copyDataToExistingBufferViaCPU(contextMtl, srcPtr, sizeToCopy, offset);
        }
    }
}

angle::Result BufferMtl::commitShadowCopy(ContextMtl *contextMtl)
{
    return commitShadowCopy(contextMtl, mGLSize);
}

angle::Result BufferMtl::commitShadowCopy(ContextMtl *contextMtl, size_t size)
{
    auto storageMode = mtl::Buffer::getStorageModeForUsage(contextMtl, mUsage);

    size_t bufferSize = (mGLSize == 0 ? mShadowCopy.size() : mGLSize);
    ANGLE_TRY(allocateNewMetalBuffer(contextMtl, storageMode, bufferSize,
                                     /*returnOldBufferImmediately=*/true));

    if (size)
    {
        uint8_t *ptr = mBuffer->mapWithOpt(contextMtl, false, true);
        std::copy(mShadowCopy.data(), mShadowCopy.data() + size, ptr);
        mBuffer->unmapAndFlushSubset(contextMtl, 0, size);
    }

    return angle::Result::Continue;
}

// SimpleWeakBufferHolderMtl implementation
SimpleWeakBufferHolderMtl::SimpleWeakBufferHolderMtl()
{
    mIsWeak = true;
}

}  // namespace rx
