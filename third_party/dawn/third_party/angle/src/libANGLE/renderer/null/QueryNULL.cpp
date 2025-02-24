//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryNULL.cpp:
//    Implements the class methods for QueryNULL.
//

#include "libANGLE/renderer/null/QueryNULL.h"

#include "common/debug.h"

namespace rx
{

QueryNULL::QueryNULL(gl::QueryType type) : QueryImpl(type) {}

QueryNULL::~QueryNULL() {}

angle::Result QueryNULL::begin(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result QueryNULL::end(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result QueryNULL::queryCounter(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result QueryNULL::getResult(const gl::Context *context, GLint *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryNULL::getResult(const gl::Context *context, GLuint *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryNULL::getResult(const gl::Context *context, GLint64 *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryNULL::getResult(const gl::Context *context, GLuint64 *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryNULL::isResultAvailable(const gl::Context *context, bool *available)
{
    *available = true;
    return angle::Result::Continue;
}

}  // namespace rx
