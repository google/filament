//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// egl_utils.cpp: Utility routines specific to the EGL->EGL implementation.

#include "libANGLE/renderer/gl/egl/egl_utils.h"

#include "common/debug.h"

namespace rx
{

namespace native_egl
{

AttributeVector TrimAttributeMap(const egl::AttributeMap &attributes,
                                 const EGLint *forwardAttribs,
                                 size_t forwardAttribsCount)
{
    AttributeVector result;
    for (size_t forwardAttribIndex = 0; forwardAttribIndex < forwardAttribsCount;
         forwardAttribIndex++)
    {
        EGLint forwardAttrib = forwardAttribs[forwardAttribIndex];
        if (attributes.contains(forwardAttrib))
        {
            result.push_back(forwardAttrib);
            result.push_back(static_cast<int>(attributes.get(forwardAttrib)));
        }
    }
    return result;
}

void FinalizeAttributeVector(AttributeVector *attributeVector)
{
    ASSERT(attributeVector);
    attributeVector->push_back(EGL_NONE);
}

}  // namespace native_egl

}  // namespace rx
