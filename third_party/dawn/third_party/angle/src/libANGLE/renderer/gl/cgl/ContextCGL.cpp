//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextCGL:
//   Mac-specific subclass of ContextGL.
//

#include "libANGLE/renderer/gl/cgl/ContextCGL.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/cgl/DisplayCGL.h"

namespace rx
{

ContextCGL::ContextCGL(DisplayCGL *display,
                       const gl::State &state,
                       gl::ErrorSet *errorSet,
                       const std::shared_ptr<RendererGL> &renderer,
                       bool usesDiscreteGPU)
    : ContextGL(state, errorSet, renderer, RobustnessVideoMemoryPurgeStatus::NOT_REQUESTED),
      mUsesDiscreteGpu(usesDiscreteGPU),
      mReleasedDiscreteGpu(false)
{
    if (mUsesDiscreteGpu)
    {
        (void)display->referenceDiscreteGPU();
    }
}

egl::Error ContextCGL::releaseHighPowerGPU(gl::Context *context)
{
    if (mUsesDiscreteGpu && !mReleasedDiscreteGpu)
    {
        mReleasedDiscreteGpu = true;
        return GetImplAs<DisplayCGL>(context->getDisplay())->unreferenceDiscreteGPU();
    }

    return egl::NoError();
}

egl::Error ContextCGL::reacquireHighPowerGPU(gl::Context *context)
{
    if (mUsesDiscreteGpu && mReleasedDiscreteGpu)
    {
        mReleasedDiscreteGpu = false;
        return GetImplAs<DisplayCGL>(context->getDisplay())->referenceDiscreteGPU();
    }

    return egl::NoError();
}

void ContextCGL::onDestroy(const gl::Context *context)
{
    if (mUsesDiscreteGpu && !mReleasedDiscreteGpu)
    {
        (void)GetImplAs<DisplayCGL>(context->getDisplay())->unreferenceDiscreteGPU();
    }
    ContextGL::onDestroy(context);
}

}  // namespace rx
