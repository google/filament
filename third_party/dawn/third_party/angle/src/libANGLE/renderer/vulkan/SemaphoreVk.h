// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SemaphoreVk.h: Defines the class interface for SemaphoreVk,
// implementing SemaphoreImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_SEMAPHOREVK_H_
#define LIBANGLE_RENDERER_VULKAN_SEMAPHOREVK_H_

#include "libANGLE/renderer/SemaphoreImpl.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{

class SemaphoreVk : public SemaphoreImpl
{
  public:
    SemaphoreVk();
    ~SemaphoreVk() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result importFd(gl::Context *context, gl::HandleType handleType, GLint fd) override;

    angle::Result importZirconHandle(gl::Context *context,
                                     gl::HandleType handleType,
                                     GLuint handle) override;

    angle::Result wait(gl::Context *context,
                       const gl::BufferBarrierVector &bufferBarriers,
                       const gl::TextureBarrierVector &textureBarriers) override;

    angle::Result signal(gl::Context *context,
                         const gl::BufferBarrierVector &bufferBarriers,
                         const gl::TextureBarrierVector &textureBarriers) override;

  private:
    angle::Result importOpaqueFd(ContextVk *contextVk, GLint fd);
    angle::Result importZirconEvent(ContextVk *contextVk, GLuint handle);

    vk::Semaphore mSemaphore;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SEMAPHOREVK_H_
