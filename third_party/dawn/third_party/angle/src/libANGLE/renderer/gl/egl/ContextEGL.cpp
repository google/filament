//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/egl/ContextEGL.h"

#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

ContextEGL::ContextEGL(const gl::State &state,
                       gl::ErrorSet *errorSet,
                       const std::shared_ptr<RendererEGL> &renderer,
                       RobustnessVideoMemoryPurgeStatus robustnessVideoMemoryPurgeStatus)
    : ContextGL(state, errorSet, renderer, robustnessVideoMemoryPurgeStatus), mRendererEGL(renderer)
{}

ContextEGL::~ContextEGL() {}

void ContextEGL::acquireExternalContext(const gl::Context *context)
{
    ASSERT(context->isExternal());

    if (!mExtState)
    {
        mExtState        = std::make_unique<ExternalContextState>();
        const auto &caps = getCaps();
        mExtState->textureBindings.resize(static_cast<size_t>(caps.maxCombinedTextureImageUnits));
    }

    getStateManager()->syncFromNativeContext(getNativeExtensions(), mExtState.get());

    // Use current FBO as the default framebuffer when the external context is current.
    // First save the current ID of the default framebuffer to restore in
    // onUnMakeCurrent().
    gl::Framebuffer *framebuffer = mState.getDefaultFramebuffer();
    auto framebufferGL           = GetImplAs<FramebufferGL>(framebuffer);
    mPrevDefaultFramebufferID    = framebufferGL->getFramebufferID();
    framebufferGL->updateDefaultFramebufferID(mExtState->framebufferBinding);
}

void ContextEGL::releaseExternalContext(const gl::Context *context)
{
    ASSERT(context->isExternal());
    ASSERT(mExtState);

    getStateManager()->restoreNativeContext(getNativeExtensions(), mExtState.get());

    // If the default framebuffer exists, update its ID (note that there can
    // be multiple consecutive onUnMakeCurrent() calls in destruction, and
    // the default FBO will have been unset by the first one).
    gl::Framebuffer *framebuffer = mState.getDefaultFramebuffer();
    if (framebuffer)
    {
        auto framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
        framebufferGL->updateDefaultFramebufferID(mPrevDefaultFramebufferID);
    }
}

EGLContext ContextEGL::getContext() const
{
    return mRendererEGL->getContext();
}

}  // namespace rx
