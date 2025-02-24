//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FenceNVVk.h:
//    Defines the class interface for FenceNVVk, implementing FenceNVImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_FENCENVVK_H_
#define LIBANGLE_RENDERER_VULKAN_FENCENVVK_H_

#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/vulkan/SyncVk.h"

namespace rx
{
class FenceNVVk : public FenceNVImpl
{
  public:
    FenceNVVk();
    ~FenceNVVk() override;

    void onDestroy(const gl::Context *context) override;
    angle::Result set(const gl::Context *context, GLenum condition) override;
    angle::Result test(const gl::Context *context, GLboolean *outFinished) override;
    angle::Result finish(const gl::Context *context) override;

  private:
    vk::SyncHelper mFenceSync;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_FENCENVVK_H_
