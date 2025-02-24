//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/wgl/RendererWGL.h"

#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"

namespace rx
{

RendererWGL::RendererWGL(std::unique_ptr<FunctionsGL> functionsGL,
                         const egl::AttributeMap &attribMap,
                         DisplayWGL *display,
                         HGLRC context)
    : RendererGL(std::move(functionsGL), attribMap, display), mDisplay(display), mContext(context)
{}

RendererWGL::~RendererWGL()
{
    mDisplay->destroyNativeContext(mContext);
    mContext = nullptr;
}

HGLRC RendererWGL::getContext() const
{
    return mContext;
}

}  // namespace rx
