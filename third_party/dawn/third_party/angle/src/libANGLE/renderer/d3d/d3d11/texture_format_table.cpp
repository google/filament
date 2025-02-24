//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Helper routines for the D3D11 texture format table.

#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

#include "libANGLE/renderer/load_functions_table.h"

namespace rx
{

namespace d3d11
{

const Format &Format::getSwizzleFormat(const Renderer11DeviceCaps &deviceCaps) const
{
    return (swizzleFormat == internalFormat ? *this : Format::Get(swizzleFormat, deviceCaps));
}

LoadFunctionMap Format::getLoadFunctions() const
{
    return GetLoadFunctionsMap(internalFormat, formatID);
}

const angle::Format &Format::format() const
{
    return angle::Format::Get(formatID);
}

}  // namespace d3d11

}  // namespace rx
