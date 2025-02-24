//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackVk.cpp:
//    Implements the class methods for TransformFeedbackVk.
//

#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"

#include "libANGLE/Context.h"
#include "libANGLE/Query.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/QueryVk.h"
#include "libANGLE/renderer/vulkan/ShaderInterfaceVariableInfoMap.h"

#include "common/debug.h"

namespace rx
{
TransformFeedbackVk::TransformFeedbackVk(const gl::TransformFeedbackState &state)
    : TransformFeedbackImpl(state),
      mRebindTransformFeedbackBuffer(false),
      mBufferHelpers{},
      mBufferHandles{},
      mBufferOffsets{},
      mBufferSizes{},
      mCounterBufferHandles{},
      mCounterBufferOffsets{}
{
    for (angle::SubjectIndex bufferIndex = 0;
         bufferIndex < gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS; ++bufferIndex)
    {
        mBufferObserverBindings.emplace_back(this, bufferIndex);
    }
}

TransformFeedbackVk::~TransformFeedbackVk() {}

void TransformFeedbackVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    releaseCounterBuffers(contextVk);
}

void TransformFeedbackVk::releaseCounterBuffers(vk::Context *context)
{
    for (vk::BufferHelper &bufferHelper : mCounterBufferHelpers)
    {
        bufferHelper.release(context);
    }
    for (VkBuffer &buffer : mCounterBufferHandles)
    {
        buffer = VK_NULL_HANDLE;
    }
    for (VkDeviceSize &offset : mCounterBufferOffsets)
    {
        offset = 0;
    }
}

void TransformFeedbackVk::initializeXFBVariables(ContextVk *contextVk, uint32_t xfbBufferCount)
{
    for (uint32_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &binding = mState.getIndexedBuffer(bufferIndex);
        ASSERT(binding.get());

        BufferVk *bufferVk = vk::GetImpl(binding.get());

        if (bufferVk->isBufferValid())
        {
            mBufferHelpers[bufferIndex] = &bufferVk->getBuffer();
            mBufferOffsets[bufferIndex] =
                binding.getOffset() + mBufferHelpers[bufferIndex]->getOffset();
            mBufferSizes[bufferIndex] = gl::GetBoundBufferAvailableSize(binding);
            mBufferObserverBindings[bufferIndex].bind(bufferVk);
        }
        else
        {
            // This can happen in error conditions.
            vk::BufferHelper &nullBuffer = contextVk->getEmptyBuffer();
            mBufferHelpers[bufferIndex]  = &nullBuffer;
            mBufferOffsets[bufferIndex]  = 0;
            mBufferSizes[bufferIndex]    = nullBuffer.getSize();
            mBufferObserverBindings[bufferIndex].reset();
        }
    }
}

angle::Result TransformFeedbackVk::begin(const gl::Context *context,
                                         gl::PrimitiveMode primitiveMode)
{
    ContextVk *contextVk = vk::GetImpl(context);

    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

    initializeXFBVariables(contextVk, static_cast<uint32_t>(xfbBufferCount));

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        mBufferHandles[bufferIndex] = mBufferHelpers[bufferIndex]->getBuffer().getHandle();
        if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
        {
            if (mCounterBufferHandles[bufferIndex] == VK_NULL_HANDLE)
            {
                vk::BufferHelper &bufferHelper = mCounterBufferHelpers[bufferIndex];
                ANGLE_TRY(contextVk->initBufferAllocation(
                    &bufferHelper, contextVk->getRenderer()->getDeviceLocalMemoryTypeIndex(), 16,
                    contextVk->getRenderer()->getDefaultBufferAlignment(),
                    BufferUsageType::Static));
                mCounterBufferHandles[bufferIndex] = bufferHelper.getBuffer().getHandle();
                mCounterBufferOffsets[bufferIndex] = bufferHelper.getOffset();
            }
        }
    }

    if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        mRebindTransformFeedbackBuffer = true;
    }

    return contextVk->onBeginTransformFeedback(xfbBufferCount, mBufferHelpers,
                                               mCounterBufferHelpers);
}

angle::Result TransformFeedbackVk::end(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // If there's an active transform feedback query, accumulate the primitives drawn.
    const gl::State &glState = context->getState();
    gl::Query *transformFeedbackQuery =
        glState.getActiveQuery(gl::QueryType::TransformFeedbackPrimitivesWritten);

    if (transformFeedbackQuery && contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        vk::GetImpl(transformFeedbackQuery)->onTransformFeedbackEnd(mState.getPrimitivesDrawn());
    }

    for (angle::ObserverBinding &bufferBinding : mBufferObserverBindings)
    {
        bufferBinding.reset();
    }

    contextVk->onEndTransformFeedback();

    releaseCounterBuffers(contextVk);

    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::pause(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    return contextVk->onPauseTransformFeedback();
}

angle::Result TransformFeedbackVk::resume(const gl::Context *context)
{
    ContextVk *contextVk                    = vk::GetImpl(context);
    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

    if (contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        initializeXFBVariables(contextVk, static_cast<uint32_t>(xfbBufferCount));
    }

    return contextVk->onBeginTransformFeedback(xfbBufferCount, mBufferHelpers,
                                               mCounterBufferHelpers);
}

angle::Result TransformFeedbackVk::bindIndexedBuffer(
    const gl::Context *context,
    size_t index,
    const gl::OffsetBindingPointer<gl::Buffer> &binding)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Make sure the transform feedback buffers are bound to the program descriptor sets.
    contextVk->invalidateCurrentTransformFeedbackBuffers();

    return angle::Result::Continue;
}

void TransformFeedbackVk::getBufferOffsets(ContextVk *contextVk,
                                           GLint drawCallFirstVertex,
                                           int32_t *offsetsOut,
                                           size_t offsetsSize) const
{
    if (!contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        return;
    }

    GLsizeiptr verticesDrawn                = mState.getVerticesDrawn();
    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    const std::vector<GLsizei> &bufferStrides = executable->getTransformFeedbackStrides();
    size_t xfbBufferCount                     = executable->getTransformFeedbackBufferCount();
    const VkDeviceSize offsetAlignment        = contextVk->getRenderer()
                                             ->getPhysicalDeviceProperties()
                                             .limits.minStorageBufferOffsetAlignment;

    ASSERT(xfbBufferCount > 0);

    // The caller should make sure the offsets array has enough space.  The maximum possible
    // number of outputs is gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS.
    ASSERT(offsetsSize >= xfbBufferCount);

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        int64_t offsetFromDescriptor =
            static_cast<int64_t>(mBufferOffsets[bufferIndex] % offsetAlignment);
        int64_t drawCallVertexOffset = static_cast<int64_t>(verticesDrawn) - drawCallFirstVertex;

        int64_t writeOffset =
            (offsetFromDescriptor + drawCallVertexOffset * bufferStrides[bufferIndex]) /
            static_cast<int64_t>(sizeof(uint32_t));

        offsetsOut[bufferIndex] = static_cast<int32_t>(writeOffset);

        // Assert on overflow.  For now, support transform feedback up to 2GB.
        ASSERT(offsetsOut[bufferIndex] == writeOffset);
    }
}

void TransformFeedbackVk::onSubjectStateChange(angle::SubjectIndex index,
                                               angle::SubjectMessage message)
{
    if (message == angle::SubjectMessage::InternalMemoryAllocationChanged)
    {
        ASSERT(index < mBufferObserverBindings.size());
        const gl::OffsetBindingPointer<gl::Buffer> &binding = mState.getIndexedBuffer(index);
        ASSERT(binding.get());

        BufferVk *bufferVk = vk::GetImpl(binding.get());

        ASSERT(bufferVk->isBufferValid());
        mBufferHelpers[index] = &bufferVk->getBuffer();
        mBufferOffsets[index] = binding.getOffset() + mBufferHelpers[index]->getOffset();
        mBufferSizes[index]   = std::min<VkDeviceSize>(gl::GetBoundBufferAvailableSize(binding),
                                                       mBufferHelpers[index]->getSize());
        mBufferObserverBindings[index].bind(bufferVk);
        mBufferHandles[index] = mBufferHelpers[index]->getBuffer().getHandle();
    }
}

void TransformFeedbackVk::updateTransformFeedbackDescriptorDesc(
    const vk::Context *context,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const vk::WriteDescriptorDescs &writeDescriptorDescs,
    const vk::BufferHelper &emptyBuffer,
    bool activeUnpaused,
    vk::DescriptorSetDescBuilder *builder) const
{
    size_t xfbBufferCount = executable.getTransformFeedbackBufferCount();
    for (uint32_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        if (mBufferHelpers[bufferIndex] && activeUnpaused)
        {
            builder->updateTransformFeedbackBuffer(context, variableInfoMap, writeDescriptorDescs,
                                                   bufferIndex, *mBufferHelpers[bufferIndex],
                                                   mBufferOffsets[bufferIndex],
                                                   mBufferSizes[bufferIndex]);
        }
        else
        {
            builder->updateTransformFeedbackBuffer(context, variableInfoMap, writeDescriptorDescs,
                                                   bufferIndex, emptyBuffer, 0,
                                                   emptyBuffer.getSize());
        }
    }
}

void TransformFeedbackVk::onNewDescriptorSet(const gl::ProgramExecutable &executable,
                                             const vk::SharedDescriptorSetCacheKey &sharedCacheKey)
{
    size_t xfbBufferCount = executable.getTransformFeedbackBufferCount();
    for (uint32_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        if (mBufferHelpers[bufferIndex])
        {
            mBufferHelpers[bufferIndex]->onNewDescriptorSet(sharedCacheKey);
        }
    }
}
}  // namespace rx
