//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayVk.cpp:
//    Implements the class methods for VertexArrayVk.
//

#include "libANGLE/renderer/vulkan/VertexArrayVk.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"

namespace rx
{
namespace
{
constexpr int kStreamIndexBufferCachedIndexCount = 6;
constexpr int kMaxCachedStreamIndexBuffers       = 4;
constexpr size_t kDefaultValueSize               = sizeof(gl::VertexAttribCurrentValueData::Values);

ANGLE_INLINE bool BindingIsAligned(const angle::Format &angleFormat,
                                   VkDeviceSize offset,
                                   GLuint stride)
{
    ASSERT(stride != 0);
    GLuint mask          = angleFormat.componentAlignmentMask;
    if (mask != std::numeric_limits<GLuint>::max())
    {
        return ((offset & mask) == 0 && (stride & mask) == 0);
    }
    else
    {
        // To perform the GPU conversion for formats with components that aren't byte-aligned
        // (for example, A2BGR10 or RGB10A2), one element has to be placed in 4 bytes to perform
        // the compute shader. So, binding offset and stride has to be aligned to formatSize.
        unsigned int formatSize = angleFormat.pixelBytes;
        return (offset % formatSize == 0) && (stride % formatSize == 0);
    }
}

ANGLE_INLINE bool ClientBindingAligned(const gl::VertexAttribute &attrib,
                                       GLuint stride,
                                       size_t alignment)
{
    return reinterpret_cast<intptr_t>(attrib.pointer) % alignment == 0 && stride % alignment == 0;
}

bool ShouldCombineAttributes(vk::Renderer *renderer,
                             const gl::VertexAttribute &attrib,
                             const gl::VertexBinding &binding)
{
    if (!renderer->getFeatures().enableMergeClientAttribBuffer.enabled)
    {
        return false;
    }
    const vk::Format &vertexFormat = renderer->getFormat(attrib.format->id);
    return !vertexFormat.getVertexLoadRequiresConversion(false) && binding.getDivisor() == 0 &&
           ClientBindingAligned(attrib, binding.getStride(),
                                vertexFormat.getVertexInputAlignment(false));
}

void WarnOnVertexFormatConversion(ContextVk *contextVk,
                                  const vk::Format &vertexFormat,
                                  bool compressed,
                                  bool insertEventMarker)
{
    if (!vertexFormat.getVertexLoadRequiresConversion(compressed))
    {
        return;
    }

    ANGLE_VK_PERF_WARNING(
        contextVk, GL_DEBUG_SEVERITY_LOW,
        "The Vulkan driver does not support vertex attribute format 0x%04X, emulating with 0x%04X",
        vertexFormat.getIntendedFormat().glInternalFormat,
        vertexFormat.getActualBufferFormat(compressed).glInternalFormat);
}

angle::Result StreamVertexData(ContextVk *contextVk,
                               vk::BufferHelper *dstBufferHelper,
                               const uint8_t *srcData,
                               size_t bytesToCopy,
                               size_t dstOffset,
                               size_t vertexCount,
                               size_t srcStride,
                               VertexCopyFunction vertexLoadFunction)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // If the source pointer is null, it should not be accessed.
    if (srcData == nullptr)
    {
        return angle::Result::Continue;
    }

    uint8_t *dst = dstBufferHelper->getMappedMemory() + dstOffset;

    if (vertexLoadFunction != nullptr)
    {
        vertexLoadFunction(srcData, srcStride, vertexCount, dst);
    }
    else
    {
        memcpy(dst, srcData, bytesToCopy);
    }

    ANGLE_TRY(dstBufferHelper->flush(renderer));

    return angle::Result::Continue;
}

angle::Result StreamVertexDataWithDivisor(ContextVk *contextVk,
                                          vk::BufferHelper *dstBufferHelper,
                                          const uint8_t *srcData,
                                          size_t bytesToAllocate,
                                          size_t srcStride,
                                          size_t dstStride,
                                          VertexCopyFunction vertexLoadFunction,
                                          uint32_t divisor,
                                          size_t numSrcVertices)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    uint8_t *dst = dstBufferHelper->getMappedMemory();

    // Each source vertex is used `divisor` times before advancing. Clamp to avoid OOB reads.
    size_t clampedSize = std::min(numSrcVertices * dstStride * divisor, bytesToAllocate);

    ASSERT(clampedSize % dstStride == 0);
    ASSERT(divisor > 0);

    uint32_t srcVertexUseCount = 0;
    for (size_t dataCopied = 0; dataCopied < clampedSize; dataCopied += dstStride)
    {
        vertexLoadFunction(srcData, srcStride, 1, dst);
        srcVertexUseCount++;
        if (srcVertexUseCount == divisor)
        {
            srcData += srcStride;
            srcVertexUseCount = 0;
        }
        dst += dstStride;
    }

    // Satisfy robustness constraints (only if extension enabled)
    if (contextVk->getExtensions().robustnessAny())
    {
        if (clampedSize < bytesToAllocate)
        {
            memset(dst, 0, bytesToAllocate - clampedSize);
        }
    }

    ANGLE_TRY(dstBufferHelper->flush(renderer));

    return angle::Result::Continue;
}

size_t GetVertexCountForRange(GLint64 srcBufferBytes,
                              uint32_t srcFormatSize,
                              uint32_t srcVertexStride)
{
    ASSERT(srcVertexStride != 0);
    ASSERT(srcFormatSize != 0);

    if (srcBufferBytes < srcFormatSize)
    {
        return 0;
    }

    size_t numVertices =
        static_cast<size_t>(srcBufferBytes + srcVertexStride - 1) / srcVertexStride;
    return numVertices;
}

size_t GetVertexCount(BufferVk *srcBuffer, const gl::VertexBinding &binding, uint32_t srcFormatSize)
{
    // Bytes usable for vertex data.
    GLint64 bytes = srcBuffer->getSize() - binding.getOffset();
    GLuint stride = binding.getStride();
    if (stride == 0)
    {
        stride = srcFormatSize;
    }
    return GetVertexCountForRange(bytes, srcFormatSize, stride);
}

angle::Result CalculateMaxVertexCountForConversion(ContextVk *contextVk,
                                                   BufferVk *srcBuffer,
                                                   VertexConversionBuffer *conversion,
                                                   const angle::Format &srcFormat,
                                                   const angle::Format &dstFormat,
                                                   size_t *maxNumVerticesOut)
{
    // Initialize numVertices to 0
    *maxNumVerticesOut = 0;

    unsigned srcFormatSize = srcFormat.pixelBytes;
    unsigned dstFormatSize = dstFormat.pixelBytes;

    uint32_t srcStride = conversion->getCacheKey().stride;
    uint32_t dstStride = dstFormatSize;

    ASSERT(srcStride != 0);
    ASSERT(conversion->dirty());

    // Start the range with the range from the the beginning of the buffer to the end of
    // buffer. Then scissor it with the dirtyRange.
    size_t srcOffset  = conversion->getCacheKey().offset;
    GLint64 srcLength = srcBuffer->getSize() - srcOffset;

    // The max number of vertices from binding to the end of the buffer
    size_t maxNumVertices = GetVertexCountForRange(srcLength, srcFormatSize, srcStride);
    if (maxNumVertices == 0)
    {
        return angle::Result::Continue;
    }

    // Allocate buffer for results
    vk::MemoryHostVisibility hostVisible = conversion->getCacheKey().hostVisible
                                               ? vk::MemoryHostVisibility::Visible
                                               : vk::MemoryHostVisibility::NonVisible;
    ANGLE_TRY(contextVk->initBufferForVertexConversion(conversion, maxNumVertices * dstStride,
                                                       hostVisible));

    // Calculate numVertices to convert
    *maxNumVerticesOut = GetVertexCountForRange(srcLength, srcFormatSize, srcStride);

    return angle::Result::Continue;
}

void CalculateOffsetAndVertexCountForDirtyRange(BufferVk *bufferVk,
                                                VertexConversionBuffer *conversion,
                                                const angle::Format &srcFormat,
                                                const angle::Format &dstFormat,
                                                const RangeDeviceSize &dirtyRange,
                                                uint32_t *srcOffsetOut,
                                                uint32_t *dstOffsetOut,
                                                uint32_t *numVerticesOut)
{
    ASSERT(!dirtyRange.empty());
    unsigned srcFormatSize = srcFormat.pixelBytes;
    unsigned dstFormatSize = dstFormat.pixelBytes;

    uint32_t srcStride = conversion->getCacheKey().stride;
    uint32_t dstStride = dstFormatSize;

    ASSERT(srcStride != 0);
    ASSERT(conversion->dirty());

    // Start the range with the range from the the beginning of the buffer to the end of
    // buffer. Then scissor it with the dirtyRange.
    size_t srcOffset = conversion->getCacheKey().offset;
    size_t dstOffset = 0;

    GLint64 srcLength = bufferVk->getSize() - srcOffset;

    // Adjust offset to the begining of the dirty range
    if (dirtyRange.low() > srcOffset)
    {
        size_t vertexCountToSkip = (static_cast<size_t>(dirtyRange.low()) - srcOffset) / srcStride;
        size_t srcBytesToSkip    = vertexCountToSkip * srcStride;
        size_t dstBytesToSkip    = vertexCountToSkip * dstStride;
        srcOffset += srcBytesToSkip;
        srcLength -= srcBytesToSkip;
        dstOffset += dstBytesToSkip;
    }

    // Adjust dstOffset to align to 4 bytes. The GPU convert code path always write a uint32_t and
    // must aligned at 4 bytes. We could possibly make it able to store at unaligned uint32_t but
    // performance will be worse than just convert a few extra data.
    while ((dstOffset % 4) != 0)
    {
        dstOffset -= dstStride;
        srcOffset -= srcStride;
        srcLength += srcStride;
    }

    // Adjust length
    if (dirtyRange.high() < static_cast<VkDeviceSize>(bufferVk->getSize()))
    {
        srcLength = dirtyRange.high() - srcOffset;
    }

    // Calculate numVertices to convert
    size_t numVertices = GetVertexCountForRange(srcLength, srcFormatSize, srcStride);

    *numVerticesOut = static_cast<uint32_t>(numVertices);
    *srcOffsetOut   = static_cast<uint32_t>(srcOffset);
    *dstOffsetOut   = static_cast<uint32_t>(dstOffset);
}
}  // anonymous namespace

VertexArrayVk::VertexArrayVk(ContextVk *contextVk, const gl::VertexArrayState &state)
    : VertexArrayImpl(state),
      mCurrentArrayBufferHandles{},
      mCurrentArrayBufferOffsets{},
      mCurrentArrayBufferRelativeOffsets{},
      mCurrentArrayBuffers{},
      mCurrentArrayBufferStrides{},
      mCurrentArrayBufferDivisors{},
      mCurrentElementArrayBuffer(nullptr),
      mLineLoopHelper(contextVk->getRenderer()),
      mDirtyLineLoopTranslation(true)
{
    vk::BufferHelper &emptyBuffer = contextVk->getEmptyBuffer();

    mCurrentArrayBufferHandles.fill(emptyBuffer.getBuffer().getHandle());
    mCurrentArrayBufferOffsets.fill(0);
    mCurrentArrayBufferRelativeOffsets.fill(0);
    mCurrentArrayBuffers.fill(&emptyBuffer);
    mCurrentArrayBufferStrides.fill(0);
    mCurrentArrayBufferDivisors.fill(0);

    mBindingDirtyBitsRequiresPipelineUpdate.set(gl::VertexArray::DIRTY_BINDING_DIVISOR);
    if (!contextVk->getFeatures().useVertexInputBindingStrideDynamicState.enabled)
    {
        mBindingDirtyBitsRequiresPipelineUpdate.set(gl::VertexArray::DIRTY_BINDING_STRIDE);
    }

    // All but DIRTY_ATTRIB_POINTER_BUFFER requires graphics pipeline update
    mAttribDirtyBitsRequiresPipelineUpdate.set(gl::VertexArray::DIRTY_ATTRIB_ENABLED);
    mAttribDirtyBitsRequiresPipelineUpdate.set(gl::VertexArray::DIRTY_ATTRIB_POINTER);
    mAttribDirtyBitsRequiresPipelineUpdate.set(gl::VertexArray::DIRTY_ATTRIB_FORMAT);
    mAttribDirtyBitsRequiresPipelineUpdate.set(gl::VertexArray::DIRTY_ATTRIB_BINDING);
}

VertexArrayVk::~VertexArrayVk() {}

void VertexArrayVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    for (std::unique_ptr<vk::BufferHelper> &buffer : mCachedStreamIndexBuffers)
    {
        buffer->release(contextVk);
    }

    mStreamedIndexData.release(contextVk);
    mTranslatedByteIndexData.release(contextVk);
    mTranslatedByteIndirectData.release(contextVk);
    mLineLoopHelper.release(contextVk);
}

angle::Result VertexArrayVk::convertIndexBufferGPU(ContextVk *contextVk,
                                                   BufferVk *bufferVk,
                                                   const void *indices)
{
    intptr_t offsetIntoSrcData = reinterpret_cast<intptr_t>(indices);
    size_t srcDataSize         = static_cast<size_t>(bufferVk->getSize()) - offsetIntoSrcData;

    // Allocate buffer for results
    ANGLE_TRY(contextVk->initBufferForVertexConversion(&mTranslatedByteIndexData,
                                                       sizeof(GLushort) * srcDataSize,
                                                       vk::MemoryHostVisibility::NonVisible));
    mCurrentElementArrayBuffer = mTranslatedByteIndexData.getBuffer();

    vk::BufferHelper *dst = mTranslatedByteIndexData.getBuffer();
    vk::BufferHelper *src = &bufferVk->getBuffer();

    // Copy relevant section of the source into destination at allocated offset.  Note that the
    // offset returned by allocate() above is in bytes. As is the indices offset pointer.
    UtilsVk::ConvertIndexParameters params = {};
    params.srcOffset                       = static_cast<uint32_t>(offsetIntoSrcData);
    params.dstOffset                       = 0;
    params.maxIndex                        = static_cast<uint32_t>(bufferVk->getSize());

    ANGLE_TRY(contextVk->getUtils().convertIndexBuffer(contextVk, dst, src, params));
    mTranslatedByteIndexData.clearDirty();

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::convertIndexBufferIndirectGPU(ContextVk *contextVk,
                                                           vk::BufferHelper *srcIndirectBuf,
                                                           VkDeviceSize srcIndirectBufOffset,
                                                           vk::BufferHelper **indirectBufferVkOut)
{
    size_t srcDataSize = static_cast<size_t>(mCurrentElementArrayBuffer->getSize());
    ASSERT(mCurrentElementArrayBuffer ==
           &vk::GetImpl(getState().getElementArrayBuffer())->getBuffer());

    vk::BufferHelper *srcIndexBuf = mCurrentElementArrayBuffer;

    // Allocate buffer for results
    ANGLE_TRY(contextVk->initBufferForVertexConversion(&mTranslatedByteIndexData,
                                                       sizeof(GLushort) * srcDataSize,
                                                       vk::MemoryHostVisibility::NonVisible));
    vk::BufferHelper *dstIndexBuf = mTranslatedByteIndexData.getBuffer();

    ANGLE_TRY(contextVk->initBufferForVertexConversion(&mTranslatedByteIndirectData,
                                                       sizeof(VkDrawIndexedIndirectCommand),
                                                       vk::MemoryHostVisibility::NonVisible));
    vk::BufferHelper *dstIndirectBuf = mTranslatedByteIndirectData.getBuffer();

    // Save new element array buffer
    mCurrentElementArrayBuffer = dstIndexBuf;
    // Tell caller what new indirect buffer is
    *indirectBufferVkOut = dstIndirectBuf;

    // Copy relevant section of the source into destination at allocated offset.  Note that the
    // offset returned by allocate() above is in bytes. As is the indices offset pointer.
    UtilsVk::ConvertIndexIndirectParameters params = {};
    params.srcIndirectBufOffset                    = static_cast<uint32_t>(srcIndirectBufOffset);
    params.srcIndexBufOffset                       = 0;
    params.dstIndexBufOffset                       = 0;
    params.maxIndex                                = static_cast<uint32_t>(srcDataSize);
    params.dstIndirectBufOffset                    = 0;

    ANGLE_TRY(contextVk->getUtils().convertIndexIndirectBuffer(
        contextVk, srcIndirectBuf, srcIndexBuf, dstIndirectBuf, dstIndexBuf, params));

    mTranslatedByteIndexData.clearDirty();
    mTranslatedByteIndirectData.clearDirty();

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::handleLineLoopIndexIndirect(ContextVk *contextVk,
                                                         gl::DrawElementsType glIndexType,
                                                         vk::BufferHelper *srcIndexBuffer,
                                                         vk::BufferHelper *srcIndirectBuffer,
                                                         VkDeviceSize indirectBufferOffset,
                                                         vk::BufferHelper **indexBufferOut,
                                                         vk::BufferHelper **indirectBufferOut)
{
    return mLineLoopHelper.streamIndicesIndirect(contextVk, glIndexType, srcIndexBuffer,
                                                 srcIndirectBuffer, indirectBufferOffset,
                                                 indexBufferOut, indirectBufferOut);
}

angle::Result VertexArrayVk::handleLineLoopIndirectDraw(const gl::Context *context,
                                                        vk::BufferHelper *indirectBufferVk,
                                                        VkDeviceSize indirectBufferOffset,
                                                        vk::BufferHelper **indexBufferOut,
                                                        vk::BufferHelper **indirectBufferOut)
{
    size_t maxVertexCount = 0;
    ContextVk *contextVk  = vk::GetImpl(context);
    const gl::AttributesMask activeAttribs =
        context->getStateCache().getActiveBufferedAttribsMask();

    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    for (size_t attribIndex : activeAttribs)
    {
        const gl::VertexAttribute &attrib = attribs[attribIndex];
        ASSERT(attrib.enabled);
        VkDeviceSize bufSize             = getCurrentArrayBuffers()[attribIndex]->getSize();
        const gl::VertexBinding &binding = bindings[attrib.bindingIndex];
        size_t stride                    = binding.getStride();
        size_t vertexCount               = static_cast<size_t>(bufSize / stride);
        if (vertexCount > maxVertexCount)
        {
            maxVertexCount = vertexCount;
        }
    }
    ANGLE_TRY(mLineLoopHelper.streamArrayIndirect(contextVk, maxVertexCount + 1, indirectBufferVk,
                                                  indirectBufferOffset, indexBufferOut,
                                                  indirectBufferOut));

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::convertIndexBufferCPU(ContextVk *contextVk,
                                                   gl::DrawElementsType indexType,
                                                   size_t indexCount,
                                                   const void *sourcePointer,
                                                   BufferBindingDirty *bindingDirty)
{
    ASSERT(!mState.getElementArrayBuffer() || indexType == gl::DrawElementsType::UnsignedByte);
    vk::Renderer *renderer = contextVk->getRenderer();
    size_t elementSize     = contextVk->getVkIndexTypeSize(indexType);
    const size_t amount    = elementSize * indexCount;

    // Applications often time draw a quad with two triangles. This is try to catch all the
    // common used element array buffer with pre-created BufferHelper objects to improve
    // performance.
    if (indexCount == kStreamIndexBufferCachedIndexCount &&
        indexType == gl::DrawElementsType::UnsignedShort)
    {
        for (std::unique_ptr<vk::BufferHelper> &buffer : mCachedStreamIndexBuffers)
        {
            void *ptr = buffer->getMappedMemory();
            if (memcmp(sourcePointer, ptr, amount) == 0)
            {
                // Found a matching cached buffer, use the cached internal index buffer.
                *bindingDirty              = mCurrentElementArrayBuffer == buffer.get()
                                                 ? BufferBindingDirty::No
                                                 : BufferBindingDirty::Yes;
                mCurrentElementArrayBuffer = buffer.get();
                return angle::Result::Continue;
            }
        }

        // If we still have capacity, cache this index buffer for future use.
        if (mCachedStreamIndexBuffers.size() < kMaxCachedStreamIndexBuffers)
        {
            std::unique_ptr<vk::BufferHelper> buffer = std::make_unique<vk::BufferHelper>();
            ANGLE_TRY(contextVk->initBufferAllocation(
                buffer.get(),
                renderer->getVertexConversionBufferMemoryTypeIndex(
                    vk::MemoryHostVisibility::Visible),
                amount, renderer->getVertexConversionBufferAlignment(), BufferUsageType::Static));
            memcpy(buffer->getMappedMemory(), sourcePointer, amount);
            ANGLE_TRY(buffer->flush(renderer));

            mCachedStreamIndexBuffers.push_back(std::move(buffer));

            *bindingDirty              = BufferBindingDirty::Yes;
            mCurrentElementArrayBuffer = mCachedStreamIndexBuffers.back().get();
            return angle::Result::Continue;
        }
    }

    ANGLE_TRY(contextVk->initBufferForVertexConversion(&mStreamedIndexData, amount,
                                                       vk::MemoryHostVisibility::Visible));
    mCurrentElementArrayBuffer = mStreamedIndexData.getBuffer();
    GLubyte *dst               = mCurrentElementArrayBuffer->getMappedMemory();
    *bindingDirty              = BufferBindingDirty::Yes;

    if (contextVk->shouldConvertUint8VkIndexType(indexType))
    {
        // Unsigned bytes don't have direct support in Vulkan so we have to expand the
        // memory to a GLushort.
        const GLubyte *in     = static_cast<const GLubyte *>(sourcePointer);
        GLushort *expandedDst = reinterpret_cast<GLushort *>(dst);
        bool primitiveRestart = contextVk->getState().isPrimitiveRestartEnabled();

        constexpr GLubyte kUnsignedByteRestartValue   = 0xFF;
        constexpr GLushort kUnsignedShortRestartValue = 0xFFFF;

        if (primitiveRestart)
        {
            for (size_t index = 0; index < indexCount; index++)
            {
                GLushort value = static_cast<GLushort>(in[index]);
                if (in[index] == kUnsignedByteRestartValue)
                {
                    // Convert from 8-bit restart value to 16-bit restart value
                    value = kUnsignedShortRestartValue;
                }
                expandedDst[index] = value;
            }
        }
        else
        {
            // Fast path for common case.
            for (size_t index = 0; index < indexCount; index++)
            {
                expandedDst[index] = static_cast<GLushort>(in[index]);
            }
        }
    }
    else
    {
        // The primitive restart value is the same for OpenGL and Vulkan,
        // so there's no need to perform any conversion.
        memcpy(dst, sourcePointer, amount);
    }

    mStreamedIndexData.clearDirty();

    return mCurrentElementArrayBuffer->flush(contextVk->getRenderer());
}

// We assume the buffer is completely full of the same kind of data and convert
// and/or align it as we copy it to a buffer. The assumption could be wrong
// but the alternative of copying it piecemeal on each draw would have a lot more
// overhead.
angle::Result VertexArrayVk::convertVertexBufferGPU(ContextVk *contextVk,
                                                    BufferVk *srcBuffer,
                                                    VertexConversionBuffer *conversion,
                                                    const angle::Format &srcFormat,
                                                    const angle::Format &dstFormat)
{
    uint32_t srcStride = conversion->getCacheKey().stride;
    ASSERT(srcStride % (srcFormat.pixelBytes / srcFormat.channelCount) == 0);

    size_t maxNumVertices;
    ANGLE_TRY(CalculateMaxVertexCountForConversion(contextVk, srcBuffer, conversion, srcFormat,
                                                   dstFormat, &maxNumVertices));
    if (maxNumVertices == 0)
    {
        return angle::Result::Continue;
    }

    vk::BufferHelper *srcBufferHelper = &srcBuffer->getBuffer();
    vk::BufferHelper *dstBuffer       = conversion->getBuffer();

    UtilsVk::OffsetAndVertexCounts additionalOffsetVertexCounts;

    UtilsVk::ConvertVertexParameters params;
    params.srcFormat   = &srcFormat;
    params.dstFormat   = &dstFormat;
    params.srcStride   = srcStride;
    params.vertexCount = 0;

    if (conversion->isEntireBufferDirty())
    {
        params.vertexCount = static_cast<uint32_t>(maxNumVertices);
        params.srcOffset   = static_cast<uint32_t>(conversion->getCacheKey().offset);
        params.dstOffset   = 0;
    }
    else
    {
        // dirtyRanges may overlap with each other. Try to do a quick merge to reduce the number of
        // dispatch calls as well as avoid redundant conversion in the overlapped area.
        conversion->consolidateDirtyRanges();

        const std::vector<RangeDeviceSize> &dirtyRanges = conversion->getDirtyBufferRanges();
        additionalOffsetVertexCounts.reserve(dirtyRanges.size());

        for (const RangeDeviceSize &dirtyRange : dirtyRanges)
        {
            if (dirtyRange.empty())
            {
                // consolidateDirtyRanges may end up with invalid range if it gets merged.
                continue;
            }

            uint32_t srcOffset, dstOffset, numVertices;
            CalculateOffsetAndVertexCountForDirtyRange(srcBuffer, conversion, srcFormat, dstFormat,
                                                       dirtyRange, &srcOffset, &dstOffset,
                                                       &numVertices);
            if (params.vertexCount == 0)
            {
                params.vertexCount = numVertices;
                params.srcOffset   = srcOffset;
                params.dstOffset   = dstOffset;
            }
            else
            {
                additionalOffsetVertexCounts.emplace_back();
                additionalOffsetVertexCounts.back().srcOffset   = srcOffset;
                additionalOffsetVertexCounts.back().dstOffset   = dstOffset;
                additionalOffsetVertexCounts.back().vertexCount = numVertices;
            }
        }
    }
    ANGLE_TRY(contextVk->getUtils().convertVertexBuffer(contextVk, dstBuffer, srcBufferHelper,
                                                        params, additionalOffsetVertexCounts));
    conversion->clearDirty();

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::convertVertexBufferCPU(ContextVk *contextVk,
                                                    BufferVk *srcBuffer,
                                                    VertexConversionBuffer *conversion,
                                                    const angle::Format &srcFormat,
                                                    const angle::Format &dstFormat,
                                                    const VertexCopyFunction vertexLoadFunction)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "VertexArrayVk::convertVertexBufferCpu");

    size_t maxNumVertices;
    ANGLE_TRY(CalculateMaxVertexCountForConversion(contextVk, srcBuffer, conversion, srcFormat,
                                                   dstFormat, &maxNumVertices));
    if (maxNumVertices == 0)
    {
        return angle::Result::Continue;
    }

    uint8_t *src = nullptr;
    ANGLE_TRY(srcBuffer->mapImpl(contextVk, GL_MAP_READ_BIT, reinterpret_cast<void **>(&src)));
    uint32_t srcStride      = conversion->getCacheKey().stride;

    if (conversion->isEntireBufferDirty())
    {
        size_t srcOffset        = conversion->getCacheKey().offset;
        size_t dstOffset        = 0;
        const uint8_t *srcBytes = src + srcOffset;
        size_t bytesToCopy      = maxNumVertices * dstFormat.pixelBytes;
        ANGLE_TRY(StreamVertexData(contextVk, conversion->getBuffer(), srcBytes, bytesToCopy,
                                   dstOffset, maxNumVertices, srcStride, vertexLoadFunction));
    }
    else
    {
        // dirtyRanges may overlap with each other. Try to do a quick merge to avoid redundant
        // conversion in the overlapped area.
        conversion->consolidateDirtyRanges();

        const std::vector<RangeDeviceSize> &dirtyRanges = conversion->getDirtyBufferRanges();
        for (const RangeDeviceSize &dirtyRange : dirtyRanges)
        {
            if (dirtyRange.empty())
            {
                // consolidateDirtyRanges may end up with invalid range if it gets merged.
                continue;
            }

            uint32_t srcOffset, dstOffset, numVertices;
            CalculateOffsetAndVertexCountForDirtyRange(srcBuffer, conversion, srcFormat, dstFormat,
                                                       dirtyRange, &srcOffset, &dstOffset,
                                                       &numVertices);

            if (numVertices > 0)
            {
                const uint8_t *srcBytes = src + srcOffset;
                size_t bytesToCopy      = maxNumVertices * dstFormat.pixelBytes;
                ANGLE_TRY(StreamVertexData(contextVk, conversion->getBuffer(), srcBytes,
                                           bytesToCopy, dstOffset, maxNumVertices, srcStride,
                                           vertexLoadFunction));
            }
        }
    }

    conversion->clearDirty();
    ANGLE_TRY(srcBuffer->unmapImpl(contextVk));

    return angle::Result::Continue;
}

void VertexArrayVk::updateCurrentElementArrayBuffer()
{
    ASSERT(mState.getElementArrayBuffer() != nullptr);
    ASSERT(mState.getElementArrayBuffer()->getSize() > 0);

    BufferVk *bufferVk         = vk::GetImpl(mState.getElementArrayBuffer());
    mCurrentElementArrayBuffer = &bufferVk->getBuffer();
}

angle::Result VertexArrayVk::syncState(const gl::Context *context,
                                       const gl::VertexArray::DirtyBits &dirtyBits,
                                       gl::VertexArray::DirtyAttribBitsArray *attribBits,
                                       gl::VertexArray::DirtyBindingBitsArray *bindingBits)
{
    ASSERT(dirtyBits.any());

    ContextVk *contextVk = vk::GetImpl(context);
    contextVk->getPerfCounters().vertexArraySyncStateCalls++;

    const std::vector<gl::VertexAttribute> &attribs = mState.getVertexAttributes();
    const std::vector<gl::VertexBinding> &bindings  = mState.getVertexBindings();

    for (auto iter = dirtyBits.begin(), endIter = dirtyBits.end(); iter != endIter; ++iter)
    {
        size_t dirtyBit = *iter;
        switch (dirtyBit)
        {
            case gl::VertexArray::DIRTY_BIT_LOST_OBSERVATION:
            {
                // If vertex array was not observing while unbound, we need to check buffer's
                // internal storage and take action if buffer storage has changed while not
                // observing.
                if (contextVk->getFeatures().compressVertexData.enabled ||
                    mContentsObservers->any())
                {
                    // We may have lost buffer content change when it became non-current. In that
                    // case we always assume buffer has changed. If compressVertexData.enabled is
                    // true, it also depends on buffer usage which may have changed.
                    iter.setLaterBits(
                        gl::VertexArray::DirtyBits(mState.getBufferBindingMask().to_ulong()
                                                   << gl::VertexArray::DIRTY_BIT_BINDING_0));
                }
                else
                {
                    for (size_t bindingIndex : mState.getBufferBindingMask())
                    {
                        const gl::Buffer *bufferGL    = bindings[bindingIndex].getBuffer().get();
                        vk::BufferSerial bufferSerial = vk::GetImpl(bufferGL)->getBufferSerial();
                        for (size_t attribIndex : bindings[bindingIndex].getBoundAttributesMask())
                        {
                            if (attribs[attribIndex].enabled &&
                                (!bufferSerial.valid() ||
                                 bufferSerial != mCurrentArrayBufferSerial[attribIndex]))
                            {
                                iter.setLaterBit(gl::VertexArray::DIRTY_BIT_BINDING_0 +
                                                 bindingIndex);
                                break;
                            }
                        }
                    }
                }
                break;
            }

            case gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER:
            case gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER_DATA:
            {
                gl::Buffer *bufferGL = mState.getElementArrayBuffer();
                if (bufferGL && bufferGL->getSize() > 0)
                {
                    // Note that just updating buffer data may still result in a new
                    // vk::BufferHelper allocation.
                    updateCurrentElementArrayBuffer();
                }
                else
                {
                    mCurrentElementArrayBuffer = nullptr;
                }

                mLineLoopBufferFirstIndex.reset();
                mLineLoopBufferLastIndex.reset();
                ANGLE_TRY(contextVk->onIndexBufferChange(mCurrentElementArrayBuffer));
                mDirtyLineLoopTranslation = true;
                break;
            }

#define ANGLE_VERTEX_DIRTY_ATTRIB_FUNC(INDEX)                                                 \
    case gl::VertexArray::DIRTY_BIT_ATTRIB_0 + INDEX:                                         \
    {                                                                                         \
        gl::VertexArray::DirtyAttribBits dirtyAttribBitsRequiresPipelineUpdate =              \
            (*attribBits)[INDEX] & mAttribDirtyBitsRequiresPipelineUpdate;                    \
        const bool bufferOnly = dirtyAttribBitsRequiresPipelineUpdate.none();                 \
        ANGLE_TRY(syncDirtyAttrib(contextVk, attribs[INDEX],                                  \
                                  bindings[attribs[INDEX].bindingIndex], INDEX, bufferOnly)); \
        (*attribBits)[INDEX].reset();                                                         \
        break;                                                                                \
    }

                ANGLE_VERTEX_INDEX_CASES(ANGLE_VERTEX_DIRTY_ATTRIB_FUNC)

// Since BINDING already implies DATA and ATTRIB change, we remove these here to avoid redundant
// processing.
#define ANGLE_VERTEX_DIRTY_BINDING_FUNC(INDEX)                                          \
    case gl::VertexArray::DIRTY_BIT_BINDING_0 + INDEX:                                  \
    {                                                                                   \
        gl::VertexArray::DirtyBindingBits dirtyBindingBitsRequirePipelineUpdate =       \
            (*bindingBits)[INDEX] & mBindingDirtyBitsRequiresPipelineUpdate;            \
                                                                                        \
        for (size_t attribIndex : bindings[INDEX].getBoundAttributesMask())             \
        {                                                                               \
            gl::VertexArray::DirtyAttribBits dirtyAttribBitsRequiresPipelineUpdate =    \
                (*attribBits)[attribIndex] & mAttribDirtyBitsRequiresPipelineUpdate;    \
            const bool bufferOnly = dirtyBindingBitsRequirePipelineUpdate.none() &&     \
                                    dirtyAttribBitsRequiresPipelineUpdate.none();       \
            ANGLE_TRY(syncDirtyAttrib(contextVk, attribs[attribIndex], bindings[INDEX], \
                                      attribIndex, bufferOnly));                        \
            iter.resetLaterBit(gl::VertexArray::DIRTY_BIT_BUFFER_DATA_0 + attribIndex); \
            iter.resetLaterBit(gl::VertexArray::DIRTY_BIT_ATTRIB_0 + attribIndex);      \
            (*attribBits)[attribIndex].reset();                                         \
        }                                                                               \
        (*bindingBits)[INDEX].reset();                                                  \
        break;                                                                          \
    }

                ANGLE_VERTEX_INDEX_CASES(ANGLE_VERTEX_DIRTY_BINDING_FUNC)

#define ANGLE_VERTEX_DIRTY_BUFFER_DATA_FUNC(INDEX)                                       \
    case gl::VertexArray::DIRTY_BIT_BUFFER_DATA_0 + INDEX:                               \
        ANGLE_TRY(syncDirtyAttrib(contextVk, attribs[INDEX],                             \
                                  bindings[attribs[INDEX].bindingIndex], INDEX, false)); \
        iter.resetLaterBit(gl::VertexArray::DIRTY_BIT_ATTRIB_0 + INDEX);                 \
        (*attribBits)[INDEX].reset();                                                    \
        break;

                ANGLE_VERTEX_INDEX_CASES(ANGLE_VERTEX_DIRTY_BUFFER_DATA_FUNC)

            default:
                UNREACHABLE();
                break;
        }
    }

    return angle::Result::Continue;
}  // namespace rx

#undef ANGLE_VERTEX_DIRTY_ATTRIB_FUNC
#undef ANGLE_VERTEX_DIRTY_BINDING_FUNC
#undef ANGLE_VERTEX_DIRTY_BUFFER_DATA_FUNC

ANGLE_INLINE angle::Result VertexArrayVk::setDefaultPackedInput(ContextVk *contextVk,
                                                                size_t attribIndex,
                                                                angle::FormatID *formatOut)
{
    const gl::State &glState = contextVk->getState();
    const gl::VertexAttribCurrentValueData &defaultValue =
        glState.getVertexAttribCurrentValues()[attribIndex];

    *formatOut = GetCurrentValueFormatID(defaultValue.Type);

    return contextVk->onVertexAttributeChange(attribIndex, 0, 0, *formatOut, false, 0, nullptr);
}

angle::Result VertexArrayVk::updateActiveAttribInfo(ContextVk *contextVk)
{
    const std::vector<gl::VertexAttribute> &attribs = mState.getVertexAttributes();
    const std::vector<gl::VertexBinding> &bindings  = mState.getVertexBindings();

    // Update pipeline cache with current active attribute info
    for (size_t attribIndex : mState.getEnabledAttributesMask())
    {
        const gl::VertexAttribute &attrib = attribs[attribIndex];
        const gl::VertexBinding &binding  = bindings[attribs[attribIndex].bindingIndex];
        const angle::FormatID format      = attrib.format->id;

        ANGLE_TRY(contextVk->onVertexAttributeChange(
            attribIndex, mCurrentArrayBufferStrides[attribIndex], binding.getDivisor(), format,
            mCurrentArrayBufferCompressed.test(attribIndex),
            mCurrentArrayBufferRelativeOffsets[attribIndex], mCurrentArrayBuffers[attribIndex]));

        mCurrentArrayBufferFormats[attribIndex] = format;
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::syncDirtyAttrib(ContextVk *contextVk,
                                             const gl::VertexAttribute &attrib,
                                             const gl::VertexBinding &binding,
                                             size_t attribIndex,
                                             bool bufferOnly)
{
    vk::Renderer *renderer = contextVk->getRenderer();
    if (attrib.enabled)
    {
        const vk::Format &vertexFormat = renderer->getFormat(attrib.format->id);

        // Init attribute offset to the front-end value
        mCurrentArrayBufferRelativeOffsets[attribIndex] = attrib.relativeOffset;
        gl::Buffer *bufferGL                            = binding.getBuffer().get();
        // Emulated and/or client-side attribs will be streamed
        bool isStreamingVertexAttrib =
            (binding.getDivisor() > renderer->getMaxVertexAttribDivisor()) || (bufferGL == nullptr);
        // If we sre switching between streaming and buffer mode, set bufferOnly to false since we
        // are actually changing the buffer.
        if (bufferOnly && isStreamingVertexAttrib != mStreamingVertexAttribsMask.test(attribIndex))
        {
            bufferOnly = false;
        }
        mStreamingVertexAttribsMask.set(attribIndex, isStreamingVertexAttrib);
        bool compressed = false;

        if (bufferGL)
        {
            mContentsObservers->disableForBuffer(bufferGL, static_cast<uint32_t>(attribIndex));
        }

        if (!isStreamingVertexAttrib && bufferGL->getSize() > 0)
        {
            BufferVk *bufferVk                  = vk::GetImpl(bufferGL);
            const angle::Format &srcFormat      = vertexFormat.getIntendedFormat();
            unsigned srcFormatSize              = srcFormat.pixelBytes;
            uint32_t srcStride    = binding.getStride() == 0 ? srcFormatSize : binding.getStride();
            size_t numVertices    = GetVertexCount(bufferVk, binding, srcFormatSize);
            bool bindingIsAligned =
                BindingIsAligned(srcFormat, binding.getOffset() + attrib.relativeOffset, srcStride);

            if (renderer->getFeatures().compressVertexData.enabled &&
                gl::IsStaticBufferUsage(bufferGL->getUsage()) &&
                vertexFormat.canCompressBufferData())
            {
                compressed = true;
            }

            bool needsConversion =
                numVertices > 0 &&
                (vertexFormat.getVertexLoadRequiresConversion(compressed) || !bindingIsAligned);

            if (needsConversion)
            {
                const angle::Format &dstFormat = vertexFormat.getActualBufferFormat(compressed);
                // Converted buffer is tightly packed
                uint32_t dstStride = dstFormat.pixelBytes;

                ASSERT(vertexFormat.getVertexInputAlignment(compressed) <=
                       vk::kVertexBufferAlignment);

                mContentsObservers->enableForBuffer(bufferGL, static_cast<uint32_t>(attribIndex));

                WarnOnVertexFormatConversion(contextVk, vertexFormat, compressed, true);

                const VertexConversionBuffer::CacheKey cacheKey{
                    srcFormat.id, srcStride,
                    static_cast<size_t>(binding.getOffset()) + attrib.relativeOffset,
                    !bindingIsAligned, false};

                VertexConversionBuffer *conversion =
                    bufferVk->getVertexConversionBuffer(renderer, cacheKey);

                // Converted attribs are packed in their own VK buffer so offset is relative to the
                // binding and coversion's offset. The conversion buffer try to reuse the existing
                // buffer as much as possible to reduce the amount of data that has to be converted.
                // When binding's offset changes, it will check if new offset and existing buffer's
                // offset are multiple of strides apart. It yes it will reuse. If new offset is
                // larger, all existing data are still valid. If the new offset is smaller it will
                // mark the newly exposed range dirty and then rely on
                // ContextVk::initBufferForVertexConversion to decide buffer's size is big enough or
                // not and reallocate (and mark entire buffer dirty) if needed.
                //
                // bufferVk:-----------------------------------------------------------------------
                //                 |                   |
                //                 |                bingding.offset + attrib.relativeOffset.
                //          conversion->getCacheKey().offset
                //
                // conversion.buffer: --------------------------------------------------------------
                //                                     |
                //                                   dstRelativeOffset
                size_t srcRelativeOffset =
                    binding.getOffset() + attrib.relativeOffset - conversion->getCacheKey().offset;
                size_t numberOfVerticesToSkip = srcRelativeOffset / srcStride;
                size_t dstRelativeOffset      = numberOfVerticesToSkip * dstStride;

                if (conversion->dirty())
                {
                    if (compressed)
                    {
                        INFO() << "Compressing vertex data in buffer " << bufferGL->id().value
                               << " from " << ToUnderlying(srcFormat.id) << " to "
                               << ToUnderlying(dstFormat.id) << ".";
                    }

                    if (bindingIsAligned)
                    {
                        ANGLE_TRY(convertVertexBufferGPU(contextVk, bufferVk, conversion, srcFormat,
                                                         dstFormat));
                    }
                    else
                    {
                        ANGLE_VK_PERF_WARNING(
                            contextVk, GL_DEBUG_SEVERITY_HIGH,
                            "GPU stall due to vertex format conversion of unaligned data");

                        ANGLE_TRY(convertVertexBufferCPU(
                            contextVk, bufferVk, conversion, srcFormat, dstFormat,
                            vertexFormat.getVertexLoadFunction(compressed)));
                    }

                    // If conversion happens, the destination buffer stride may be changed,
                    // therefore an attribute change needs to be called. Note that it may trigger
                    // unnecessary vulkan PSO update when the destination buffer stride does not
                    // change, but for simplicity just make it conservative
                    bufferOnly = false;
                }

                vk::BufferHelper *bufferHelper         = conversion->getBuffer();
                mCurrentArrayBuffers[attribIndex]      = bufferHelper;
                mCurrentArrayBufferSerial[attribIndex] = bufferHelper->getBufferSerial();
                VkDeviceSize bufferOffset;
                mCurrentArrayBufferHandles[attribIndex] =
                    bufferHelper
                        ->getBufferForVertexArray(contextVk, bufferHelper->getSize(), &bufferOffset)
                        .getHandle();
                ASSERT(BindingIsAligned(dstFormat, bufferOffset + dstRelativeOffset, dstStride));
                mCurrentArrayBufferOffsets[attribIndex]         = bufferOffset + dstRelativeOffset;
                mCurrentArrayBufferRelativeOffsets[attribIndex] = 0;
                mCurrentArrayBufferStrides[attribIndex]         = dstStride;
            }
            else
            {
                if (numVertices == 0)
                {
                    vk::BufferHelper &emptyBuffer = contextVk->getEmptyBuffer();

                    mCurrentArrayBuffers[attribIndex]       = &emptyBuffer;
                    mCurrentArrayBufferSerial[attribIndex]  = emptyBuffer.getBufferSerial();
                    mCurrentArrayBufferHandles[attribIndex] = emptyBuffer.getBuffer().getHandle();
                    mCurrentArrayBufferOffsets[attribIndex] = emptyBuffer.getOffset();
                    mCurrentArrayBufferStrides[attribIndex] = 0;
                }
                else
                {
                    vk::BufferHelper &bufferHelper         = bufferVk->getBuffer();
                    mCurrentArrayBuffers[attribIndex]      = &bufferHelper;
                    mCurrentArrayBufferSerial[attribIndex] = bufferHelper.getBufferSerial();
                    VkDeviceSize bufferOffset;
                    mCurrentArrayBufferHandles[attribIndex] =
                        bufferHelper
                            .getBufferForVertexArray(contextVk, bufferVk->getSize(), &bufferOffset)
                            .getHandle();

                    // Vulkan requires the offset is within the buffer. We use robust access
                    // behaviour to reset the offset if it starts outside the buffer.
                    mCurrentArrayBufferOffsets[attribIndex] =
                        binding.getOffset() < static_cast<GLint64>(bufferVk->getSize())
                            ? binding.getOffset() + bufferOffset
                            : bufferOffset;

                    mCurrentArrayBufferStrides[attribIndex] = binding.getStride();
                }
            }
        }
        else
        {
            vk::BufferHelper &emptyBuffer           = contextVk->getEmptyBuffer();
            mCurrentArrayBuffers[attribIndex]       = &emptyBuffer;
            mCurrentArrayBufferSerial[attribIndex]  = emptyBuffer.getBufferSerial();
            mCurrentArrayBufferHandles[attribIndex] = emptyBuffer.getBuffer().getHandle();
            mCurrentArrayBufferOffsets[attribIndex] = emptyBuffer.getOffset();

            bool combined = ShouldCombineAttributes(renderer, attrib, binding);
            mCurrentArrayBufferStrides[attribIndex] =
                combined ? binding.getStride()
                         : vertexFormat.getActualBufferFormat(compressed).pixelBytes;
        }

        if (bufferOnly)
        {
            ANGLE_TRY(contextVk->onVertexBufferChange(mCurrentArrayBuffers[attribIndex]));
        }
        else
        {
            const angle::FormatID format = attrib.format->id;
            ANGLE_TRY(contextVk->onVertexAttributeChange(
                attribIndex, mCurrentArrayBufferStrides[attribIndex], binding.getDivisor(), format,
                compressed, mCurrentArrayBufferRelativeOffsets[attribIndex],
                mCurrentArrayBuffers[attribIndex]));

            mCurrentArrayBufferFormats[attribIndex]    = format;
            mCurrentArrayBufferCompressed[attribIndex] = compressed;
            mCurrentArrayBufferDivisors[attribIndex]   = binding.getDivisor();
        }
    }
    else
    {
        contextVk->invalidateDefaultAttribute(attribIndex);

        // These will be filled out by the ContextVk.
        vk::BufferHelper &emptyBuffer                   = contextVk->getEmptyBuffer();
        mCurrentArrayBuffers[attribIndex]               = &emptyBuffer;
        mCurrentArrayBufferSerial[attribIndex]          = emptyBuffer.getBufferSerial();
        mCurrentArrayBufferHandles[attribIndex]         = emptyBuffer.getBuffer().getHandle();
        mCurrentArrayBufferOffsets[attribIndex]         = emptyBuffer.getOffset();
        mCurrentArrayBufferStrides[attribIndex]         = 0;
        mCurrentArrayBufferDivisors[attribIndex]        = 0;
        mCurrentArrayBufferCompressed[attribIndex]      = false;
        mCurrentArrayBufferRelativeOffsets[attribIndex] = 0;

        ANGLE_TRY(setDefaultPackedInput(contextVk, attribIndex,
                                        &mCurrentArrayBufferFormats[attribIndex]));
    }

    return angle::Result::Continue;
}

gl::AttributesMask VertexArrayVk::mergeClientAttribsRange(
    vk::Renderer *renderer,
    const gl::AttributesMask activeStreamedAttribs,
    size_t startVertex,
    size_t endVertex,
    std::array<AttributeRange, gl::MAX_VERTEX_ATTRIBS> &mergeRangesOut,
    std::array<size_t, gl::MAX_VERTEX_ATTRIBS> &mergedIndexesOut) const
{
    const std::vector<gl::VertexAttribute> &attribs = mState.getVertexAttributes();
    const std::vector<gl::VertexBinding> &bindings  = mState.getVertexBindings();
    gl::AttributesMask attributeMaskCanCombine;
    angle::FixedVector<size_t, gl::MAX_VERTEX_ATTRIBS> combinedIndexes;
    for (size_t attribIndex : activeStreamedAttribs)
    {
        const gl::VertexAttribute &attrib = attribs[attribIndex];
        ASSERT(attrib.enabled);
        const gl::VertexBinding &binding = bindings[attrib.bindingIndex];
        const vk::Format &vertexFormat   = renderer->getFormat(attrib.format->id);
        bool combined                    = ShouldCombineAttributes(renderer, attrib, binding);
        attributeMaskCanCombine.set(attribIndex, combined);
        if (combined)
        {
            combinedIndexes.push_back(attribIndex);
        }
        GLuint pixelBytes      = vertexFormat.getActualBufferFormat(false).pixelBytes;
        size_t destStride      = combined ? binding.getStride() : pixelBytes;
        uintptr_t startAddress = reinterpret_cast<uintptr_t>(attrib.pointer);
        mergeRangesOut[attribIndex].startAddr = startAddress;
        mergeRangesOut[attribIndex].endAddr =
            startAddress + (endVertex - 1) * destStride + pixelBytes;
        mergeRangesOut[attribIndex].copyStartAddr =
            startAddress + startVertex * binding.getStride();
        mergedIndexesOut[attribIndex] = attribIndex;
    }
    if (attributeMaskCanCombine.none())
    {
        return attributeMaskCanCombine;
    }
    auto comp = [&mergeRangesOut](size_t a, size_t b) -> bool {
        return mergeRangesOut[a] < mergeRangesOut[b];
    };
    // Only sort combined range indexes.
    std::sort(combinedIndexes.begin(), combinedIndexes.end(), comp);
    // Merge combined range span.
    auto next = combinedIndexes.begin();
    auto cur  = next++;
    while (next != combinedIndexes.end() || (cur != next))
    {
        // Cur and next overlaps: merge next into cur and move next.
        if (next != combinedIndexes.end() &&
            mergeRangesOut[*cur].endAddr >= mergeRangesOut[*next].startAddr)
        {
            mergeRangesOut[*cur].endAddr =
                std::max(mergeRangesOut[*cur].endAddr, mergeRangesOut[*next].endAddr);
            mergeRangesOut[*cur].copyStartAddr =
                std::min(mergeRangesOut[*cur].copyStartAddr, mergeRangesOut[*next].copyStartAddr);
            mergedIndexesOut[*next] = mergedIndexesOut[*cur];
            ++next;
        }
        else
        {
            ++cur;
            if (cur != next)
            {
                mergeRangesOut[*cur] = mergeRangesOut[*(cur - 1)];
            }
            else if (next != combinedIndexes.end())
            {
                ++next;
            }
        }
    }
    return attributeMaskCanCombine;
}

// Handle copying client attribs and/or expanding attrib buffer in case where attribute
// divisor value has to be emulated.
angle::Result VertexArrayVk::updateStreamedAttribs(const gl::Context *context,
                                                   GLint firstVertex,
                                                   GLsizei vertexOrIndexCount,
                                                   GLsizei instanceCount,
                                                   gl::DrawElementsType indexTypeOrInvalid,
                                                   const void *indices)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    const gl::AttributesMask activeAttribs =
        context->getStateCache().getActiveClientAttribsMask() |
        context->getStateCache().getActiveBufferedAttribsMask();
    const gl::AttributesMask activeStreamedAttribs = mStreamingVertexAttribsMask & activeAttribs;

    // Early return for corner case where emulated buffered attribs are not active
    if (!activeStreamedAttribs.any())
    {
        return angle::Result::Continue;
    }

    GLint startVertex;
    size_t vertexCount;
    ANGLE_TRY(GetVertexRangeInfo(context, firstVertex, vertexOrIndexCount, indexTypeOrInvalid,
                                 indices, 0, &startVertex, &vertexCount));

    ASSERT(vertexCount > 0);
    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    std::array<size_t, gl::MAX_VERTEX_ATTRIBS> mergedIndexes;
    std::array<AttributeRange, gl::MAX_VERTEX_ATTRIBS> mergeRanges;
    std::array<vk::BufferHelper *, gl::MAX_VERTEX_ATTRIBS> attribBufferHelper = {};
    auto mergeAttribMask =
        mergeClientAttribsRange(renderer, activeStreamedAttribs, startVertex,
                                startVertex + vertexCount, mergeRanges, mergedIndexes);

    for (size_t attribIndex : activeStreamedAttribs)
    {
        const gl::VertexAttribute &attrib = attribs[attribIndex];
        ASSERT(attrib.enabled);
        const gl::VertexBinding &binding = bindings[attrib.bindingIndex];

        const vk::Format &vertexFormat = renderer->getFormat(attrib.format->id);
        const angle::Format &dstFormat = vertexFormat.getActualBufferFormat(false);
        GLuint pixelBytes              = dstFormat.pixelBytes;

        const bool compressed = false;
        ASSERT(vertexFormat.getVertexInputAlignment(false) <= vk::kVertexBufferAlignment);

        vk::BufferHelper *vertexDataBuffer = nullptr;
        const uint8_t *src                 = static_cast<const uint8_t *>(attrib.pointer);
        const uint32_t divisor             = binding.getDivisor();

        bool combined            = mergeAttribMask.test(attribIndex);
        GLuint stride            = combined ? binding.getStride() : pixelBytes;
        VkDeviceSize startOffset = 0;
        if (divisor > 0)
        {
            // Instanced attrib
            if (divisor > renderer->getMaxVertexAttribDivisor())
            {
                // Divisor will be set to 1 & so update buffer to have 1 attrib per instance
                size_t bytesToAllocate = instanceCount * stride;

                // Allocate buffer for results
                ANGLE_TRY(contextVk->allocateStreamedVertexBuffer(attribIndex, bytesToAllocate,
                                                                  &vertexDataBuffer));

                gl::Buffer *bufferGL = binding.getBuffer().get();
                if (bufferGL != nullptr)
                {
                    // Only do the data copy if src buffer is valid.
                    if (bufferGL->getSize() > 0)
                    {
                        // Map buffer to expand attribs for divisor emulation
                        BufferVk *bufferVk = vk::GetImpl(binding.getBuffer().get());
                        void *buffSrc      = nullptr;
                        ANGLE_TRY(bufferVk->mapImpl(contextVk, GL_MAP_READ_BIT, &buffSrc));
                        src = reinterpret_cast<const uint8_t *>(buffSrc) + binding.getOffset();

                        uint32_t srcAttributeSize =
                            static_cast<uint32_t>(ComputeVertexAttributeTypeSize(attrib));

                        size_t numVertices = GetVertexCount(bufferVk, binding, srcAttributeSize);

                        ANGLE_TRY(StreamVertexDataWithDivisor(
                            contextVk, vertexDataBuffer, src, bytesToAllocate, binding.getStride(),
                            stride, vertexFormat.getVertexLoadFunction(compressed), divisor,
                            numVertices));

                        ANGLE_TRY(bufferVk->unmapImpl(contextVk));
                    }
                    else if (contextVk->getExtensions().robustnessAny())
                    {
                        // Satisfy robustness constraints (only if extension enabled)
                        uint8_t *dst = vertexDataBuffer->getMappedMemory();
                        memset(dst, 0, bytesToAllocate);
                    }
                }
                else
                {
                    size_t numVertices = instanceCount;
                    ANGLE_TRY(StreamVertexDataWithDivisor(
                        contextVk, vertexDataBuffer, src, bytesToAllocate, binding.getStride(),
                        stride, vertexFormat.getVertexLoadFunction(compressed), divisor,
                        numVertices));
                }
            }
            else
            {
                ASSERT(binding.getBuffer().get() == nullptr);
                size_t count           = UnsignedCeilDivide(instanceCount, divisor);
                size_t bytesToAllocate = count * stride;

                // Allocate buffer for results
                ANGLE_TRY(contextVk->allocateStreamedVertexBuffer(attribIndex, bytesToAllocate,
                                                                  &vertexDataBuffer));

                ANGLE_TRY(StreamVertexData(contextVk, vertexDataBuffer, src, bytesToAllocate, 0,
                                           count, binding.getStride(),
                                           vertexFormat.getVertexLoadFunction(compressed)));
            }
        }
        else
        {
            ASSERT(binding.getBuffer().get() == nullptr);
            size_t mergedAttribIdx      = mergedIndexes[attribIndex];
            const AttributeRange &range = mergeRanges[attribIndex];
            if (attribBufferHelper[mergedAttribIdx] == nullptr)
            {
                size_t destOffset =
                    combined ? range.copyStartAddr - range.startAddr : startVertex * stride;
                size_t bytesToAllocate = range.endAddr - range.startAddr;
                ANGLE_TRY(contextVk->allocateStreamedVertexBuffer(
                    mergedAttribIdx, bytesToAllocate, &attribBufferHelper[mergedAttribIdx]));
                ANGLE_TRY(StreamVertexData(
                    contextVk, attribBufferHelper[mergedAttribIdx],
                    (const uint8_t *)range.copyStartAddr, bytesToAllocate - destOffset, destOffset,
                    vertexCount, binding.getStride(),
                    combined ? nullptr : vertexFormat.getVertexLoadFunction(compressed)));
            }
            vertexDataBuffer = attribBufferHelper[mergedAttribIdx];
            startOffset      = combined ? (uintptr_t)attrib.pointer - range.startAddr : 0;
        }
        ASSERT(vertexDataBuffer != nullptr);
        mCurrentArrayBuffers[attribIndex]      = vertexDataBuffer;
        mCurrentArrayBufferSerial[attribIndex] = vertexDataBuffer->getBufferSerial();
        VkDeviceSize bufferOffset;
        mCurrentArrayBufferHandles[attribIndex] =
            vertexDataBuffer
                ->getBufferForVertexArray(contextVk, vertexDataBuffer->getSize(), &bufferOffset)
                .getHandle();
        mCurrentArrayBufferOffsets[attribIndex]  = bufferOffset + startOffset;
        mCurrentArrayBufferStrides[attribIndex]  = stride;
        mCurrentArrayBufferDivisors[attribIndex] = divisor;
        ASSERT(BindingIsAligned(dstFormat, mCurrentArrayBufferOffsets[attribIndex],
                                mCurrentArrayBufferStrides[attribIndex]));
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::handleLineLoop(ContextVk *contextVk,
                                            GLint firstVertex,
                                            GLsizei vertexOrIndexCount,
                                            gl::DrawElementsType indexTypeOrInvalid,
                                            const void *indices,
                                            vk::BufferHelper **indexBufferOut,
                                            uint32_t *indexCountOut)
{
    if (indexTypeOrInvalid != gl::DrawElementsType::InvalidEnum)
    {
        // Handle GL_LINE_LOOP drawElements.
        if (mDirtyLineLoopTranslation)
        {
            gl::Buffer *elementArrayBuffer = mState.getElementArrayBuffer();

            if (!elementArrayBuffer)
            {
                ANGLE_TRY(mLineLoopHelper.streamIndices(
                    contextVk, indexTypeOrInvalid, vertexOrIndexCount,
                    reinterpret_cast<const uint8_t *>(indices), indexBufferOut, indexCountOut));
            }
            else
            {
                // When using an element array buffer, 'indices' is an offset to the first element.
                intptr_t offset                = reinterpret_cast<intptr_t>(indices);
                BufferVk *elementArrayBufferVk = vk::GetImpl(elementArrayBuffer);
                ANGLE_TRY(mLineLoopHelper.getIndexBufferForElementArrayBuffer(
                    contextVk, elementArrayBufferVk, indexTypeOrInvalid, vertexOrIndexCount, offset,
                    indexBufferOut, indexCountOut));
            }
        }

        // If we've had a drawArrays call with a line loop before, we want to make sure this is
        // invalidated the next time drawArrays is called since we use the same index buffer for
        // both calls.
        mLineLoopBufferFirstIndex.reset();
        mLineLoopBufferLastIndex.reset();
        return angle::Result::Continue;
    }

    // Note: Vertex indexes can be arbitrarily large.
    uint32_t clampedVertexCount = gl::clampCast<uint32_t>(vertexOrIndexCount);

    // Handle GL_LINE_LOOP drawArrays.
    size_t lastVertex = static_cast<size_t>(firstVertex + clampedVertexCount);
    if (!mLineLoopBufferFirstIndex.valid() || !mLineLoopBufferLastIndex.valid() ||
        mLineLoopBufferFirstIndex != firstVertex || mLineLoopBufferLastIndex != lastVertex)
    {
        ANGLE_TRY(mLineLoopHelper.getIndexBufferForDrawArrays(contextVk, clampedVertexCount,
                                                              firstVertex, indexBufferOut));

        mLineLoopBufferFirstIndex = firstVertex;
        mLineLoopBufferLastIndex  = lastVertex;
    }
    else
    {
        *indexBufferOut = mLineLoopHelper.getCurrentIndexBuffer();
    }
    *indexCountOut = vertexOrIndexCount + 1;

    return angle::Result::Continue;
}

angle::Result VertexArrayVk::updateDefaultAttrib(ContextVk *contextVk, size_t attribIndex)
{
    if (!mState.getEnabledAttributesMask().test(attribIndex))
    {
        vk::BufferHelper *bufferHelper;
        ANGLE_TRY(
            contextVk->allocateStreamedVertexBuffer(attribIndex, kDefaultValueSize, &bufferHelper));

        const gl::VertexAttribCurrentValueData &defaultValue =
            contextVk->getState().getVertexAttribCurrentValues()[attribIndex];
        uint8_t *ptr = bufferHelper->getMappedMemory();
        memcpy(ptr, &defaultValue.Values, kDefaultValueSize);
        ANGLE_TRY(bufferHelper->flush(contextVk->getRenderer()));

        VkDeviceSize bufferOffset;
        mCurrentArrayBufferHandles[attribIndex] =
            bufferHelper->getBufferForVertexArray(contextVk, kDefaultValueSize, &bufferOffset)
                .getHandle();
        mCurrentArrayBufferOffsets[attribIndex]  = bufferOffset;
        mCurrentArrayBuffers[attribIndex]        = bufferHelper;
        mCurrentArrayBufferSerial[attribIndex]   = bufferHelper->getBufferSerial();
        mCurrentArrayBufferStrides[attribIndex]  = 0;
        mCurrentArrayBufferDivisors[attribIndex] = 0;

        ANGLE_TRY(setDefaultPackedInput(contextVk, attribIndex,
                                        &mCurrentArrayBufferFormats[attribIndex]));
    }

    return angle::Result::Continue;
}
}  // namespace rx
