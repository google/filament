//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTargetD3D.cpp: Implements serial handling for rx::RenderTargetD3D

#include "libANGLE/renderer/d3d/RenderTargetD3D.h"

namespace rx
{
unsigned int RenderTargetD3D::mCurrentSerial = 1;

RenderTargetD3D::RenderTargetD3D() : mSerial(issueSerials(1)) {}

RenderTargetD3D::~RenderTargetD3D() {}

unsigned int RenderTargetD3D::getSerial() const
{
    return mSerial;
}

unsigned int RenderTargetD3D::issueSerials(unsigned int count)
{
    unsigned int firstSerial = mCurrentSerial;
    mCurrentSerial += count;
    return firstSerial;
}

}  // namespace rx
