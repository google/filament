// GENERATED FILE - DO NOT EDIT. See dxgi_support_data.json.
//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// dxgi_support_table:
//   Queries for DXGI support of various texture formats. Depends on DXGI
//   version, D3D feature level, and is sometimes guaranteed or optional.
//

#include "libANGLE/renderer/dxgi_support_table.h"

#include "common/debug.h"

namespace rx
{

namespace d3d11
{

#define F_2D D3D11_FORMAT_SUPPORT_TEXTURE2D
#define F_3D D3D11_FORMAT_SUPPORT_TEXTURE3D
#define F_CUBE D3D11_FORMAT_SUPPORT_TEXTURECUBE
#define F_SAMPLE D3D11_FORMAT_SUPPORT_SHADER_SAMPLE
#define F_RT D3D11_FORMAT_SUPPORT_RENDER_TARGET
#define F_MS D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET
#define F_DS D3D11_FORMAT_SUPPORT_DEPTH_STENCIL
#define F_MIPGEN D3D11_FORMAT_SUPPORT_MIP_AUTOGEN

namespace
{

const DXGISupport &GetDefaultSupport()
{
    static UINT AllSupportFlags =
        D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_TEXTURE3D |
        D3D11_FORMAT_SUPPORT_TEXTURECUBE | D3D11_FORMAT_SUPPORT_SHADER_SAMPLE |
        D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET |
        D3D11_FORMAT_SUPPORT_DEPTH_STENCIL | D3D11_FORMAT_SUPPORT_MIP_AUTOGEN;
    static const DXGISupport defaultSupport(0, 0, AllSupportFlags);
    return defaultSupport;
}

const DXGISupport &GetDXGISupport_9_3(DXGI_FORMAT dxgiFormat)
{
    // clang-format off
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_420_OPAQUE:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_AI44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_AYUV:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G6R5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        {
            static const DXGISupport info(F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_MIPGEN | F_RT, F_DS, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_BC1_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_SNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_SNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_SF16:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_UF16:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_D16_UNORM:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_IA44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_NV11:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_NV12:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P010:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P016:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        {
            static const DXGISupport info(0, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        {
            static const DXGISupport info(0, F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D);
            return info;
        }
        case DXGI_FORMAT_R11G11B10_FLOAT:
        {
            static const DXGISupport info(0, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        {
            static const DXGISupport info(0, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_FLOAT:
        {
            static const DXGISupport info(0, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SNORM:
        {
            static const DXGISupport info(0, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R1_UNORM:
        {
            static const DXGISupport info(F_2D, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        {
            static const DXGISupport info(0, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_SNORM:
        {
            static const DXGISupport info(F_2D | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_UNORM:
        {
            static const DXGISupport info(0, F_DS, F_MS | F_RT | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R8_SINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SNORM:
        {
            static const DXGISupport info(0, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8_UINT:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_UNKNOWN:
        {
            static const DXGISupport info(0, F_2D | F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_Y210:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y216:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y410:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y416:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_YUY2:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }

        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }
    // clang-format on
}

const DXGISupport &GetDXGISupport_10_0(DXGI_FORMAT dxgiFormat)
{
    // clang-format off
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_420_OPAQUE:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_AI44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_AYUV:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G6R5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        {
            static const DXGISupport info(F_MIPGEN, F_DS, F_2D | F_3D | F_CUBE | F_MS | F_RT | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_MIPGEN | F_RT, F_DS, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_BC1_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_SF16:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_UF16:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_D16_UNORM:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_IA44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_NV11:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_NV12:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P010:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P016:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        {
            static const DXGISupport info(0, F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D);
            return info;
        }
        case DXGI_FORMAT_R11G11B10_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R1_UNORM:
        {
            static const DXGISupport info(F_2D, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R32G32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT, F_DS, F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_R32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_UNKNOWN:
        {
            static const DXGISupport info(0, F_2D | F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_Y210:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y216:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y410:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y416:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_YUY2:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }

        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }
    // clang-format on
}

const DXGISupport &GetDXGISupport_10_1(DXGI_FORMAT dxgiFormat)
{
    // clang-format off
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_420_OPAQUE:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_AI44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_AYUV:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G6R5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        {
            static const DXGISupport info(F_MIPGEN, F_DS, F_2D | F_3D | F_CUBE | F_MS | F_RT | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_MIPGEN | F_RT, F_DS, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS | F_SAMPLE);
            return info;
        }
        case DXGI_FORMAT_BC1_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_SF16:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_UF16:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_D16_UNORM:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_IA44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_NV11:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_NV12:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P010:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P016:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        {
            static const DXGISupport info(0, F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D);
            return info;
        }
        case DXGI_FORMAT_R11G11B10_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R1_UNORM:
        {
            static const DXGISupport info(F_2D, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_SAMPLE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_SAMPLE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_UNKNOWN:
        {
            static const DXGISupport info(0, F_2D | F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_Y210:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y216:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y410:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y416:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_YUY2:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }

        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }
    // clang-format on
}

const DXGISupport &GetDXGISupport_11_0(DXGI_FORMAT dxgiFormat)
{
    // clang-format off
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_420_OPAQUE:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_AI44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_AYUV:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G6R5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        {
            static const DXGISupport info(F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        {
            static const DXGISupport info(F_RT | F_SAMPLE, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            static const DXGISupport info(F_RT | F_SAMPLE, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_BC1_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_SF16:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_UF16:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_D16_UNORM:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_IA44:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_NV11:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_NV12:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P010:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P016:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P8:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        {
            static const DXGISupport info(0, F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D);
            return info;
        }
        case DXGI_FORMAT_R11G11B10_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_SAMPLE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_SAMPLE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_UNKNOWN:
        {
            static const DXGISupport info(0, F_2D | F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_Y210:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y216:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y410:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y416:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_YUY2:
        {
            static const DXGISupport info(0, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }

        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }
    // clang-format on
}

const DXGISupport &GetDXGISupport_11_1(DXGI_FORMAT dxgiFormat)
{
    // clang-format off
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_420_OPAQUE:
        {
            static const DXGISupport info(F_2D, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8P8:
        {
            static const DXGISupport info(F_2D, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_AI44:
        {
            static const DXGISupport info(F_2D, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_AYUV:
        {
            static const DXGISupport info(F_2D | F_RT | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_SAMPLE, F_DS, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_B5G6R5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        {
            static const DXGISupport info(F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        {
            static const DXGISupport info(0, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D | F_CUBE);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        {
            static const DXGISupport info(F_RT | F_SAMPLE, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            static const DXGISupport info(F_RT | F_SAMPLE, F_DS | F_MIPGEN, F_2D | F_3D | F_CUBE | F_MS);
            return info;
        }
        case DXGI_FORMAT_BC1_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC4_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC5_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_SF16:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC6H_UF16:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_D16_UNORM:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_DS, F_3D | F_MIPGEN | F_RT | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_IA44:
        {
            static const DXGISupport info(F_2D, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_NV11:
        {
            static const DXGISupport info(F_2D | F_RT | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_NV12:
        {
            static const DXGISupport info(F_2D | F_RT | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P010:
        {
            static const DXGISupport info(F_2D | F_RT | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P016:
        {
            static const DXGISupport info(F_2D | F_RT | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN, F_MS);
            return info;
        }
        case DXGI_FORMAT_P8:
        {
            static const DXGISupport info(F_2D, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        {
            static const DXGISupport info(0, F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, F_2D | F_3D);
            return info;
        }
        case DXGI_FORMAT_R11G11B10_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16G16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16G16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R16_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R16_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R1_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_SAMPLE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32A32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32B32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_SAMPLE, F_MS | F_RT);
            return info;
        }
        case DXGI_FORMAT_R32G32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32G32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_CUBE | F_SAMPLE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R32_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R32_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R32_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8G8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8G8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_SNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_TYPELESS:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE, F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_R8_UINT:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_RT, F_DS | F_MIPGEN | F_SAMPLE, F_MS);
            return info;
        }
        case DXGI_FORMAT_R8_UNORM:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_MIPGEN | F_RT | F_SAMPLE, F_DS, F_MS);
            return info;
        }
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        {
            static const DXGISupport info(F_2D | F_3D | F_CUBE | F_SAMPLE, F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_UNKNOWN:
        {
            static const DXGISupport info(0, F_2D | F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        {
            static const DXGISupport info(F_2D | F_CUBE, F_3D | F_DS | F_MIPGEN | F_MS | F_RT | F_SAMPLE, 0);
            return info;
        }
        case DXGI_FORMAT_Y210:
        {
            static const DXGISupport info(F_2D | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y216:
        {
            static const DXGISupport info(F_2D | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y410:
        {
            static const DXGISupport info(F_2D | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_Y416:
        {
            static const DXGISupport info(F_2D | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }
        case DXGI_FORMAT_YUY2:
        {
            static const DXGISupport info(F_2D | F_SAMPLE, F_3D | F_CUBE | F_DS | F_MIPGEN | F_MS | F_RT, 0);
            return info;
        }

        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }
    // clang-format on
}

}  // namespace

#undef F_2D
#undef F_3D
#undef F_CUBE
#undef F_SAMPLE
#undef F_RT
#undef F_MS
#undef F_DS
#undef F_MIPGEN

const DXGISupport &GetDXGISupport(DXGI_FORMAT dxgiFormat, D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_9_3:
            return GetDXGISupport_9_3(dxgiFormat);
        case D3D_FEATURE_LEVEL_10_0:
            return GetDXGISupport_10_0(dxgiFormat);
        case D3D_FEATURE_LEVEL_10_1:
            return GetDXGISupport_10_1(dxgiFormat);
        case D3D_FEATURE_LEVEL_11_0:
            return GetDXGISupport_11_0(dxgiFormat);
        case D3D_FEATURE_LEVEL_11_1:
            return GetDXGISupport_11_1(dxgiFormat);
        default:
            return GetDefaultSupport();
    }
}

}  // namespace d3d11

}  // namespace rx
