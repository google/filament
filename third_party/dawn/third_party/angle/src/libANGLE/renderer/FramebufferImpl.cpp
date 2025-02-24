//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferImpl.cpp: Implements the class methods for FramebufferImpl.

#include "libANGLE/renderer/FramebufferImpl.h"

namespace rx
{

namespace
{
angle::Result InitAttachment(const gl::Context *context,
                             const gl::FramebufferAttachment *attachment)
{
    ASSERT(attachment->isAttached());
    if (attachment->initState() == gl::InitState::MayNeedInit)
    {
        ANGLE_TRY(attachment->initializeContents(context));
    }
    return angle::Result::Continue;
}
}  // namespace

angle::Result FramebufferImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result FramebufferImpl::ensureAttachmentsInitialized(
    const gl::Context *context,
    const gl::DrawBufferMask &colorAttachments,
    bool depth,
    bool stencil)
{
    // Default implementation iterates over the attachments and individually initializes them

    for (auto colorIndex : colorAttachments)
    {
        ANGLE_TRY(InitAttachment(context, mState.getColorAttachment(colorIndex)));
    }

    if (depth)
    {
        ANGLE_TRY(InitAttachment(context, mState.getDepthAttachment()));
    }

    if (stencil)
    {
        ANGLE_TRY(InitAttachment(context, mState.getStencilAttachment()));
    }

    return angle::Result::Continue;
}

}  // namespace rx
