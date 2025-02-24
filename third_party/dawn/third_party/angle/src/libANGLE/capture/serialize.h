//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// serialize.h:
//   ANGLE GL state serialization.
//

#ifndef LIBANGLE_SERIALIZE_H_
#define LIBANGLE_SERIALIZE_H_

#include "libANGLE/Error.h"

namespace gl
{
class Context;
}  // namespace gl

namespace angle
{
Result SerializeContextToString(const gl::Context *context, std::string *stringOut);
}  // namespace angle
#endif  // LIBANGLE_SERIALIZE_H_
