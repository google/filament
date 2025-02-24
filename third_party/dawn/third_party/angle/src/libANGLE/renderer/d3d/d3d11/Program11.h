//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Program11: D3D11 implementation of an OpenGL Program.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_PROGRAM11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_PROGRAM11_H_

#include "libANGLE/renderer/d3d/ProgramD3D.h"

namespace rx
{
class Renderer11;

class Program11 : public ProgramD3D
{
  public:
    Program11(const gl::ProgramState &programState, Renderer11 *renderer11);
    ~Program11() override;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_PROGRAM11_H_
