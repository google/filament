//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncNULL.h:
//    Defines the class interface for SyncNULL, implementing SyncImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_FENCESYNCNULL_H_
#define LIBANGLE_RENDERER_NULL_FENCESYNCNULL_H_

#include "libANGLE/renderer/SyncImpl.h"

namespace rx
{
class SyncNULL : public SyncImpl
{
  public:
    SyncNULL();
    ~SyncNULL() override;

    angle::Result set(const gl::Context *context, GLenum condition, GLbitfield flags) override;
    angle::Result clientWait(const gl::Context *context,
                             GLbitfield flags,
                             GLuint64 timeout,
                             GLenum *outResult) override;
    angle::Result serverWait(const gl::Context *context,
                             GLbitfield flags,
                             GLuint64 timeout) override;
    angle::Result getStatus(const gl::Context *context, GLint *outResult) override;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_FENCESYNCNULL_H_
