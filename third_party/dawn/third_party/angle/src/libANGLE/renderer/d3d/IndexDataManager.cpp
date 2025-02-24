//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexDataManager.cpp: Defines the IndexDataManager, a class that
// runs the Buffer translation process for index buffers.

#include "libANGLE/renderer/d3d/IndexDataManager.h"

#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"

namespace rx
{

namespace
{

template <typename InputT, typename DestT>
void ConvertIndexArray(const void *input,
                       gl::DrawElementsType sourceType,
                       void *output,
                       gl::DrawElementsType destinationType,
                       GLsizei count,
                       bool usePrimitiveRestartFixedIndex)
{
    const InputT *in = static_cast<const InputT *>(input);
    DestT *out       = static_cast<DestT *>(output);

    if (usePrimitiveRestartFixedIndex)
    {
        InputT srcRestartIndex = static_cast<InputT>(gl::GetPrimitiveRestartIndex(sourceType));
        DestT destRestartIndex = static_cast<DestT>(gl::GetPrimitiveRestartIndex(destinationType));
        for (GLsizei i = 0; i < count; i++)
        {
            out[i] = (in[i] == srcRestartIndex ? destRestartIndex : static_cast<DestT>(in[i]));
        }
    }
    else
    {
        for (GLsizei i = 0; i < count; i++)
        {
            out[i] = static_cast<DestT>(in[i]);
        }
    }
}

void ConvertIndices(gl::DrawElementsType sourceType,
                    gl::DrawElementsType destinationType,
                    const void *input,
                    GLsizei count,
                    void *output,
                    bool usePrimitiveRestartFixedIndex)
{
    if (sourceType == destinationType)
    {
        const GLuint dstTypeSize = gl::GetDrawElementsTypeSize(destinationType);
        memcpy(output, input, count * dstTypeSize);
        return;
    }

    if (sourceType == gl::DrawElementsType::UnsignedByte)
    {
        ASSERT(destinationType == gl::DrawElementsType::UnsignedShort);
        ConvertIndexArray<GLubyte, GLushort>(input, sourceType, output, destinationType, count,
                                             usePrimitiveRestartFixedIndex);
    }
    else if (sourceType == gl::DrawElementsType::UnsignedShort)
    {
        ASSERT(destinationType == gl::DrawElementsType::UnsignedInt);
        ConvertIndexArray<GLushort, GLuint>(input, sourceType, output, destinationType, count,
                                            usePrimitiveRestartFixedIndex);
    }
    else
        UNREACHABLE();
}

angle::Result StreamInIndexBuffer(const gl::Context *context,
                                  IndexBufferInterface *buffer,
                                  const void *data,
                                  unsigned int count,
                                  gl::DrawElementsType srcType,
                                  gl::DrawElementsType dstType,
                                  bool usePrimitiveRestartFixedIndex,
                                  unsigned int *offset)
{
    const GLuint dstTypeBytesShift = gl::GetDrawElementsTypeShift(dstType);

    bool check = (count > (std::numeric_limits<unsigned int>::max() >> dstTypeBytesShift));
    ANGLE_CHECK(GetImplAs<ContextD3D>(context), !check,
                "Reserving indices exceeds the maximum buffer size.", GL_OUT_OF_MEMORY);

    unsigned int bufferSizeRequired = count << dstTypeBytesShift;
    ANGLE_TRY(buffer->reserveBufferSpace(context, bufferSizeRequired, dstType));

    void *output = nullptr;
    ANGLE_TRY(buffer->mapBuffer(context, bufferSizeRequired, &output, offset));

    ConvertIndices(srcType, dstType, data, count, output, usePrimitiveRestartFixedIndex);

    ANGLE_TRY(buffer->unmapBuffer(context));
    return angle::Result::Continue;
}
}  // anonymous namespace

// IndexDataManager implementation.
IndexDataManager::IndexDataManager(BufferFactoryD3D *factory)
    : mFactory(factory), mStreamingBufferShort(), mStreamingBufferInt()
{}

IndexDataManager::~IndexDataManager() {}

void IndexDataManager::deinitialize()
{
    mStreamingBufferShort.reset();
    mStreamingBufferInt.reset();
}

// This function translates a GL-style indices into DX-style indices, with their description
// returned in translated.
// GL can specify vertex data in immediate mode (pointer to CPU array of indices), which is not
// possible in DX and requires streaming (Case 1). If the GL indices are specified with a buffer
// (Case 2), in a format supported by DX (subcase a) then all is good.
// When we have a buffer with an unsupported format (subcase b) then we need to do some translation:
// we will start by falling back to streaming, and after a while will start using a static
// translated copy of the index buffer.
angle::Result IndexDataManager::prepareIndexData(const gl::Context *context,
                                                 gl::DrawElementsType srcType,
                                                 gl::DrawElementsType dstType,
                                                 GLsizei count,
                                                 gl::Buffer *glBuffer,
                                                 const void *indices,
                                                 TranslatedIndexData *translated)
{
    GLuint srcTypeBytes = gl::GetDrawElementsTypeSize(srcType);
    GLuint srcTypeShift = gl::GetDrawElementsTypeShift(srcType);
    GLuint dstTypeShift = gl::GetDrawElementsTypeShift(dstType);

    BufferD3D *buffer = glBuffer ? GetImplAs<BufferD3D>(glBuffer) : nullptr;

    translated->indexType                 = dstType;
    translated->srcIndexData.srcBuffer    = buffer;
    translated->srcIndexData.srcIndices   = indices;
    translated->srcIndexData.srcIndexType = srcType;
    translated->srcIndexData.srcCount     = count;

    // Context can be nullptr in perf tests.
    bool primitiveRestartFixedIndexEnabled =
        context ? context->getState().isPrimitiveRestartEnabled() : false;

    // Case 1: the indices are passed by pointer, which forces the streaming of index data
    if (glBuffer == nullptr)
    {
        translated->storage = nullptr;
        return streamIndexData(context, indices, count, srcType, dstType,
                               primitiveRestartFixedIndexEnabled, translated);
    }

    // Case 2: the indices are already in a buffer
    unsigned int offset = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(indices));
    ASSERT(srcTypeBytes * static_cast<unsigned int>(count) + offset <= buffer->getSize());

    bool offsetAligned = IsOffsetAligned(srcType, offset);

    // Case 2a: the buffer can be used directly
    if (offsetAligned && buffer->supportsDirectBinding() && dstType == srcType)
    {
        translated->storage     = buffer;
        translated->indexBuffer = nullptr;
        translated->serial      = buffer->getSerial();
        translated->startIndex  = (offset >> srcTypeShift);
        translated->startOffset = offset;
        return angle::Result::Continue;
    }

    translated->storage = nullptr;

    // Case 2b: use a static translated copy or fall back to streaming
    StaticIndexBufferInterface *staticBuffer = buffer->getStaticIndexBuffer();

    bool staticBufferInitialized = staticBuffer && staticBuffer->getBufferSize() != 0;
    bool staticBufferUsable =
        staticBuffer && offsetAligned && staticBuffer->getIndexType() == dstType;

    if (staticBufferInitialized && !staticBufferUsable)
    {
        buffer->invalidateStaticData(context);
        staticBuffer = nullptr;
    }

    if (staticBuffer == nullptr || !offsetAligned)
    {
        const uint8_t *bufferData = nullptr;
        ANGLE_TRY(buffer->getData(context, &bufferData));
        ASSERT(bufferData != nullptr);

        ANGLE_TRY(streamIndexData(context, bufferData + offset, count, srcType, dstType,
                                  primitiveRestartFixedIndexEnabled, translated));
        buffer->promoteStaticUsage(context, count << srcTypeShift);
    }
    else
    {
        if (!staticBufferInitialized)
        {
            const uint8_t *bufferData = nullptr;
            ANGLE_TRY(buffer->getData(context, &bufferData));
            ASSERT(bufferData != nullptr);

            unsigned int convertCount =
                static_cast<unsigned int>(buffer->getSize()) >> srcTypeShift;
            ANGLE_TRY(StreamInIndexBuffer(context, staticBuffer, bufferData, convertCount, srcType,
                                          dstType, primitiveRestartFixedIndexEnabled, nullptr));
        }
        ASSERT(offsetAligned && staticBuffer->getIndexType() == dstType);

        translated->indexBuffer = staticBuffer->getIndexBuffer();
        translated->serial      = staticBuffer->getSerial();
        translated->startIndex  = (offset >> srcTypeShift);
        translated->startOffset = (offset >> srcTypeShift) << dstTypeShift;
    }

    return angle::Result::Continue;
}

angle::Result IndexDataManager::streamIndexData(const gl::Context *context,
                                                const void *data,
                                                unsigned int count,
                                                gl::DrawElementsType srcType,
                                                gl::DrawElementsType dstType,
                                                bool usePrimitiveRestartFixedIndex,
                                                TranslatedIndexData *translated)
{
    const GLuint dstTypeShift = gl::GetDrawElementsTypeShift(dstType);

    IndexBufferInterface *indexBuffer = nullptr;
    ANGLE_TRY(getStreamingIndexBuffer(context, dstType, &indexBuffer));
    ASSERT(indexBuffer != nullptr);

    unsigned int offset;
    ANGLE_TRY(StreamInIndexBuffer(context, indexBuffer, data, count, srcType, dstType,
                                  usePrimitiveRestartFixedIndex, &offset));

    translated->indexBuffer = indexBuffer->getIndexBuffer();
    translated->serial      = indexBuffer->getSerial();
    translated->startIndex  = (offset >> dstTypeShift);
    translated->startOffset = offset;

    return angle::Result::Continue;
}

angle::Result IndexDataManager::getStreamingIndexBuffer(const gl::Context *context,
                                                        gl::DrawElementsType destinationIndexType,
                                                        IndexBufferInterface **outBuffer)
{
    ASSERT(outBuffer);
    ASSERT(destinationIndexType == gl::DrawElementsType::UnsignedShort ||
           destinationIndexType == gl::DrawElementsType::UnsignedInt);

    auto &streamingBuffer = (destinationIndexType == gl::DrawElementsType::UnsignedInt)
                                ? mStreamingBufferInt
                                : mStreamingBufferShort;

    if (!streamingBuffer)
    {
        StreamingBuffer newBuffer(new StreamingIndexBufferInterface(mFactory));
        ANGLE_TRY(newBuffer->reserveBufferSpace(context, INITIAL_INDEX_BUFFER_SIZE,
                                                destinationIndexType));
        streamingBuffer = std::move(newBuffer);
    }

    *outBuffer = streamingBuffer.get();
    return angle::Result::Continue;
}

angle::Result GetIndexTranslationDestType(const gl::Context *context,
                                          GLsizei indexCount,
                                          gl::DrawElementsType indexType,
                                          const void *indices,
                                          bool usePrimitiveRestartWorkaround,
                                          gl::DrawElementsType *destTypeOut)
{
    // Avoid D3D11's primitive restart index value
    // see http://msdn.microsoft.com/en-us/library/windows/desktop/bb205124(v=vs.85).aspx
    if (usePrimitiveRestartWorkaround)
    {
        // Conservatively assume we need to translate the indices for draw indirect.
        // This is a bit of a trick. We assume the count for an indirect draw is zero.
        if (indexCount == 0)
        {
            *destTypeOut = gl::DrawElementsType::UnsignedInt;
            return angle::Result::Continue;
        }

        gl::IndexRange indexRange;
        ANGLE_TRY(context->getState().getVertexArray()->getIndexRange(
            context, indexType, indexCount, indices, &indexRange));
        if (indexRange.end == gl::GetPrimitiveRestartIndex(indexType))
        {
            *destTypeOut = gl::DrawElementsType::UnsignedInt;
            return angle::Result::Continue;
        }
    }

    *destTypeOut = (indexType == gl::DrawElementsType::UnsignedInt)
                       ? gl::DrawElementsType::UnsignedInt
                       : gl::DrawElementsType::UnsignedShort;
    return angle::Result::Continue;
}

}  // namespace rx
