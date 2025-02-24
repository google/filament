//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DXGI format info:
//   Determining metadata about a DXGI format.

#ifndef LIBANGLE_RENDERER_DXGI_FORMAT_MAP_H_
#define LIBANGLE_RENDERER_DXGI_FORMAT_MAP_H_

#include "common/platform.h"

namespace rx
{
namespace d3d11
{
GLenum GetComponentType(DXGI_FORMAT dxgiFormat);
}  // namespace d3d11

namespace d3d11_angle
{
const angle::Format &GetFormat(DXGI_FORMAT dxgiFormat);
}  // namespace d3d11_angle
}  // namespace rx

#endif  // LIBANGLE_RENDERER_DXGI_FORMAT_MAP_H_
