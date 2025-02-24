//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackVk.h:
//    Defines the class interface for TransformFeedbackVk, implementing TransformFeedbackImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_TRANSFORMFEEDBACKVK_H_
#define LIBANGLE_RENDERER_VULKAN_TRANSFORMFEEDBACKVK_H_

#include "libANGLE/renderer/TransformFeedbackImpl.h"

#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace gl
{
class ProgramState;
}  // namespace gl

namespace rx
{
class UpdateDescriptorSetsBuilder;
class ShaderInterfaceVariableInfoMap;

namespace vk
{
class DescriptorSetLayoutDesc;
}  // namespace vk

class TransformFeedbackVk : public TransformFeedbackImpl, public angle::ObserverInterface
{
  public:
    TransformFeedbackVk(const gl::TransformFeedbackState &state);
    ~TransformFeedbackVk() override;
    void onDestroy(const gl::Context *context) override;

    angle::Result begin(const gl::Context *context, gl::PrimitiveMode primitiveMode) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result pause(const gl::Context *context) override;
    angle::Result resume(const gl::Context *context) override;

    angle::Result bindIndexedBuffer(const gl::Context *context,
                                    size_t index,
                                    const gl::OffsetBindingPointer<gl::Buffer> &binding) override;

    void getBufferOffsets(ContextVk *contextVk,
                          GLint drawCallFirstVertex,
                          int32_t *offsetsOut,
                          size_t offsetsSize) const;

    bool getAndResetBufferRebindState()
    {
        bool retVal                    = mRebindTransformFeedbackBuffer;
        mRebindTransformFeedbackBuffer = false;
        return retVal;
    }

    const gl::TransformFeedbackBuffersArray<vk::BufferHelper *> &getBufferHelpers() const
    {
        return mBufferHelpers;
    }

    const gl::TransformFeedbackBuffersArray<VkBuffer> &getBufferHandles() const
    {
        return mBufferHandles;
    }

    const gl::TransformFeedbackBuffersArray<VkDeviceSize> &getBufferOffsets() const
    {
        return mBufferOffsets;
    }

    const gl::TransformFeedbackBuffersArray<VkDeviceSize> &getBufferSizes() const
    {
        return mBufferSizes;
    }

    gl::TransformFeedbackBuffersArray<vk::BufferHelper> &getCounterBufferHelpers()
    {
        return mCounterBufferHelpers;
    }

    const gl::TransformFeedbackBuffersArray<VkBuffer> &getCounterBufferHandles() const
    {
        return mCounterBufferHandles;
    }

    void updateTransformFeedbackDescriptorDesc(
        const vk::Context *context,
        const gl::ProgramExecutable &executable,
        const ShaderInterfaceVariableInfoMap &variableInfoMap,
        const vk::WriteDescriptorDescs &writeDescriptorDescs,
        const vk::BufferHelper &emptyBuffer,
        bool activeUnpaused,
        vk::DescriptorSetDescBuilder *builder) const;

    const gl::TransformFeedbackBuffersArray<VkDeviceSize> &getCounterBufferOffsets() const
    {
        return mCounterBufferOffsets;
    }

    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    void onNewDescriptorSet(const gl::ProgramExecutable &executable,
                            const vk::SharedDescriptorSetCacheKey &sharedCacheKey);

  private:
    void initializeXFBVariables(ContextVk *contextVk, uint32_t xfbBufferCount);

    void releaseCounterBuffers(vk::Context *context);

    // This member variable is set when glBindTransformFeedbackBuffers/glBeginTransformFeedback
    // is called and unset in dirty bit handler for transform feedback state change. If this
    // value is true, vertex shader will record transform feedback varyings from the beginning
    // of the buffer.
    bool mRebindTransformFeedbackBuffer;

    gl::TransformFeedbackBuffersArray<vk::BufferHelper *> mBufferHelpers;
    gl::TransformFeedbackBuffersArray<VkBuffer> mBufferHandles;
    gl::TransformFeedbackBuffersArray<VkDeviceSize> mBufferOffsets;
    gl::TransformFeedbackBuffersArray<VkDeviceSize> mBufferSizes;

    // Counter buffer used for pause and resume.
    gl::TransformFeedbackBuffersArray<vk::BufferHelper> mCounterBufferHelpers;
    gl::TransformFeedbackBuffersArray<VkBuffer> mCounterBufferHandles;
    gl::TransformFeedbackBuffersArray<VkDeviceSize> mCounterBufferOffsets;

    // Buffer binding points
    std::vector<angle::ObserverBinding> mBufferObserverBindings;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_TRANSFORMFEEDBACKVK_H_
