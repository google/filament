//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer9_utils.h: Conversion functions and other utility routines
// specific to the D3D9 renderer

#ifndef LIBANGLE_RENDERER_D3D_D3D9_RENDERER9UTILS_H_
#define LIBANGLE_RENDERER_D3D_D3D9_RENDERER9UTILS_H_

#include "common/Color.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Error.h"
#include "platform/autogen/FeaturesD3D_autogen.h"
#include "platform/autogen/FrontendFeatures_autogen.h"

namespace gl
{
class FramebufferAttachment;
}

namespace rx
{
class RenderTarget9;

namespace gl_d3d9
{

D3DCMPFUNC ConvertComparison(GLenum comparison);
D3DCOLOR ConvertColor(gl::ColorF color);
D3DBLEND ConvertBlendFunc(GLenum blend);
D3DBLENDOP ConvertBlendOp(GLenum blendOp);
D3DSTENCILOP ConvertStencilOp(GLenum stencilOp);
D3DTEXTUREADDRESS ConvertTextureWrap(GLenum wrap);
D3DCULL ConvertCullMode(gl::CullFaceMode cullFace, GLenum frontFace);
D3DCUBEMAP_FACES ConvertCubeFace(gl::TextureTarget cubeFace);
DWORD ConvertColorMask(bool red, bool green, bool blue, bool alpha);
D3DTEXTUREFILTERTYPE ConvertMagFilter(GLenum magFilter, float maxAnisotropy);
void ConvertMinFilter(GLenum minFilter,
                      D3DTEXTUREFILTERTYPE *d3dMinFilter,
                      D3DTEXTUREFILTERTYPE *d3dMipFilter,
                      float *d3dLodBias,
                      float maxAnisotropy,
                      size_t baseLevel);
D3DQUERYTYPE ConvertQueryType(gl::QueryType type);

D3DMULTISAMPLE_TYPE GetMultisampleType(GLuint samples);

}  // namespace gl_d3d9

namespace d3d9_gl
{

unsigned int GetReservedVaryingVectors();

unsigned int GetReservedVertexUniformVectors();

unsigned int GetReservedFragmentUniformVectors();

GLsizei GetSamplesCount(D3DMULTISAMPLE_TYPE type);

bool IsFormatChannelEquivalent(D3DFORMAT d3dformat, GLenum format);

void GenerateCaps(IDirect3D9 *d3d9,
                  IDirect3DDevice9 *device,
                  D3DDEVTYPE deviceType,
                  UINT adapter,
                  gl::Caps *caps,
                  gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions,
                  gl::Limitations *limitations);
}  // namespace d3d9_gl

namespace d3d9
{

GLuint ComputeBlockSize(D3DFORMAT format, GLuint width, GLuint height);

void MakeValidSize(bool isImage,
                   D3DFORMAT format,
                   GLsizei *requestWidth,
                   GLsizei *requestHeight,
                   int *levelOffset);

inline bool isDeviceLostError(HRESULT errorCode)
{
    switch (errorCode)
    {
        case D3DERR_DRIVERINTERNALERROR:
        case D3DERR_DEVICELOST:
        case D3DERR_DEVICEHUNG:
        case D3DERR_DEVICEREMOVED:
            return true;
        default:
            return false;
    }
}

void InitializeFeatures(angle::FeaturesD3D *features, DWORD VendorID);
void InitializeFrontendFeatures(angle::FrontendFeatures *features, DWORD VendorID);
}  // namespace d3d9

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_RENDERER9UTILS_H_
