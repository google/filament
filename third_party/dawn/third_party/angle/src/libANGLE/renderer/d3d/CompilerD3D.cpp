//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerD3D:
//   Implementation of the D3D compiler methods.
//

#include "libANGLE/renderer/d3d/CompilerD3D.h"

namespace rx
{

CompilerD3D::CompilerD3D(ShShaderOutput translatorOutputType)
    : mTranslatorOutputType(translatorOutputType)
{}

ShShaderOutput CompilerD3D::getTranslatorOutputType() const
{
    return mTranslatorOutputType;
}

}  // namespace rx
