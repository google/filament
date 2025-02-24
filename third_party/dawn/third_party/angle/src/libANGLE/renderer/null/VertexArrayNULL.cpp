//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayNULL.cpp:
//    Implements the class methods for VertexArrayNULL.
//

#include "libANGLE/renderer/null/VertexArrayNULL.h"

#include "common/debug.h"

namespace rx
{

VertexArrayNULL::VertexArrayNULL(const gl::VertexArrayState &data) : VertexArrayImpl(data) {}

angle::Result VertexArrayNULL::syncState(const gl::Context *context,
                                         const gl::VertexArray::DirtyBits &dirtyBits,
                                         gl::VertexArray::DirtyAttribBitsArray *attribBits,
                                         gl::VertexArray::DirtyBindingBitsArray *bindingBits)
{
    // Clear the dirty bits in the back-end here.
    memset(attribBits, 0, sizeof(gl::VertexArray::DirtyAttribBitsArray));
    memset(bindingBits, 0, sizeof(gl::VertexArray::DirtyBindingBitsArray));

    return angle::Result::Continue;
}

}  // namespace rx
