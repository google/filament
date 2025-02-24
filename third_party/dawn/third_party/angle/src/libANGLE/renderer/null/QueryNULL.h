//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryNULL.h:
//    Defines the class interface for QueryNULL, implementing QueryImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_QUERYNULL_H_
#define LIBANGLE_RENDERER_NULL_QUERYNULL_H_

#include "libANGLE/renderer/QueryImpl.h"

namespace rx
{

class QueryNULL : public QueryImpl
{
  public:
    QueryNULL(gl::QueryType type);
    ~QueryNULL() override;

    angle::Result begin(const gl::Context *context) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result queryCounter(const gl::Context *context) override;
    angle::Result getResult(const gl::Context *context, GLint *params) override;
    angle::Result getResult(const gl::Context *context, GLuint *params) override;
    angle::Result getResult(const gl::Context *context, GLint64 *params) override;
    angle::Result getResult(const gl::Context *context, GLuint64 *params) override;
    angle::Result isResultAvailable(const gl::Context *context, bool *available) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_QUERYNULL_H_
