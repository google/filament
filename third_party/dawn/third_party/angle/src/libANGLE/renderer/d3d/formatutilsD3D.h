//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils9.h: Queries for GL image formats and their translations to D3D
// formats.

#ifndef LIBANGLE_RENDERER_D3D_FORMATUTILSD3D_H_
#define LIBANGLE_RENDERER_D3D_FORMATUTILSD3D_H_

namespace rx
{
enum VertexConversionType
{
    VERTEX_CONVERT_NONE = 0,
    VERTEX_CONVERT_CPU  = 1,
    VERTEX_CONVERT_GPU  = 2,
    VERTEX_CONVERT_BOTH = 3
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_FORMATUTILSD3D_H_
