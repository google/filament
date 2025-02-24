//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncNULL.cpp:
//    Implements the class methods for SyncNULL.
//

#include "libANGLE/renderer/null/SyncNULL.h"

#include "common/debug.h"

namespace rx
{

SyncNULL::SyncNULL() : SyncImpl() {}

SyncNULL::~SyncNULL() {}

angle::Result SyncNULL::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    return angle::Result::Continue;
}

angle::Result SyncNULL::clientWait(const gl::Context *context,
                                   GLbitfield flags,
                                   GLuint64 timeout,
                                   GLenum *outResult)
{
    *outResult = GL_ALREADY_SIGNALED;
    return angle::Result::Continue;
}

angle::Result SyncNULL::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    return angle::Result::Continue;
}

angle::Result SyncNULL::getStatus(const gl::Context *context, GLint *outResult)
{
    *outResult = GL_SIGNALED;
    return angle::Result::Continue;
}

}  // namespace rx
