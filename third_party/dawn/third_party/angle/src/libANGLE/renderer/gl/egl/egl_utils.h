//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// egl_utils.h: Utility routines specific to the EGL->EGL implementation.

#ifndef LIBANGLE_RENDERER_GL_EGL_EGLUTILS_H_
#define LIBANGLE_RENDERER_GL_EGL_EGLUTILS_H_

#include <vector>

#include "common/platform.h"
#include "libANGLE/AttributeMap.h"

namespace rx
{

namespace native_egl
{

using AttributeVector = std::vector<EGLint>;

// Filter the attribute map and return a vector of attributes that can be passed to the native
// driver.  Does NOT append EGL_NONE to the vector.
AttributeVector TrimAttributeMap(const egl::AttributeMap &attributes,
                                 const EGLint *forwardAttribs,
                                 size_t forwardAttribsCount);

template <size_t N>
AttributeVector TrimAttributeMap(const egl::AttributeMap &attributes,
                                 const EGLint (&forwardAttribs)[N])
{
    return TrimAttributeMap(attributes, forwardAttribs, N);
}

// Append EGL_NONE to the attribute vector so that it can be passed to a native driver.
void FinalizeAttributeVector(AttributeVector *attributeVector);

}  // namespace native_egl

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_EGLUTILS_H_
