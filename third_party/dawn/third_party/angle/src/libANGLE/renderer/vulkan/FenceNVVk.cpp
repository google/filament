//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FenceNVVk.cpp:
//    Implements the class methods for FenceNVVk.
//

#include "libANGLE/renderer/vulkan/FenceNVVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

FenceNVVk::FenceNVVk() : FenceNVImpl() {}

FenceNVVk::~FenceNVVk() {}

void FenceNVVk::onDestroy(const gl::Context *context)
{
    mFenceSync.releaseToRenderer(vk::GetImpl(context)->getRenderer());
}

angle::Result FenceNVVk::set(const gl::Context *context, GLenum condition)
{
    ASSERT(condition == GL_ALL_COMPLETED_NV);
    return mFenceSync.initialize(vk::GetImpl(context), SyncFenceScope::CurrentContextToShareGroup);
}

angle::Result FenceNVVk::test(const gl::Context *context, GLboolean *outFinished)
{
    ContextVk *contextVk = vk::GetImpl(context);
    bool signaled        = false;
    ANGLE_TRY(mFenceSync.getStatus(contextVk, contextVk, &signaled));

    ASSERT(outFinished);
    *outFinished = signaled ? GL_TRUE : GL_FALSE;
    return angle::Result::Continue;
}

angle::Result FenceNVVk::finish(const gl::Context *context)
{
    return mFenceSync.finish(vk::GetImpl(context));
}

}  // namespace rx
