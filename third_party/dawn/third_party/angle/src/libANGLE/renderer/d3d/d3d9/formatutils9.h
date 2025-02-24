//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils9.h: Queries for GL image formats and their translations to D3D9
// formats.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_FORMATUTILS9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_FORMATUTILS9_H_

#include <map>

#include "common/platform.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/copyvertex.h"
#include "libANGLE/renderer/d3d/formatutilsD3D.h"
#include "libANGLE/renderer/d3d_format.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{

class Renderer9;

namespace d3d9
{

struct VertexFormat
{
    VertexFormat();

    VertexConversionType conversionType;
    size_t outputElementSize;
    VertexCopyFunction copyFunction;
    D3DDECLTYPE nativeFormat;
    GLenum componentType;
};
const VertexFormat &GetVertexFormatInfo(DWORD supportedDeclTypes, angle::FormatID vertexFormatID);

struct TextureFormat
{
    TextureFormat();

    D3DFORMAT texFormat;
    D3DFORMAT renderFormat;

    InitializeTextureDataFunction dataInitializerFunction;

    LoadImageFunction loadFunction;
};
const TextureFormat &GetTextureFormatInfo(GLenum internalFormat);
}  // namespace d3d9
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_FORMATUTILS9_H_
