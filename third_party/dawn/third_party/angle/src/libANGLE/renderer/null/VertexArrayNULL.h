//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayNULL.h:
//    Defines the class interface for VertexArrayNULL, implementing VertexArrayImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_VERTEXARRAYNULL_H_
#define LIBANGLE_RENDERER_NULL_VERTEXARRAYNULL_H_

#include "libANGLE/renderer/VertexArrayImpl.h"

namespace rx
{

class VertexArrayNULL : public VertexArrayImpl
{
  public:
    VertexArrayNULL(const gl::VertexArrayState &data);

    angle::Result syncState(const gl::Context *context,
                            const gl::VertexArray::DirtyBits &dirtyBits,
                            gl::VertexArray::DirtyAttribBitsArray *attribBits,
                            gl::VertexArray::DirtyBindingBitsArray *bindingBits) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_VERTEXARRAYNULL_H_
