//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramNULL.h:
//    Defines the class interface for ProgramNULL, implementing ProgramImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_PROGRAMNULL_H_
#define LIBANGLE_RENDERER_NULL_PROGRAMNULL_H_

#include "libANGLE/renderer/ProgramImpl.h"

namespace rx
{

class ProgramNULL : public ProgramImpl
{
  public:
    ProgramNULL(const gl::ProgramState &state);
    ~ProgramNULL() override;

    angle::Result load(const gl::Context *context,
                       gl::BinaryInputStream *stream,
                       std::shared_ptr<LinkTask> *loadTaskOut,
                       egl::CacheGetResult *resultOut) override;
    void save(const gl::Context *context, gl::BinaryOutputStream *stream) override;
    void setBinaryRetrievableHint(bool retrievable) override;
    void setSeparable(bool separable) override;

    angle::Result link(const gl::Context *context, std::shared_ptr<LinkTask> *linkTaskOut) override;
    GLboolean validate(const gl::Caps &caps) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_PROGRAMNULL_H_
