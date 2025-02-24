//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// texture_format_table:
//   Queries for full textureFormat information based on internalFormat
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_TEXTUREFORMATTABLE_H_
#define LIBANGLE_RENDERER_D3D_D3D11_TEXTUREFORMATTABLE_H_

#include <map>

#include "common/angleutils.h"
#include "common/platform.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/d3d/formatutilsD3D.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{

struct Renderer11DeviceCaps;

namespace d3d11
{

// For sized GL internal formats, there are several possible corresponding D3D11 formats depending
// on device capabilities.
// This structure allows querying for the DXGI texture formats to use for textures, SRVs, RTVs and
// DSVs given a GL internal format.
struct Format final : private angle::NonCopyable
{
    inline constexpr Format();
    inline constexpr Format(GLenum internalFormat,
                            angle::FormatID formatID,
                            DXGI_FORMAT texFormat,
                            DXGI_FORMAT srvFormat,
                            DXGI_FORMAT uavFormat,
                            DXGI_FORMAT rtvFormat,
                            DXGI_FORMAT dsvFormat,
                            DXGI_FORMAT blitSRVFormat,
                            DXGI_FORMAT stencilSRVFormat,
                            DXGI_FORMAT linearSRVFormat,
                            DXGI_FORMAT typelessFormat,
                            GLenum swizzleFormat,
                            InitializeTextureDataFunction internalFormatInitializer);

    static const Format &Get(GLenum internalFormat, const Renderer11DeviceCaps &deviceCaps);

    const Format &getSwizzleFormat(const Renderer11DeviceCaps &deviceCaps) const;
    LoadFunctionMap getLoadFunctions() const;
    const angle::Format &format() const;

    GLenum internalFormat;
    angle::FormatID formatID;

    DXGI_FORMAT texFormat;
    DXGI_FORMAT srvFormat;
    DXGI_FORMAT uavFormat;
    DXGI_FORMAT rtvFormat;
    DXGI_FORMAT dsvFormat;

    DXGI_FORMAT blitSRVFormat;
    DXGI_FORMAT stencilSRVFormat;
    DXGI_FORMAT linearSRVFormat;
    DXGI_FORMAT typelessFormat;

    GLenum swizzleFormat;

    InitializeTextureDataFunction dataInitializerFunction;
};

constexpr Format::Format()
    : internalFormat(GL_NONE),
      formatID(angle::FormatID::NONE),
      texFormat(DXGI_FORMAT_UNKNOWN),
      srvFormat(DXGI_FORMAT_UNKNOWN),
      uavFormat(DXGI_FORMAT_UNKNOWN),
      rtvFormat(DXGI_FORMAT_UNKNOWN),
      dsvFormat(DXGI_FORMAT_UNKNOWN),
      blitSRVFormat(DXGI_FORMAT_UNKNOWN),
      stencilSRVFormat(DXGI_FORMAT_UNKNOWN),
      linearSRVFormat(DXGI_FORMAT_UNKNOWN),
      typelessFormat(DXGI_FORMAT_UNKNOWN),
      swizzleFormat(GL_NONE),
      dataInitializerFunction(nullptr)
{}

constexpr Format::Format(GLenum internalFormat,
                         angle::FormatID formatID,
                         DXGI_FORMAT texFormat,
                         DXGI_FORMAT srvFormat,
                         DXGI_FORMAT uavFormat,
                         DXGI_FORMAT rtvFormat,
                         DXGI_FORMAT dsvFormat,
                         DXGI_FORMAT blitSRVFormat,
                         DXGI_FORMAT stencilSRVFormat,
                         DXGI_FORMAT linearSRVFormat,
                         DXGI_FORMAT typelessFormat,
                         GLenum swizzleFormat,
                         InitializeTextureDataFunction internalFormatInitializer)
    : internalFormat(internalFormat),
      formatID(formatID),
      texFormat(texFormat),
      srvFormat(srvFormat),
      uavFormat(uavFormat),
      rtvFormat(rtvFormat),
      dsvFormat(dsvFormat),
      blitSRVFormat(blitSRVFormat),
      stencilSRVFormat(stencilSRVFormat),
      linearSRVFormat(linearSRVFormat),
      typelessFormat(typelessFormat),
      swizzleFormat(swizzleFormat),
      dataInitializerFunction(internalFormatInitializer)
{}

}  // namespace d3d11

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_TEXTUREFORMATTABLE_H_
