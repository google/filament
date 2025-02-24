//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayImpl.h: Implementation methods of egl::Display

#ifndef LIBANGLE_RENDERER_SHAREGROUPIMPL_H_
#define LIBANGLE_RENDERER_SHAREGROUPIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/ShareGroup.h"

namespace egl
{
class Display;
}  // namespace egl

namespace gl
{
class Context;
}  // namespace gl

namespace rx
{
class ShareGroupImpl : angle::NonCopyable
{
  public:
    ShareGroupImpl(const egl::ShareGroupState &state) : mState(state) {}
    virtual ~ShareGroupImpl() {}
    virtual void onDestroy(const egl::Display *display) {}

    virtual void onContextAdd() {}

  protected:
    const egl::ShareGroupState &mState;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_SHAREGROUPIMPL_H_
