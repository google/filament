//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.cpp: Implements the class methods for VertexArrayGL.

#include "libANGLE/renderer/gl/VertexArrayGL.h"

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

using namespace gl;

namespace rx
{
namespace
{

GLuint GetNativeBufferID(const gl::Buffer *frontendBuffer)
{
    return frontendBuffer ? GetImplAs<BufferGL>(frontendBuffer)->getBufferID() : 0;
}

bool SameVertexAttribFormat(const VertexAttributeGL &a, const VertexAttribute &b)
{
    return a.format == b.format && a.relativeOffset == b.relativeOffset;
}

bool SameVertexBuffer(const VertexBindingGL &a, const VertexBinding &b)
{
    return a.stride == b.getStride() && a.offset == b.getOffset() &&
           a.buffer == GetNativeBufferID(b.getBuffer().get());
}

bool SameIndexBuffer(const VertexArrayStateGL *a, const gl::Buffer *frontendBuffer)
{
    return a->elementArrayBuffer == GetNativeBufferID(frontendBuffer);
}

bool SameAttribPointer(const VertexAttributeGL &a, const VertexAttribute &b)
{
    return a.pointer == b.pointer;
}

bool IsVertexAttribPointerSupported(size_t attribIndex, const VertexAttribute &attrib)
{
    return (attribIndex == attrib.bindingIndex && attrib.relativeOffset == 0);
}

GLuint GetAdjustedDivisor(GLuint numViews, GLuint divisor)
{
    return numViews * divisor;
}

static angle::Result ValidateStateHelperGetIntegerv(const gl::Context *context,
                                                    const GLuint localValue,
                                                    const GLenum pname,
                                                    const char *localName,
                                                    const char *driverName)
{
    const FunctionsGL *functions = GetFunctionsGL(context);

    GLint queryValue;
    ANGLE_GL_TRY(context, functions->getIntegerv(pname, &queryValue));
    if (localValue != static_cast<GLuint>(queryValue))
    {
        WARN() << localName << " (" << localValue << ") != " << driverName << " (" << queryValue
               << ")";
        // Re-add ASSERT: http://anglebug.com/42262547
        // ASSERT(false);
    }

    return angle::Result::Continue;
}

static angle::Result ValidateStateHelperGetVertexAttribiv(const gl::Context *context,
                                                          const GLint index,
                                                          const GLuint localValue,
                                                          const GLenum pname,
                                                          const char *localName,
                                                          const char *driverName)
{
    const FunctionsGL *functions = GetFunctionsGL(context);

    GLint queryValue;
    ANGLE_GL_TRY(context, functions->getVertexAttribiv(index, pname, &queryValue));
    if (localValue != static_cast<GLuint>(queryValue))
    {
        WARN() << localName << "[" << index << "] (" << localValue << ") != " << driverName << "["
               << index << "] (" << queryValue << ")";
        // Re-add ASSERT: http://anglebug.com/42262547
        // ASSERT(false);
    }

    return angle::Result::Continue;
}
}  // anonymous namespace

VertexArrayGL::VertexArrayGL(const VertexArrayState &state, GLuint id)
    : VertexArrayImpl(state),
      mVertexArrayID(id),
      mOwnsNativeState(true),
      mNativeState(new VertexArrayStateGL(state.getMaxAttribs(), state.getMaxBindings()))
{
    mForcedStreamingAttributesFirstOffsets.fill(0);
}

VertexArrayGL::VertexArrayGL(const gl::VertexArrayState &state,
                             GLuint id,
                             VertexArrayStateGL *sharedState)
    : VertexArrayImpl(state), mVertexArrayID(id), mOwnsNativeState(false), mNativeState(sharedState)
{
    ASSERT(mNativeState);
    mForcedStreamingAttributesFirstOffsets.fill(0);
}

VertexArrayGL::~VertexArrayGL() {}

void VertexArrayGL::destroy(const gl::Context *context)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);

    if (mOwnsNativeState)
    {
        stateManager->deleteVertexArray(mVertexArrayID);
    }
    mVertexArrayID   = 0;
    mAppliedNumViews = 1;

    mElementArrayBuffer.set(context, nullptr);
    for (gl::BindingPointer<gl::Buffer> &binding : mArrayBuffers)
    {
        binding.set(context, nullptr);
    }

    stateManager->deleteBuffer(mStreamingElementArrayBuffer);
    mStreamingElementArrayBufferSize = 0;
    mStreamingElementArrayBuffer     = 0;

    stateManager->deleteBuffer(mStreamingArrayBuffer);
    mStreamingArrayBufferSize = 0;
    mStreamingArrayBuffer     = 0;

    if (mOwnsNativeState)
    {
        delete mNativeState;
    }
    mNativeState = nullptr;
}

angle::Result VertexArrayGL::syncClientSideData(const gl::Context *context,
                                                const gl::AttributesMask &activeAttributesMask,
                                                GLint first,
                                                GLsizei count,
                                                GLsizei instanceCount) const
{
    return syncDrawState(context, activeAttributesMask, first, count,
                         gl::DrawElementsType::InvalidEnum, nullptr, instanceCount, false, nullptr);
}

angle::Result VertexArrayGL::updateElementArrayBufferBinding(const gl::Context *context) const
{
    gl::Buffer *elementArrayBuffer = mState.getElementArrayBuffer();
    if (!SameIndexBuffer(mNativeState, elementArrayBuffer))
    {
        GLuint elementArrayBufferId =
            elementArrayBuffer ? GetNativeBufferID(elementArrayBuffer) : 0;

        StateManagerGL *stateManager = GetStateManagerGL(context);
        stateManager->bindBuffer(gl::BufferBinding::ElementArray, elementArrayBufferId);
        mElementArrayBuffer.set(context, elementArrayBuffer);
        mNativeState->elementArrayBuffer = elementArrayBufferId;
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::syncDrawState(const gl::Context *context,
                                           const gl::AttributesMask &activeAttributesMask,
                                           GLint first,
                                           GLsizei count,
                                           gl::DrawElementsType type,
                                           const void *indices,
                                           GLsizei instanceCount,
                                           bool primitiveRestartEnabled,
                                           const void **outIndices) const
{
    const FunctionsGL *functions = GetFunctionsGL(context);

    // Check if any attributes need to be streamed, determines if the index range needs to be
    // computed
    gl::AttributesMask needsStreamingAttribs =
        context->getStateCache().getActiveClientAttribsMask();
    if (nativegl::CanUseClientSideArrays(functions, mVertexArrayID))
    {
        needsStreamingAttribs.reset();
    }

    // Determine if an index buffer needs to be streamed and the range of vertices that need to be
    // copied
    IndexRange indexRange;
    const angle::FeaturesGL &features = GetFeaturesGL(context);
    if (type != gl::DrawElementsType::InvalidEnum)
    {
        ANGLE_TRY(syncIndexData(context, count, type, indices, primitiveRestartEnabled,
                                needsStreamingAttribs.any(), &indexRange, outIndices));
    }
    else
    {
        // Not an indexed call, set the range to [first, first + count - 1]
        indexRange.start = first;
        indexRange.end   = first + count - 1;

        if (features.shiftInstancedArrayDataWithOffset.enabled && first > 0)
        {
            gl::AttributesMask updatedStreamingAttribsMask = needsStreamingAttribs;
            auto candidateAttributesMask =
                mInstancedAttributesMask & mProgramActiveAttribLocationsMask;
            for (auto attribIndex : candidateAttributesMask)
            {

                if (mForcedStreamingAttributesFirstOffsets[attribIndex] != first)
                {
                    updatedStreamingAttribsMask.set(attribIndex);
                    mForcedStreamingAttributesForDrawArraysInstancedMask.set(attribIndex);
                    mForcedStreamingAttributesFirstOffsets[attribIndex] = first;
                }
            }

            // We need to recover attributes whose divisor used to be > 0 but is reset to 0 now if
            // any
            auto forcedStreamingAttributesNeedRecoverMask =
                candidateAttributesMask ^ mForcedStreamingAttributesForDrawArraysInstancedMask;
            if (forcedStreamingAttributesNeedRecoverMask.any())
            {
                ANGLE_TRY(recoverForcedStreamingAttributesForDrawArraysInstanced(
                    context, &forcedStreamingAttributesNeedRecoverMask));
                mForcedStreamingAttributesForDrawArraysInstancedMask = candidateAttributesMask;
            }

            if (updatedStreamingAttribsMask.any())
            {
                ANGLE_TRY(streamAttributes(context, updatedStreamingAttribsMask, instanceCount,
                                           indexRange, true));
            }
            return angle::Result::Continue;
        }
    }

    if (needsStreamingAttribs.any())
    {
        ANGLE_TRY(
            streamAttributes(context, needsStreamingAttribs, instanceCount, indexRange, false));
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::syncIndexData(const gl::Context *context,
                                           GLsizei count,
                                           gl::DrawElementsType type,
                                           const void *indices,
                                           bool primitiveRestartEnabled,
                                           bool attributesNeedStreaming,
                                           IndexRange *outIndexRange,
                                           const void **outIndices) const
{
    ASSERT(outIndices);

    const FunctionsGL *functions = GetFunctionsGL(context);

    gl::Buffer *elementArrayBuffer = mState.getElementArrayBuffer();

    // Need to check the range of indices if attributes need to be streamed
    if (elementArrayBuffer)
    {
        ASSERT(SameIndexBuffer(mNativeState, elementArrayBuffer));
        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            ptrdiff_t elementArrayBufferOffset = reinterpret_cast<ptrdiff_t>(indices);
            ANGLE_TRY(mState.getElementArrayBuffer()->getIndexRange(
                context, type, elementArrayBufferOffset, count, primitiveRestartEnabled,
                outIndexRange));
        }

        // Indices serves as an offset into the index buffer in this case, use the same value for
        // the draw call
        *outIndices = indices;
    }
    else if (nativegl::CanUseClientSideArrays(functions, mVertexArrayID))
    {
        ASSERT(SameIndexBuffer(mNativeState, nullptr));

        // Use the client index data directly
        *outIndices = indices;
    }
    else
    {
        StateManagerGL *stateManager = GetStateManagerGL(context);

        // Need to stream the index buffer

        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            *outIndexRange = ComputeIndexRange(type, indices, count, primitiveRestartEnabled);
        }

        // Allocate the streaming element array buffer
        if (mStreamingElementArrayBuffer == 0)
        {
            ANGLE_GL_TRY(context, functions->genBuffers(1, &mStreamingElementArrayBuffer));
            mStreamingElementArrayBufferSize = 0;
        }

        stateManager->bindVertexArray(mVertexArrayID, mNativeState);

        stateManager->bindBuffer(gl::BufferBinding::ElementArray, mStreamingElementArrayBuffer);
        mElementArrayBuffer.set(context, nullptr);
        mNativeState->elementArrayBuffer = mStreamingElementArrayBuffer;

        // Make sure the element array buffer is large enough
        const GLuint indexTypeBytes        = gl::GetDrawElementsTypeSize(type);
        size_t requiredStreamingBufferSize = indexTypeBytes * count;
        if (requiredStreamingBufferSize > mStreamingElementArrayBufferSize)
        {
            // Copy the indices in while resizing the buffer
            ANGLE_GL_TRY(context,
                         functions->bufferData(GL_ELEMENT_ARRAY_BUFFER, requiredStreamingBufferSize,
                                               indices, GL_DYNAMIC_DRAW));
            mStreamingElementArrayBufferSize = requiredStreamingBufferSize;
        }
        else
        {
            // Put the indices at the beginning of the buffer
            ANGLE_GL_TRY(context, functions->bufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                                                           requiredStreamingBufferSize, indices));
        }

        // Set the index offset for the draw call to zero since the supplied index pointer is to
        // client data
        *outIndices = nullptr;
    }

    return angle::Result::Continue;
}

void VertexArrayGL::computeStreamingAttributeSizes(const gl::AttributesMask &attribsToStream,
                                                   GLsizei instanceCount,
                                                   const gl::IndexRange &indexRange,
                                                   size_t *outStreamingDataSize,
                                                   size_t *outMaxAttributeDataSize) const
{
    *outStreamingDataSize    = 0;
    *outMaxAttributeDataSize = 0;

    ASSERT(attribsToStream.any());

    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    for (auto idx : attribsToStream)
    {
        const auto &attrib  = attribs[idx];
        const auto &binding = bindings[attrib.bindingIndex];

        // If streaming is going to be required, compute the size of the required buffer
        // and how much slack space at the beginning of the buffer will be required by determining
        // the attribute with the largest data size.
        size_t typeSize        = ComputeVertexAttributeTypeSize(attrib);
        GLuint adjustedDivisor = GetAdjustedDivisor(mAppliedNumViews, binding.getDivisor());
        *outStreamingDataSize +=
            typeSize * ComputeVertexBindingElementCount(adjustedDivisor, indexRange.vertexCount(),
                                                        instanceCount);
        *outMaxAttributeDataSize = std::max(*outMaxAttributeDataSize, typeSize);
    }
}

angle::Result VertexArrayGL::streamAttributes(
    const gl::Context *context,
    const gl::AttributesMask &attribsToStream,
    GLsizei instanceCount,
    const gl::IndexRange &indexRange,
    bool applyExtraOffsetWorkaroundForInstancedAttributes) const
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    StateManagerGL *stateManager = GetStateManagerGL(context);

    // Sync the vertex attribute state and track what data needs to be streamed
    size_t streamingDataSize    = 0;
    size_t maxAttributeDataSize = 0;

    computeStreamingAttributeSizes(attribsToStream, instanceCount, indexRange, &streamingDataSize,
                                   &maxAttributeDataSize);

    if (streamingDataSize == 0)
    {
        return angle::Result::Continue;
    }

    if (mStreamingArrayBuffer == 0)
    {
        ANGLE_GL_TRY(context, functions->genBuffers(1, &mStreamingArrayBuffer));
        mStreamingArrayBufferSize = 0;
    }

    // If first is greater than zero, a slack space needs to be left at the beginning of the buffer
    // for each attribute so that the same 'first' argument can be passed into the draw call.
    const size_t bufferEmptySpace =
        attribsToStream.count() * maxAttributeDataSize * indexRange.start;
    const size_t requiredBufferSize = streamingDataSize + bufferEmptySpace;

    stateManager->bindBuffer(gl::BufferBinding::Array, mStreamingArrayBuffer);
    if (requiredBufferSize > mStreamingArrayBufferSize)
    {
        ANGLE_GL_TRY(context, functions->bufferData(GL_ARRAY_BUFFER, requiredBufferSize, nullptr,
                                                    GL_DYNAMIC_DRAW));
        mStreamingArrayBufferSize = requiredBufferSize;
    }

    stateManager->bindVertexArray(mVertexArrayID, mNativeState);

    // Unmapping a buffer can return GL_FALSE to indicate that the system has corrupted the data
    // somehow (such as by a screen change), retry writing the data a few times and return
    // OUT_OF_MEMORY if that fails.
    GLboolean unmapResult     = GL_FALSE;
    size_t unmapRetryAttempts = 5;
    while (unmapResult != GL_TRUE && --unmapRetryAttempts > 0)
    {
        uint8_t *bufferPointer = MapBufferRangeWithFallback(functions, GL_ARRAY_BUFFER, 0,
                                                            requiredBufferSize, GL_MAP_WRITE_BIT);
        size_t curBufferOffset = maxAttributeDataSize * indexRange.start;

        const auto &attribs  = mState.getVertexAttributes();
        const auto &bindings = mState.getVertexBindings();

        for (auto idx : attribsToStream)
        {
            const auto &attrib = attribs[idx];
            ASSERT(IsVertexAttribPointerSupported(idx, attrib));

            const auto &binding = bindings[attrib.bindingIndex];

            GLuint adjustedDivisor = GetAdjustedDivisor(mAppliedNumViews, binding.getDivisor());
            // streamedVertexCount is only going to be modified by
            // shiftInstancedArrayDataWithOffset workaround, otherwise it's const
            size_t streamedVertexCount = ComputeVertexBindingElementCount(
                adjustedDivisor, indexRange.vertexCount(), instanceCount);

            const size_t sourceStride = ComputeVertexAttributeStride(attrib, binding);
            const size_t destStride   = ComputeVertexAttributeTypeSize(attrib);

            // Vertices do not apply the 'start' offset when the divisor is non-zero even when doing
            // a non-instanced draw call
            const size_t firstIndex =
                (adjustedDivisor == 0 || applyExtraOffsetWorkaroundForInstancedAttributes)
                    ? indexRange.start
                    : 0;

            // Attributes using client memory ignore the VERTEX_ATTRIB_BINDING state.
            // https://www.opengl.org/registry/specs/ARB/vertex_attrib_binding.txt
            const uint8_t *inputPointer = static_cast<const uint8_t *>(attrib.pointer);
            // store batchMemcpySize since streamedVertexCount could be changed by workaround
            const size_t batchMemcpySize = destStride * streamedVertexCount;

            size_t batchMemcpyInputOffset                    = sourceStride * firstIndex;
            bool needsUnmapAndRebindStreamingAttributeBuffer = false;
            size_t firstIndexForSeparateCopy                 = firstIndex;

            if (applyExtraOffsetWorkaroundForInstancedAttributes && adjustedDivisor > 0)
            {
                const size_t originalStreamedVertexCount = streamedVertexCount;
                streamedVertexCount =
                    (instanceCount + indexRange.start + adjustedDivisor - 1u) / adjustedDivisor;

                const size_t copySize =
                    sourceStride *
                    originalStreamedVertexCount;  // the real data in the buffer we are streaming

                const gl::Buffer *bindingBufferPointer = binding.getBuffer().get();
                if (!bindingBufferPointer)
                {
                    if (!inputPointer)
                    {
                        continue;
                    }
                    inputPointer = static_cast<const uint8_t *>(attrib.pointer);
                }
                else
                {
                    needsUnmapAndRebindStreamingAttributeBuffer = true;
                    const auto buffer = GetImplAs<BufferGL>(bindingBufferPointer);
                    stateManager->bindBuffer(gl::BufferBinding::Array, buffer->getBufferID());
                    // The workaround is only for latest Mac Intel so glMapBufferRange should be
                    // supported
                    ASSERT(CanMapBufferForRead(functions));
                    // Validate if there is OOB access of the input buffer.
                    angle::CheckedNumeric<GLint64> inputRequiredSize;
                    inputRequiredSize = copySize;
                    inputRequiredSize += static_cast<unsigned int>(binding.getOffset());
                    ANGLE_CHECK(GetImplAs<ContextGL>(context),
                                inputRequiredSize.IsValid() && inputRequiredSize.ValueOrDie() <=
                                                                   bindingBufferPointer->getSize(),
                                "Failed to map buffer range of the attribute buffer.",
                                GL_OUT_OF_MEMORY);
                    uint8_t *inputBufferPointer = MapBufferRangeWithFallback(
                        functions, GL_ARRAY_BUFFER, binding.getOffset(), copySize, GL_MAP_READ_BIT);
                    ASSERT(inputBufferPointer);
                    inputPointer = inputBufferPointer;
                }

                batchMemcpyInputOffset    = 0;
                firstIndexForSeparateCopy = 0;
            }

            // Pack the data when copying it, user could have supplied a very large stride that
            // would cause the buffer to be much larger than needed.
            if (destStride == sourceStride)
            {
                // Can copy in one go, the data is packed
                memcpy(bufferPointer + curBufferOffset, inputPointer + batchMemcpyInputOffset,
                       batchMemcpySize);
            }
            else
            {
                for (size_t vertexIdx = 0; vertexIdx < streamedVertexCount; vertexIdx++)
                {
                    uint8_t *out = bufferPointer + curBufferOffset + (destStride * vertexIdx);
                    const uint8_t *in =
                        inputPointer + sourceStride * (vertexIdx + firstIndexForSeparateCopy);
                    memcpy(out, in, destStride);
                }
            }

            if (needsUnmapAndRebindStreamingAttributeBuffer)
            {
                ANGLE_GL_TRY(context, functions->unmapBuffer(GL_ARRAY_BUFFER));
                stateManager->bindBuffer(gl::BufferBinding::Array, mStreamingArrayBuffer);
            }

            // Compute where the 0-index vertex would be.
            const size_t vertexStartOffset = curBufferOffset - (firstIndex * destStride);

            ANGLE_TRY(callVertexAttribPointer(context, static_cast<GLuint>(idx), attrib,
                                              static_cast<GLsizei>(destStride),
                                              static_cast<GLintptr>(vertexStartOffset)));

            // Update the state to track the streamed attribute
            mNativeState->attributes[idx].format = attrib.format;

            mNativeState->attributes[idx].relativeOffset = 0;
            mNativeState->attributes[idx].bindingIndex   = static_cast<GLuint>(idx);

            mNativeState->bindings[idx].stride = static_cast<GLsizei>(destStride);
            mNativeState->bindings[idx].offset = static_cast<GLintptr>(vertexStartOffset);
            mArrayBuffers[idx].set(context, nullptr);
            mNativeState->bindings[idx].buffer = mStreamingArrayBuffer;

            // There's maxAttributeDataSize * indexRange.start of empty space allocated for each
            // streaming attributes
            curBufferOffset +=
                destStride * streamedVertexCount + maxAttributeDataSize * indexRange.start;
        }

        unmapResult = ANGLE_GL_TRY(context, functions->unmapBuffer(GL_ARRAY_BUFFER));
    }

    ANGLE_CHECK(GetImplAs<ContextGL>(context), unmapResult == GL_TRUE,
                "Failed to unmap the client data streaming buffer.", GL_OUT_OF_MEMORY);
    return angle::Result::Continue;
}

angle::Result VertexArrayGL::recoverForcedStreamingAttributesForDrawArraysInstanced(
    const gl::Context *context) const
{
    return recoverForcedStreamingAttributesForDrawArraysInstanced(
        context, &mForcedStreamingAttributesForDrawArraysInstancedMask);
}

angle::Result VertexArrayGL::recoverForcedStreamingAttributesForDrawArraysInstanced(
    const gl::Context *context,
    gl::AttributesMask *attributeMask) const
{
    if (attributeMask->none())
    {
        return angle::Result::Continue;
    }

    StateManagerGL *stateManager = GetStateManagerGL(context);

    stateManager->bindVertexArray(mVertexArrayID, mNativeState);

    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();
    for (auto idx : *attributeMask)
    {
        const auto &attrib = attribs[idx];
        ASSERT(IsVertexAttribPointerSupported(idx, attrib));

        const auto &binding = bindings[attrib.bindingIndex];
        const auto buffer   = GetImplAs<BufferGL>(binding.getBuffer().get());
        stateManager->bindBuffer(gl::BufferBinding::Array, buffer->getBufferID());

        ANGLE_TRY(callVertexAttribPointer(context, static_cast<GLuint>(idx), attrib,
                                          static_cast<GLsizei>(binding.getStride()),
                                          static_cast<GLintptr>(binding.getOffset())));

        // Restore the state to track their original buffers
        mNativeState->attributes[idx].format = attrib.format;

        mNativeState->attributes[idx].relativeOffset = 0;
        mNativeState->attributes[idx].bindingIndex   = static_cast<GLuint>(attrib.bindingIndex);

        mNativeState->bindings[idx].stride = binding.getStride();
        mNativeState->bindings[idx].offset = binding.getOffset();
        mArrayBuffers[idx].set(context, binding.getBuffer().get());
        mNativeState->bindings[idx].buffer = buffer->getBufferID();
    }

    attributeMask->reset();
    mForcedStreamingAttributesFirstOffsets.fill(0);

    return angle::Result::Continue;
}

GLuint VertexArrayGL::getVertexArrayID() const
{
    return mVertexArrayID;
}

rx::VertexArrayStateGL *VertexArrayGL::getNativeState() const
{
    return mNativeState;
}

angle::Result VertexArrayGL::updateAttribEnabled(const gl::Context *context, size_t attribIndex)
{
    const bool enabled = mState.getVertexAttribute(attribIndex).enabled &&
                         mProgramActiveAttribLocationsMask.test(attribIndex);
    if (mNativeState->attributes[attribIndex].enabled == enabled)
    {
        return angle::Result::Continue;
    }

    const FunctionsGL *functions = GetFunctionsGL(context);

    if (enabled)
    {
        ANGLE_GL_TRY(context, functions->enableVertexAttribArray(static_cast<GLuint>(attribIndex)));
    }
    else
    {
        ANGLE_GL_TRY(context,
                     functions->disableVertexAttribArray(static_cast<GLuint>(attribIndex)));
    }

    mNativeState->attributes[attribIndex].enabled = enabled;
    return angle::Result::Continue;
}

angle::Result VertexArrayGL::updateAttribPointer(const gl::Context *context, size_t attribIndex)
{
    const angle::FeaturesGL &features = GetFeaturesGL(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);

    const VertexAttribute &attrib = mState.getVertexAttribute(attribIndex);

    // According to SPEC, VertexAttribPointer should update the binding indexed attribIndex instead
    // of the binding indexed attrib.bindingIndex (unless attribIndex == attrib.bindingIndex).
    const VertexBinding &binding = mState.getVertexBinding(attribIndex);

    const bool canUseClientArrays = nativegl::CanUseClientSideArrays(functions, mVertexArrayID);

    // Early return when the vertex attribute isn't using a buffer object:
    // - If we need to stream, defer the attribPointer to the draw call.
    // - Skip the attribute that is disabled and uses a client memory pointer.
    // - Skip the attribute whose buffer is detached by BindVertexBuffer. Since it cannot have a
    //   client memory pointer either, it must be disabled and shouldn't affect the draw.
    const auto &bindingBuffer = binding.getBuffer();
    gl::Buffer *arrayBuffer   = bindingBuffer.get();
    if (arrayBuffer == nullptr && !canUseClientArrays)
    {
        // Mark the applied binding isn't using a buffer by setting its buffer to nullptr so that if
        // it starts to use a buffer later, there is no chance that the caching will skip it.

        mArrayBuffers[attribIndex].set(context, nullptr);
        mNativeState->bindings[attribIndex].buffer = 0;
        return angle::Result::Continue;
    }

    // We do not need to compare attrib.pointer because when we use a different client memory
    // pointer, we don't need to update mAttributesNeedStreaming by binding.buffer and we won't
    // update attribPointer in this function.
    if (SameVertexAttribFormat(mNativeState->attributes[attribIndex], attrib) &&
        (mNativeState->attributes[attribIndex].bindingIndex == attrib.bindingIndex) &&
        SameVertexBuffer(mNativeState->bindings[attribIndex], binding) &&
        (!canUseClientArrays || SameAttribPointer(mNativeState->attributes[attribIndex], attrib)))
    {
        return angle::Result::Continue;
    }

    StateManagerGL *stateManager = GetStateManagerGL(context);
    GLuint bufferId              = 0;
    if (arrayBuffer != nullptr)
    {
        // When ANGLE uses non-zero VAO, we cannot use a client memory pointer on it:
        // [OpenGL ES 3.0.2] Section 2.8 page 24:
        // An INVALID_OPERATION error is generated when a non-zero vertex array object is bound,
        // zero is bound to the ARRAY_BUFFER buffer object binding point, and the pointer argument
        // is not NULL.

        BufferGL *bufferGL = GetImplAs<BufferGL>(arrayBuffer);
        bufferId           = bufferGL->getBufferID();
        stateManager->bindBuffer(gl::BufferBinding::Array, bufferId);
        if (features.ensureNonEmptyBufferIsBoundForDraw.enabled && bufferGL->getBufferSize() == 0)
        {
            constexpr uint32_t data = 0;
            ANGLE_TRY(bufferGL->setData(context, gl::BufferBinding::Array, &data, sizeof(data),
                                        gl::BufferUsage::StaticDraw));
            ASSERT(bufferGL->getBufferSize() > 0);
        }
        ANGLE_TRY(callVertexAttribPointer(context, static_cast<GLuint>(attribIndex), attrib,
                                          binding.getStride(), binding.getOffset()));
    }
    else
    {
        ASSERT(canUseClientArrays);
        stateManager->bindBuffer(gl::BufferBinding::Array, 0);
        ANGLE_TRY(callVertexAttribPointer(context, static_cast<GLuint>(attribIndex), attrib,
                                          binding.getStride(),
                                          reinterpret_cast<GLintptr>(attrib.pointer)));
    }

    mNativeState->attributes[attribIndex].format = attrib.format;

    // After VertexAttribPointer, attrib.relativeOffset is set to 0 and attrib.bindingIndex is set
    // to attribIndex in driver. If attrib.relativeOffset != 0 or attrib.bindingIndex !=
    // attribIndex, they should be set in updateAttribFormat and updateAttribBinding. The cache
    // should be consistent with driver so that we won't miss anything.
    mNativeState->attributes[attribIndex].pointer        = attrib.pointer;
    mNativeState->attributes[attribIndex].relativeOffset = 0;
    mNativeState->attributes[attribIndex].bindingIndex   = static_cast<GLuint>(attribIndex);

    mNativeState->bindings[attribIndex].stride = binding.getStride();
    mNativeState->bindings[attribIndex].offset = binding.getOffset();
    mArrayBuffers[attribIndex].set(context, arrayBuffer);
    mNativeState->bindings[attribIndex].buffer = bufferId;

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::callVertexAttribPointer(const gl::Context *context,
                                                     GLuint attribIndex,
                                                     const VertexAttribute &attrib,
                                                     GLsizei stride,
                                                     GLintptr offset) const
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    const GLvoid *pointer        = reinterpret_cast<const GLvoid *>(offset);
    const angle::Format &format  = *attrib.format;
    if (format.isPureInt())
    {
        ASSERT(!format.isNorm());
        ANGLE_GL_TRY(context, functions->vertexAttribIPointer(attribIndex, format.channelCount,
                                                              gl::ToGLenum(format.vertexAttribType),
                                                              stride, pointer));
    }
    else
    {
        ANGLE_GL_TRY(context, functions->vertexAttribPointer(attribIndex, format.channelCount,
                                                             gl::ToGLenum(format.vertexAttribType),
                                                             format.isNorm(), stride, pointer));
    }

    return angle::Result::Continue;
}

bool VertexArrayGL::supportVertexAttribBinding(const gl::Context *context) const
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    ASSERT(functions);
    // Vertex attrib bindings are not supported on the default VAO so if we're syncing to the
    // default VAO due to the feature, disable bindings.
    return (functions->vertexAttribBinding != nullptr) && mVertexArrayID != 0;
}

angle::Result VertexArrayGL::updateAttribFormat(const gl::Context *context, size_t attribIndex)
{
    ASSERT(supportVertexAttribBinding(context));

    const VertexAttribute &attrib = mState.getVertexAttribute(attribIndex);
    if (SameVertexAttribFormat(mNativeState->attributes[attribIndex], attrib))
    {
        return angle::Result::Continue;
    }

    const FunctionsGL *functions = GetFunctionsGL(context);

    const angle::Format &format = *attrib.format;
    if (format.isPureInt())
    {
        ASSERT(!format.isNorm());
        ANGLE_GL_TRY(context, functions->vertexAttribIFormat(
                                  static_cast<GLuint>(attribIndex), format.channelCount,
                                  gl::ToGLenum(format.vertexAttribType), attrib.relativeOffset));
    }
    else
    {
        ANGLE_GL_TRY(context, functions->vertexAttribFormat(
                                  static_cast<GLuint>(attribIndex), format.channelCount,
                                  gl::ToGLenum(format.vertexAttribType), format.isNorm(),
                                  attrib.relativeOffset));
    }

    mNativeState->attributes[attribIndex].format         = attrib.format;
    mNativeState->attributes[attribIndex].relativeOffset = attrib.relativeOffset;
    return angle::Result::Continue;
}

angle::Result VertexArrayGL::updateAttribBinding(const gl::Context *context, size_t attribIndex)
{
    ASSERT(supportVertexAttribBinding(context));

    GLuint bindingIndex = mState.getVertexAttribute(attribIndex).bindingIndex;
    if (mNativeState->attributes[attribIndex].bindingIndex == bindingIndex)
    {
        return angle::Result::Continue;
    }

    const FunctionsGL *functions = GetFunctionsGL(context);
    ANGLE_GL_TRY(context,
                 functions->vertexAttribBinding(static_cast<GLuint>(attribIndex), bindingIndex));

    mNativeState->attributes[attribIndex].bindingIndex = bindingIndex;

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::updateBindingBuffer(const gl::Context *context, size_t bindingIndex)
{
    ASSERT(supportVertexAttribBinding(context));

    const VertexBinding &binding = mState.getVertexBinding(bindingIndex);
    if (SameVertexBuffer(mNativeState->bindings[bindingIndex], binding))
    {
        return angle::Result::Continue;
    }

    gl::Buffer *arrayBuffer = binding.getBuffer().get();
    GLuint bufferId         = GetNativeBufferID(arrayBuffer);

    const FunctionsGL *functions = GetFunctionsGL(context);
    ANGLE_GL_TRY(context, functions->bindVertexBuffer(static_cast<GLuint>(bindingIndex), bufferId,
                                                      binding.getOffset(), binding.getStride()));

    mNativeState->bindings[bindingIndex].stride = binding.getStride();
    mNativeState->bindings[bindingIndex].offset = binding.getOffset();
    mArrayBuffers[bindingIndex].set(context, arrayBuffer);
    mNativeState->bindings[bindingIndex].buffer = bufferId;

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::updateBindingDivisor(const gl::Context *context, size_t bindingIndex)
{
    GLuint adjustedDivisor =
        GetAdjustedDivisor(mAppliedNumViews, mState.getVertexBinding(bindingIndex).getDivisor());
    if (mNativeState->bindings[bindingIndex].divisor == adjustedDivisor)
    {
        return angle::Result::Continue;
    }

    const FunctionsGL *functions = GetFunctionsGL(context);
    if (supportVertexAttribBinding(context))
    {
        ANGLE_GL_TRY(context, functions->vertexBindingDivisor(static_cast<GLuint>(bindingIndex),
                                                              adjustedDivisor));
    }
    else
    {
        // We can only use VertexAttribDivisor on platforms that don't support Vertex Attrib
        // Binding.
        ANGLE_GL_TRY(context, functions->vertexAttribDivisor(static_cast<GLuint>(bindingIndex),
                                                             adjustedDivisor));
    }

    if (adjustedDivisor > 0)
    {
        mInstancedAttributesMask.set(bindingIndex);
    }
    else if (mInstancedAttributesMask.test(bindingIndex))
    {
        // divisor is reset to 0
        mInstancedAttributesMask.reset(bindingIndex);
    }

    mNativeState->bindings[bindingIndex].divisor = adjustedDivisor;

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::syncDirtyAttrib(
    const gl::Context *context,
    size_t attribIndex,
    const gl::VertexArray::DirtyAttribBits &dirtyAttribBits)
{
    ASSERT(dirtyAttribBits.any());

    for (size_t dirtyBit : dirtyAttribBits)
    {
        switch (dirtyBit)
        {
            case VertexArray::DIRTY_ATTRIB_ENABLED:
                ANGLE_TRY(updateAttribEnabled(context, attribIndex));
                break;

            case VertexArray::DIRTY_ATTRIB_POINTER_BUFFER:
            case VertexArray::DIRTY_ATTRIB_POINTER:
                ANGLE_TRY(updateAttribPointer(context, attribIndex));
                break;

            case VertexArray::DIRTY_ATTRIB_FORMAT:
                ASSERT(supportVertexAttribBinding(context));
                ANGLE_TRY(updateAttribFormat(context, attribIndex));
                break;

            case VertexArray::DIRTY_ATTRIB_BINDING:
                ASSERT(supportVertexAttribBinding(context));
                ANGLE_TRY(updateAttribBinding(context, attribIndex));
                break;

            default:
                UNREACHABLE();
                break;
        }
    }
    return angle::Result::Continue;
}

angle::Result VertexArrayGL::syncDirtyBinding(
    const gl::Context *context,
    size_t bindingIndex,
    const gl::VertexArray::DirtyBindingBits &dirtyBindingBits)
{
    // Dependent state changes in buffers can trigger updates with no dirty bits set.

    for (auto iter = dirtyBindingBits.begin(), endIter = dirtyBindingBits.end(); iter != endIter;
         ++iter)
    {
        size_t dirtyBit = *iter;
        switch (dirtyBit)
        {
            case VertexArray::DIRTY_BINDING_BUFFER:
            case VertexArray::DIRTY_BINDING_STRIDE:
            case VertexArray::DIRTY_BINDING_OFFSET:
                ASSERT(supportVertexAttribBinding(context));
                ANGLE_TRY(updateBindingBuffer(context, bindingIndex));
                // Clear these bits to avoid repeated processing
                iter.resetLaterBits(gl::VertexArray::DirtyBindingBits{
                    VertexArray::DIRTY_BINDING_BUFFER, VertexArray::DIRTY_BINDING_STRIDE,
                    VertexArray::DIRTY_BINDING_OFFSET});
                break;

            case VertexArray::DIRTY_BINDING_DIVISOR:
                ANGLE_TRY(updateBindingDivisor(context, bindingIndex));
                break;

            default:
                UNREACHABLE();
                break;
        }
    }
    return angle::Result::Continue;
}

#define ANGLE_DIRTY_ATTRIB_FUNC(INDEX)                                    \
    case VertexArray::DIRTY_BIT_ATTRIB_0 + INDEX:                         \
        ANGLE_TRY(syncDirtyAttrib(context, INDEX, (*attribBits)[INDEX])); \
        (*attribBits)[INDEX].reset();                                     \
        break;

#define ANGLE_DIRTY_BINDING_FUNC(INDEX)                                     \
    case VertexArray::DIRTY_BIT_BINDING_0 + INDEX:                          \
        ANGLE_TRY(syncDirtyBinding(context, INDEX, (*bindingBits)[INDEX])); \
        (*bindingBits)[INDEX].reset();                                      \
        break;

#define ANGLE_DIRTY_BUFFER_DATA_FUNC(INDEX)            \
    case VertexArray::DIRTY_BIT_BUFFER_DATA_0 + INDEX: \
        break;

angle::Result VertexArrayGL::syncState(const gl::Context *context,
                                       const gl::VertexArray::DirtyBits &dirtyBits,
                                       gl::VertexArray::DirtyAttribBitsArray *attribBits,
                                       gl::VertexArray::DirtyBindingBitsArray *bindingBits)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    stateManager->bindVertexArray(mVertexArrayID, mNativeState);

    for (auto iter = dirtyBits.begin(), endIter = dirtyBits.end(); iter != endIter; ++iter)
    {
        size_t dirtyBit = *iter;
        switch (dirtyBit)
        {
            case gl::VertexArray::DIRTY_BIT_LOST_OBSERVATION:
            {
                // If vertex array was not observing while unbound, we need to check buffer's
                // internal storage and take action if buffer has changed while not observing.
                // For now we just simply assume buffer storage has changed and always dirty all
                // binding points.
                iter.setLaterBits(
                    gl::VertexArray::DirtyBits(mState.getBufferBindingMask().to_ulong()
                                               << gl::VertexArray::DIRTY_BIT_BINDING_0));
                break;
            }

            case VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER:
                ANGLE_TRY(updateElementArrayBufferBinding(context));
                break;

            case VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER_DATA:
                break;

                ANGLE_VERTEX_INDEX_CASES(ANGLE_DIRTY_ATTRIB_FUNC)
                ANGLE_VERTEX_INDEX_CASES(ANGLE_DIRTY_BINDING_FUNC)
                ANGLE_VERTEX_INDEX_CASES(ANGLE_DIRTY_BUFFER_DATA_FUNC)

            default:
                UNREACHABLE();
                break;
        }
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::applyNumViewsToDivisor(const gl::Context *context, int numViews)
{
    if (numViews != mAppliedNumViews)
    {
        StateManagerGL *stateManager = GetStateManagerGL(context);
        stateManager->bindVertexArray(mVertexArrayID, mNativeState);
        mAppliedNumViews = numViews;
        for (size_t index = 0u; index < mNativeState->bindings.size(); ++index)
        {
            ANGLE_TRY(updateBindingDivisor(context, index));
        }
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::applyActiveAttribLocationsMask(const gl::Context *context,
                                                            const gl::AttributesMask &activeMask)
{
    gl::AttributesMask updateMask = mProgramActiveAttribLocationsMask ^ activeMask;
    if (!updateMask.any())
    {
        return angle::Result::Continue;
    }

    ASSERT(mVertexArrayID == GetStateManagerGL(context)->getVertexArrayID());
    mProgramActiveAttribLocationsMask = activeMask;

    for (size_t attribIndex : updateMask)
    {
        ANGLE_TRY(updateAttribEnabled(context, attribIndex));
    }

    return angle::Result::Continue;
}

angle::Result VertexArrayGL::validateState(const gl::Context *context) const
{
    const FunctionsGL *functions = GetFunctionsGL(context);

    // Ensure this vao is currently bound
    ANGLE_TRY(ValidateStateHelperGetIntegerv(context, mVertexArrayID, GL_VERTEX_ARRAY_BINDING,
                                             "mVertexArrayID", "GL_VERTEX_ARRAY_BINDING"));

    // Element array buffer
    ANGLE_TRY(ValidateStateHelperGetIntegerv(
        context, mNativeState->elementArrayBuffer, GL_ELEMENT_ARRAY_BUFFER_BINDING,
        "mNativeState->elementArrayBuffer", "GL_ELEMENT_ARRAY_BUFFER_BINDING"));

    // ValidateStateHelperGetIntegerv but with > comparison instead of !=
    GLint queryValue;
    ANGLE_GL_TRY(context, functions->getIntegerv(GL_MAX_VERTEX_ATTRIBS, &queryValue));
    if (mNativeState->attributes.size() > static_cast<GLuint>(queryValue))
    {
        WARN() << "mNativeState->attributes.size() (" << mNativeState->attributes.size()
               << ") > GL_MAX_VERTEX_ATTRIBS (" << queryValue << ")";
        // Re-add ASSERT: http://anglebug.com/42262547
        // ASSERT(false);
    }

    // Check each applied attribute/binding
    for (GLuint index = 0; index < mNativeState->attributes.size(); index++)
    {
        VertexAttributeGL &attribute = mNativeState->attributes[index];
        ASSERT(attribute.bindingIndex < mNativeState->bindings.size());
        VertexBindingGL &binding = mNativeState->bindings[attribute.bindingIndex];

        ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
            context, index, attribute.enabled, GL_VERTEX_ATTRIB_ARRAY_ENABLED,
            "mNativeState->attributes.enabled", "GL_VERTEX_ATTRIB_ARRAY_ENABLED"));

        if (attribute.enabled)
        {
            // Applied attributes
            ASSERT(attribute.format);
            ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                context, index, ToGLenum(attribute.format->vertexAttribType),
                GL_VERTEX_ATTRIB_ARRAY_TYPE, "mNativeState->attributes.format->vertexAttribType",
                "GL_VERTEX_ATTRIB_ARRAY_TYPE"));
            ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                context, index, attribute.format->channelCount, GL_VERTEX_ATTRIB_ARRAY_SIZE,
                "attribute.format->channelCount", "GL_VERTEX_ATTRIB_ARRAY_SIZE"));
            ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                context, index, attribute.format->isNorm(), GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,
                "attribute.format->isNorm()", "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED"));
            ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                context, index, attribute.format->isPureInt(), GL_VERTEX_ATTRIB_ARRAY_INTEGER,
                "attribute.format->isPureInt()", "GL_VERTEX_ATTRIB_ARRAY_INTEGER"));
            if (supportVertexAttribBinding(context))
            {
                ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                    context, index, attribute.relativeOffset, GL_VERTEX_ATTRIB_RELATIVE_OFFSET,
                    "attribute.relativeOffset", "GL_VERTEX_ATTRIB_RELATIVE_OFFSET"));
                ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                    context, index, attribute.bindingIndex, GL_VERTEX_ATTRIB_BINDING,
                    "attribute.bindingIndex", "GL_VERTEX_ATTRIB_BINDING"));
            }

            // Applied bindings
            ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                context, index, binding.buffer, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,
                "binding.buffer", "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING"));
            if (binding.buffer != 0)
            {
                ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                    context, index, binding.stride, GL_VERTEX_ATTRIB_ARRAY_STRIDE, "binding.stride",
                    "GL_VERTEX_ATTRIB_ARRAY_STRIDE"));
                ANGLE_TRY(ValidateStateHelperGetVertexAttribiv(
                    context, index, binding.divisor, GL_VERTEX_ATTRIB_ARRAY_DIVISOR,
                    "binding.divisor", "GL_VERTEX_ATTRIB_ARRAY_DIVISOR"));
            }
        }
    }
    return angle::Result::Continue;
}

}  // namespace rx
